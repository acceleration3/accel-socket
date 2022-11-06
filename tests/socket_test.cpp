#include <iostream>
#include <accel/socket.hpp>

using namespace accel;

int main(int argc, char* argv[])
{
    endpoint ep(ip_address_v4::resolve("www.google.pt"), 80);
    std::cout << "Resolved www.google.pt to " << ep.string() << "\n";

    socket sock(ip_versions::version_4, protocols::tcp);
    sock.connect(ep);

    std::string data = "GET http://www.google.com/index.html HTTP/1.1\nConnection: close\n\n";
    sock.send(reinterpret_cast<const uint8_t*>(data.data()), data.size());

    std::size_t total_size = 0;
    while (true)
    {
        std::uint8_t buffer[1024];
        auto chunk_size = sock.receive(buffer, 1024);
        if (chunk_size == 0)
            break;
        total_size += chunk_size;
    }

    std::cout << "Total size: " << total_size << "\n";
    std::cout << "Test completed successfully.\n";

    return 0;
}