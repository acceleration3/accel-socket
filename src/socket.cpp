#include <accel/socket.hpp>

#if defined(_MSC_VER) || defined(__MINGW32__)
	#define IS_WINDOWS 1
#else
	#define IS_WINDOWS 0
#endif

#include <stdexcept>
#include <variant>

#if IS_WINDOWS
	#include <WinSock2.h>
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
#endif

using socket_t = decltype(socket(0, 0, 0));

namespace accel
{
#if IS_WINDOWS
	static void initialize_wsa()
	{
		static bool s_initialized = false;
		if (s_initialized)
			return;

		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
			throw socket_exception();
		s_initialized = true;
	}
#endif

	// -----------------------------------------------------------------------------------
	// IP Address
	// -----------------------------------------------------------------------------------

	// IPv4
	ip_address_v4 ip_address_v4::any()
	{
		return ip_address_v4(INADDR_ANY);
	}

	ip_address_v4 ip_address_v4::localhost()
	{
		return ip_address_v4(INADDR_LOOPBACK);
	}

	ip_address_v4 ip_address_v4::broadcast()
	{
		return ip_address_v4(INADDR_BROADCAST);
	}

	ip_address_v4 ip_address_v4::resolve(const std::string_view hostname)
	{
#if IS_WINDOWS
		initialize_wsa();
#endif
		struct addrinfo* results = nullptr;
		struct addrinfo hints{};
		hints.ai_family = AF_INET;
		if (getaddrinfo(hostname.data(), nullptr, &hints, &results) != 0)
			throw socket_exception();

		auto addr = reinterpret_cast<struct sockaddr_in*>(results->ai_addr);
		return ip_address_v4(addr->sin_addr.S_un.S_addr);
	}

	ip_address_v4::ip_address_v4() : m_value(0) {}

	ip_address_v4::ip_address_v4(std::uint32_t numeric) : m_value(numeric) {}

	ip_address_v4::ip_address_v4(const std::string_view ip)
	{
		struct in_addr address;
		if (!inet_pton(AF_INET, ip.data(), &address))
			throw std::invalid_argument("Failed to convert text to an IPv4 address.");
		m_value = ntohl(address.S_un.S_addr);
	}

	std::string ip_address_v4::string() const
	{
		char buffer[17];
		struct in_addr address;
		address.S_un.S_addr = htonl(m_value);
		if (!inet_ntop(AF_INET, &address, buffer, 17))
			throw std::runtime_error("Failed to convert IPv4 from binary to the text representation.");
		return std::string(buffer);
	}

	// IPv6
	ip_address_v6 ip_address_v6::any()
	{
		return ip_address_v6("::");
	}

	ip_address_v6 ip_address_v6::localhost()
	{
		return ip_address_v6("::1");
	}

	ip_address_v6 ip_address_v6::resolve(const std::string_view hostname)
	{
#if IS_WINDOWS
		initialize_wsa();
#endif
		struct addrinfo* results = nullptr;
		struct addrinfo hints {};
		hints.ai_family = AF_INET6;
		if (getaddrinfo(hostname.data(), nullptr, &hints, &results) != 0)
			throw socket_exception();
		auto address = reinterpret_cast<struct sockaddr_in6*>(results->ai_addr);
		std::array<std::uint16_t, 8> values;
		std::copy(std::begin(address->sin6_addr.u.Word), std::end(address->sin6_addr.u.Word), values.begin());
		return ip_address_v6(values);
	}

	ip_address_v6::ip_address_v6() : m_values() {}

	ip_address_v6::ip_address_v6(const std::array<std::uint16_t, 8>& shorts) : m_values(shorts) {}

	ip_address_v6::ip_address_v6(const std::string_view ip)
	{
		struct in_addr6 address;
		if (!inet_pton(AF_INET6, ip.data(), &address))
			throw std::invalid_argument("Failed to convert text to an IPv6 address.");
		std::copy(std::begin(address.u.Word), std::end(address.u.Word), m_values.begin());
	}

	std::string ip_address_v6::string() const
	{
		char buffer[40];
		struct in_addr6 address;
		std::copy(m_values.begin(), m_values.end(), std::begin(address.u.Word));
		if (!inet_ntop(AF_INET6, &address, buffer, 40))
			throw std::runtime_error("Failed to convert IPv6 from binary to the text representation.");
		return std::string(buffer);
	}


	// -----------------------------------------------------------------------------------
	// Endpoint
	// -----------------------------------------------------------------------------------

	endpoint::endpoint() : 
		m_ip(0),
		m_port(0) {}

	endpoint::endpoint(const any_address& ip, std::uint16_t port) :
		m_ip(ip),
		m_port(port) {}

	endpoint::any_address endpoint::ip() const
	{
		return m_ip;
	}

	std::uint16_t endpoint::port() const
	{
		return m_port;
	}

	std::string endpoint::string() const
	{
		struct visitor
		{
			std::string operator()(const ip_address_v4& ip) { return ip.string(); }
			std::string operator()(const ip_address_v6& ip) { return "[" + ip.string() + "]"; }
		};

		auto ip_string = std::visit(visitor{}, m_ip);
		return ip_string + ":" + std::to_string(m_port);
	}


	// -----------------------------------------------------------------------------------
	// Exception
	// -----------------------------------------------------------------------------------

	socket_errors get_error(int code)
	{
#if IS_WINDOWS
		switch (code)
		{
			case WSAEBADF: return socket_errors::bad_file_descriptor;
			case WSAEINVAL: return socket_errors::invalid;
			case WSAEFAULT: return socket_errors::fault;
			case WSAENOTSOCK: return socket_errors::not_a_socket;
			case WSAEACCES: return socket_errors::access;
			case WSAECONNABORTED: return socket_errors::connection_aborted;
			case WSAEADDRINUSE: return socket_errors::address_in_use;
			case WSAEOPNOTSUPP: return socket_errors::operation_not_supported;
			case WSAEWOULDBLOCK: return socket_errors::would_block;
			default: return socket_errors::unknown;
		}
#else
		switch (code)
		{
			case EBADF: return socket_errors::bad_file_descriptor;
			case EINVAL: return socket_errors::invalid;
			case EFAULT: return socket_errors::fault;
			case ENOTSOCK: return socket_errors::not_a_socket;
			case EACCES: return socket_errors::access;
			case ECONNABORTED: return socket_errors::connection_aborted;
			case EADDRINUSE: return socket_errors::address_in_use;
			case EOPNOTSUPP: return socket_errors::operation_not_supported;
			case EWOULDBLOCK: return socket_errors::would_block;
			case EAGAIN: return socket_errors::would_block;
			default: return socket_errors::unknown;
		}
#endif
	}

	socket_exception::socket_exception()
	{
#if IS_WINDOWS
		int code = WSAGetLastError();
		m_error = get_error(code);

		char buffer[1024];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 1024, nullptr);
		m_message = std::string(buffer);
#else
		int code = errno();
		m_message = strerror(code);
#endif
	}

	const char* socket_exception::what() const noexcept
	{
		return m_message.c_str();
	}

	socket_errors socket_exception::error() const
	{
		return m_error;
	}


	// -----------------------------------------------------------------------------------
	// Socket
	// -----------------------------------------------------------------------------------

	struct socket::impl
	{
		short family;
		socket_t sock;
	};

	socket::socket(void* socket_fd)
	{
#if IS_WINDOWS
		initialize_wsa();
#endif
		struct sockaddr addr{};
		int length = sizeof(addr);
		if (getsockname(reinterpret_cast<socket_t>(socket_fd), reinterpret_cast<sockaddr*>(&addr), &length) != 0)
			throw socket_exception();
	}

	socket::socket(ip_versions version, protocols protocol) :
		m_impl(std::make_unique<impl>())
	{
#if IS_WINDOWS
		initialize_wsa();
#endif
		if (version == ip_versions::version_4)
			m_impl->family = AF_INET;
		else if (version == ip_versions::version_6)
			m_impl->family = AF_INET6;
		else
			throw std::invalid_argument("Invalid IP version selected.");

		if (protocol == protocols::tcp)
			m_impl->sock = ::socket(m_impl->family, SOCK_STREAM, IPPROTO_TCP);
		else
			m_impl->sock = ::socket(m_impl->family, SOCK_DGRAM, IPPROTO_UDP);

		if (m_impl->sock == INVALID_SOCKET)
			throw socket_exception();
	}

	socket::~socket()
	{
#if IS_WINDOWS
		if (m_impl && m_impl->sock)
			::closesocket(m_impl->sock);
#else
		if (m_impl && m_impl->sock)
			::close(m_impl->sock);
#endif
	}

	socket::socket(socket&& other)
	{
		m_impl.swap(other.m_impl);
	}

	socket& socket::operator=(socket&& other)
	{
		m_impl.swap(other.m_impl);
		return *this;
	}

	void socket::bind(endpoint& endpoint)
	{
		if ((endpoint.m_ip.index() == 0 && m_impl->family == AF_INET6) || (endpoint.m_ip.index() == 1 && m_impl->family == AF_INET))
			throw std::invalid_argument("Can't bind socket to endpoint of different version.");

		if (m_impl->family == AF_INET)
		{
			struct sockaddr_in address{};
			address.sin_family = AF_INET;
			address.sin_addr.S_un.S_addr = htonl(std::get<ip_address_v4>(endpoint.m_ip).m_value);
			address.sin_port = htons(endpoint.m_port);
			if (::bind(m_impl->sock, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
				throw socket_exception();
		}
		else
		{
			struct sockaddr_in6 address{};
			auto& ipv6 = std::get<ip_address_v6>(endpoint.m_ip);
			std::copy(ipv6.m_values.begin(), ipv6.m_values.end(), std::begin(address.sin6_addr.u.Word));
			address.sin6_family = AF_INET6;
			address.sin6_port = htons(endpoint.m_port);
			if (::bind(m_impl->sock, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
				throw socket_exception();
		}

		endpoint = get_endpoint();
	}

	void socket::listen()
	{
		if (::listen(m_impl->sock, -1) != 0)
			throw socket_exception();
	}

	void socket::connect(const endpoint& endpoint)
	{
		if ((endpoint.m_ip.index() == 0 && m_impl->family == AF_INET6) || (endpoint.m_ip.index() == 1 && m_impl->family == AF_INET))
			throw std::invalid_argument("Can't connect socket to endpoint of different version.");

		if (m_impl->family == AF_INET)
		{
			struct sockaddr_in address{};
			address.sin_family = AF_INET;
			address.sin_addr.S_un.S_addr = std::get<ip_address_v4>(endpoint.m_ip).m_value;
			address.sin_port = htons(endpoint.m_port);
			if (::connect(m_impl->sock, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
				throw socket_exception();
		}
		else
		{
			struct sockaddr_in6 address{};
			auto& ipv6 = std::get<ip_address_v6>(endpoint.m_ip);
			std::copy(ipv6.m_values.begin(), ipv6.m_values.end(), std::begin(address.sin6_addr.u.Word));
			address.sin6_family = AF_INET6;
			address.sin6_port = htons(endpoint.m_port);
			if (::connect(m_impl->sock, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
				throw socket_exception();
		}
	}

	socket::connection socket::accept()
	{
		socket_t client_socket = INVALID_SOCKET;

		if (m_impl->family == AF_INET)
		{
			struct sockaddr_in address{};
			socklen_t length = sizeof(address);
			client_socket = ::accept(m_impl->sock, reinterpret_cast<sockaddr*>(&address), &length);
			if (client_socket == INVALID_SOCKET)
				throw socket_exception();
			return std::make_pair<socket, endpoint>(socket(reinterpret_cast<void*>(client_socket)), endpoint(ip_address_v4(ntohl(address.sin_addr.S_un.S_addr)), ntohs(address.sin_port)));
		}
		else
		{
			struct sockaddr_in6 address{};
			socklen_t length = sizeof(address);
			client_socket = ::accept(m_impl->sock, reinterpret_cast<sockaddr*>(&address), &length);
			if (client_socket == INVALID_SOCKET)
				throw socket_exception();
			std::array<std::uint16_t, 8> values;
			std::copy(std::begin(address.sin6_addr.u.Word), std::end(address.sin6_addr.u.Word), values.begin());
			return std::make_pair<socket, endpoint>(socket(reinterpret_cast<void*>(client_socket)), endpoint(ip_address_v6(values), ntohs(address.sin6_port)));
		}
	}

	std::ptrdiff_t socket::receive(std::uint8_t* data, int length)
	{
		std::ptrdiff_t data_length = recv(m_impl->sock, reinterpret_cast<char*>(data), length, 0);
		if (data_length < 0)
			throw socket_exception();
		return data_length;
	}

	std::ptrdiff_t socket::send(const std::uint8_t* data, int length)
	{
		std::ptrdiff_t data_length = ::send(m_impl->sock, reinterpret_cast<const char*>(data), length, 0);
		if (data_length < 0)
			throw socket_exception();
		return data_length;
	}

	std::ptrdiff_t socket::receive_from(endpoint& ep, std::uint8_t* data, int length)
	{
		struct sockaddr_storage address{};
		socklen_t address_length = sizeof(address);
		
		std::ptrdiff_t data_length = recvfrom(m_impl->sock, reinterpret_cast<char*>(data), length, 0, reinterpret_cast<sockaddr*>(&address), &address_length);
		if (data_length < 0)
			throw socket_exception();

		if (address.ss_family == AF_INET)
		{
			auto ipv4 = reinterpret_cast<sockaddr_in*>(std::addressof(address));
			ep.m_ip = ip_address_v4(ntohl(ipv4->sin_addr.S_un.S_addr));
			ep.m_port = ntohs(ipv4->sin_port);
			return data_length;
		}
		else
		{
			auto ipv6 = reinterpret_cast<sockaddr_in6*>(std::addressof(address));
			std::array<std::uint16_t, 8> values;
			std::copy(std::begin(ipv6->sin6_addr.u.Word), std::end(ipv6->sin6_addr.u.Word), values.begin());
			ep.m_ip = ip_address_v6(values);
			ep.m_port = ipv6->sin6_port;
			return data_length;
		}
	}

	std::ptrdiff_t socket::send_to(const endpoint& ep, const std::uint8_t* data, int length)
	{
		if (m_impl->family == AF_INET)
		{
			auto& ipv4 = std::get<ip_address_v4>(ep.m_ip);
			struct sockaddr_in address{};
			address.sin_family = AF_INET;
			address.sin_addr.S_un.S_addr = htonl(ipv4.m_value);
			address.sin_port = htons(ep.m_port);
			socklen_t from_length = sizeof(address);
			std::ptrdiff_t data_length = sendto(m_impl->sock, reinterpret_cast<const char*>(data), length, 0, reinterpret_cast<const sockaddr*>(&address), from_length);
			if (data_length < 0)
				throw socket_exception();
			return data_length;
		}
		else
		{
			auto& ipv6 = std::get<ip_address_v6>(ep.m_ip);
			struct sockaddr_in6 address;
			std::copy(ipv6.m_values.begin(), ipv6.m_values.end(), std::begin(address.sin6_addr.u.Word));
			address.sin6_family = AF_INET6;
			address.sin6_port = htons(ep.m_port);
			socklen_t from_length = sizeof(address);
			std::ptrdiff_t data_length = sendto(m_impl->sock, reinterpret_cast<const char*>(data), length, 0, reinterpret_cast<const sockaddr*>(&address), from_length);
			if (data_length < 0)
				throw socket_exception();
			return data_length;
		}
	}

	void socket::set_non_blocking(bool state)
	{
#if IS_WINDOWS
		unsigned long mode = state ? 1 : 0;
		if (ioctlsocket(m_impl->sock, FIONBIO, &mode) != 0)
			throw socket_exception();
#else
		int flags = fcntl(m_impl->sock, F_GETFL, 0);
		if (flags == -1)
			throw socket_exception();
		flags = state ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
		if (fcntl(m_impl->sock, F_SETFL, flags) != 0)
			throw socket_exception();
#endif
	}

	void socket::set_broadcast(bool state)
	{
		int mode = state ? 1 : 0;
		if (setsockopt(m_impl->sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&mode), sizeof(mode)) != 0)
			throw std::runtime_error("Failed to set socket to broadcasting mode.");
	}

	void socket::set_tcp_nodelay(bool state)
	{
		unsigned long mode = state ? 1 : 0;
		if (setsockopt(m_impl->sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&mode), sizeof(mode)) != 0)
			throw std::runtime_error("Failed to set socket to no delay mode.");
	}

	endpoint socket::get_endpoint() const
	{
		struct sockaddr_storage address;

		socklen_t length = sizeof(address);
		if (getsockname(m_impl->sock, reinterpret_cast<sockaddr*>(&address), &length) < 0)
			throw socket_exception();

		if (address.ss_family == AF_INET)
		{
			sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(&address);
			return endpoint(ip_address_v4(ipv4->sin_addr.s_addr), ntohs(ipv4->sin_port));
		}
		else
		{
			sockaddr_in6* ipv6 = reinterpret_cast<sockaddr_in6*>(&address);
			std::array<std::uint16_t, 8> shorts;
			std::copy(std::begin(ipv6->sin6_addr.u.Word), std::end(ipv6->sin6_addr.u.Word), std::begin(shorts));
			return endpoint(ip_address_v6(shorts), ntohs(ipv6->sin6_port));
		}
	}

}