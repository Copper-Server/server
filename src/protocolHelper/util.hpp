#ifndef SRC_PROTOCOLHELPER_UTIL
#define SRC_PROTOCOLHELPER_UTIL
#include <exception>
#include <library/enbt.hpp>
#include <src/ClientHandleHelper.hpp>
#include <src/api/configuration.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/ptr_optional.hpp>
#include <src/base_objects/response.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/registers.hpp>
#include <src/util/readers.hpp>
#include <string>
#include <zlib.h>

namespace copper_server {
    class KeepAliveSolution {
        std::function<Response()> callback;
        boost::asio::deadline_timer timeout_timer;
        boost::asio::deadline_timer send_keep_alive_timer;
        bool send_keep_alive_requested = false;
        bool need_to_send = false;
        bool got_keep_alive = true;
        TCPsession* session;

        void _keep_alive_sended() {
            need_to_send = false;
            got_keep_alive = false;

            timeout_timer.expires_from_now(boost::posix_time::seconds(Server::instance().timeout_seconds));
            last_keep_alive = std::chrono::system_clock::now();
            timeout_timer.async_wait([this](const boost::system::error_code& ec) {
                if (ec == boost::asio::error::operation_aborted)
                    return;
                session->disconnect();
            });
        }

        void _keep_alive_request() {
            if (!callback) {
                send_keep_alive_requested = false;
                return;
            }
            auto seconds = std::min<uint16_t>(Server::instance().timeout_seconds, 2);
            send_keep_alive_requested = true;
            send_keep_alive_timer.cancel();
            send_keep_alive_timer.expires_from_now(boost::posix_time::seconds(seconds));
            send_keep_alive_timer.async_wait([this](const boost::system::error_code& ec) {
                if (ec == boost::asio::error::operation_aborted)
                    return;
                need_to_send = true;
                _keep_alive_request();
            });
        }

        std::chrono::time_point<std::chrono::system_clock> last_keep_alive;

    public:
        KeepAliveSolution(TCPsession* session)
            : timeout_timer(session->sock.get_executor()), send_keep_alive_timer(session->sock.get_executor()), session(session) {
        }

        ~KeepAliveSolution() {
            timeout_timer.cancel();
            send_keep_alive_timer.cancel();
        }

        void set_callback(const std::function<Response()>& fun) {
            callback = fun;
            _keep_alive_request();
            need_to_send = true;
        }

        void keep_alive_sended() {
            if (!callback)
                return;
            _keep_alive_sended();
            send_keep_alive_timer.cancel();
        }

        Response send_keep_alive() {
            if (!callback || !need_to_send)
                return {};
            _keep_alive_sended();
            return callback();
        }

        //returns elapsed time from last keep_alive
        std::chrono::system_clock::duration got_valid_keep_alive() {
            got_keep_alive = true;
            timeout_timer.cancel();
            return last_keep_alive - std::chrono::system_clock::now();
        }

        Response no_response() {
            if (got_keep_alive && callback) {
                _keep_alive_sended();
                return callback();
            } else
                return {};
        }
    };

    class TCPClientHandle : public TCPclient {

        list_array<uint8_t> LegacyMotdHelper(const std::u8string& motd) {
            if (motd.size() + 20 > 0xFFFF)
                throw std::invalid_argument("motd too long");
            list_array<char16_t> legacy_motd;
            legacy_motd.reserve(motd.size() + 20);
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
                    it = enbt::endian_helpers::convert_endian(std::endian::big, it);
            list_array<uint8_t> response;
            response.push_back(0xFF); //KICK packet
            uint16_t len = legacy_motd.size();
            if constexpr (std::endian::native != std::endian::big)
                len = enbt::endian_helpers::convert_endian(std::endian::big, len);
            response.push_back(uint8_t(len >> 8));
            response.push_back(uint8_t(len & 0xFF));
            response.push_back(reinterpret_cast<uint8_t*>(legacy_motd.data()), legacy_motd.size() * 2);
            return response;
        }

    protected:
        static uint64_t generate_random_int() {
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() ^ gen();
        }
        static inline std::unordered_set<boost::asio::ip::address> banned_clients;
        static inline size_t max_packet_size = 4096;
        TCPclient* next_handler = nullptr;
        TCPsession* session;
        base_objects::ptr_optional<KeepAliveSolution> keep_alive_solution;

        list_array<uint8_t> PrepareIncoming(ArrayStream& packet) {
            if (session->compression_threshold == -1) {
                return packet.to_vector();
            } else {
                int32_t uncompressed_packet_len = ReadVar<int32_t>(packet);

                list_array<uint8_t> compressed_packet;
                z_stream stream;
                stream.zalloc = Z_NULL;
                stream.zfree = Z_NULL;
                stream.opaque = Z_NULL;
                stream.avail_in = packet.size_read();
                stream.next_in = packet.data_read();
                int ret = inflateInit(&stream);
                if (ret != Z_OK)
                    throw std::exception("inflateInit failed");
                stream.avail_out = uncompressed_packet_len;
                stream.next_out = (uint8_t*)malloc(stream.avail_out);
                ret = inflate(&stream, Z_FINISH);
                if (ret != Z_STREAM_END)
                    throw std::exception("inflate failed");
                WriteVar<int32_t>(stream.total_out, compressed_packet);
                compressed_packet.push_back(stream.next_out, stream.total_out);
                free(stream.next_out);
                stream.next_out = nullptr;
                ret = inflateEnd(&stream);
                if (ret != Z_OK)
                    throw std::exception("inflateEnd failed");
                return compressed_packet;
            }
        }

        list_array<uint8_t> PrepareSend(Response::Item&& packet_item) {
            list_array<uint8_t>& packet = packet_item.data;
            list_array<uint8_t> build_packet;
            if (session->compression_threshold == -1) {
                WriteVar<int32_t>(packet.size(), build_packet);
                build_packet.push_back(std::move(packet));
            } else {
                if (packet.size() > session->compression_threshold) {
                    WriteVar<int32_t>(packet.size(), build_packet);
                    build_packet.push_back(std::move(packet));
                } else {
                    list_array<uint8_t> compressed_packet = std::move(packet);
                    z_stream stream;
                    stream.zalloc = Z_NULL;
                    stream.zfree = Z_NULL;
                    stream.opaque = Z_NULL;
                    stream.avail_in = compressed_packet.size();
                    stream.next_in = compressed_packet.data();
                    int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
                    if (ret != Z_OK)
                        throw std::exception("deflateInit failed");
                    stream.avail_out = compressed_packet.size() + compressed_packet.size() / 1000 + 12 + 1;
                    stream.next_out = (uint8_t*)malloc(stream.avail_out);
                    ret = deflate(&stream, Z_FINISH);
                    if (ret != Z_STREAM_END)
                        throw std::exception("deflate failed");
                    WriteVar<int32_t>(compressed_packet.size(), build_packet);
                    build_packet.push_back(stream.next_out, stream.total_out);
                    free(stream.next_out);
                    stream.next_out = nullptr;
                    ret = deflateEnd(&stream);
                    if (ret != Z_OK)
                        throw std::exception("deflateEnd failed");
                }
                list_array<uint8_t> compressed_size;
                compressed_size.reserve(5);
                WriteVar<int32_t>(build_packet.size(), compressed_size);
                build_packet.push_front(compressed_size);
            }

            if (packet_item.apply_compression)
                session->compression_threshold = packet_item.compression_threshold;
            return build_packet;
        }

        list_array<list_array<uint8_t>> PrepareSend(auto&& packet) {
            list_array<list_array<uint8_t>> answer;
            if (packet.data.size()) {
                for (auto& resp : packet.data)
                    answer.push_back(PrepareSend(std::move(resp)));
            }
            return answer;
        }

        virtual Response WorkPacket(ArrayStream& packet) = 0;
        virtual Response TooLargePacket() = 0;
        virtual Response Exception(const std::exception& ex) = 0;
        virtual Response UnexpectedException() = 0;

        virtual Response OnSwitching() {
            return Response::Empty();
        }

        Response WorkPackets(list_array<uint8_t>& combined) {
            assert(session);
            if (!session->is_not_legacy) {
                if (combined.size() < 2)
                    return Response::Empty();
                if (combined[0] == 0xFE && combined[1] == 0x01) {
                    log::debug("protocol", "handle legacy status");
                    auto& config = api::configuration::get();
                    if (!config.status.enable)
                        return Response::Disconnect();
                    else
                        return Response::Disconnect({LegacyMotdHelper(std::u8string((char8_t*)config.status.description.data(), config.status.description.size()))});
                }
            }
            list_array<list_array<uint8_t>> answer;
            list_array<uint8_t> processed; //incoming packet
            ArrayStream data(combined.data(), combined.size());
            size_t valid_till = 0;

            while (!data.empty()) {
                int32_t packet_len = ReadVar<int32_t>(data);
                if (!data.can_read(packet_len)) {
                    if (packet_len > max_packet_size)
                        return TooLargePacket();
                    break;
                }

                ArrayStream packet = data.range_read(packet_len);
                try {
                    Response answer_it = Response::Empty();
                    if (session->compression_threshold != -1) {
                        list_array<uint8_t> compressed_packet = PrepareIncoming(packet);
                        ArrayStream compressed_data(compressed_packet.data(), compressed_packet.size());
                        answer_it = WorkPacket(compressed_data);
                    } else
                        answer_it = WorkPacket(packet);

                    answer.push_back(PrepareSend(answer_it));
                    if (answer_it.do_disconnect)
                        return Response::Disconnect(std::move(answer));

                    if (answer_it.do_disconnect_after_send)
                        return Response::Disconnect(std::move(answer));
                    valid_till = data.r;
                    if (next_handler)
                        break;
                } catch (const std::exception& ex) {
                    return Exception(ex);
                } catch (...) {
                    return UnexpectedException();
                }
            }
            if (answer.empty() && next_handler == nullptr)
                if (keep_alive_solution)
                    answer.push_back(PrepareSend(keep_alive_solution->no_response()));
            return Response::Answer(std::move(answer), valid_till);
        }


    public:
        TCPClientHandle(TCPsession* session)
            : session(session), keep_alive_solution(session ? new KeepAliveSolution(session) : nullptr) {
        }

        ~TCPClientHandle() override {}

        TCPclient* RedefineHandler() override {
            return next_handler;
        }

        Response WorkClient(list_array<uint8_t>& clientData) final {
            return WorkPackets(clientData);
        }

        Response OnSwitch() final {
            try {
                auto res = OnSwitching();
                if (!res.data.empty()) {
                    list_array<list_array<uint8_t>> answer;
                    for (auto& resp : res.data)
                        answer.push_back(PrepareSend(std::move(resp)));
                    res.data = answer.take().convert<Response::Item>([](list_array<uint8_t>&& item) { return Response::Item(std::move(item)); });
                    return res;
                } else if (res.do_disconnect || res.do_disconnect_after_send)
                    return res;
                else
                    return {};
            } catch (const std::exception& ex) {
                return Exception(ex);
            } catch (...) {
                return UnexpectedException();
            }
        }

        bool DoDisconnect(boost::asio::ip::address ip) override {
            return banned_clients.contains(ip);
        }
    };
}

#endif /* SRC_PROTOCOLHELPER_UTIL */
