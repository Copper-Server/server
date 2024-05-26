
#include <bit>
#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/shared_lock_guard.hpp>

#include "ClientHandleHelper.hpp"
#include "base_objects/chat.hpp"
#include "library/enbt.hpp"
#include "log.hpp"


#include "api/players.hpp"


#define OPENSSL_CHECK(OPERATION, console_output) \
    if ((OPERATION) <= 0) {                      \
        log::fatal("OpenSSL", console_output);   \
    }
#define NOT_NULL(X, console_output)            \
    if (!(X)) {                                \
        log::fatal("OpenSSL", console_output); \
    }

namespace crafted_craft {

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

    TCPsession::TCPsession(boost::asio::ip::tcp::socket&& s, TCPclient* client_handler, uint64_t& set_timeout, TCPserver* server)
        : sock(std::move(s)), timeout(set_timeout), server(server) {
        chandler = client_handler->DefineOurself(this);
        read_data.resize(1024);
    }

    TCPsession::~TCPsession() {
        if (_sharedData) {
            if (_sharedData->packets_state.state != SharedClientData::packets_state_t::protocol_state::initialization)
                api::players::handlers::on_player_leave(sharedDataRef());
            server->online_players.remove_player(sharedDataRef());
        }
        if (chandler)
            delete chandler;
    }

    base_objects::client_data_holder& TCPsession::sharedDataRef() {
        return _sharedData ? _sharedData : _sharedData = server->online_players.allocate_player();
    }

    SharedClientData& TCPsession::sharedData() {
        return *sharedDataRef();
    }

    TCPserver& TCPsession::serverData() {
        return *server;
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
            server->close_session(this);
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

    boost::asio::const_buffer TCPsession::public_key() {
        return server->public_key_buffer();
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
            read_data_cached.remove(0, tmp.valid_till);
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

    void TCPserver::make_clean_up() {
        std::unique_lock<std::mutex> lock(close_mutex);
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

    void TCPserver::AsyncWork(TCPsession* session) {
        boost::system::error_code err;
        if (first_client_holder->DoDisconnect(session->sock.remote_endpoint().address()))
            session->sock.close();
        else {
            std::unique_lock<std::mutex> lock(close_mutex);
            if (sessions.insert(session).second)
                session->connect();
            else
                lock.unlock();
        }
    }

    void TCPserver::Worker() {
        make_clean_up();
        TCPsession* session = new TCPsession(boost::asio::ip::tcp::socket(make_strand(threads)), first_client_holder, all_connections_timeout, this);
        TCPacceptor.async_accept(session->sock, [this, session](const boost::system::error_code& error) {
            Worker();
            AsyncWork(session);
        });
    }

    auto TCPserver::resolveEndpoint(const std::string& ip, uint16_t port) {
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(ip, std::to_string(port));
        auto list = resolver.resolve(query);
        auto endpoint = list.begin()->endpoint();

        //iterate all endpoints
        for (auto& endpoints : list) {
            auto address = endpoints.endpoint().address();
            if (address.is_loopback())
                local_server = true;
            log::debug("Server", "Server address: " + address.to_string());
        }


        return endpoint;
    }

    boost::asio::io_service& TCPserver::getService() {
        return *service;
    }

    bool TCPserver::is_local_server() {
        return local_server;
    }

    mojang::api::session_server& TCPserver::getSessionServer() {
        return session_server;
    }

    std::string TCPserver::get_ip() const {
        return ip;
    }

    bool TCPserver::decrypt_data(list_array<uint8_t>& data) const {
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

    bool TCPserver::encrypt_data(list_array<uint8_t>& data) const {
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

    boost::asio::const_buffer TCPserver::private_key_buffer() {
        return boost::asio::const_buffer(server_private_key.data(), server_private_key.size());
    }

    boost::asio::const_buffer TCPserver::public_key_buffer() {
        return boost::asio::const_buffer(server_public_key.data(), server_public_key.size());
    }

    size_t TCPserver::key_length() {
        return ssl_key_length;
    }

    const list_array<uint8_t>& TCPserver::legacyMotdResponse() {
        boost::shared_lock_guard lock(data_lock);
        return legacy_motd;
    }

    void TCPserver::setLegacyMotd(const std::u8string& motd) {
        if (motd.size() + 20 > 0xFFFF)
            throw std::invalid_argument("motd too long");
        list_array<char16_t> legacy_motd;
        legacy_motd.reserve_push_back(motd.size() + 20);
        legacy_motd.push_back(u'\u00A7');
        legacy_motd.push_back(u'1');
        legacy_motd.push_back(u'\0'); //default color
        legacy_motd.push_back(u'1');
        legacy_motd.push_back(u'2');
        legacy_motd.push_back(u'7');
        legacy_motd.push_back(u'\0'); //protocol version(always incompatible)
        legacy_motd.push_back(u'0');
        legacy_motd.push_back(u'.');
        legacy_motd.push_back(u'0');
        legacy_motd.push_back(u'.');
        legacy_motd.push_back(u'0');
        legacy_motd.push_back(u'\0'); //server version
        utf8::utf8to16(motd.begin(), motd.end(), std::back_inserter(legacy_motd));
        legacy_motd.push_back(u'\0');
        legacy_motd.push_back(u'0');
        legacy_motd.push_back(u'\0');
        legacy_motd.push_back(u'0');
        legacy_motd.push_back(u'\0'); //why legacy need to know about online players?
        if constexpr (std::endian::native != std::endian::big)
            for (char16_t& it : legacy_motd)
                it = ENBT::ConvertEndian(std::endian::big, it);
        list_array<uint8_t> response;
        response.push_back(0xFF); //KICK packet
        uint16_t len = legacy_motd.size();
        if constexpr (std::endian::native != std::endian::big)
            len = ENBT::ConvertEndian(std::endian::big, len);
        response.push_back(uint8_t(len >> 8));
        response.push_back(uint8_t(len & 0xFF));
        response.push_back(reinterpret_cast<uint8_t*>(legacy_motd.data()), legacy_motd.size() * 2);

        std::lock_guard<std::shared_mutex> lock(data_lock);
        this->legacy_motd = std::move(response);
    }

    TCPserver::TCPserver(boost::asio::io_service* io_service, const std::string& ip, uint16_t port, size_t threads = 0, size_t ssl_key_length = 1024)
        : TCPacceptor(*io_service, resolveEndpoint(ip, port)),
          threads(threads ? threads : std::thread::hardware_concurrency()),
          ssl_key_length(ssl_key_length),
          ip(ip) {
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
            server_config.offline_mode = true;
    }

    TCPserver::~TCPserver() {
        if (!disabled)
            stop();
        if (server_rsa_key)
            RSA_free(server_rsa_key);
    }

    void TCPserver::start() {
        if (disabled) {
            Worker();
            disabled = false;
            service->run();
        } else
            throw std::exception("tcp server already run");
    }

    void TCPserver::stop() {
        if (!disabled) {
            make_clean_up();
            TCPacceptor.close();
            {
                std::lock_guard<std::mutex> lock(close_mutex);
                for (auto it : sessions)
                    it->disconnect();
            }

            service->stop();
            disabled = true;
        } else
            throw std::exception("tcp server already stoped");
    }
}