#ifndef SRC_MOJANG_API_SESSION_SERVER
#define SRC_MOJANG_API_SESSION_SERVER
#include <chrono>
#include <library/enbt.hpp>
#include <random>
#include <string>

namespace mojang {
    namespace api {
        class session_server {
            static enbt::raw_uuid generateOfflineUUID() {
                static std::random_device rd;
                static std::mt19937_64 gen(rd());
                static std::uniform_int_distribution<uint64_t> dis;
                uint64_t data[2]{dis(gen), dis(gen)};
                uint8_t* ptr = (uint8_t*)data;

                enbt::raw_uuid result{ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]};
                //set version to 3
                result.data[6] = (result.data[6] & 0x0F) | 0x30;
                return result;
            }

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
}


#endif /* SRC_MOJANG_API_SESSION_SERVER */
