/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_PLUGIN_MAIN
#define SRC_PLUGIN_MAIN

#include <library/fast_task.hpp>
#include <src/plugin/registration.hpp>
#include <src/util/cts.hpp>

namespace copper_server {
    namespace __internal__ {
        class delayed_construct_base {
        public:
            virtual plugin_registration_ptr construct() = 0;
        };

        void register_configuration(const plugin_registration_ptr& self);
        void register_play(const plugin_registration_ptr& self);
        void register_ecs_system_registrator(const plugin_registration_ptr& self);

        template <class T>
        class delayed_construct : public delayed_construct_base {
        public:
            plugin_registration_ptr construct() override {
                auto tmp_ = std::make_shared<T>();
                if (
                    &T::on_configuration != &plugin_registration::on_configuration
                    || &T::on_configuration_got_known_packs != &plugin_registration::on_configuration_got_known_packs
                )
                    register_configuration(tmp_);
                if (
                    &T::register_systems != &plugin_registration::register_systems
                )
                    register_ecs_system_registrator(tmp_);
                if (
                    &T::on_play_pre_initialize != &plugin_registration::on_play_pre_initialize
                    || &T::on_play_initialize != &plugin_registration::on_play_initialize
                    || &T::on_play_initialize_compatible != &plugin_registration::on_play_initialize_compatible
                    || &T::on_play_post_initialize != &plugin_registration::on_play_post_initialize
                    || &T::on_play_post_initialize_compatible != &plugin_registration::on_play_post_initialize_compatible
                    || &T::on_play_uninitialized != &plugin_registration::on_play_uninitialized
                    || &T::on_play_uninitialized_compatible != &plugin_registration::on_play_uninitialized_compatible
                    || &T::player_joined != &plugin_registration::player_joined
                    || &T::player_leave != &plugin_registration::player_leave
                )
                    register_play(tmp_);
                return tmp_;
            }
        };

        std::vector<std::pair<std::string, std::shared_ptr<delayed_construct_base>>>& registration_list();

        template <class T, util::CTS name>
        static void register_value() {
            registration_list().emplace_back(name.data, std::make_shared<delayed_construct<T>>());
        }

        void info(std::string_view source, std::string_view message);
        void error(std::string_view source, std::string_view message);
        void warn(std::string_view source, std::string_view message);
        void debug(std::string_view source, std::string_view message);
        void debug_error(std::string_view source, std::string_view message);
        void fatal(std::string_view source, std::string_view message);
    }

    class plugin_management_system {
        struct protected_values_t {
            std::unordered_map<std::string, plugin_registration_ptr> plugins;

            struct ___ {
                struct {
                    std::unordered_map<std::string, plugin_registration_ptr> plugins;
                    std::unordered_map<std::string, plugin_registration_ptr> cookies;
                } login;

                struct {
                    std::unordered_map<std::string, plugin_registration_ptr> plugins;
                    std::unordered_map<std::string, plugin_registration_ptr> cookies;
                    list_array<plugin_registration_ptr> on_init;
                } configuration;

                struct {
                    std::unordered_map<std::string, plugin_registration_ptr> plugins;
                    std::unordered_map<std::string, plugin_registration_ptr> cookies;
                    list_array<plugin_registration_ptr> on_init;
                } play;

                std::unordered_map<std::string, plugin_registration_ptr> ecs_system_providers;

                void unregister(plugin_registration_ptr& plugin);
            } registration;
        };

        fast_task::protected_value<protected_values_t> protected_values;

    public:
        enum class registration_on {
            login,
            configuration,
            play
        };

        void register_plugin_ecs_system(plugin_registration_ptr plugin);

        void register_plugin_on(plugin_registration_ptr plugin, registration_on on);

        void bind_plugin_on(const std::string& channel, plugin_registration_ptr plugin, registration_on on);

        void bind_plugin_cookies_on(const std::string& cookie_id, plugin_registration_ptr plugin, registration_on on);

        template <class FN>
        void inspect_plugin_registration(registration_on on, FN&& fn) const {
            list_array<plugin_registration_ptr> on_init;
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
            list_array<plugin_registration_ptr> on_init;
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
            using ret_t = std::invoke_result_t<FN, plugin_registration_ptr>;
            return fast_task::future_tool::accumulate<ret_t>(
                on_init.convert<std::shared_ptr<fast_task::future_ptr<ret_t>>>(
                    [&](auto& it) {
                        return fast_task::future<ret_t>::start([fn = fn, it]() {
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
            std::unordered_map<std::string, plugin_registration_ptr> plugins;
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
            std::unordered_map<std::string, plugin_registration_ptr> plugins;
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
            using ret_t = std::invoke_result_t<FN, std::pair<std::string, plugin_registration_ptr>>;
            list_array<std::shared_ptr<fast_task::future_ptr<ret_t>>> futures;
            for (auto& it : plugins) {
                futures.push_back(fast_task::future<ret_t>::start([fn = fn, it]() {
                    return fn(it);
                }));
            }
            return fast_task::future_tool::accumulate<ret_t>(futures);
        }

        template <class FN>
        auto inspect_plugin_bind_async_accumulate(registration_on on, FN&& fn) const {
            return inspect_plugin_bind_future_accumulate(on, std::forward<FN>(fn)).take();
        }

        plugin_registration_ptr get_bind_plugin(registration_on on, const std::string& channel) const;
        plugin_registration_ptr get_bind_cookies(registration_on on, const std::string& cookie_id) const;
        void register_plugin(plugin_registration_ptr plugin);
        plugin_registration_ptr get_plugin(const std::string& name) const;

        template <class Plugin>
        std::shared_ptr<Plugin> request_plugin(const std::string& name) const {
            static_assert(std::is_base_of<plugin_registration, Plugin>::value, "Plugin must derive from plugin_registration");
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
        std::shared_ptr<Plugin> request_plugin() const {
            static_assert(std::is_base_of<plugin_registration, Plugin>::value, "Plugin must derive from plugin_registration");
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

        void unload_plugin(const std::string& name);
        list_array<plugin_registration_ptr> registered_plugins() const;
        //pass empty to register all
        void ecs_registrators(const list_array<std::string>& names, api::ecs::scheduler& sched) const;
        void auto_register();
        void call_initialization();
        void call_load();
        void call_unload();
        void call_fault_unload();
        void unregister_all();
    };

    template <class Self, bool>
    class plugin_handling_fixer {};

    template <class Self>
    struct plugin_handling_fixer<Self, true> : public plugin_registration {
        bool on_configuration_got_known_packs(base_objects::shared_client_data&, const api::packets::server_bound::config::select_known_packs&) override {
            return false;
        }
    };

    template <class Self>
    struct plugin_handling_fixer<Self, false> : public plugin_registration {
    };

    template <class Self>
        struct plugin_handling_fix : public plugin_handling_fixer < Self,
        requires {
        &Self::on_configuration != &plugin_registration::on_configuration && &Self::on_configuration_got_known_packs == &plugin_registration::on_configuration_got_known_packs;
    }>{};

    template <util::CTS name, class Self>
    class plugin_auto_register : public plugin_handling_fix<Self> {
    public:
#if defined(__GNUC__) || defined(__clang__)
        __attribute__((constructor, used)) static void ___perform_auto_registration() {
            __internal__::register_value<Self, name>();
        }

        static inline const std::string registered_name = name.data;
#else
        static inline const std::string registered_name = []() {
            __internal__::register_value<Self, name>();
            return name.data;
        }();
#endif
        const std::string& get_name() const override final {
            return registered_name;
        }

        struct log {
            static inline void info(std::string_view message) {
                __internal__::info(registered_name, message);
            }

            static inline void error(std::string_view message) {
                __internal__::error(registered_name, message);
            }

            static inline void warn(std::string_view message) {
                __internal__::warn(registered_name, message);
            }

            static inline void debug(std::string_view message) {
                __internal__::debug(registered_name, message);
            }

            static inline void debug_error(std::string_view message) {
                __internal__::debug_error(registered_name, message);
            }

            static inline void fatal(std::string_view message) {
                __internal__::fatal(registered_name, message);
            }
        };

        virtual ~plugin_auto_register() noexcept {}
    };

    extern plugin_management_system plugin_management;
}
#endif /* SRC_PLUGIN_MAIN */
