#ifndef SRC_BUILD_IN_PLUGINS_SPECIAL_STATUS
#define SRC_BUILD_IN_PLUGINS_SPECIAL_STATUS
#include "../../base_objects/server_configuaration.hpp"
#include "../../plugin/special.hpp"
#include "../../storage/memory/online_player.hpp"
#include "../../util/conversions.hpp"

#include "../../library/fast_task.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        namespace special {
            class Status : public SpecialPluginStatus {
                storage::memory::online_player_storage& online_players;

                fast_task::task_mutex cached_icon_mutex;
                std::string cached_icon;
                const uint8_t* icon_data = nullptr;

            public:
                Status(const base_objects::ServerConfiguration& config, storage::memory::online_player_storage& online_players)
                    : online_players(online_players), SpecialPluginStatus(config) {}

                std::string StatusResponseVersionName() override {
                    return config.status.server_name;
                }

                bool ShowConnectionStatus() override {
                    return config.status.show_players;
                }

                size_t MaxPlayers() override {
                    return config.protocol.max_players;
                }

                size_t OnlinePlayers() override {
                    return online_players.online_players();
                }

                //string also can be Chat json object!
                list_array<std::pair<std::string, ENBT::UUID>> OnlinePlayersSample() override {
                    list_array<std::pair<std::string, ENBT::UUID>> result;
                    size_t i = 0;
                    online_players.iterate_players_not_state(crafted_craft::base_objects::SharedClientData::packets_state_t::protocol_state::initialization, [&](const crafted_craft::base_objects::SharedClientData& player) {
                        if (i < config.status.sample_players_count) {
                            if (!player.data)
                                return true;
                            result.push_back({player.name, player.data->uuid});
                            return true;
                        } else
                            return false;
                    });
                    return result;
                }

                Chat Description() override {
                    return config.status.description;
                }

                bool ConnectionAvailable(int32_t protocol_version) override {
                    return protocol_version == 765;
                }

                //return empty string if no icon, icon must be 64x64 and png format in base64
                virtual std::string ServerIcon() {
                    auto& icon = config.status.favicon;
                    std::lock_guard lock(cached_icon_mutex);
                    if (cached_icon.size() > 0)
                        if (icon_data == icon.data())
                            return cached_icon;

                    if (icon.size() == 0) {
                        return "";
                    } else {
                        icon_data = icon.data();
                        return cached_icon = util::conversions::base64::encode(icon);
                    }
                }
            };
        }
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_SPECIAL_STATIS */
