#include <exception>
#include <functional>
#include <library/enbt/enbt.hpp>
#include <src/api/configuration.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/registers.hpp>
#include <string>
#include <utf8.h>
#include <zlib.h>

namespace copper_server::build_in_plugins::network::tcp {

    struct keep_alive_solution::handle_t {
        std::function<void(int64_t, base_objects::SharedClientData&)> callback;
        fast_task::deadline_timer timeout_timer;
        fast_task::deadline_timer next_keep_alive;
        api::network::tcp::session* session;
        std::chrono::system_clock::time_point last_keep_alive;

        handle_t(api::network::tcp::session* session)
            : session(session), last_keep_alive(std::chrono::system_clock::time_point::min()) {
        }

        void _keep_alive_sended() {
            timeout_timer.expires_from_now(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(api::configuration::get().protocol.timeout_seconds)));

            last_keep_alive = std::chrono::system_clock::now();
            timeout_timer.async_wait([this](fast_task::deadline_timer::status status) {
                if (status == fast_task::deadline_timer::status::timeouted)
                    session->disconnect();
            });
        }

        std::chrono::system_clock::duration got_valid_keep_alive(int64_t check) {
            timeout_timer.cancel();
            if (check != last_keep_alive.time_since_epoch().count())
                throw std::runtime_error("got invalid keep alive packet");
            auto res = last_keep_alive - std::chrono::system_clock::now();

            next_keep_alive.expires_from_now(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(api::configuration::get().protocol.keep_alive_send_each_seconds)));
            next_keep_alive.async_wait([this](fast_task::deadline_timer::status status) {
                if (status == fast_task::deadline_timer::status::timeouted)
                    start();
            });
            return res;
        }

        void start() {
            if (callback) {
                _keep_alive_sended();
                callback(last_keep_alive.time_since_epoch().count(), session->shared_data());
            }
        }

        void make_keep_alive_packet() {
            if (timeout_timer.timed_out() && !next_keep_alive.timed_out()) {
                if (next_keep_alive.cancel()) {
                    _keep_alive_sended();
                    callback(last_keep_alive.time_since_epoch().count(), session->shared_data());
                }
            }
        }
    };

    keep_alive_solution::keep_alive_solution(api::network::tcp::session* session)
        : handle(std::make_shared<handle_t>(session)) {}

    keep_alive_solution::~keep_alive_solution() {
        handle->next_keep_alive.cancel();
        handle->timeout_timer.cancel();
    }

    void keep_alive_solution::set_callback(const std::function<void(int64_t, base_objects::SharedClientData&)>& fun) {
        handle->callback = fun;
    }

    std::chrono::system_clock::duration keep_alive_solution::got_valid_keep_alive(int64_t check) {
        return handle->got_valid_keep_alive(check);
    }

    void keep_alive_solution::make_keep_alive_packet() {
        handle->make_keep_alive_packet();
    }

    void keep_alive_solution::start() {
        handle->start();
    }

    list_array<uint8_t> tcp_client_handle::legacy_motd_helper(const std::u8string& motd) {
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
        if (legacy_motd.size() > INT16_MAX)
            throw std::invalid_argument("motd too long");
        uint16_t len = (uint16_t)legacy_motd.size();
        if constexpr (std::endian::native != std::endian::big)
            len = enbt::endian_helpers::convert_endian(std::endian::big, len);
        response.push_back(uint8_t(len >> 8));
        response.push_back(uint8_t(len & 0xFF));
        response.push_back(reinterpret_cast<uint8_t*>(legacy_motd.data()), legacy_motd.size() * 2);
        return response;
    }

    uint64_t tcp_client_handle::generate_random_int() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() ^ gen();
    }

    list_array<uint8_t> tcp_client_handle::prepare_incoming(ArrayStream& packet) {
        if (session->compression_threshold == -1) {
            return packet.to_vector();
        } else {
            int32_t uncompressed_packet_len = packet.read_var<int32_t>();

            list_array<uint8_t> compressed_packet;
            z_stream stream;
            stream.zalloc = Z_NULL;
            stream.zfree = Z_NULL;
            stream.opaque = Z_NULL;
            if ((decltype(stream.avail_in)(-1)) < packet.size_read())
                throw std::overflow_error("packet size is too large for zlib");
            stream.avail_in = (decltype(stream.avail_in))packet.size_read();
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

    list_array<uint8_t> tcp_client_handle::prepare_send(base_objects::network::response::item&& packet_item, api::network::tcp::session* session) {
        list_array<uint8_t>& packet = packet_item.data;
        list_array<uint8_t> build_packet;
        if (session->compression_threshold == -1) {
            WriteVar<int32_t>(packet.size(), build_packet);
            build_packet.push_back(std::move(packet));
        } else {
            if (packet.size() < session->compression_threshold) {
                build_packet.push_back(0);
                build_packet.push_back(std::move(packet));
            } else {
                list_array<uint8_t> compressed_packet = std::move(packet);
                z_stream stream;
                stream.zalloc = Z_NULL;
                stream.zfree = Z_NULL;
                stream.opaque = Z_NULL;
                if ((decltype(stream.avail_in)(-1)) < compressed_packet.size())
                    throw std::overflow_error("compressed packet size is too large for zlib");
                stream.avail_in = (decltype(stream.avail_in))compressed_packet.size();
                stream.next_in = compressed_packet.data();
                int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
                if (ret != Z_OK)
                    throw std::exception("deflateInit failed");
                auto to_decompress = compressed_packet.size() + compressed_packet.size() / 1000 + 12 + 1;
                if ((decltype(stream.avail_in)(-1)) < to_decompress)
                    throw std::overflow_error("packet size is too large for zlib");
                stream.avail_in = (decltype(stream.avail_in))to_decompress;
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

    list_array<list_array<uint8_t>> tcp_client_handle::prepare_send(base_objects::network::response&& packet, api::network::tcp::session* session) {
        list_array<list_array<uint8_t>> answer;
        if (packet.data.size()) {
            for (auto& resp : packet.data)
                answer.push_back(prepare_send(std::move(resp), session));
        }
        return answer;
    }

    base_objects::network::response tcp_client_handle::on_switching() {
        return base_objects::network::response::empty();
    }

    base_objects::network::response tcp_client_handle::work_packets(list_array<uint8_t>& combined) {
        assert(session);
        if (!session->is_not_legacy) {
            if (combined.size() < 2)
                return base_objects::network::response::empty();
            if (combined[0] == 0xFE && combined[1] == 0x01) {
                log::debug("protocol", "handle legacy status");
                auto& config = api::configuration::get();
                if (!config.status.enable)
                    return base_objects::network::response::disconnect();
                else
                    return base_objects::network::response::disconnect({legacy_motd_helper(std::u8string((char8_t*)config.status.description.data(), config.status.description.size()))});
            }
        }
        list_array<list_array<uint8_t>> answer;
        list_array<uint8_t> processed; //incoming packet
        ArrayStream data(combined.data(), combined.size());
        size_t valid_till = 0;

        while (!data.empty()) {
            int32_t packet_len = data.read_var<int32_t>();
            if (!data.can_read(packet_len)) {
                if (packet_len > api::configuration::get().protocol.max_accept_packet_size)
                    return too_large_packet();
                break;
            }

            ArrayStream packet = data.range_read(packet_len);
            base_objects::network::response answer_it = base_objects::network::response::empty();
            try {
                if (session->compression_threshold != -1) {
                    list_array<uint8_t> compressed_packet = prepare_incoming(packet);
                    ArrayStream compressed_data(compressed_packet.data(), compressed_packet.size());
                    answer_it = work_packet(compressed_data);
                } else
                    answer_it = work_packet(packet);

                answer.push_back(prepare_send(std::move(answer_it), session));
                if ((answer_it.do_disconnect || answer_it.do_disconnect_after_send) && answer.size())
                    break;

                if (answer_it.do_disconnect)
                    break;

                valid_till = data.r;
                if (next_handler)
                    break;
            } catch (const std::exception& ex) {
                answer_it = exception(ex);
                answer.push_back(prepare_send(std::move(answer_it), session));
            } catch (...) {
                answer_it = unexpected_exception();
                answer.push_back(prepare_send(std::move(answer_it), session));
            }
            if ((answer_it.do_disconnect || answer_it.do_disconnect_after_send) && answer.size())
                return base_objects::network::response::disconnect(std::move(answer));

            if (answer_it.do_disconnect)
                return base_objects::network::response::disconnect();
        }
        return base_objects::network::response::answer(std::move(answer), valid_till);
    }

    tcp_client_handle::tcp_client_handle(api::network::tcp::session* session)
        : session(session), _keep_alive_solution(session ? new keep_alive_solution(session) : nullptr) {
    }

    tcp_client_handle::~tcp_client_handle() {}

    base_objects::network::tcp::client* tcp_client_handle::redefine_handler() {
        return next_handler;
    }

    base_objects::network::response tcp_client_handle::work_client(list_array<uint8_t>& clientData) {
        return work_packets(clientData);
    }

    base_objects::network::response tcp_client_handle::on_switch() {
        base_objects::network::response res;
        try {
            res = on_switching();
        } catch (const std::exception& ex) {
            res = exception(ex);
        } catch (...) {
            res = unexpected_exception();
        }
        list_array<list_array<uint8_t>> answer;
        for (auto& resp : res.data)
            answer.push_back(prepare_send(std::move(resp), session));
        res.data = answer.take().convert<base_objects::network::response::item>([](list_array<uint8_t>&& item) { return base_objects::network::response::item(std::move(item)); });
        return res;
    }
}