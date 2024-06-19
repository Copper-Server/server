#ifndef SRC_BASE_OBJECTS_RESPONSE
#define SRC_BASE_OBJECTS_RESPONSE
#include "../library/list_array.hpp"

namespace crafted_craft {
    namespace base_objects {

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

            Response()
                : Response({}, 0) {}

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

            Response& operator+=(Response&& other) {
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

            void reserve(size_t size) {
                data.reserve(size);
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

    } // namespace base_objects
}
using Response = crafted_craft::base_objects::Response;


#endif /* SRC_BASE_OBJECTS_RESPOSNE */
