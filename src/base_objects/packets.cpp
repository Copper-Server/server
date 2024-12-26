#include <string>
#include <unordered_map>

namespace copper_server::base_objects::packets {
    int32_t java_name_to_protocol(const std::string& name_or_number) {
        static const std::unordered_map<std::string, int32_t> map{
            {"765", 765},
            {"766", 766},
            {"767", 767},
            {"768", 768},

            {"1.20.3", 765},
            {"1.20.4", 765},

            {"1.20.5", 766},
            {"1.20.6", 766},

            {"1.21", 767},
            {"1.21.1", 767},

            {"1.21.2", 768},
            {"1.21.3", 768},
        };
        return map.at(name_or_number);
    }
}