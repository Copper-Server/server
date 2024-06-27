#ifndef SRC_PLUGIN_MAIN
#define SRC_PLUGIN_MAIN
#include "registration.hpp"

namespace crafted_craft {
    class PluginManagement {
        std::unordered_map<std::string, PluginRegistrationPtr> plugins;

    public:
        void registerPlugin(const std::string& name, PluginRegistrationPtr plugin) {
            plugins[name] = plugin;
            plugin->OnRegister(plugin);
        }

        PluginRegistrationPtr getPlugin(const std::string& name) {
            auto it = plugins.find(name);
            if (it == plugins.end())
                return nullptr;
            return it->second;
        }

        template <class Plugin>
        std::shared_ptr<Plugin> requestPlugin(const std::string& name) {
            static_assert(std::is_base_of<PluginRegistration, Plugin>::value, "Plugin must derive from PluginRegistration");

            auto it = plugins.find(name);
            if (it == plugins.end())
                return nullptr;

            auto pluginPtr = it->second;
            return std::dynamic_pointer_cast<Plugin>(pluginPtr);
        }

        void unloadPlugin(const std::string& name) {
            auto it = plugins.find(name);
            if (it == plugins.end())
                return;
            auto& plugin = it->second;
            plugin->OnUnload(plugin);
            plugins.erase(it);
        }

        list_array<PluginRegistrationPtr> registeredPlugins() {
            list_array<PluginRegistrationPtr> result;
            result.reserve(plugins.size());
            for (auto& [name, plugin] : plugins)
                result.push_back(plugin);

            return result;
        }

        void callLoad() {
            for (auto& [name, plugin] : plugins) {
                plugin->OnLoad(plugin);
            }
            for (auto& [name, plugin] : plugins)
                plugin->OnPostLoad(plugin);
        }
    };

    extern PluginManagement pluginManagement;
}
#endif /* SRC_PLUGIN_MAIN */
