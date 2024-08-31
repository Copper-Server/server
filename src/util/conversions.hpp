#ifndef SRC_UTIL_CONVERSIONS
#define SRC_UTIL_CONVERSIONS
#include "../library/enbt.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace crafted_craft {
    namespace util {
        namespace conversions {
            namespace base64 {
                std::string encode(const void* data, std::size_t size);

                std::string encode(const std::vector<std::uint8_t>& data);

                std::vector<std::uint8_t> decode(const std::string& data);
            }

            namespace hex {
                std::string encode(const void* data, std::size_t size);

                std::string encode(const std::vector<std::uint8_t>& data);
                std::vector<std::uint8_t> decode(const std::string& data);
            }

            namespace uuid {
                std::string to(ENBT::UUID id);
                ENBT::UUID from(std::string_view id);
            }

            namespace string {
                std::string to_direct(const std::string& string);
                size_t direct_find(const std::string& string, char item);
                std::string to_transport(const std::string& string);
            }
        }
    }
}


#endif /* SRC_UTIL_CONVERSIONS */
