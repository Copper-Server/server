#ifndef SRC_BUILD_IN_PLUGINS_SPECIAL_STATUS
#define SRC_BUILD_IN_PLUGINS_SPECIAL_STATUS
#include "../../api/configuration.hpp"
#include "../../api/players.hpp"
#include "../../library/fast_task.hpp"
#include "../../plugin/special.hpp"
#include "../../storage/memory/online_player.hpp"
#include "../../util/conversions.hpp"
namespace crafted_craft {
    namespace build_in_plugins {
        namespace special {
            class Status : public SpecialPluginStatus {
                fast_task::task_mutex cached_icon_mutex;
                std::string cached_icon;
                const uint8_t* icon_data = nullptr;

            public:
                Status() {}

                std::string StatusResponseVersionName() override {
                    return api::configuration::get().status.server_name;
                }

                bool ShowConnectionStatus() override {
                    return api::configuration::get().status.show_players;
                }

                size_t MaxPlayers() override {
                    return api::configuration::get().protocol.max_players;
                }

                size_t OnlinePlayers() override {
                    return api::players::online_players();
                }

                //string also can be Chat json object!
                list_array<std::pair<std::string, enbt::raw_uuid>> OnlinePlayersSample() override {
                    list_array<std::pair<std::string, enbt::raw_uuid>> result;
                    size_t i = 0;
                    api::players::iterate_players_not_state(crafted_craft::base_objects::SharedClientData::packets_state_t::protocol_state::initialization, [&](const crafted_craft::base_objects::SharedClientData& player) {
                        if (i < api::configuration::get().status.sample_players_count) {
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
                    return api::configuration::get().status.description;
                }

                bool ConnectionAvailable(int32_t protocol_version) override {
                    switch (protocol_version) {
                    case 765:
                    case 766:
                    case 767:
                        return true;
                    default:
                        return false;
                    }
                }

                //return empty string if no icon, icon must be 64x64 and png format in base64
                std::string ServerIcon() override {
                    auto& icon = api::configuration::get().status.favicon;
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
