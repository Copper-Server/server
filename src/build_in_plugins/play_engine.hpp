#ifndef SRC_BUILD_IN_PLUGINS_PLAY_ENGINE
#define SRC_BUILD_IN_PLUGINS_PLAY_ENGINE
#include <array>
#include <library/fast_task.hpp>
#include <library/list_array.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    //handles clients with play state, allows players to access world and other things through api
    class PlayEngine : public PluginAutoRegister<"play_engine", PlayEngine> {
        fast_task::task_mutex messages_order;
        list_array<std::array<uint8_t, 256>> lastset_messages;

    public:
        PlayEngine();
        void OnLoad(const PluginRegistrationPtr& self) override;
        void OnUnload(const PluginRegistrationPtr& self) override;

        void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
        plugin_response PlayerJoined(base_objects::client_data_holder& client_ref) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_PLAY_ENGINE */
