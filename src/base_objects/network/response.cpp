#include <library/enbt/enbt.hpp>
#include <src/base_objects/network/response.hpp>

namespace copper_server::base_objects::network {
    namespace util {
        template <class T>
        static T fromVar(uint8_t* ch, size_t& len) {
            constexpr int max_offset = (sizeof(T) / 5 * 5 + ((sizeof(T) % 5) > 0)) * 8;
            T decodedInt = 0;
            T bitOffset = 0;
            char currentByte = 0;
            size_t i = 0;
            do {
                if (i >= len)
                    throw std::overflow_error("VarInt is too big");
                if (bitOffset == max_offset)
                    throw std::overflow_error("VarInt is too big");
                currentByte = ch[i++];
                decodedInt |= (currentByte & 0b01111111) << bitOffset;
                bitOffset += 7;
            } while ((currentByte & 0b10000000) != 0);
            len = i;
            return decodedInt;
        }

        template <class T>
        static size_t toVar(uint8_t* buf, size_t buf_len, T val) {
            size_t i = 0;
            do {
                if (i >= buf_len)
                    throw std::overflow_error("VarInt is too big");
                buf[i] = (uint8_t)(val & 0b01111111);
                val >>= 7;
                if (val != 0)
                    buf[i] |= 0b10000000;
                i++;
            } while (val != 0);
            return i;
        }
    }

    response::item::item()
        : apply_compression(false), compression_threshold(-1) {}

    response::item::item(const list_array<uint8_t>& data, int32_t compression_threshold, bool apply_compression)
        : data(data), compression_threshold(compression_threshold), apply_compression(apply_compression) {}

    response::item::item(list_array<uint8_t>&& data, int32_t compression_threshold, bool apply_compression)
        : data(std::move(data)), compression_threshold(compression_threshold), apply_compression(apply_compression) {}

    template <class T>
    static void _write_value_tem(T val, list_array<uint8_t>& data) {
        if constexpr (sizeof(T) == 1) {
            data.push_back(reinterpret_cast<uint8_t*>(&val)[0]);
        } else {
            val = enbt::endian_helpers::convert_endian(std::endian::big, val);
            for (size_t i = 0; i < sizeof(T); i++)
                data.push_back(reinterpret_cast<uint8_t*>(&val)[i]);
        }
    }

    void response::item::write_id(uint8_t id) {
        _write_value_tem(id, data);
    }

    void response::item::write_value(const enbt::raw_uuid& val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(int8_t val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(int16_t val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(int32_t val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(int64_t val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(uint8_t val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(uint16_t val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(uint32_t val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(uint64_t val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(float val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(double val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(char val) {
        _write_value_tem(val, data);
    }

    void response::item::write_value(bool val) {
        _write_value_tem(val, data);
    }

    void response::item::write_var32(int32_t value) {
        constexpr size_t buf_len = sizeof(int32_t) + (sizeof(int32_t) / 7) + 1;
        uint8_t buf[buf_len];
        size_t len = util::toVar(buf, buf_len, enbt::endian_helpers::convert_endian(std::endian::little, value));
        for (size_t i = 0; i < len; i++)
            data.push_back(buf[i]);
    }

    void response::item::write_var64(int64_t value) {
        constexpr size_t buf_len = sizeof(int64_t) + (sizeof(int64_t) / 7) + 1;
        uint8_t buf[buf_len];
        size_t len = util::toVar(buf, buf_len, enbt::endian_helpers::convert_endian(std::endian::little, value));
        for (size_t i = 0; i < len; i++)
            data.push_back(buf[i]);
    }

    void response::item::write_string(const std::string& str, int32_t max_string_len) {
        size_t actual_len = str.size();
        if (actual_len != str.size())
            throw std::out_of_range("actual string len out of range");
        if (actual_len > max_string_len)
            throw std::out_of_range("actual string len out of range");
        write_var32_check(actual_len);
        data.push_back((uint8_t*)str.data(), str.size());
    }

    void response::item::write_identifier(const std::string& str) {
        write_string(str, 32767);
    }

    void response::item::write_json_component(const std::string& str) {
        write_string(str, 262144);
    }

    void response::item::write_direct(const list_array<uint8_t>& data) {
        this->data.push_back(data);
    }

    void response::item::write_direct(list_array<uint8_t>&& data) {
        this->data.push_back(std::move(data));
    }

    void response::item::write_direct(const uint8_t* data, size_t size) {
        this->data.push_back(data, size);
    }

    response::response() = default;

    response::response(const item& copy)
        : data({copy}), do_disconnect(false), do_disconnect_after_send(false), valid_till(0) {}

    response::response(item&& move)
        : data({std::move(move)}), do_disconnect(false), do_disconnect_after_send(false), valid_till(0) {}

    response::response(response&& move)
        : data(std::move(move.data)), do_disconnect(move.do_disconnect), do_disconnect_after_send(move.do_disconnect_after_send), valid_till(move.valid_till) {}

    response::response(const response& copy)
        : data(copy.data), do_disconnect(copy.do_disconnect), do_disconnect_after_send(copy.do_disconnect_after_send), valid_till(copy.valid_till) {}

    response::~response()
        = default;

    response& response::operator=(response&& move) {
        data = std::move(move.data);
        do_disconnect = move.do_disconnect;
        do_disconnect_after_send = move.do_disconnect_after_send;
        valid_till = move.valid_till;
        return *this;
    }

    response response::enable_compress_answer(const list_array<uint8_t>& data, int32_t compression_threshold, size_t valid_till) {
        return response({item(data, compression_threshold, true)}, valid_till);
    }

    response response::answer(const list_array<item>& data, size_t valid_till) {
        return response(data.copy(), valid_till);
    }

    response response::answer(list_array<item>&& data, size_t valid_till) {
        return response(data.take(), valid_till);
    }

    response response::empty() {
        return response({}, 0);
    }

    response response::disconnect() {
        return response({}, 0, true);
    }

    response response::disconnect(const list_array<item>& data, size_t valid_till) {
        return response(data.copy(), valid_till, false, true);
    }

    response response::disconnect(list_array<item>&& data, size_t valid_till) {
        return response(data.take(), valid_till, false, true);
    }

    bool response::is_disconnect() const {
        return do_disconnect || do_disconnect_after_send;
    }

    response& response::operator+=(const response& other) {
        if (do_disconnect_after_send || do_disconnect)
            return *this;
        else if (other.do_disconnect) {
            data.clear();
            do_disconnect = true;
        } else {
            data.push_back(other.data);
            do_disconnect_after_send |= other.do_disconnect_after_send;
        }
        return *this;
    }

    response& response::operator+=(response&& other) {
        if (do_disconnect_after_send || do_disconnect)
            return *this;
        else if (other.do_disconnect) {
            data.clear();
            do_disconnect = true;
        } else {
            data.push_back(std::move(other.data));
            do_disconnect_after_send |= other.do_disconnect_after_send;
        }
        return *this;
    }

    void response::reserve(size_t size) {
        data.reserve(size);
    }

    bool response::has_data() const {
        return !data.empty() || do_disconnect || do_disconnect_after_send;
    }

    response::response(const list_array<item>& response_bytes, size_t valid_till, bool disconnect, bool disconnect_after_send)
        : valid_till(valid_till) {
        data = response_bytes;
        do_disconnect = disconnect;
        do_disconnect_after_send = disconnect_after_send;
    }

    response::response(list_array<item>&& response_bytes, size_t valid_till, bool disconnect, bool disconnect_after_send)
        : valid_till(valid_till) {
        data = std::move(response_bytes);
        do_disconnect = disconnect;
        do_disconnect_after_send = disconnect_after_send;
    }
}