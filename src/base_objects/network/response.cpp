#include <src/base_objects/network/response.hpp>

namespace copper_server::base_objects::network {
    response::item::item()
        : apply_compression(false), compression_threshold(-1) {}

    response::item::item(const list_array<uint8_t>& data, int32_t compression_threshold, bool apply_compression)
        : data(data), compression_threshold(compression_threshold), apply_compression(apply_compression) {}

    response::item::item(list_array<uint8_t>&& data, int32_t compression_threshold, bool apply_compression)
        : data(std::move(data)), compression_threshold(compression_threshold), apply_compression(apply_compression) {}

    response::response() = default;

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

    response response::answer(const list_array<list_array<uint8_t>>& data, size_t valid_till) {
        return response(data.copy().convert<item>([](auto&& i) { return item(std::move(i)); }), valid_till);
    }

    response response::answer(list_array<list_array<uint8_t>>&& data, size_t valid_till) {
        return response(data.take().convert<item>([](auto&& i) { return item(std::move(i)); }), valid_till);
    }

    response response::empty() {
        return response({}, 0);
    }

    response response::disconnect() {
        return response({}, true);
    }

    response response::disconnect(const list_array<list_array<uint8_t>>& data, size_t valid_till) {
        return response(data.copy().convert<item>([](auto&& i) { return item(std::move(i)); }), valid_till, false, true);
    }

    response response::disconnect(list_array<list_array<uint8_t>>&& data, size_t valid_till) {
        return response(data.take().convert<item>([](auto&& i) { return item(std::move(i)); }), valid_till, false, true);
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