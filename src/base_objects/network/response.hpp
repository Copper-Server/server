#ifndef SRC_BASE_OBJECTS_NETWORK_RESPONSE
#define SRC_BASE_OBJECTS_NETWORK_RESPONSE
#include <library/list_array.hpp>
#include <optional>

namespace copper_server::base_objects::network {
    struct response {
        struct item {
            list_array<uint8_t> data;
            int32_t compression_threshold;
            bool apply_compression;

            item();
            item(const list_array<uint8_t>& data, int32_t compression_threshold = -1, bool apply_compression = false);
            item(list_array<uint8_t>&& data, int32_t compression_threshold = -1, bool apply_compression = false);
        };

        response();
        response(response&& move);
        response(const response& copy);
        ~response();

        response& operator=(response&& move);

        list_array<item> data;
        bool do_disconnect = false;
        bool do_disconnect_after_send = false;
        size_t valid_till = 0;

        static response enable_compress_answer(const list_array<uint8_t>& data, int32_t compression_threshold, size_t valid_till = 0);
        static response answer(const list_array<list_array<uint8_t>>& data, size_t valid_till = 0);
        static response answer(list_array<list_array<uint8_t>>&& data, size_t valid_till = 0);
        static response empty();
        static response disconnect();
        static response disconnect(const list_array<list_array<uint8_t>>& data, size_t valid_till = 0);
        static response disconnect(list_array<list_array<uint8_t>>&& data, size_t valid_till = 0);

        bool is_disconnect() const;
        response& operator+=(const response& other);
        response& operator+=(response&& other);
        void reserve(size_t size);

    private:
        response(const list_array<item>& response_bytes, size_t valid_till, bool disconnect = false, bool disconnect_after_send = false);
        response(list_array<item>&& response_bytes, size_t valid_till, bool disconnect = false, bool disconnect_after_send = false);
    };

    using plugin_response = std::optional<response>;

}

#endif /* SRC_BASE_OBJECTS_NETWORK_RESPONSE */
