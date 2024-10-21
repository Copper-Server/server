#ifndef SRC_PLUGIN_MAIN
#define SRC_PLUGIN_MAIN
#include "../library/fast_task.hpp"
#include "registration.hpp"

namespace crafted_craft {

    namespace __internal__ {
        template <std::size_t N>
        struct CTS {
            char data[N]{};

            consteval CTS(const char (&str)[N]) {
                std::copy_n(str, N, data);
            }
        };

        template <class T, CTS name>
        struct static_registry {
            struct proxy {
                inline proxy();
            };

            static proxy p;
        };

        template <class T, CTS name>
        typename static_registry<T, name>::proxy static_registry<T, name>::p;

        class delayed_construct_base {
        public:
            virtual PluginRegistrationPtr construct() = 0;
        };

        void register_configuration(const PluginRegistrationPtr& self);
        void register_play(const PluginRegistrationPtr& self);

        template <class T>
        class delayed_construct : public delayed_construct_base {
        public:
            PluginRegistrationPtr construct() override {
                auto tmp_ = std::make_shared<T>();
                if (
                    &T::OnConfiguration != &PluginRegistration::OnConfiguration
                    || &T::OnConfiguration_PlayerSettingsChanged != &PluginRegistration::OnConfiguration_PlayerSettingsChanged
                    || &T::OnConfiguration_gotKnownPacks != &PluginRegistration::OnConfiguration_gotKnownPacks
                )
                    register_configuration(tmp_);
                if (
                    &T::OnPlay_initialize != &PluginRegistration::OnPlay_initialize
                    || &T::OnPlay_uninitialized != &PluginRegistration::OnPlay_uninitialized
                )
                    register_play(tmp_);
                return tmp_;
            }
        };

        std::vector<std::pair<std::string, std::shared_ptr<delayed_construct_base>>>& registration_list();

        template <class T, CTS name>
        static_registry<T, name>::proxy::proxy() {
            registration_list().push_back({name.data, std::make_shared<delayed_construct<T>>()});
        }

        template <class T, CTS name>
        static void register_value() {
            static_registry<T, name>::p;
        }
    }

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

        void autoRegister() {
            for (auto& [name, plugin] : __internal__::registration_list()) {
                registerPlugin(name, plugin->construct());
            }
            __internal__::registration_list().clear();
        }

        void callInitialization() {
            std::unordered_map<std::string, PluginRegistrationPtr> plugins;
            protected_values.get(
                [&](const protected_values_t& vals) {
                    plugins = vals.plugins;
                }
            );
            for (auto& [name, plugin] : plugins)
                plugin->OnInitialization(plugin);
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

            for (auto& [name, plugin] : plugins)
                plugin->OnLoadComplete(plugin);
        }

        void callUnload() {
            std::unordered_map<std::string, PluginRegistrationPtr> plugins;
            protected_values.get(
                [&](const protected_values_t& vals) {
                    plugins = vals.plugins;
                }
            );
            for (auto& [name, plugin] : plugins)
                plugin->OnUnload(plugin);

            for (auto& [name, plugin] : plugins)
                plugin->OnPostUnload(plugin);

            for (auto& [name, plugin] : plugins) {
                plugin->OnUnloadComplete(plugin);
                plugin->clean_up_registered_events();
            }
        }

        void callFaultUnload() {
            std::unordered_map<std::string, PluginRegistrationPtr> plugins;
            protected_values.get(
                [&](const protected_values_t& vals) {
                    plugins = vals.plugins;
                }
            );
            for (auto& [name, plugin] : plugins) {
                plugin->OnFaultUnload(plugin);
                plugin->clean_up_registered_events();
            }
        }

        void unregisterAll() {
            protected_values.set(
                [&](protected_values_t& vals) {
                    vals.plugins.clear();
                }
            );
        }
    };

    template <__internal__::CTS name, class Self>
    class PluginAutoRegister : public PluginRegistration {
    protected:
        static inline const std::string registered_name = []() {
            __internal__::register_value<Self, name>();
            return name.data;
        }();
    };

    extern PluginManagement pluginManagement;
}
#endif /* SRC_PLUGIN_MAIN */
