#ifndef SRC_MOJANG_API_SESSION_SERVER
#define SRC_MOJANG_API_SESSION_SERVER
#include <chrono>
#include <library/enbt/enbt.hpp>
#include <random>
#include <string>

namespace mojang::api {
    class session_server {
    public:
        struct player_data {
            struct property {
                std::string name;
                std::string value;
                std::optional<std::string> signature;
            };

            std::string uuid_str;
            enbt::raw_uuid uuid;
            std::chrono::system_clock::time_point last_check;
            bool online_data;

            std::vector<property> properties;
        };

        std::unordered_map<std::string, std::shared_ptr<player_data>> cache;
        std::chrono::system_clock::duration cache_duration = std::chrono::minutes(20);

        const std::shared_ptr<player_data> hasJoined(const std::string& username, const std::string& serverId, bool online_mode, bool cache_result = true);
    };
}
#endif /* SRC_MOJANG_API_SESSION_SERVER */
