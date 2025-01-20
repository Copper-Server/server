#ifndef SRC_BASE_OBJECTS_NETWORK_RESPONSE
#define SRC_BASE_OBJECTS_NETWORK_RESPONSE
#include <library/list_array.hpp>
#include <optional>

namespace enbt {
    class raw_uuid;
}

namespace copper_server::base_objects::network {
    struct response {
        struct item {
            list_array<uint8_t> data;
            int32_t compression_threshold;
            bool apply_compression;

            item();
            item(const list_array<uint8_t>& data, int32_t compression_threshold = -1, bool apply_compression = false);
            item(list_array<uint8_t>&& data, int32_t compression_threshold = -1, bool apply_compression = false);


            //same as write_value(uint8_t)
            void write_id(uint8_t id);

            void write_value(const enbt::raw_uuid& val);
            void write_value(int8_t flo);
            void write_value(int16_t flo);
            void write_value(int32_t flo);
            void write_value(int64_t flo);
            void write_value(uint8_t flo);
            void write_value(uint16_t flo);
            void write_value(uint32_t flo);
            void write_value(uint64_t flo);
            void write_value(float flo);
            void write_value(double flo);
            void write_value(char flo);
            void write_value(bool flo);
            void write_var32(int32_t value);
            void write_var64(int64_t value);
            void write_string(const std::string& str, int32_t max_string_len = INT32_MAX);
            void write_identifier(const std::string& str);
            void write_json_component(const std::string& str);
            void write_direct(const list_array<uint8_t>& data);
            void write_direct(list_array<uint8_t>&& data);
            void write_direct(const uint8_t* data, size_t size);

            template <class T>
            void write_var32_check(T value) {
                if constexpr (!std::is_same<T, int32_t>::value)
                    if ((int32_t)value != value)
                        throw std::out_of_range("Value out of range");
                write_var32(value);
            }

            template <class T>
            void write_var64_check(T value) {
                if constexpr (!std::is_same<T, int64_t>::value)
                    if ((int64_t)value != value)
                        throw std::out_of_range("Value out of range");
                write_var64(value);
            }

            template <class ArrayT>
            void write_array(const ArrayT& arr) {
                write_var32_check(arr.size());
                if constexpr (std::is_same_v<ArrayT, list_array<uint8_t>>)
                    data.push_back(arr);
                else
                    for (auto& it : arr)
                        write_value(it);
            }
        };

        response();
        response(const item& move);
        response(item&& move);
        response(response&& move);
        response(const response& copy);
        ~response();

        response& operator=(response&& move);

        list_array<item> data;
        bool do_disconnect = false;
        bool do_disconnect_after_send = false;
        size_t valid_till = 0;

        static response enable_compress_answer(const list_array<uint8_t>& data, int32_t compression_threshold, size_t valid_till = 0);
        static response answer(const list_array<item>& data, size_t valid_till = 0);
        static response answer(list_array<item>&& data, size_t valid_till = 0);
        static response empty();
        static response disconnect();
        static response disconnect(const list_array<item>& data, size_t valid_till = 0);
        static response disconnect(list_array<item>&& data, size_t valid_till = 0);

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
