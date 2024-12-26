#ifndef SRC_BUILD_IN_PLUGINS_SPECIAL_STATUS
#define SRC_BUILD_IN_PLUGINS_SPECIAL_STATUS
#include <library/fast_task.hpp>
#include <src/api/configuration.hpp>
#include <src/api/players.hpp>
#include <src/plugin/special.hpp>
#include <src/storage/memory/online_player.hpp>
#include <src/util/conversions.hpp>

namespace copper_server::build_in_plugins::special {
    class Status : public SpecialPluginStatus {
        fast_task::task_mutex cached_mutex;
        std::string cached_icon;
        list_array<std::pair<std::string, enbt::raw_uuid>> sample_cache;
        size_t sample_cache_check_size = -1;
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
            return api::configuration::get().server.max_players;
        }

        size_t OnlinePlayers() override {
            return api::players::online_players();
        }

        //string also can be Chat json object!
        list_array<std::pair<std::string, enbt::raw_uuid>> OnlinePlayersSample() override {
            std::lock_guard lock(cached_mutex);
            if (sample_cache_check_size == api::players::size())
                return sample_cache;

            list_array<std::pair<std::string, enbt::raw_uuid>> result;
            size_t i = 0;
            api::players::iterate_players_not_state(copper_server::base_objects::SharedClientData::packets_state_t::protocol_state::initialization, [&](const copper_server::base_objects::SharedClientData& player) {
                if (i < api::configuration::get().status.sample_players_count) {
                    if (!player.data)
                        return true;
                    if (player.allow_server_listings)
                        result.push_back({player.name, player.data->uuid});
                    else
                        result.push_back({"Anonymous Player", enbt::raw_uuid::as_null()});
                    return true;
                } else
                    return false;
            });
            return sample_cache = result;
        }

        std::optional<bool> PreventsChatReports() override {
            return api::configuration::get().server.prevent_chat_reports;
        }

        Chat Description() override {
            return api::configuration::get().status.description;
        }

        bool ConnectionAvailable(int32_t protocol_version) override {
            return api::configuration::get().protocol.allowed_versions_processed.contains(protocol_version);
        }

        //return empty string if no icon, icon must be 64x64 and png format in base64
        std::string ServerIcon() override {
            auto& icon = api::configuration::get().status.favicon;
            std::lock_guard lock(cached_mutex);
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
#endif /* SRC_BUILD_IN_PLUGINS_SPECIAL_STATIS */
