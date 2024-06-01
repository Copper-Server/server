#ifndef SRC_PROTOCOLHELPER_UTIL
#define SRC_PROTOCOLHELPER_UTIL
#include "../ClientHandleHelper.hpp"
#include "../base_objects/packets.hpp"
#include "../base_objects/ptr_optional.hpp"
#include "../base_objects/response.hpp"
#include "../library/enbt.hpp"
#include "../plugin/main.hpp"
#include "../registers.hpp"
#include "../log.hpp"
#include <array>
#include <exception>
#include <string>
#include <vector>
#include <zlib.h>

namespace crafted_craft {
    struct ArrayStream {
        uint8_t* arrau;
        size_t mi;

        ArrayStream(uint8_t* arr, size_t size) {
            arrau = arr;
            mi = size;
            read_only = false;
        }

        ArrayStream(const uint8_t* arr, size_t size) {
            arrau = const_cast<uint8_t*>(arr);
            mi = size;
            read_only = true;
        }

        size_t r = 0;
        size_t w = 0;

        void write(uint8_t value) {
            if (mi <= w)
                throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try write in: " + std::to_string(w));
            if (read_only)
                throw std::exception("Readonly Mode");
            arrau[w++] = value;
        }

        uint8_t read() {
            if (mi <= r)
                throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try read in: " + std::to_string(r));
            return arrau[r++];
        }

        uint8_t peek() {
            if (mi <= r)
                throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try read in: " + std::to_string(r));
            return arrau[r];
        }

        bool empty() {
            return r >= mi;
        }

        ArrayStream range(size_t start, size_t end) {
            if (start > end)
                throw std::out_of_range("start > end");
            if (end > mi)
                throw std::out_of_range("end > max_index");
            return ArrayStream(arrau + start, end - start);
        }

        ArrayStream range_read(size_t size) {
            if (r + size > mi)
                throw std::out_of_range("r + size > max_index");

            ArrayStream tmp(arrau + r, size);
            r += size;
            return tmp;
        }

        ArrayStream read_left() {
            size_t left = mi - r;
            ArrayStream tmp(arrau + r, left);
            r = mi;
            return tmp;
        }

        size_t size_read() {
            return mi - r;
        }

        uint8_t* data_read() {
            return arrau + r;
        }

        size_t size_write() {
            return mi - w;
        }

        uint8_t* data_write() {
            return arrau + w;
        }

        bool can_read(size_t size) {
            return r + size <= mi;
        }

        list_array<uint8_t> to_vector() {
            return list_array<uint8_t>(arrau + r, arrau + mi + r);
        }


    private:
        bool read_only;
    };

    template <class Res>
    static Res ReadVar(ArrayStream& data) {
        size_t len = sizeof(Res);
        Res res = ENBT::fromVar<Res>(data.arrau + data.r, len);
        data.r += len;
        ENBT::ConvertEndian(std::endian::little, res);
        return res;
    }

    template <class ResultT, class T>
    static void WriteVar(T val, list_array<uint8_t>& data) {
        if (!std::is_same<ResultT, T>::value)
            if ((ResultT)val != val)
                throw std::out_of_range("Value out of range");

        constexpr size_t buf_len = sizeof(ResultT) + (sizeof(ResultT) / 7) + 1;
        uint8_t buf[buf_len];
        size_t len = ENBT::toVar(buf, buf_len, ENBT::ConvertEndian<ResultT>(std::endian::little, (ResultT)val));
        for (size_t i = 0; i < len; i++)
            data.push_back(buf[i]);
    }

    template <class ResultT, class T>
    static void WriteVar(T val, ArrayStream& data) {
        if (!std::is_same<ResultT, T>::value)
            if ((ResultT)val != val)
                throw std::out_of_range("Value out of range");

        constexpr size_t buf_len = sizeof(T) + (sizeof(T) / 7) + 1;
        uint8_t buf[buf_len];
        size_t len = ENBT::toVar(buf, buf_len, ENBT::ConvertEndian<ResultT>(std::endian::little, (ResultT)val));
        for (size_t i = 0; i < len; i++)
            data.write(buf[i]);
    }

    static void WriteUUID(const ENBT::UUID& val, list_array<uint8_t>& data) {
        ENBT::UUID temp = ENBT::ConvertEndian(std::endian::big, val);
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < 16; i++)
            data.push_back(tmp[i]);
    }

    static ENBT::UUID ReadUUID(ArrayStream& data) {
        ENBT::UUID temp;
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < 16; i++)
            tmp[i] = data.read();
        return ENBT::ConvertEndian(std::endian::big, temp);
    }

    template <class T>
    static void WriteValue(const T& val, list_array<uint8_t>& data) {
        T temp = ENBT::ConvertEndian(std::endian::big, val);
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < sizeof(T); i++)
            data.push_back(tmp[i]);
    }

    template <class T>
    static T ReadValue(ArrayStream& data) {
        uint8_t tmp[sizeof(T)];
        for (size_t i = 0; i < sizeof(T); i++)
            tmp[i] = data.read();
        return ENBT::ConvertEndian(std::endian::big, *(T*)tmp);
    }

    static std::string ReadString(ArrayStream& data, int32_t max_string_len) {
        std::string res = "";
        int32_t actual_len = ReadVar<int32_t>(data);
        if (actual_len > max_string_len)
            throw std::out_of_range("actual string len out of range");
        if (actual_len < 0)
            throw std::out_of_range("actual string len out of range");
        for (int32_t i = 0; i < actual_len;) {
            char tmp = (char)data.read();
            i += (tmp & 0xc0) != 0x80;
            res += tmp;
        }
        return res;
    }

    static std::string ReadIdentifier(ArrayStream& data) {
        return ReadString(data, 32767);
    }

    static void WriteString(list_array<uint8_t>& data, const std::string& str, int32_t max_string_len = INT32_MAX) {
        int32_t actual_len = str.size();
        if (actual_len != str.size())
            throw std::out_of_range("actual string len out of range");
        if (actual_len > max_string_len)
            throw std::out_of_range("actual string len out of range");
        WriteVar<int32_t>(actual_len, data);
        data.push_back((uint8_t*)str.data(), str.size());
    }

    static void WriteIdentifier(list_array<uint8_t>& data, const std::string& str) {
        WriteString(data, str, 32767);
    }

    static void WriteJSONComponent(list_array<uint8_t>& data, const std::string& str) {
        WriteString(data, str, 262144);
    }

    static void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot) {
        data.push_back((bool)slot);
        if (slot) {
            WriteVar<int32_t>(slot->id, data);
            data.push_back(slot->count);
            if (slot->nbt)
                data.push_back(NBT::build(slot->nbt.value()).get_as_network());
            else
                data.push_back(0); //TAG_End
        }
    }

    static base_objects::slot ReadSlot(ArrayStream& data) {
        base_objects::slot slot;
        if (!data.read())
            return slot;
        slot = base_objects::slot();
        slot->id = ReadVar<int32_t>(data);
        slot->count = data.read();
        if (data.read() == 0) {
            size_t readed = 0;
            slot->nbt = NBT::extract_from_array(data.data_read(), readed, data.size_read());
            data.r += readed;
        }
        return slot;
    }

    static std::string UUID2String(const ENBT::UUID& uuid) {
        char buf[36];
        size_t index = 0;
        for (size_t i = 0; i < 16; i++) {
            if (i == 4 || i == 6 || i == 8 || i == 10)
                buf[index++] = '-';
            uint8_t tmp = uuid.data[i];
            buf[index++] = "0123456789abcdef"[tmp >> 4];
            buf[index++] = "0123456789abcdef"[tmp & 0x0F];
        }
        return std::string(buf, 36);
    }

    static list_array<uint8_t> ReadListArray(ArrayStream& data) {
        int32_t len = ReadVar<int32_t>(data);
        if (len < 0)
            throw std::out_of_range("list array len out of range");
        list_array<uint8_t> res;
        res.reserve_push_back(len);
        for (int32_t i = 0; i < len; i++)
            res.push_back(data.read());
        return res;
    }

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
            timeout_timer.expires_from_now(boost::posix_time::seconds(session->serverData().timeout_seconds));
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
            auto seconds = std::min<uint16_t>(session->serverData().timeout_seconds, 2);
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
    protected:
        static uint64_t generate_random_int() {
            static std::random_device rd;
            static std::mt19937_64 gen;
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() ^ gen();
        }
        static std::unordered_set<boost::asio::ip::address> banned_clients;
        static size_t max_packet_size;
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
                compressed_size.reserve_push_back(5);
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
                    auto& server = session->serverData();
                    if (!server.handle_legacy)
                        return Response::Disconnect(list_array<list_array<uint8_t>>());
                    else
                        return Response::Disconnect({server.legacyMotdResponse()});
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
            auto res = OnSwitching();
            if (!res.data.empty()) {
                list_array<list_array<uint8_t>> answer;
                for (auto& resp : res.data)
                    answer.push_back(PrepareSend(std::move(resp)));
                res.data = std::move(answer);
                return res;
            } else if (res.do_disconnect || res.do_disconnect_after_send)
                return res;
            else
                return {};
        }

        bool DoDisconnect(boost::asio::ip::address ip) override {
            return banned_clients.contains(ip);
        }
    };
}

#endif /* SRC_PROTOCOLHELPER_UTIL */
