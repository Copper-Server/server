#ifndef SRC_PLUGIN_MAIN
#define SRC_PLUGIN_MAIN
#include <src/plugin/registration.hpp>
#include <src/util/task_management.hpp>

namespace copper_server {
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
                    || &T::OnPlay_initialize_compatible != &PluginRegistration::OnPlay_initialize_compatible
                    || &T::OnPlay_uninitialized != &PluginRegistration::OnPlay_uninitialized
                    || &T::OnPlay_uninitialized_compatible != &PluginRegistration::OnPlay_uninitialized_compatible
                    || &T::PlayerJoined != &PluginRegistration::PlayerJoined
                    || &T::PlayerLeave != &PluginRegistration::PlayerLeave
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

            struct ___ {
                struct {
                    std::unordered_map<std::string, PluginRegistrationPtr> plugins;
                    std::unordered_map<std::string, PluginRegistrationPtr> cookies;
                } login;

                struct {
                    std::unordered_map<std::string, PluginRegistrationPtr> plugins;
                    std::unordered_map<std::string, PluginRegistrationPtr> cookies;
                    list_array<PluginRegistrationPtr> on_init;
                } configuration;

                struct {
                    std::unordered_map<std::string, PluginRegistrationPtr> plugins;
                    std::unordered_map<std::string, PluginRegistrationPtr> cookies;
                    list_array<PluginRegistrationPtr> on_init;
                } play;

                void unregister(PluginRegistrationPtr& plugin);
            } registration;
        };

        fast_task::protected_value<protected_values_t> protected_values;

    public:
        enum class registration_on {
            login,
            configuration,
            play
        };

        void registerPluginOn(PluginRegistrationPtr plugin, registration_on on);

        void bindPluginOn(const std::string& channel, PluginRegistrationPtr plugin, registration_on on);

        void bindPluginCookiesOn(const std::string& cookie_id, PluginRegistrationPtr plugin, registration_on on);

        template <class FN>
        void inspect_plugin_registration(registration_on on, FN&& fn) const {
            list_array<PluginRegistrationPtr> on_init;
            protected_values.get([&](const protected_values_t& vals) {
                switch (on) {
                case registration_on::login:
                    break;
                case registration_on::configuration:
                    on_init = vals.registration.configuration.on_init;
                    break;
                case registration_on::play:
                    on_init = vals.registration.play.on_init;
                    break;
                default:
                    break;
                }
            });
            on_init.for_each(fn);
        }

        template <class FN>
        auto inspect_plugin_registration_future_accumulate(registration_on on, FN&& fn) const {
            list_array<PluginRegistrationPtr> on_init;
            protected_values.get([&](const protected_values_t& vals) {
                switch (on) {
                case registration_on::login:
                    break;
                case registration_on::configuration:
                    on_init = vals.registration.configuration.on_init;
                    break;
                case registration_on::play:
                    on_init = vals.registration.play.on_init;
                    break;
                default:
                    break;
                }
            });
            using ret_t = std::invoke_result_t<FN, PluginRegistrationPtr>;
            using fut = Future<ret_t>;
            return future::accumulate<ret_t>(
                on_init.convert<std::shared_ptr<fut>>(
                    [&](auto& it) {
                        return fut::start([fn = fn, it]() {
                            return fn(it);
                        });
                    }
                )
            );
        }

        template <class FN>
        auto inspect_plugin_registration_async_accumulate(registration_on on, FN&& fn) const {
            return inspect_plugin_registration_future_accumulate(on, std::forward<FN>(fn)).take();
        }


        template <class FN>
        void inspect_plugin_bind(registration_on on, FN&& fn) const {
            std::unordered_map<std::string, PluginRegistrationPtr> plugins;
            protected_values.get([&](const protected_values_t& vals) {
                switch (on) {
                case registration_on::login:
                    plugins = vals.registration.login.plugins;
                    break;
                case registration_on::configuration:
                    plugins = vals.registration.configuration.plugins;
                    break;
                case registration_on::play:
                    plugins = vals.registration.play.plugins;
                    break;
                default:
                    break;
                }
            });
            for (auto& it : plugins)
                fn(it);
        }

        template <class FN>
        auto inspect_plugin_bind_future_accumulate(registration_on on, FN&& fn) const {
            std::unordered_map<std::string, PluginRegistrationPtr> plugins;
            protected_values.get([&](const protected_values_t& vals) {
                switch (on) {
                case registration_on::login:
                    plugins = vals.registration.login.plugins;
                    break;
                case registration_on::configuration:
                    plugins = vals.registration.configuration.plugins;
                    break;
                case registration_on::play:
                    plugins = vals.registration.play.plugins;
                    break;
                default:
                    break;
                }
            });
            using ret_t = std::invoke_result_t<FN, std::pair<std::string, PluginRegistrationPtr>>;
            using fut = Future<ret_t>;
            list_array<std::shared_ptr<fut>> futs;
            for (auto& it : plugins) {
                futs.push_back(fut::start([fn = fn, it]() {
                    return fn(it);
                }));
            }
            return future::accumulate<ret_t>(futs);
        }

        template <class FN>
        auto inspect_plugin_bind_async_accumulate(registration_on on, FN&& fn) const {
            return inspect_plugin_bind_future_accumulate(on, std::forward<FN>(fn)).take();
        }

        PluginRegistrationPtr get_bind_plugin(registration_on on, const std::string& channel) const;
        PluginRegistrationPtr get_bind_cookies(registration_on on, const std::string& cookie_id) const;
        void registerPlugin(const std::string& name, PluginRegistrationPtr plugin);
        PluginRegistrationPtr getPlugin(const std::string& name) const;

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

        template <class Plugin>
        std::shared_ptr<Plugin> requestPlugin() const {
            static_assert(std::is_base_of<PluginRegistration, Plugin>::value, "Plugin must derive from PluginRegistration");
            return protected_values.get(
                [&](const protected_values_t& vals) -> std::shared_ptr<Plugin> {
                    auto it = vals.plugins.find(Plugin::registered_name);
                    if (it == vals.plugins.end())
                        return nullptr;
                    auto pluginPtr = it->second;
                    return std::dynamic_pointer_cast<Plugin>(pluginPtr);
                }
            );
        }

        void unloadPlugin(const std::string& name);
        list_array<PluginRegistrationPtr> registeredPlugins() const;
        void autoRegister();
        void callInitialization();
        void callLoad();
        void callUnload();
        void callFaultUnload();
        void unregisterAll();
    };

    template <__internal__::CTS name, class Self>
    class PluginAutoRegister : public PluginRegistration {
    public:
        static inline const std::string registered_name = []() {
            __internal__::register_value<Self, name>();
            return name.data;
        }();
    };

    extern PluginManagement pluginManagement;
}
#endif /* SRC_PLUGIN_MAIN */
