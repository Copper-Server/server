#ifndef SRC_MOJANG_API_HTTP
#define SRC_MOJANG_API_HTTP
#if defined(_MSC_VER)
    #include <SDKDDKVer.h>
#endif
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

namespace mojang::api {
    class http {
    public:
        static std::string request(const std::string& mode, const std::string& address, const std::string& query, uint16_t port = 80, uint8_t max_redirects = 4) {
            auto port_string = std::to_string(port);
            boost::asio::io_context ioc;
            boost::asio::ip::tcp::socket socket(ioc);

            {
                boost::asio::ip::tcp::resolver resolver(ioc);
                boost::asio::ip::tcp::resolver::query address_query(address, port_string);
                boost::system::error_code ec;
                auto const results = resolver.resolve(address_query, ec);

                if (results.empty() || ec)
                    throw std::runtime_error("Failed to resolve " + address);
                boost::asio::connect(socket, results.begin(), results.end(), ec);
                if (ec)
                    throw std::runtime_error("Failed to connect " + address);
            }

            boost::beast::flat_buffer buf;
            boost::beast::http::response<boost::beast::http::string_body> res;
            {
                boost::asio::streambuf request;
                std::ostream request_stream(&request);
                request_stream << mode << " " << query << " HTTP/1.1\r\n";
                request_stream << "Host: " << address + (port != 80 ? (":" + port_string) : "") << "\r\n";
                request_stream << "Accept: */*\r\n";
                request_stream << "Connection: close\r\n\r\n";
                boost::asio::write(socket, request);
            }
            boost::beast::http::read(socket, buf, res);


            switch (res.result()) {
            case boost::beast::http::status::ok:
                return res.body();
            case boost::beast::http::status::temporary_redirect: {
                if (max_redirects == 0)
                    throw std::runtime_error("Too many redirects");
                //parse location, extract new address, query and port
                std::string new_address = (std::string)res["Location"];
                std::string new_query;
                uint16_t new_port = 80;
                size_t pos = new_address.find("://");
                if (pos != std::string::npos) {
                    new_address = new_address.substr(pos + 3);
                    pos = new_address.find('/');
                    if (pos != std::string::npos) {
                        new_query = new_address.substr(pos);
                        new_address = new_address.substr(0, pos);
                    }
                    pos = new_address.find(':');
                    if (pos != std::string::npos) {
                        new_port = std::stoi(new_address.substr(pos + 1));
                        new_address = new_address.substr(0, pos);
                    }
                }
                return http::request(mode, new_address, new_query, new_port, max_redirects - 1);
            }
            default:
                throw std::runtime_error("HTTP" + (std::string)boost::beast::http::obsolete_reason(res.result()));
            }
        }

        static std::string get(const std::string& address, const std::string& query, uint16_t port = 80, uint8_t max_redirects = 4) {
            return request("GET", address, query, port, max_redirects);
        }

        static std::string post(const std::string& address, const std::string& query, uint16_t port = 80, uint8_t max_redirects = 4) {
            return request("POST", address, query, port, max_redirects);
        }
    };
}
#endif /* SRC_MOJANG_API_HTTP */
