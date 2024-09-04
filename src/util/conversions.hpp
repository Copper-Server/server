#ifndef SRC_UTIL_CONVERSIONS
#define SRC_UTIL_CONVERSIONS
#include "../library/enbt.hpp"
#include <boost/json.hpp>
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

                std::vector<std::uint8_t> decode(std::string_view data);
            }

            namespace hex {
                std::string encode(const void* data, std::size_t size);

                std::string encode(const std::vector<std::uint8_t>& data);
                std::vector<std::uint8_t> decode(std::string_view data);
            }

            namespace uuid {
                std::string to(ENBT::UUID id);
                ENBT::UUID from(std::string_view id);
            }

            namespace string {
                std::string to_direct(std::string_view string);
                size_t direct_find(const std::string& string, char item);
                std::string to_transport(std::string_view string);
            }

            namespace json {
                boost::json::value to_json(const ENBT& enbt);
                ENBT from_json(const boost::json::value& json);
            }
        }
    }
}


#endif /* SRC_UTIL_CONVERSIONS */
