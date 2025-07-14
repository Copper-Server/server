#include <src/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server {
    void unregisterEvery(PluginRegistrationPtr& plugin, std::unordered_map<std::string, PluginRegistrationPtr>& container) {
        for (
            std::unordered_map<std::string, PluginRegistrationPtr>::iterator it;
            it != container.end();
            it = std::find_if(container.begin(), container.end(), [&plugin](auto& item) {
                return item.second == plugin;
            })
        )
            container.erase(it);
    }

    void PluginManagement::protected_values_t::___::unregister(PluginRegistrationPtr& plugin) {
        //login
        unregisterEvery(plugin, login.plugins);
        unregisterEvery(plugin, login.cookies);
        //configuration
        configuration.on_init.remove(plugin);
        unregisterEvery(plugin, configuration.plugins);
        unregisterEvery(plugin, configuration.cookies);
        //play
        play.on_init.remove(plugin);
        unregisterEvery(plugin, play.plugins);
        unregisterEvery(plugin, play.cookies);
    }

    void PluginManagement::registerPluginOn(PluginRegistrationPtr plugin, registration_on on) {
        protected_values.set([&](protected_values_t& vals) {
            switch (on) {
            case registration_on::login:
                break;
            case registration_on::configuration:
                vals.registration.configuration.on_init.push_back(plugin);
                break;
            case registration_on::play:
                vals.registration.play.on_init.push_back(plugin);
                break;
            default:
                break;
            }
        });
    }

    void PluginManagement::bindPluginOn(const std::string& channel, PluginRegistrationPtr plugin, registration_on on) {
        protected_values.set([&](protected_values_t& vals) {
            switch (on) {
            case registration_on::login:
                vals.registration.login.plugins[channel] = plugin;
                break;
            case registration_on::configuration:
                vals.registration.configuration.plugins[channel] = plugin;
                break;
            case registration_on::play:
                vals.registration.play.plugins[channel] = plugin;
                break;
            default:
                break;
            }
        });
    }

    void PluginManagement::bindPluginCookiesOn(const std::string& cookie_id, PluginRegistrationPtr plugin, registration_on on) {
        protected_values.set([&](protected_values_t& vals) {
            switch (on) {
            case registration_on::login:
                vals.registration.login.cookies[cookie_id] = plugin;
                break;
            case registration_on::configuration:
                vals.registration.configuration.cookies[cookie_id] = plugin;
                break;
            case registration_on::play:
                vals.registration.play.cookies[cookie_id] = plugin;
                break;
            default:
                break;
            }
        });
    }

    PluginRegistrationPtr PluginManagement::get_bind_plugin(registration_on on, const std::string& channel) const {
        return protected_values.get([&](const protected_values_t& vals) -> PluginRegistrationPtr {
            switch (on) {
            case registration_on::login: {
                auto it = vals.registration.login.plugins.find(channel);
                if (it != vals.registration.login.plugins.end())
                    return it->second;
                else
                    return nullptr;
            }
            case registration_on::configuration: {
                auto it = vals.registration.configuration.plugins.find(channel);
                if (it != vals.registration.configuration.plugins.end())
                    return it->second;
                else
                    return nullptr;
            }
            case registration_on::play: {
                auto it = vals.registration.play.plugins.find(channel);
                if (it != vals.registration.play.plugins.end())
                    return it->second;
                else
                    return nullptr;
            }
            default:
                throw std::runtime_error("Unknown registration");
            }
        });
    }

    PluginRegistrationPtr PluginManagement::get_bind_cookies(registration_on on, const std::string& cookie_id) const {
        return protected_values.get([&](const protected_values_t& vals) -> PluginRegistrationPtr {
            switch (on) {
            case registration_on::login: {
                auto it = vals.registration.login.plugins.find(cookie_id);
                if (it != vals.registration.login.plugins.end())
                    return it->second;
                else
                    return nullptr;
            }
            case registration_on::configuration: {
                auto it = vals.registration.configuration.plugins.find(cookie_id);
                if (it != vals.registration.configuration.plugins.end())
                    return it->second;
                else
                    return nullptr;
            }
            case registration_on::play: {
                auto it = vals.registration.play.plugins.find(cookie_id);
                if (it != vals.registration.play.plugins.end())
                    return it->second;
                else
                    return nullptr;
            }
            default:
                throw std::runtime_error("Unknown registration");
            }
        });
    }

    void PluginManagement::registerPlugin(const std::string& name, PluginRegistrationPtr plugin) {
        protected_values.set(
            [&](protected_values_t& vals) {
                vals.plugins[name] = plugin;
            }
        );
        plugin->OnRegister(plugin);
    }

    PluginRegistrationPtr PluginManagement::getPlugin(const std::string& name) const {
        return protected_values.get(
            [&](const protected_values_t& vals) -> PluginRegistrationPtr {
                auto it = vals.plugins.find(name);
                if (it == vals.plugins.end())
                    return nullptr;
                return it->second;
            }
        );
    }

    void PluginManagement::unloadPlugin(const std::string& name) {
        PluginRegistrationPtr plugin;
        protected_values.set(
            [&](protected_values_t& vals) {
                auto it = vals.plugins.find(name);
                if (it == vals.plugins.end())
                    return;
                plugin = it->second;
                vals.plugins.erase(it);
                vals.registration.unregister(plugin);
            }
        );
        if (!plugin->is_loaded)
            return;
        plugin->OnUnload(plugin);
        plugin->OnPostUnload(plugin);
        plugin->OnUnloadComplete(plugin);
        plugin->is_loaded = false;
        plugin->clean_up_registered_events();
        plugin->OnUnregister(plugin);
    }

    list_array<PluginRegistrationPtr> PluginManagement::registeredPlugins() const {
        list_array<PluginRegistrationPtr> result;
        return protected_values.get(
            [&](const protected_values_t& vals) {
                result.reserve(vals.plugins.size());
                for (auto& [name, plugin] : vals.plugins)
                    result.push_back(plugin);
                return result;
            }
        );
    }

    void PluginManagement::autoRegister() {
        for (auto& [name, plugin] : __internal__::registration_list()) {
            registerPlugin(name, plugin->construct());
        }
        __internal__::registration_list().clear();
    }

    void PluginManagement::callInitialization() {
        std::unordered_map<std::string, PluginRegistrationPtr> plugins;
        protected_values.get(
            [&](const protected_values_t& vals) {
                plugins = vals.plugins;
            }
        );
        for (auto& [name, plugin] : plugins)
            plugin->OnInitialization(plugin);
    }

    void PluginManagement::callLoad() {
        std::unordered_map<std::string, PluginRegistrationPtr> plugins;
        protected_values.get(
            [&](const protected_values_t& vals) {
                plugins = vals.plugins;
            }
        );

        future::forEach(plugins, [](auto& plugin) {
            if (!plugin.second->is_loaded)
                plugin.second->OnLoad(plugin.second);
        })->wait();

        future::forEach(plugins, [](auto& plugin) {
            if (!plugin.second->is_loaded)
                plugin.second->OnPostLoad(plugin.second);
        })->wait();

        future::forEach(plugins, [](auto& plugin) {
            if (!plugin.second->is_loaded)
                plugin.second->OnLoadComplete(plugin.second);
            plugin.second->is_loaded = true;
        })->wait();
    }

    void PluginManagement::callUnload() {
        std::unordered_map<std::string, PluginRegistrationPtr> plugins;
        protected_values.get(
            [&](const protected_values_t& vals) {
                plugins = vals.plugins;
            }
        );
        for (auto& [name, plugin] : plugins)
            if (plugin->is_loaded)
                plugin->OnUnload(plugin);

        for (auto& [name, plugin] : plugins)
            if (plugin->is_loaded)
                plugin->OnPostUnload(plugin);

        for (auto& [name, plugin] : plugins) {
            if (plugin->is_loaded) {
                plugin->OnUnloadComplete(plugin);
                plugin->clean_up_registered_events();
                plugin->is_loaded = false;
            }
        }
    }

    void PluginManagement::callFaultUnload() {
        std::unordered_map<std::string, PluginRegistrationPtr> plugins;
        protected_values.get(
            [&](const protected_values_t& vals) {
                plugins = vals.plugins;
            }
        );
        for (auto& [name, plugin] : plugins) {
            if (plugin->is_loaded) {
                plugin->is_loaded = false;
                plugin->OnFaultUnload(plugin);
                plugin->clean_up_registered_events();
            }
        }
    }

    void PluginManagement::unregisterAll() {
        protected_values.set(
            [&](protected_values_t& vals) {
                vals.plugins.clear();
            }
        );
    }

    PluginManagement pluginManagement;
}