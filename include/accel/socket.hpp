#ifndef ACCEL_SOCKET_H
#define ACCEL_SOCKET_H

#include <string>
#include <string_view>
#include <span>
#include <ostream>
#include <cstdint>
#include <memory>
#include <array>
#include <variant>
#include <utility>

#include <accel/macros.hpp>

namespace accel
{
	enum class ip_versions
	{
		version_4,
		version_6,
	};

	enum class protocols
	{
		udp,
		tcp,
	};

	class ip_address_v4;
	class ip_address_v6;
	class endpoint;
	class socket;

	class ip_address_v4
	{
	public:
		friend class socket;

		ACC_EXPORT static ip_address_v4 any();
		ACC_EXPORT static ip_address_v4 localhost();
		ACC_EXPORT static ip_address_v4 broadcast();

		ACC_EXPORT static ip_address_v4 resolve(const std::string_view hostname);

		ACC_EXPORT ip_address_v4(std::uint32_t numeric);
		ACC_EXPORT ip_address_v4(const std::string_view ip);

		ACC_EXPORT ip_address_v4(const ip_address_v4&) = default;
		ACC_EXPORT ip_address_v4& operator=(const ip_address_v4&) = default;

		ACC_EXPORT ip_address_v4(ip_address_v4&&) = default;
		ACC_EXPORT ip_address_v4& operator=(ip_address_v4&&) = default;

		ACC_EXPORT std::string string() const;

	protected:
		std::uint32_t m_value;
	};

	class ip_address_v6
	{
	public:
		friend class socket;

		ACC_EXPORT static ip_address_v6 any();
		ACC_EXPORT static ip_address_v6 localhost();

		ACC_EXPORT static ip_address_v6 resolve(const std::string_view hostname);

		ACC_EXPORT ip_address_v6(const std::array<std::uint16_t, 8>& shorts);
		ACC_EXPORT ip_address_v6(const std::string_view ip);

		ACC_EXPORT ip_address_v6(const ip_address_v6&) = default;
		ACC_EXPORT ip_address_v6& operator=(const ip_address_v6&) = default;

		ACC_EXPORT ip_address_v6(ip_address_v6&&) = default;
		ACC_EXPORT ip_address_v6& operator=(ip_address_v6&&) = default;

		ACC_EXPORT std::string string() const;

	protected:
		std::array<std::uint16_t, 8> m_values;
	};

	class endpoint
	{
	public:
		friend class socket;

		using any_address = std::variant<ip_address_v4, ip_address_v6>;

		ACC_EXPORT endpoint(const any_address& ip, std::uint16_t port);

		ACC_EXPORT endpoint(const endpoint&) = default;
		ACC_EXPORT endpoint& operator=(const endpoint&) = default;

		ACC_EXPORT endpoint(endpoint&&) = default;
		ACC_EXPORT endpoint& operator=(endpoint&&) = default;

		ACC_EXPORT any_address ip() const;
		ACC_EXPORT std::uint16_t port() const;
		ACC_EXPORT std::string string() const;

	private:
		any_address m_ip;
		std::uint16_t m_port;
	};

	enum class socket_errors
	{
		unknown,
		bad_file_descriptor,
		invalid,
		fault,
		not_a_socket,
		access,
		permission,
		connection_aborted,
		address_in_use,
		operation_not_supported,
		would_block,
	};

	class socket_exception : public std::exception
	{
	public:
		ACC_EXPORT socket_exception();
		ACC_EXPORT const char* what() const noexcept override;
		ACC_EXPORT socket_errors error() const;

	private:
		socket_errors m_error;
		std::string m_message;
	};

	class socket
	{
	public:
		ACC_EXPORT socket(void* socket_fd);
		ACC_EXPORT socket(ip_versions version, protocols protocol);
		ACC_EXPORT ~socket();

		ACC_EXPORT socket(const socket&) = default;
		ACC_EXPORT socket& operator=(const socket&) = default;

		ACC_EXPORT socket(socket&&) = default;
		ACC_EXPORT socket& operator=(socket&&) = default;

		ACC_EXPORT void bind(const endpoint& endpoint);
		ACC_EXPORT void listen();
		ACC_EXPORT void connect(const endpoint& endpoint);

		using connection = std::pair<socket, endpoint>;
		ACC_EXPORT connection accept();

		ACC_EXPORT std::size_t receive(std::uint8_t* data, int length);
		ACC_EXPORT std::size_t send(const std::uint8_t* data, int length);
		ACC_EXPORT std::size_t receive_from(endpoint& ep, std::uint8_t* data, int length);
		ACC_EXPORT std::size_t send_to(const endpoint& ep, const std::uint8_t* data, int length);

		template<typename T>
		std::size_t receive(std::span<T> data)
		{
			return receive(reinterpret_cast<std::uint8_t*>(data.data()), static_cast<int>(data.size_bytes()));
		}

		template<typename T>
		std::size_t send(const std::span<T> data)
		{
			return send(reinterpret_cast<const std::uint8_t*>(data.data()), static_cast<int>(data.size_bytes()));
		}

		template<typename T>
		std::size_t receive_from(endpoint& ep, std::span<T> data)
		{
			return receive_from(ep, reinterpret_cast<std::uint8_t*>(data.data()), static_cast<int>(data.size_bytes()));
		}

		template<typename T>
		std::size_t send_to(const endpoint& ep, const std::span<T> data)
		{
			return send_to(ep, reinterpret_cast<const std::uint8_t*>(data.data()), static_cast<int>(data.size_bytes()));
		}

		ACC_EXPORT void set_blocking(bool state);
		ACC_EXPORT void set_broadcast(bool state);
		ACC_EXPORT void set_tcp_nodelay(bool state);

	private:
		struct impl;
		std::unique_ptr<impl> m_impl;
	};
}

#endif