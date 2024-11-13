
#include <bit>
#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/shared_lock_guard.hpp>

#include <library/enbt.hpp>
#include <src/ClientHandleHelper.hpp>
#include <src/api/configuration.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/chat.hpp>
#include <src/log.hpp>


#define OPENSSL_CHECK(OPERATION, console_output) \
    if ((OPERATION) <= 0) {                      \
        log::fatal("OpenSSL", console_output);   \
    }
#define NOT_NULL(X, console_output)            \
    if (!(X)) {                                \
        log::fatal("OpenSSL", console_output); \
    }

namespace copper_server {
    TCPclient* first_client_holder;
    std::atomic_uint64_t TCPsession::id_gen;
    bool TCPsession::do_log_connection_errors = true;


    base_objects::Response TCPclient::OnSwitch() {
        return base_objects::Response::Empty();
    }

    //will return nullptr if Redefine not needed
    TCPclient::~TCPclient() {}

    void TCPclient::logConsole(const std::string& prefix, const list_array<uint8_t>& data, size_t size) {
        std::string output = prefix + " ";
        output.reserve(size * 2);
        static const char hex_chars[] = "0123456789ABCDEF";
        for (size_t i = 0; i < size; i++) {
            output.push_back(hex_chars[data[i] >> 4]);
            output.push_back(hex_chars[data[i] & 0xF]);
        }
        log::debug("Debug tools", output);
    }

    TCPsession::TCPsession(boost::asio::ip::tcp::socket&& s, TCPclient* client_handler, uint64_t& set_timeout)
        : sock(std::move(s)), timeout(set_timeout) {
        chandler = client_handler->DefineOurself(this);
        read_data.resize(1024);
    }

    TCPsession::~TCPsession() {
        if (_sharedData) {
            if (_sharedData->packets_state.state != SharedClientData::packets_state_t::protocol_state::initialization)
                api::players::handlers::on_player_leave(sharedDataRef());
            api::players::remove_player(sharedDataRef());
        }
        if (chandler)
            delete chandler;
    }

    base_objects::client_data_holder& TCPsession::sharedDataRef() {
        return _sharedData ? _sharedData : _sharedData = api::players::allocate_player();
    }

    SharedClientData& TCPsession::sharedData() {
        return *sharedDataRef();
    }


    void TCPsession::connect() {
        active = true;
        post(sock.get_executor(), [this] { req_loop(); return true; });
    }

    void TCPsession::connect(std::vector<uint8_t>& connection_data, boost::system::error_code ec) {
        if (checked(ec, "connect")) {
            active = true;
            read_data = connection_data;
            on_request(ec, read_data.size());
        }
    }

    void TCPsession::disconnect() {
        post(sock.get_executor(), [this] {
            if (!sock.is_open()) {
                log::error("protocol", "[error] [" + std::to_string(id) + "] connection aborted by client.");
            } else
                sock.cancel();
            sock.close();
            active = false;
            Server::instance().close_session(this);
        });
    }

    bool TCPsession::isActive() {
        return active;
    }

    bool TCPsession::start_symmetric_encryption(const list_array<uint8_t>& encryption_key, const list_array<uint8_t>& encryption_iv) {
        if (!encryption.initialize(encryption_key, encryption_iv))
            return false;
        encryption_enabled = true;
        return true;
    }

    void TCPsession::send(Response&& resp) {
        //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
            for (auto& it : resp.data)
                TCPclient::logConsole("S (" + std::to_string(id) + ")", it.data, it.data.size());
        //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        if (resp.do_disconnect)
            disconnect();
        else if (resp.data.size()) {
            list_array<uint8_t> response_data;
            for (auto& it : resp.data)
                response_data.push_back(std::move(it.data));


            if (encryption_enabled)
                encryption.encrypt(response_data, response_data);


            std::shared_ptr<std::vector<uint8_t>> send_data = std::make_shared<std::vector<uint8_t>>(response_data.begin(), response_data.end());
            if (resp.do_disconnect_after_send) {
                boost::asio::async_write(sock, boost::asio::buffer(*send_data), [this, send_data](boost::system::error_code, size_t completed) {disconnect();return completed; });
            } else
                boost::asio::async_write(sock, boost::asio::buffer(*send_data), [this, send_data](boost::system::error_code ec, size_t completed) { if (checked(ec, "response")) req_loop();  return completed; });
        } else
            req_loop();
    }

    bool TCPsession::checked(boost::system::error_code ec, std::string const& msg) {
        if (ec || sock.is_open() == false) {
            if (ec != boost::asio::error::operation_aborted)
                return false;
            if (do_log_connection_errors)
                log::error("protocol", "[" + msg + "] [" + std::to_string(id) + "]" + ec.message());
            disconnect();
        }
        return !ec.failed() && sock.is_open();
    }

    void TCPsession::req_loop() {
        sock.async_read_some(boost::asio::buffer(read_data), [this](boost::system::error_code ec, size_t size) {
            if (checked(ec, "req_loop"))
                on_request(ec, size);
        });
    }

    void TCPsession::consume_data(size_t read_size) {
        list_array<uint8_t> convert_data(read_data.data(), read_size);
        //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
            TCPclient::logConsole("P (" + std::to_string(id) + ")", convert_data, read_size);
        //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>

        if (encryption_enabled) {
            encryption.decrypt(convert_data, convert_data);
            read_data_cached.push_back(std::move(convert_data));
        } else
            read_data_cached.push_back(list_array<uint8_t>(convert_data.data(), read_size));
    }

    Response TCPsession::proceed_data() {
        while (true) {
            Response tmp(chandler->WorkClient(read_data_cached));
            read_data_cached.erase(0, tmp.valid_till);
            if (auto redefHandler = chandler->RedefineHandler(); redefHandler && !tmp.isDisconnect()) {
                tmp.valid_till = 0;
                delete chandler;
                chandler = redefHandler;
                tmp = chandler->OnSwitch();
                if (tmp.data.empty() || tmp.do_disconnect)
                    continue;
            }
            return tmp;
        }
    }

    void TCPsession::on_request(boost::system::error_code ec, size_t read_size) {
        if (checked(ec, "on_request")) {
            consume_data(read_size);
            send(proceed_data());
        }
    }

    TCPclient& TCPsession::handler() {
        return *chandler;
    }

    Server* Server::global_instance = nullptr;

    Server& Server::instance() {
        return global_instance ? *global_instance : throw std::runtime_error("Server not initialized");
    }

    void Server::make_clean_up() {
        std::unique_lock lock(close_mutex);
        std::list<TCPsession*> to_clean;
        to_clean.swap(queried_close);
        for (auto it : to_clean)
            if (!sessions.erase(it)) {
                log::error("protocol", "Failed to erase session from list");
                return;
            }
        lock.unlock();
        for (auto it : to_clean)
            delete it;
    }

    void Server::close_session(TCPsession* session) {
        std::unique_lock lock(close_mutex);
        queried_close.push_back(session);
    }

    void Server::AsyncWork(TCPsession* session) {
        boost::system::error_code err;
        if (session->handler().DoDisconnect(session->sock.remote_endpoint().address()))
            session->sock.close();
        else {
            std::unique_lock lock(close_mutex);
            if (sessions.insert(session).second)
                session->connect();
            else
                lock.unlock();
        }
    }

    void Server::Worker() {
        make_clean_up();
        TCPsession* session = new TCPsession(boost::asio::ip::tcp::socket(make_strand(threads)), first_client_holder, all_connections_timeout);
        TCPacceptor.async_accept(session->sock, [this, session](const boost::system::error_code& error) {
            if (error == boost::asio::error::operation_aborted || disabled)
                return;
            Worker();
            AsyncWork(session);
        });
    }

    auto Server::resolveEndpoint(const std::string& ip, uint16_t port) {
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(ip, std::to_string(port));
        auto list = resolver.resolve(query);
        auto endpoint = list.begin()->endpoint();

        local_server = false;
        //iterate all endpoints
        for (auto& endpoints : list) {
            auto address = endpoints.endpoint().address();
            if (address.is_loopback())
                local_server = true;
            log::debug("Server", "Server address: " + address.to_string());
        }


        return endpoint;
    }

    boost::asio::io_service& Server::getService() {
        return *service;
    }

    bool Server::is_local_server() {
        return local_server;
    }

    std::string Server::get_ip() const {
        return ip;
    }

    bool Server::decrypt_data(list_array<uint8_t>& data) const {
        if (data.size() > INT32_MAX)
            return false;
        if (!server_rsa_key)
            return false;
        int result = RSA_private_decrypt(data.size(), data.data(), data.data(), server_rsa_key, RSA_PKCS1_PADDING);
        if (result < 0)
            return false;
        data.resize(result);
        return true;
    }

    bool Server::encrypt_data(list_array<uint8_t>& data) const {
        if (data.size() > INT32_MAX)
            return false;
        if (!server_rsa_key)
            return false;
        int result = RSA_public_encrypt(data.size(), data.data(), data.data(), server_rsa_key, RSA_PKCS1_PADDING);
        if (result < 0)
            return false;
        data.resize(result);
        return true;
    }

    boost::asio::const_buffer Server::private_key_buffer() {
        return boost::asio::const_buffer(server_private_key.data(), server_private_key.size());
    }

    boost::asio::const_buffer Server::public_key_buffer() {
        return boost::asio::const_buffer(server_public_key.data(), server_public_key.size());
    }

    size_t Server::key_length() {
        return ssl_key_length;
    }

    Server::Server(boost::asio::io_service* io_service, const std::string& ip, uint16_t port, size_t threads, size_t ssl_key_length)
        : TCPacceptor(*io_service, resolveEndpoint(ip, port)),
          threads(threads ? threads : std::thread::hardware_concurrency()),
          ssl_key_length(ssl_key_length),
          ip(ip) {
        if (global_instance)
            throw std::runtime_error("Server already initialized");
        global_instance = this;
        service = io_service;
        if (ssl_key_length) {
            server_rsa_key = RSA_generate_key(ssl_key_length, RSA_F4, nullptr, nullptr);
            NOT_NULL(server_rsa_key, "Failed to generate RSA key");
            BIO* bio = BIO_new(BIO_s_mem());
            NOT_NULL(bio, "Failed to create BIO");
            OPENSSL_CHECK(PEM_write_bio_RSAPrivateKey(bio, server_rsa_key, nullptr, nullptr, 0, nullptr, nullptr), "Failed to write RSA private key");
            server_private_key.resize(BIO_pending(bio));
            OPENSSL_CHECK(BIO_read(bio, server_private_key.data(), server_private_key.size()), "Failed to read RSA private key");
            OPENSSL_CHECK(BIO_free(bio), "Failed to free BIO");
            { //public key in DEM format
                uint8_t* der = nullptr;
                int32_t len = i2d_RSA_PUBKEY(server_rsa_key, &der);
                if (len < 0)
                    throw std::runtime_error("Failed to convert RSA key to DER format");
                NOT_NULL(der, "Failed to convert RSA key to DER format");
                server_public_key = list_array<uint8_t>(der, len);
                OPENSSL_free(der);
            }
        }
        if (local_server)
            api::configuration::get().protocol.offline_mode = true;
    }

    Server::~Server() {
        if (!disabled)
            stop();
        if (server_rsa_key)
            RSA_free(server_rsa_key);
    }

    void Server::start() {
        if (disabled) {
            Worker();
            disabled = false;
            service->run();
        } else
            throw std::exception("tcp server already run");
    }

    void Server::stop() {
        if (!disabled) {
            std::lock_guard lock(close_mutex);
            make_clean_up();
            TCPacceptor.close();
            for (auto it : sessions)
                it->disconnect();
            disabled = true;
            service->stop();
        } else
            throw std::exception("tcp server already stoped");
    }
}
