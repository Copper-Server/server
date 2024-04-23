#ifndef SRC_PLUGIN_MAIN
#define SRC_PLUGIN_MAIN
#include "registration.hpp"

namespace crafted_craft {
    class PluginManagement {
        std::unordered_map<std::string, PluginRegistrationPtr> plugins;

    public:
        void registerPlugin(const std::string& name, PluginRegistrationPtr plugin) {
            plugins[name] = plugin;
            plugin->OnLoad(plugin);
        }

        PluginRegistrationPtr getPlugin(const std::string& name) {
            return plugins[name];
        }
    };

    extern PluginManagement pluginManagement;
}
#endif /* SRC_PLUGIN_MAIN */
