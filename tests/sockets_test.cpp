#include <iostream>

#include <accel/macros>
#include <accel/sockets>

using namespace accel::sockets;

int main(int argc, char* argv[])
{
    ACC_ASSERT(ip_address_v4::localhost().to_string() == "127.0.0.1");
    ACC_ASSERT(ip_address_v4::broadcast().to_string() == "255.255.255.255");
    ACC_ASSERT(ip_address_v4::any().to_string() == "0.0.0.0");
    ACC_ASSERT(ip_address_v4("1.2.3.4").to_string() == "1.2.3.4");

    {
        auto google_ip = ip_address_v4::resolve("www.google.com");
        std::cout << "Resolved www.google.com to IPv4: " << google_ip.to_string() << "\n";

        endpoint_v4 ep(google_ip, 80);
        accel::sockets::socket sock(ip_versions::version_4, protocols::tcp);
        sock.connect(ep);

        std::string data = "GET http://www.google.com/index.html HTTP/1.1\nConnection: close\n\n";
        sock.send(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());

        std::size_t total_size = 0;
        while (true)
        {
            std::uint8_t buffer[1024] = { 0 };
            auto result = sock.receive(buffer, 1024);
            if (result.has_value())
            {
                auto chunk_size = result.get_value();
                if (chunk_size == 0)
                {
                    std::cout << "\nEnd of stream\n";
                    break;
                }
                std::cout << buffer;
                total_size += chunk_size;   
            }
            else
            { 
                auto& error = result.get_error();
                std::cout << "Socket error: " << error.get_message().to_string() << "\n";
                break;
            };
        }

        std::cout << "Total size: " << total_size << "\n";

        std::cout << "IPv4 test completed successfully.\n";
    }
    

    {
        auto google_ip = ip_address_v6::resolve("www.google.com");
        std::cout << "Resolved www.google.com to IPv6: " << google_ip.to_string() << "\n";

        endpoint_v6 ep(google_ip, 80);
        accel::sockets::socket sock(ip_versions::version_6, protocols::tcp);
        sock.connect(ep);

        std::string data = "GET http://www.google.com/index.html HTTP/1.1\nConnection: close\n\n";
        sock.send(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());

        std::size_t total_size = 0;
        while (true)
        {
            std::uint8_t buffer[1024] = { 0 };
            auto result = sock.receive(buffer, 1024);
            if (result.has_value())
            {
                auto chunk_size = result.get_value();
                if (chunk_size == 0)
                {
                    std::cout << "\nEnd of stream\n";
                    break;
                }
                std::cout << buffer;
                total_size += chunk_size;   
            }
            else
            { 
                auto& error = result.get_error();
                std::cout << "Socket error: " << error.get_message().to_string() << "\n";
                break;
            };
        }

        std::cout << "Total size: " << total_size << "\n";
        std::cout << "IPv6 test completed successfully.\n";
    }

    std::cout << "Test completed successfully.\n";

    return 0;
}