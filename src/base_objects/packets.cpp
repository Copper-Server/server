#include <string>
#include <unordered_map>

namespace copper_server::base_objects::packets {
    int32_t java_name_to_protocol(const std::string& name_or_number) {
        static const std::unordered_map<std::string, int32_t> map{
            {"1.21.5", 770},
            {"1.21.4", 769},
            {"1.21.3", 768},
            {"1.21.2", 768},
            {"1.21.1", 767},
            {"1.21", 767}
        };
        return map.at(name_or_number);
    }

    const char* protocol_to_java_name(int32_t id) {
        static const std::unordered_map<int32_t, const char*> map{
            {770, "1.21.5"},
            {769, "1.21.4"},
            {768, "1.21.3"},
            {768, "1.21.2"},
            {767, "1.21.1"},
            {767, "1.21"}
        };
        return map.at(id);
    }
}