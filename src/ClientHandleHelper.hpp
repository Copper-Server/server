#pragma once
#include <stdint.h>


#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <ctime>
#include <iostream>
#include <string>


constexpr bool CONSTEXPR_DEBUG_DATA_TRANSPORT = false;

namespace bai {
    using namespace boost::asio;
}

namespace baip {
    using namespace boost::asio::ip;
}

class TCPclient {
protected:
public:
    struct Response {
        Response(const std::vector<std::vector<uint8_t>>& response_bytes, bool disconnect = false, bool disconnect_after_send = false) {
            data = response_bytes;
            do_disconnect = disconnect;
            do_disconnect_after_send = disconnect_after_send;
        }

        Response(std::vector<std::vector<uint8_t>>&& response_bytes, bool disconnect = false, bool disconnect_after_send = false) {
            data = std::move(response_bytes);
            do_disconnect = disconnect;
            do_disconnect_after_send = disconnect_after_send;
        }

        std::vector<std::vector<uint8_t>> data;
        bool do_disconnect = false;
        bool do_disconnect_after_send = false;
    };

    virtual Response WorkClient(const std::vector<std::vector<uint8_t>>&, uint64_t timeout) = 0;
    //will return nullptr if Redefine not neded
    virtual TCPclient* RedefineHandler() = 0;
    virtual bool DoDisconnect(baip::address) = 0;
    virtual TCPclient* DefineOurself(baip::tcp::socket& sock) = 0;

    virtual ~TCPclient() {}

    static void logConsole(const std::vector<uint8_t>& data) {
        static const char hex_chars[] = "0123456789ABCDEF";
        for (auto& it : data)
            std::cout << hex_chars[(it & 0xF0) >> 4] << hex_chars[it & 0x0F] << " ";
        std::cout << std::endl;
    }

    virtual void PrepareData(std::vector<std::vector<uint8_t>>&) = 0;

    uint16_t processed_packets = 0;
};

extern TCPclient* first_client_holder;

struct TCPsession {
    static bool do_log_connection_errors;
    static std::atomic_uint64_t id_gen;
    uint64_t id = id_gen++;

    TCPsession(baip::tcp::socket&& s, TCPclient* client_handler, uint64_t& set_timeout)
        : sock(std::move(s)), timeout(set_timeout) {
        chandler = client_handler->DefineOurself(sock);
    }

    TCPsession(TCPsession&& move) noexcept
        : sock(std::move(move.sock)), timeout(move.timeout) {
        chandler = move.chandler;
    }

    ~TCPsession() {
        if (chandler)
            delete chandler;
    }

    void connect() {
        active = true;
        post(sock.get_executor(), [this] { req_loop(); });
    }

    void connect(std::vector<uint8_t>& connection_data, boost::system::error_code ec) {
        active = true;
        data = connection_data;
        post(sock.get_executor(), [this, ec] { on_request(ec, data.size()); });
    }

    void disconnect() {
        post(sock.get_executor(), [this] {
            sock.cancel();
            active = false;
        });
    }

    bool isActive() {
        return active;
    }

    baip::tcp::socket sock;

private:
    bool checked(boost::system::error_code ec, std::string const& msg = "error") {
        if (ec && do_log_connection_errors) {
            std::cout << msg << "(" << id << "): " << ec.message() << std::endl;
            disconnect();
        }
        return !ec.failed();
    }

    void req_loop(boost::system::error_code ec = {}) {
        if (checked(ec, "req_loop")) {
            sock.async_wait(sock.wait_read, [this](boost::system::error_code ec) {
                if (checked(ec, "on_wait")) {
                    if (size_t aviable = sock.available()) {
                        boost::system::error_code ec;
                        data.resize(aviable);
                        sock.read_some(boost::asio::buffer(data), ec);
                        on_request(ec, data.size());
                    } else
                        async_read(sock, boost::asio::dynamic_buffer(data, 2097154), [this](boost::system::error_code ec, size_t xfr) { on_request(ec, xfr); });
                }
            });
        }
    }

    void on_request(boost::system::error_code ec, size_t n) {
        if (checked(ec, "on_request")) {
            data.resize(n);
            std::vector<std::vector<uint8_t>> combined_packets;
            combined_packets.push_back(data);
        prepare_packs:
            if (size_t aviable = sock.available()) {
                boost::system::error_code ec;
                data.resize(aviable);
                data.resize(sock.read_some(boost::asio::buffer(data), ec));
                combined_packets.push_back(data);
                goto prepare_packs;
            }

            //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
            if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
                for (auto& it : combined_packets) {
                    std::cout << "P (" << id << ") ";
                    TCPclient::logConsole(it);
                }
            //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>

            chandler->PrepareData(combined_packets);
        repeartDecode:
            TCPclient::Response&& tmp = chandler->WorkClient(combined_packets, timeout);
            if (auto redefHandler = chandler->RedefineHandler()) {
                if (chandler->processed_packets)
                    combined_packets.erase(combined_packets.begin(), combined_packets.begin() + chandler->processed_packets);
                delete chandler;
                chandler = redefHandler;
                goto repeartDecode;
            }
            if (tmp.do_disconnect)
                disconnect();
            else if (size_t response_len = tmp.data.size(); response_len) {
                --response_len;
                for (size_t i = 0; i < response_len; i++)
                    write(sock, bai::buffer(tmp.data[i]), [this](boost::system::error_code ec, size_t a) { return checked(ec); });

                data = tmp.data[response_len];
                if (tmp.do_disconnect_after_send)
                    async_write(sock, bai::buffer(data), [this](boost::system::error_code, size_t) { disconnect(); });
                else
                    async_write(sock, bai::buffer(data), [this](boost::system::error_code ec, size_t) { req_loop(ec); });
            } else
                req_loop();

            //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disabe this block>
            if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
                for (auto& it : tmp.data) {
                    std::cout << "S (" << id << ") ";
                    TCPclient::logConsole(it);
                }
            //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disabe this block>
        }
    }

    uint64_t& timeout;
    std::vector<uint8_t> data;
    TCPclient* chandler = nullptr;
    std::atomic_bool active{false};
};

class TCPserver {
    boost::asio::io_service* service;
    baip::tcp::acceptor TCPacceptor;
    boost::asio::thread_pool threads;
    std::list<TCPsession*> sessions;

    uint8_t reseter = 0;

    void AsyncWork(TCPsession* session, std::vector<uint8_t>* buf) {
        boost::system::error_code err;
        if (first_client_holder->DoDisconnect(session->sock.remote_endpoint().address()))
            session->sock.cancel();
        else
            sessions.emplace_back(session)->connect(*buf, err);
        delete buf;
        sessions.remove_if([](TCPsession* s) {
            if (!s->isActive()) {
                delete s;
                return true;
            }
            return false;
        });
    }

    void Worker() {
        TCPsession* session = new TCPsession(baip::tcp::socket(make_strand(threads)), first_client_holder, all_connections_timeout);
        TCPacceptor.async_accept(session->sock, [this, session](const boost::system::error_code& error) {
            Worker();
            std::vector<uint8_t>* buf = new std::vector<uint8_t>(max_accept_buffer);
            boost::system::error_code err;
            buf->resize(session->sock.read_some(boost::asio::buffer(*buf), err));
            if (!buf->size()) {
                async_read(session->sock, boost::asio::dynamic_buffer(*buf, max_accept_buffer), [this, session, buf](boost::system::error_code ec, size_t xfr) { AsyncWork(session, buf); });
            } else
                AsyncWork(session, buf);
        });
    }

    std::atomic_bool disabled = true;

public:
    uint16_t max_accept_buffer = 100;
    //30 sec
    uint64_t all_connections_timeout = 30000;

    TCPserver(boost::asio::io_service* io_service, baip::tcp protocol_adress_version, baip::port_type port_num, size_t threads = 0)
        : TCPacceptor(*io_service, baip::tcp::endpoint(protocol_adress_version, port_num)),
          threads(threads ? threads : std::thread::hardware_concurrency()) {
        service = io_service;
    }

    void start() {
        if (disabled) {
            Worker();
            disabled = false;
            service->run();
        } else
            throw std::exception("tcp server already run");
    }

    void hotRestart() {
        threads.stop();
        Worker();
    }

    void stop() {
        if (!disabled) {
            for (auto it : sessions)
                if (it->isActive()) {
                    it->disconnect();
                    delete it;
                }
            service->stop();
            sessions.clear();
            disabled = true;
        } else
            throw std::exception("tcp server already stoped");
    }

    void restart() {
        if (!disabled)
            stop();
        start();
    }
};
