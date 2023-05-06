#include <iostream>
#include <accel/socket>

int main(int argc, char* argv[])
{
    auto localhost = accel::ip_address_v4::localhost();
  
    accel::endpoint_v4 ep(accel::ip_address_v4::resolve("www.google.com"), 80);
    std::cout << "Resolved www.google.com to " << ep.to_string() << "\n";

    accel::socket sock(accel::ip_versions::version_4, accel::protocols::tcp);
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
        std::cout << buffer;
        total_size += chunk_size;
    }

    std::cout << "Total size: " << total_size << "\n";
    std::cout << "Test completed successfully.\n";

    return 0;
}