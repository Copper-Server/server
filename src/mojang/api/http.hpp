#ifndef SRC_MOJANG_API_HTTP
#define SRC_MOJANG_API_HTTP
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/parser.hpp>
#include <string>

#include <library/fast_task/include/networking.hpp>
#include <library/list_array.hpp>

namespace mojang::api {
    class http {
    public:
        static std::string request(const std::string& mode, const std::string& address, const std::string& query, uint16_t port = 80, uint8_t max_redirects = 4) {
            auto port_string = std::to_string(port);

            std::unique_ptr<fast_task::networking::TcpClientSocket> client(fast_task::networking::TcpClientSocket::connect({address, port}));


            boost::beast::flat_buffer buf;
            boost::beast::http::response<boost::beast::http::string_body> res;
            {
                std::stringstream request_stream;
                request_stream << mode << " " << query << " HTTP/1.1\r\n";
                request_stream << "Host: " << address + (port != 80 ? (":" + port_string) : "") << "\r\n";
                request_stream << "Accept: */*\r\n";
                request_stream << "Connection: close\r\n\r\n";
                std::string str = request_stream.str();
                client->send((uint8_t*)str.data(), (int32_t)str.size());
            }

            {
                boost::beast::http::parser<false, boost::beast::http::string_body> parser;
                list_array<uint8_t> answer;
                uint8_t recv_buf[1024];
                while (auto received = client->recv(recv_buf, 1024))
                    answer.push_back((uint8_t*)recv_buf, received);

                try {
                    boost::beast::error_code ec;
                    boost::asio::const_buffer buffer(answer.data(), answer.size());
                    parser.put(buffer, ec);
                    parser.put_eof(ec);
                    res = parser.get();
                } catch (const std::exception& ex) {
                    throw std::runtime_error("HTTP parse error: " + std::string(ex.what()));
                }
            }


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
                        new_port = (uint16_t)std::stoi(new_address.substr(pos + 1));
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
