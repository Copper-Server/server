#if defined(_MSC_VER)
    #include <SDKDDKVer.h>
#endif
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <exception>
#include <functional>
#include <library/enbt/enbt.hpp>
#include <src/api/configuration.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/util.hpp>
#include <src/registers.hpp>
#include <string>
#include <utf8.h>
#include <zlib.h>

namespace copper_server {
    void keep_alive_solution::_keep_alive_sended() {
        need_to_send = false;
        got_keep_alive = false;

        timeout_timer.expires_from_now(boost::posix_time::seconds(api::configuration::get().server.timeout_seconds));
        last_keep_alive = std::chrono::system_clock::now();
        timeout_timer.async_wait([this](const boost::system::error_code& ec) {
            if (ec == boost::asio::error::operation_aborted)
                return;
            session->disconnect();
        });
    }

    void keep_alive_solution::_keep_alive_request() {
        if (!callback) {
            send_keep_alive_requested = false;
            return;
        }
        auto seconds = std::min<uint16_t>(api::configuration::get().server.timeout_seconds, 2);
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

    keep_alive_solution::keep_alive_solution(base_objects::network::tcp::session* session)
        : timeout_timer(session->sock.get_executor()), send_keep_alive_timer(session->sock.get_executor()), session(session) {
    }

    keep_alive_solution::~keep_alive_solution() {
        timeout_timer.cancel();
        send_keep_alive_timer.cancel();
    }

    void keep_alive_solution::set_callback(const std::function<base_objects::network::response(int64_t)>& fun) {
        callback = fun;
        _keep_alive_request();
        need_to_send = true;
    }

    void keep_alive_solution::keep_alive_sended() {
        if (!callback)
            return;
        _keep_alive_sended();
        send_keep_alive_timer.cancel();
    }

    base_objects::network::response keep_alive_solution::send_keep_alive() {
        if (!callback || !need_to_send)
            return {};
        _keep_alive_sended();
        return callback(last_keep_alive.time_since_epoch().count());
    }

    std::chrono::system_clock::duration keep_alive_solution::got_valid_keep_alive(int64_t check) {
        got_keep_alive = true;
        timeout_timer.cancel();
        if (check != last_keep_alive.time_since_epoch().count())
            throw std::runtime_error("got invalid keep alive packet");
        return last_keep_alive - std::chrono::system_clock::now();
    }

    void keep_alive_solution::ignore_keep_alive() {
        got_keep_alive = true;
        timeout_timer.cancel();
    }

    base_objects::network::response keep_alive_solution::no_response() {
        if (got_keep_alive && callback) {
            _keep_alive_sended();
            return callback(last_keep_alive.time_since_epoch().count());
        } else
            return {};
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
        uint16_t len = legacy_motd.size();
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

    list_array<uint8_t> tcp_client_handle::prepare_send(base_objects::network::response::item&& packet_item) {
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

    list_array<list_array<uint8_t>> tcp_client_handle::prepare_send(auto&& packet) {
        list_array<list_array<uint8_t>> answer;
        if (packet.data.size()) {
            for (auto& resp : packet.data)
                answer.push_back(prepare_send(std::move(resp)));
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
                if (packet_len > max_packet_size)
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

                answer.push_back(prepare_send(answer_it));
                if ((answer_it.do_disconnect || answer_it.do_disconnect_after_send) && answer.size())
                    break;

                if (answer_it.do_disconnect)
                    break;

                valid_till = data.r;
                if (next_handler)
                    break;
            } catch (const std::exception& ex) {
                answer_it = exception(ex);
                answer.push_back(prepare_send(answer_it));
            } catch (...) {
                answer_it = unexpected_exception();
                answer.push_back(prepare_send(answer_it));
            }
            if ((answer_it.do_disconnect || answer_it.do_disconnect_after_send) && answer.size())
                return base_objects::network::response::disconnect(std::move(answer));

            if (answer_it.do_disconnect)
                return base_objects::network::response::disconnect();
        }
        if (answer.empty() && next_handler == nullptr)
            if (_keep_alive_solution)
                answer.push_back(prepare_send(_keep_alive_solution->no_response()));
        return base_objects::network::response::answer(std::move(answer), valid_till);
    }

    tcp_client_handle::tcp_client_handle(base_objects::network::tcp::session* session)
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
            answer.push_back(prepare_send(std::move(resp)));
        res.data = answer.take().convert<base_objects::network::response::item>([](list_array<uint8_t>&& item) { return base_objects::network::response::item(std::move(item)); });
        return res;
    }
}