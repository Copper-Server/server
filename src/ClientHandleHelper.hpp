#ifndef SRC_CLIENTHANDLEHELPER
#define SRC_CLIENTHANDLEHELPER

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <chrono>
#include <ctime>
#include <iostream>
#include <openssl/aes.h>
#include <string>
#include <unordered_set>
#include <utf8.h>


#include "base_objects/chat.hpp"
#include "library/enbt.hpp"
#include "library/list_array.hpp"

#include "mojang_api/session_server.hpp"
using namespace boost::placeholders;
#define OPENSSL_CHECK(OPERATION, console_output)  \
    if ((OPERATION) <= 0) {                       \
        std::cerr << console_output << std::endl; \
        std::terminate();                         \
    }
#define NOT_NULL(X, console_output)               \
    if (!(X)) {                                   \
        std::cerr << console_output << std::endl; \
        std::terminate();                         \
    }

struct SharedClientData {
    std::string name;
    std::shared_ptr<mojang::api::session_server::player_data> data;
    std::string client_brand;


    std::string locale; //max 16 chars
    std::unordered_set<std::string> compatible_plugins;
    uint8_t view_distance = 0;
    enum class ChatMode : uint8_t {
        ENABLED = 0,
        COMMANDS_ONLY = 1,
        HIDDEN = 2
    } chat_mode = ChatMode::ENABLED;

    union {
        struct {
            bool cape_enabled : 1;
            bool jacket_enabled : 1;
            bool left_sleeve_enabled : 1;
            bool right_sleeve_enabled : 1;
            bool left_pants_leg_enabled : 1;
            bool right_pants_leg_enabled : 1;
            bool hat_enabled : 1;
            bool _unused : 1;
        };

        uint8_t mask = UINT8_MAX - 1;
    } skin_parts;

    enum class MainHand : uint8_t {
        LEFT = 0,
        RIGHT = 1
    } main_hand = MainHand::RIGHT;

    bool enable_filtering : 1 = false;
    bool allow_server_listings : 1 = true;
    bool enable_chat_colors : 1 = true;

    struct ResourcePackData {
        bool required : 1 = false;
    };

    struct packets_state_t {
        std::unordered_set<ENBT::UUID> active_resource_packs;
        std::unordered_map<ENBT::UUID, ResourcePackData> pending_resource_packs;
        std::unordered_map<ENBT::UUID, uint32_t> entity_id_holder;
        std::list<int32_t> pending_teleport_ids;
        int32_t entity_id_generator;
        int32_t current_block_sequence_id = 0;
        int32_t container_state_id = 0;


        int32_t protocol_version = -1;
    } packets_state;

    std::chrono::milliseconds ping = std::chrono::milliseconds(0);

    void registerPlugin(std::string& plugin) {
        compatible_plugins.insert(plugin);
    }

    void unregisterPlugin(std::string& plugin) {
        compatible_plugins.erase(plugin);
    }

    bool isCompatiblePlugin(std::string& plugin) {
        return compatible_plugins.contains(plugin);
    }

    std::mutex safety_lock;

    uint32_t registerEntityId(const ENBT::UUID& id) {
        std::lock_guard<std::mutex> lock(safety_lock);
        if (auto it = packets_state.entity_id_holder.find(id); it != packets_state.entity_id_holder.end())
            return it->second;
        else
            return packets_state.entity_id_holder[id] = packets_state.entity_id_generator++;
    }

    bool unregisterEntityIt(const ENBT::UUID& id) {
        std::lock_guard<std::mutex> lock(safety_lock);
        return packets_state.entity_id_holder.erase(id);
    }
};

struct ServerConfiguration {
    struct RCON {
        std::string password;
        uint16_t port;
        bool enabled;
        bool broadcast_to_ops;
    } rcon;

    struct Query {
        uint16_t port;
        bool enabled;
    } query;

    struct World {
        std::string name;
        std::string seed;
        std::string type;

        bool generate_structures;

        std::unordered_map<std::string, std::string> generator_settings;
    } world;

    struct GamePlay {
        std::string difficulty;
        uint64_t max_chained_neighbor_updates;
        uint32_t max_tick_time;
        uint32_t view_distance;
        uint32_t simulation_distance;
        uint32_t max_word_size;
        uint32_t spawn_protection;
        uint32_t player_idle_timeout;
        bool hardcore;
        bool pvp;
        bool spawn_animals;
        bool spawn_monsters;
        bool allow_flight;
        bool sync_chunk_writes;
    } game_play;

    struct Protocol {
        uint32_t compression_threshold = -1;
        uint32_t max_players = 0; //0 for unlimited
        uint32_t rate_limit = 0;

        bool prevent_proxy_connections = false; //	If the ISP/AS sent from the server is different from the one from Mojang Studios' authentication server, the player is kicked.


    } protocol;

    //in this struct everything can be disabled by setting to zero
    struct AntiCheat {
        struct Fly {
            bool prevent_illegal_flying; //when player not have fly attribute then these checks will be performed for player
            std::chrono::milliseconds allow_flying_time;
        } fly;

        struct Speed {
            bool prevent_illegal_speed;
            float max_speed;
        } speed;

        struct XRay {
            uint32_t visibility_distance; //hides blocks that are farther than this distance(in blocks)
            bool send_fake_blocks;
            bool hide_surrounded_blocks;
            std::unordered_set<std::string> block_ids;
        } xray;

        bool check_block_breaking_time;

        float reach_threshold;

        struct KillAura {
            float angle_threshold;
        } killaura;

        bool nofall;

        struct Scaffold {
            //list of checks
            bool enable_all;
        } scaffold;

        struct FastPlace {
            uint32_t max_clicks;
        } fastplace;

        struct NoSlowDown {
            bool detect_baritone;
        } movement;
    } anti_cheat;

    struct Mojang {
        bool enforce_secure_profile = true;


    } mojang;

    struct Status {
        bool enable = true;
        bool show_players = true;

    } status;

    std::unordered_set<std::string> allowed_dimensions;

    std::string gamemode = "survival";
    uint32_t max_players = 20;
    bool enable_command_block = false;

    bool offline_mode = false;
    bool allowlist_mode = false; //if true, then only players in allowlist can join,  if false, then players in "allowlist" can't join
    std::unordered_set<std::string> allowlist;
};

constexpr bool CONSTEXPR_DEBUG_DATA_TRANSPORT = true;

namespace bai {
    using namespace boost::asio;
}

namespace baip {
    using namespace boost::asio::ip;
}

class TCPserver;
struct TCPsession;

namespace __internal___ {
    inline void query_close_session(TCPserver* server, TCPsession* client);
    inline boost::asio::const_buffer get_private_key(TCPserver* server);
    inline boost::asio::const_buffer get_public_key(TCPserver* server);
}

class AESencryption {
    AESencryption(const AESencryption&) = delete;
    AESencryption& operator=(const AESencryption&) = delete;

public:
    AESencryption() = default;

    bool initialize(const list_array<uint8_t>& key, const list_array<uint8_t>& iv) {
        if (key.size() != 16 || iv.size() != 16)
            return false;
        if (AES_set_encrypt_key(key.data(), 128, &enc_key) != 0)
            return false;
        if (AES_set_decrypt_key(key.data(), 128, &dec_key) != 0)
            return false;
        memcpy(iv_data, iv.data(), 8);
        return true;
    }

    void encrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out) {
        out.resize(data.size());
        AES_cfb8_encrypt(data.data(), out.data(), data.size(), &enc_key, iv_data, &num, AES_ENCRYPT);
    }

    void decrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out) {
        out.resize(data.size());
        AES_cfb8_encrypt(data.data(), out.data(), data.size(), &dec_key, iv_data, &num, AES_DECRYPT);
    }

private:
    AES_KEY enc_key;
    AES_KEY dec_key;
    uint8_t iv_data[8];
    int num = 0;
};

class TCPclient {
protected:
public:
    struct Response {
        struct Item {
            list_array<uint8_t> data;
            int32_t compression_threshold;
            bool apply_compression;

            Item()
                : apply_compression(false) {}

            Item(const list_array<uint8_t>& data, int32_t compression_threshold = -1, bool apply_compression = false)
                : data(data), compression_threshold(compression_threshold), apply_compression(apply_compression) {}

            Item(list_array<uint8_t>&& data, int32_t compression_threshold = -1, bool apply_compression = false)
                : data(std::move(data)), compression_threshold(compression_threshold), apply_compression(apply_compression) {}
        };

        ~Response() = default;

        list_array<Item> data;
        bool do_disconnect = false;
        bool do_disconnect_after_send = false;
        size_t valid_till = 0;

        static Response EnableCompressAnswer(const list_array<uint8_t>& data, int32_t compression_threshold, size_t valid_till = 0) {
            return Response({Item(data, compression_threshold, true)}, valid_till);
        }

        static Response Answer(const list_array<list_array<uint8_t>>& data, size_t valid_till = 0) {
            return Response(data.copy().convert<Item>([](auto&& item) { return Item(std::move(item)); }), valid_till);
        }

        static Response Answer(list_array<list_array<uint8_t>>&& data, size_t valid_till = 0) {
            return Response(data.take().convert<Item>([](auto&& item) { return Item(std::move(item)); }), valid_till);
        }

        static Response Empty() {
            return Response({}, 0);
        }

        static Response Disconnect() {
            return Response({}, true);
        }

        static Response Disconnect(const list_array<list_array<uint8_t>>& data, size_t valid_till = 0) {
            return Response(data.copy().convert<Item>([](auto&& item) { return Item(std::move(item)); }), valid_till, false, true);
        }

        static Response Disconnect(list_array<list_array<uint8_t>>&& data, size_t valid_till = 0) {
            return Response(data.take().convert<Item>([](auto&& item) { return Item(std::move(item)); }), valid_till, false, true);
        }

        bool isDisconnect() {
            return do_disconnect || do_disconnect_after_send;
        }

        Response& operator+=(const Response& other) {
            data.push_back(other.data);
            return *this;
        }

        Response& operator+=(Response&& other) {
            data.push_back(std::move(other.data));
            return *this;
        }

        void reserve(size_t size) {
            data.reserve_push_back(size);
        }

    private:
        Response(const list_array<Item>& response_bytes, size_t valid_till, bool disconnect = false, bool disconnect_after_send = false)
            : valid_till(valid_till) {
            data = response_bytes;
            do_disconnect = disconnect;
            do_disconnect_after_send = disconnect_after_send;
        }

        Response(list_array<Item>&& response_bytes, size_t valid_till, bool disconnect = false, bool disconnect_after_send = false)
            : valid_till(valid_till) {
            data = std::move(response_bytes);
            do_disconnect = disconnect;
            do_disconnect_after_send = disconnect_after_send;
        }
    };

    virtual Response WorkClient(list_array<uint8_t>&, uint64_t timeout) = 0;

    virtual Response OnSwitch() {
        return Response::Empty();
    }

    //will return nullptr if Redefine not needed
    virtual TCPclient* RedefineHandler() = 0;
    virtual bool DoDisconnect(baip::address) = 0;
    virtual TCPclient* DefineOurself(TCPsession* session) = 0;

    virtual ~TCPclient() {}

    static void logConsole(const list_array<uint8_t>& data) {
        static const char hex_chars[] = "0123456789ABCDEF";
        for (auto& it : data)
            std::cout << hex_chars[(it & 0xF0) >> 4] << hex_chars[it & 0x0F] << " ";
        std::cout << std::endl;
    }
};

extern TCPclient* first_client_holder;

struct TCPsession {
    static bool do_log_connection_errors;
    static std::atomic_uint64_t id_gen;
    AESencryption encryption;
    uint64_t id = id_gen++;
    baip::tcp::socket sock;
    int32_t protocol_version = -1;

    TCPsession(baip::tcp::socket&& s, TCPclient* client_handler, uint64_t& set_timeout, TCPserver* server = nullptr)
        : sock(std::move(s)), timeout(set_timeout), server(server) {
        chandler = client_handler->DefineOurself(this);
    }

    ~TCPsession() {
        if (chandler)
            delete chandler;
        if (_sharedData)
            delete _sharedData;
    }

    SharedClientData& sharedData() {
        if (!_sharedData)
            _sharedData = new SharedClientData;
        _sharedData->packets_state.protocol_version = protocol_version;
        return *_sharedData;
    }

    TCPserver& serverData() {
        return *server;
    }

    bool sharedDataExists() {
        return _sharedData;
    }

    void connect() {
        active = true;
        post(sock.get_executor(), [this] { req_loop(); return true; });
    }

    void connect(std::vector<uint8_t>& connection_data, boost::system::error_code ec) {
        active = true;
        data = connection_data;
        post(sock.get_executor(), [this, ec] { on_request(ec, data.size()); return true; });
    }

    void disconnect() {
        post(sock.get_executor(), [this] {
            sock.cancel();
            sock.close();
            active = false;
            __internal___::query_close_session(server, this);
        });
    }

    bool isActive() {
        return active;
    }

    bool start_symmetric_encryption(const list_array<uint8_t>& encryption_key, const list_array<uint8_t>& encryption_iv) {
        if (!encryption.initialize(encryption_key, encryption_iv))
            return false;
        encryption_enabled = true;
        return true;
    }

    boost::asio::const_buffer public_key() {
        return __internal___::get_public_key(server);
    }

    list_array<uint8_t> encrypt_data_(const list_array<uint8_t>& data) {
        list_array<uint8_t> out;
        encryption.encrypt(data, out);
        return out;
    }

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
            sock.async_wait(baip::tcp::socket::wait_read, [this](boost::system::error_code ec) {
                if (checked(ec, "on_wait")) {
                    if (size_t available = sock.available()) {
                        boost::system::error_code ec;
                        data.resize(available);
                        boost::asio::read(sock, boost::asio::buffer(data), ec);
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
            list_array<uint8_t> combined_packets;
            combined_packets.push_back(data.data(), data.size());
        prepare_packs:
            if (size_t available = sock.available()) {
                boost::system::error_code ec;
                data.resize(available);
                data.resize(boost::asio::read(sock, boost::asio::buffer(data), ec));
                combined_packets.push_back(data.data(), data.size());
                goto prepare_packs;
            }

            //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
            if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT) {
                std::cout << "P (" << id << ") ";
                TCPclient::logConsole(combined_packets);
            }
            //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        repeatDecode:
            if (encryption_enabled)
                encryption.decrypt(combined_packets, combined_packets);

            TCPclient::Response tmp = chandler->WorkClient(combined_packets, timeout);

            if (auto redefHandler = chandler->RedefineHandler(); redefHandler && !tmp.isDisconnect()) {
                combined_packets.remove(0, tmp.valid_till);
                tmp.valid_till = 0;
                delete chandler;
                chandler = redefHandler;
                tmp = chandler->OnSwitch();
                if(tmp.data.empty() || tmp.do_disconnect)
                    goto repeatDecode;
            }
            if (tmp.do_disconnect)
                disconnect();
            else if (tmp.data.size()) {
                list_array<uint8_t> tmp_data;
                for (auto& it : tmp.data)
                    tmp_data.push_back(std::move(it.data));


                if (encryption_enabled)
                    encryption.encrypt(tmp_data, tmp_data);


                data = std::move(std::vector<uint8_t>(tmp_data.begin(), tmp_data.end()));
                if (tmp.do_disconnect_after_send) {
                    boost::asio::async_write(sock, bai::buffer(data), [this](boost::system::error_code, size_t completed) {disconnect();return completed; });
                } else
                    boost::asio::async_write(sock, bai::buffer(data), [this](boost::system::error_code ec, size_t completed) { req_loop(ec);  return completed; });
            } else
                req_loop();

            //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
            if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
                for (auto& it : tmp.data) {
                    std::cout << "S (" << id << ") ";
                    TCPclient::logConsole(it.data);
                }
            //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        }
    }

private:
    std::vector<uint8_t> data;
    uint64_t& timeout;
    SharedClientData* _sharedData = nullptr;
    TCPclient* chandler = nullptr;
    std::atomic_bool active{false};
    TCPserver* server;
    bool encryption_enabled : 1 = false;

public:
    bool is_not_legacy : 1 = false;
    int32_t compression_threshold = -1;
};

class TCPserver {
    friend void __internal___::query_close_session(TCPserver* server, TCPsession* client);
    boost::asio::io_service* service;
    baip::tcp::acceptor TCPacceptor;
    boost::asio::thread_pool threads;

    std::mutex close_mutex;
    std::list<TCPsession*> queried_close;
    std::unordered_set<TCPsession*> sessions;
    RSA* server_rsa_key;
    list_array<uint8_t> server_private_key; //PEM
    list_array<uint8_t> server_public_key;  //PEM
    size_t ssl_key_length;

    void make_clean_up() {
        std::unique_lock<std::mutex> lock(close_mutex);
        std::list<TCPsession*> to_clean;
        to_clean.swap(queried_close);
        for (auto it : to_clean)
            sessions.erase(it);
        lock.unlock();
        for (auto it : to_clean)
            delete it;
    }

    void AsyncWork(TCPsession* session) {
        boost::system::error_code err;
        if (first_client_holder->DoDisconnect(session->sock.remote_endpoint().address()))
            session->sock.cancel();
        else {
            std::unique_lock<std::mutex> lock(close_mutex);
            if (sessions.insert(session).second)
                session->connect();
            else
                lock.unlock();
        }
    }

    void Worker() {
        make_clean_up();
        TCPsession* session = new TCPsession(baip::tcp::socket(make_strand(threads)), first_client_holder, all_connections_timeout, this);
        TCPacceptor.async_accept(session->sock, [this, session](const boost::system::error_code& error) {
            Worker();
            AsyncWork(session);
        });
    }

    std::shared_mutex data_lock;
    std::atomic_bool disabled = true;

    list_array<uint8_t> legacy_motd;
    std::string ip;
    bool local_server;

    auto resolveEndpoint(const std::string& ip, uint16_t port) {
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(ip, std::to_string(port));
        auto list = resolver.resolve(query);
        auto endpoint = list.begin()->endpoint();
        std::cout << "Server address: " << endpoint.address().to_string() << std::endl;

        //iterate all endpoints
        for (auto& endpoints : list) {
            auto address = endpoints.endpoint().address();
            if (address.is_loopback())
                local_server = true;
            std::cout << "Server address: " << address.to_string() << std::endl;
        }


        return endpoint;
    }

    mojang::api::session_server session_server;

public:
    ServerConfiguration server_config;

    boost::asio::io_service& getService() {
        return *service;
    }

    bool is_local_server() {
        return local_server;
    }

    mojang::api::session_server& getSessionServer() {
        return session_server;
    }

    std::string get_ip() const {
        return ip;
    }

    bool decrypt_data(list_array<uint8_t>& data) const {
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

    bool encrypt_data(list_array<uint8_t>& data) const {
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

    boost::asio::const_buffer private_key_buffer() {
        return boost::asio::const_buffer(server_private_key.data(), server_private_key.size());
    }

    boost::asio::const_buffer public_key_buffer() {
        return boost::asio::const_buffer(server_public_key.data(), server_public_key.size());
    }

    size_t key_length() {
        return ssl_key_length;
    }

    bool handle_legacy = false;
    uint16_t timeout_seconds = 30;
    uint16_t max_accept_buffer = 100;
    //30 sec
    uint64_t all_connections_timeout = 30000;

    const list_array<uint8_t>& legacyMotdResponse() {
        boost::shared_lock_guard lock(data_lock);
        return legacy_motd;
    }

    void setLegacyMotd(const std::u8string& motd) {
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

    TCPserver(boost::asio::io_service* io_service, const std::string& ip, uint16_t port, size_t threads = 0, size_t ssl_key_length = 1024)
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

    ~TCPserver() {
        if (!disabled)
            stop();
        if (server_rsa_key)
            RSA_free(server_rsa_key);
    }
    void start() {
        if (disabled) {
            Worker();
            disabled = false;
            service->run();
        } else
            throw std::exception("tcp server already run");
    }

    void stop() {
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
};

namespace __internal___ {
    inline void query_close_session(TCPserver* server, TCPsession* client) {
        std::unique_lock<std::mutex> lock(server->close_mutex);
        server->queried_close.push_back(client);
    }

    inline boost::asio::const_buffer get_private_key(TCPserver* server) {
        return server->private_key_buffer();
    }

    inline boost::asio::const_buffer get_public_key(TCPserver* server) {
        return server->public_key_buffer();
    }
}


#endif /* SRC_CLIENTHANDLEHELPER */
