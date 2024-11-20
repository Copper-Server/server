#ifndef SRC_UTIL_CONVERSIONS
#define SRC_UTIL_CONVERSIONS
#include <boost/json.hpp>
#include <cstddef>
#include <cstdint>
#include <library/enbt/enbt.hpp>
#include <string>
#include <vector>

namespace copper_server {
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
                std::string to(enbt::raw_uuid id);
                enbt::raw_uuid from(std::string_view id);
            }

            namespace string {
                std::string to_direct(std::string_view string);
                size_t direct_find(const std::string& string, char item);
                std::string to_transport(std::string_view string);
            }

            namespace json {
                boost::json::value to_json(const enbt::value& enbt);
                enbt::value from_json(const boost::json::value& json);
                enbt::value from_json(const boost::json::object& json);
                enbt::value from_json(const boost::json::array& json);
            }
        }
    }
}


#endif /* SRC_UTIL_CONVERSIONS */
