#ifndef SRC_PLUGIN_MAIN
#define SRC_PLUGIN_MAIN
#include "../library/fast_task.hpp"
#include "registration.hpp"

namespace crafted_craft {
    class PluginManagement {
        struct protected_values_t {
            std::unordered_map<std::string, PluginRegistrationPtr> plugins;

            struct {
                struct {
                    std::unordered_map<std::string, PluginRegistrationPtr> plugins;
                } login;

                struct {
                    std::unordered_map<std::string, PluginRegistrationPtr> plugins;
                    list_array<PluginRegistrationPtr> on_init;
                } configuration;

                struct {
                    std::unordered_map<std::string, PluginRegistrationPtr> plugins;
                    list_array<PluginRegistrationPtr> on_init;
                } play;

                static void unregisterEvery(PluginRegistrationPtr& plugin, std::unordered_map<std::string, PluginRegistrationPtr>& plugins) {
                    for (
                        std::unordered_map<std::string, PluginRegistrationPtr>::iterator it;
                        it != plugins.end();
                        it = std::find_if(plugins.begin(), plugins.end(), [&plugin](auto& item) {
                            return item.second == plugin;
                        })
                    )
                        plugins.erase(it);
                }

                void unregister(PluginRegistrationPtr& plugin) {
                    //login
                    unregisterEvery(plugin, login.plugins);
                    //configuration
                    configuration.on_init.remove(plugin);
                    unregisterEvery(plugin, configuration.plugins);
                    //play
                    play.on_init.remove(plugin);
                    unregisterEvery(plugin, play.plugins);
                }
            } registration;
        };

        fast_task::protected_value<protected_values_t> protected_values;


    public:
        enum class registration_on {
            login,
            configuration,
            play
        };

        void registerPluginOn(PluginRegistrationPtr plugin, registration_on on) {
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

        void bindPluginOn(const std::string& channel, PluginRegistrationPtr plugin, registration_on on) {
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

        template <class FN>
        void inspect_plugin_registration(registration_on on, FN&& fn) const {
            protected_values.get([&](const protected_values_t& vals) {
                switch (on) {
                case registration_on::login:
                    break;
                case registration_on::configuration:
                    vals.registration.configuration.on_init.for_each(fn);
                    break;
                case registration_on::play:
                    vals.registration.play.on_init.for_each(fn);
                    break;
                default:
                    break;
                }
            });
        }

        template <class FN>
        void inspect_plugin_bind(registration_on on, FN&& fn) const {
            protected_values.get([&](const protected_values_t& vals) {
                switch (on) {
                case registration_on::login:
                    for (auto& it : vals.registration.login.plugins)
                        fn(it);
                    break;
                case registration_on::configuration:
                    for (auto& it : vals.registration.configuration.plugins)
                        fn(it);
                    break;
                case registration_on::play:
                    for (auto& it : vals.registration.play.plugins)
                        fn(it);
                    break;
                default:
                    break;
                }
            });
        }

        PluginRegistrationPtr get_bind_plugin(registration_on on, const std::string& channel) const {
            return protected_values.get([&](const protected_values_t& vals) {
                switch (on) {
                case registration_on::login: {
                    auto it = vals.registration.login.plugins.find(channel);
                    if (it != vals.registration.login.plugins.end())
                        return it->second;
                    else
                        nullptr;
                }
                case registration_on::configuration: {
                    auto it = vals.registration.configuration.plugins.find(channel);
                    if (it != vals.registration.configuration.plugins.end())
                        return it->second;
                    else
                        nullptr;
                }
                case registration_on::play: {
                    auto it = vals.registration.play.plugins.find(channel);
                    if (it != vals.registration.play.plugins.end())
                        return it->second;
                    else
                        nullptr;
                }
                default:
                    throw std::runtime_error("Unknown registration");
                }
            });
        }


        void registerPlugin(const std::string& name, PluginRegistrationPtr plugin) {
            protected_values.set(
                [&](protected_values_t& vals) {
                    vals.plugins[name] = plugin;
                }
            );
            plugin->OnRegister(plugin);
        }

        PluginRegistrationPtr getPlugin(const std::string& name) const {
            return protected_values.get(
                [&](const protected_values_t& vals) -> PluginRegistrationPtr {
                    auto it = vals.plugins.find(name);
                    if (it == vals.plugins.end())
                        return nullptr;
                    return it->second;
                }
            );
        }

        template <class Plugin>
        std::shared_ptr<Plugin> requestPlugin(const std::string& name) const {
            static_assert(std::is_base_of<PluginRegistration, Plugin>::value, "Plugin must derive from PluginRegistration");
            return protected_values.get(
                [&](const protected_values_t& vals) -> std::shared_ptr<Plugin> {
                    auto it = vals.plugins.find(name);
                    if (it == vals.plugins.end())
                        return nullptr;
                    auto pluginPtr = it->second;
                    return std::dynamic_pointer_cast<Plugin>(pluginPtr);
                }
            );
        }

        void unloadPlugin(const std::string& name) {
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
            plugin->OnUnload(plugin);
        }

        list_array<PluginRegistrationPtr> registeredPlugins() const {
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

        void callLoad() {
            std::unordered_map<std::string, PluginRegistrationPtr> plugins;
            protected_values.get(
                [&](const protected_values_t& vals) {
                    plugins = vals.plugins;
                }
            );
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
