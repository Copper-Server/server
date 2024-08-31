#ifndef SRC_MOJANG_API_SESSION_SERVER
#define SRC_MOJANG_API_SESSION_SERVER
#include "../library/enbt.hpp"
#include "../util/conversions.hpp"
#include "http.hpp"
#include <boost/json.hpp>
#include <chrono>
#include <random>
#include <string>

namespace mojang {
    namespace api {
        class session_server {
            static ENBT::UUID generateOfflineUUID() {
                static std::random_device rd;
                static std::mt19937_64 gen(rd());
                static std::uniform_int_distribution<uint64_t> dis;
                uint64_t data[2]{dis(gen), dis(gen)};
                uint8_t* ptr = (uint8_t*)data;

                ENBT::UUID result{ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]};
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
                ENBT::UUID uuid;
                std::chrono::system_clock::time_point last_check;
                bool online_data;

                std::vector<property> properties;
            };

            std::unordered_map<std::string, std::shared_ptr<player_data>> cache;
            std::chrono::system_clock::duration cache_duration = std::chrono::minutes(20);

            const std::shared_ptr<player_data> hasJoined(const std::string& username, const std::string& serverId, bool online_mode, bool cache_result = true) {
                auto cache_key = cache.find(username);
                if (cache_key != cache.end()) {
                    if (!cache_key->second->online_data)
                        if (!online_mode)
                            return cache_key->second;

                    if (std::chrono::system_clock::now() - cache_key->second->last_check < cache_duration)
                        return cache_key->second;
                    cache.erase(cache_key);
                }

                if (online_mode) {
                    std::string response = http::get("sessionserver.mojang.com", "/session/minecraft/hasJoined?username=" + username + "&serverId=" + serverId);
                    auto value = boost::json::parse(response).as_object();
                    player_data data;

                    data.uuid = crafted_craft::util::conversions::uuid::from(value["id"].as_string());
                    data.uuid_str = crafted_craft::util::conversions::uuid::to(data.uuid);
                    data.online_data = true;
                    if (value.contains("properties")) {
                        std::vector<player_data::property> properties;
                        properties.reserve(value["properties"].as_array().size());
                        for (auto& prop : value["properties"].as_array()) {
                            auto& tree = prop.as_object();
                            player_data::property convert;
                            convert.name = tree["name"].as_string();
                            convert.name = tree["value"].as_string();
                            if (tree.contains("signature"))
                                convert.signature = tree["signature"].as_string();
                            properties.push_back(std::move(convert));
                        }
                        data.properties = std::move(properties);
                    }
                    data.last_check = std::chrono::system_clock::now();
                    return cache[username] = std::make_shared<player_data>(std::move(data));
                } else {
                    auto uuid = generateOfflineUUID();
                    return (cache[username] = std::make_shared<player_data>(crafted_craft::util::conversions::uuid::to(uuid), uuid, std::chrono::system_clock::now(), false));
                }
            }
        };
    }
}


#endif /* SRC_MOJANG_API_SESSION_SERVER */
