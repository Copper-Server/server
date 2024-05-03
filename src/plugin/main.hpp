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

        void unloadPlugin(const std::string& name) {
            auto plugin = plugins[name];
            plugin->OnUnload(plugin);
            plugins.erase(name);
        }

        list_array<PluginRegistrationPtr> registeredPlugins() {
            list_array<PluginRegistrationPtr> result;
            result.reserve(plugins.size());
            for (auto& [name, plugin] : plugins)
                result.push_back(plugin);

            return result;
        }
    };

    extern PluginManagement pluginManagement;
}
#endif /* SRC_PLUGIN_MAIN */
