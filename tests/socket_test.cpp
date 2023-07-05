#include <iostream>

#include <accel/macros>
#include <accel/socket>

int main(int argc, char* argv[])
{
    ACC_ASSERT(accel::ip_address_v4::localhost().to_string() == "127.0.0.1");
    ACC_ASSERT(accel::ip_address_v4::broadcast().to_string() == "255.255.255.255");
    ACC_ASSERT(accel::ip_address_v4::any().to_string() == "0.0.0.0");
    ACC_ASSERT(accel::ip_address_v4("1.2.3.4").to_string() == "1.2.3.4");

    {
        auto google_ip = accel::ip_address_v4::resolve("www.google.com");
        std::cout << "Resolved www.google.com to IPv4: " << google_ip.to_string() << "\n";

        accel::endpoint_v4 ep(google_ip, 80);
        accel::socket sock(accel::ip_versions::version_4, accel::protocols::tcp);
        sock.connect(ep);

        std::string data = "GET http://www.google.com/index.html HTTP/1.1\nConnection: close\n\n";
        sock.send(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());

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

        std::cout << "IPv4 test completed successfully.\n";
    }
    

    {
        auto google_ip = accel::ip_address_v6::resolve("www.google.com");
        std::cout << "Resolved www.google.com to IPv6: " << google_ip.to_string() << "\n";

        accel::endpoint_v6 ep(google_ip, 80);
        accel::socket sock(accel::ip_versions::version_6, accel::protocols::tcp);
        sock.connect(ep);

        std::string data = "GET http://www.google.com/index.html HTTP/1.1\nConnection: close\n\n";
        sock.send(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());

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
        std::cout << "IPv4 test completed successfully.\n";
    }

    std::cout << "Test completed successfully.\n";

    return 0;
}