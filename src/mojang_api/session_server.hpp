#ifndef SRC_MOJANG_API_SESSION_SERVER
#define SRC_MOJANG_API_SESSION_SERVER
#include "../library/enbt.hpp"
#include "http.hpp"
#include <boost/json.hpp>
#include <chrono>
#include <random>
#include <string>

namespace mojang {
    namespace api {
        class session_server {
            static uint8_t unHex_0(char ch0) {
                switch (ch0) {
                case '0':
                    return 0;
                case '1':
                    return 1;
                case '2':
                    return 2;
                case '3':
                    return 3;
                case '4':
                    return 4;
                case '5':
                    return 5;
                case '6':
                    return 6;
                case '7':
                    return 7;
                case '8':
                    return 8;
                case '9':
                    return 9;
                case 'a':
                case 'A':
                    return 0xa;
                case 'b':
                case 'B':
                    return 0xb;
                case 'c':
                case 'C':
                    return 0xc;
                case 'd':
                case 'D':
                    return 0xd;
                case 'e':
                case 'E':
                    return 0xe;
                case 'f':
                case 'F':
                    return 0xf;
                default:
                    throw std::invalid_argument("This function accepts only hex symbols");
                }
            }

            static uint8_t unHex(char ch0, char ch1) {
                return unHex_0(ch0) & (unHex_0(ch1) << 4);
            }

            static void addHex(std::string& str, uint8_t i) {
                static constexpr char map[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
                str += map[i & 0xF];
                str += map[(i >> 4) & 0xF0];
            }

            static std::string stringizeUUID(ENBT::UUID id) {
                std::string res;
                addHex(res, id.data[0]);
                addHex(res, id.data[1]);
                addHex(res, id.data[2]);
                addHex(res, id.data[3]);
                res += '-';
                addHex(res, id.data[4]);
                addHex(res, id.data[5]);
                res += '-';
                addHex(res, id.data[6]);
                addHex(res, id.data[7]);
                res += '-';
                addHex(res, id.data[8]);
                addHex(res, id.data[9]);
                res += '-';
                addHex(res, id.data[10]);
                addHex(res, id.data[11]);
                addHex(res, id.data[12]);
                addHex(res, id.data[13]);
                addHex(res, id.data[14]);
                addHex(res, id.data[15]);
                return res;
            }

            static ENBT::UUID unstringingUUID(const boost::json::string& id) {
                ENBT::UUID res;
                uint8_t index = 0;
                char cache;
                bool cached = false;
                for (char ch : id) {
                    if (ch == '-')
                        continue;
                    if (!cached) {
                        cache = ch;
                        cached = true;
                    } else {
                        res.data[index++] = unHex(cached, ch);
                        cached = false;
                    }
                    if (index == 16)
                        break;
                }
                return res;
            }

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

                    data.uuid = unstringingUUID(value["id"].as_string());
                    data.uuid_str = stringizeUUID(data.uuid);
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
                    return (cache[username] = std::make_shared<player_data>(stringizeUUID(uuid), uuid, std::chrono::system_clock::now(), false));
                }
            }
        };
    }
}


#endif /* SRC_MOJANG_API_SESSION_SERVER */
