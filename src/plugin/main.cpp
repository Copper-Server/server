/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/configuration.hpp>
#include <src/api/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server {
    void __internal__::info(std::string_view source, std::string_view message) {
        api::log::info(source, message);
    }

    void __internal__::error(std::string_view source, std::string_view message) {
        api::log::error(source, message);
    }

    void __internal__::warn(std::string_view source, std::string_view message) {
        api::log::warn(source, message);
    }

    void __internal__::debug(std::string_view source, std::string_view message) {
        api::log::debug(source, message);
    }

    void __internal__::debug_error(std::string_view source, std::string_view message) {
        api::log::debug_error(source, message);
    }

    void __internal__::fatal(std::string_view source, std::string_view message) {
        api::log::fatal(source, message);
    }

    void unregisterEvery(plugin_registration_ptr& plugin, std::unordered_map<std::string, plugin_registration_ptr>& container) {
        for (
            std::unordered_map<std::string, plugin_registration_ptr>::iterator it = container.begin();
            it != container.end();
            it = std::find_if(container.begin(), container.end(), [&plugin](auto& item) {
                return item.second == plugin;
            })
        )
            container.erase(it);
    }

    void plugin_management_system::protected_values_t::___::unregister(plugin_registration_ptr& plugin) {
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

        ecs_system_providers.erase(plugin->get_name());
    }

    void plugin_management_system::register_plugin_on(plugin_registration_ptr plugin, registration_on on) {
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

    void plugin_management_system::register_plugin_ecs_system(plugin_registration_ptr plugin) {
        protected_values.set([&](protected_values_t& vals) {
            vals.registration.ecs_system_providers[plugin->get_name()] = plugin;
        });
    }

    void plugin_management_system::bind_plugin_on(const std::string& channel, plugin_registration_ptr plugin, registration_on on) {
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

    void plugin_management_system::bind_plugin_cookies_on(const std::string& cookie_id, plugin_registration_ptr plugin, registration_on on) {
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

    plugin_registration_ptr plugin_management_system::get_bind_plugin(registration_on on, const std::string& channel) const {
        return protected_values.get([&](const protected_values_t& vals) -> plugin_registration_ptr {
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

    plugin_registration_ptr plugin_management_system::get_bind_cookies(registration_on on, const std::string& cookie_id) const {
        return protected_values.get([&](const protected_values_t& vals) -> plugin_registration_ptr {
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

    void plugin_management_system::register_plugin(plugin_registration_ptr plugin) {
        if (!plugin)
            return;
        protected_values.set(
            [&](protected_values_t& vals) {
                vals.plugins[plugin->get_name()] = plugin;
            }
        );
        plugin->on_register(plugin);
    }

    plugin_registration_ptr plugin_management_system::get_plugin(const std::string& name) const {
        return protected_values.get(
            [&](const protected_values_t& vals) -> plugin_registration_ptr {
                auto it = vals.plugins.find(name);
                if (it == vals.plugins.end())
                    return nullptr;
                return it->second;
            }
        );
    }

    void plugin_management_system::unload_plugin(const std::string& name) {
        plugin_registration_ptr plugin;
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
        plugin->on_unload(plugin);
        plugin->on_post_unload(plugin);
        plugin->on_unload_complete(plugin);
        plugin->is_loaded = false;
        plugin->clean_up_registered_events();
        plugin->on_unregister(plugin);
    }

    list_array<plugin_registration_ptr> plugin_management_system::registered_plugins() const {
        list_array<plugin_registration_ptr> result;
        return protected_values.get(
            [&](const protected_values_t& vals) {
                result.reserve(vals.plugins.size());
                for (auto& [name, plugin] : vals.plugins)
                    result.push_back(plugin);
                return result;
            }
        );
    }

    //pass empty to register all
    void plugin_management_system::ecs_registrators(const list_array<std::string>& names, api::ecs::scheduler& sched) const {
        protected_values.get(
            [&](const protected_values_t& vals) {
                if (names.empty()) {
                    for (auto& [name, plugin] : vals.registration.ecs_system_providers)
                        plugin->register_systems(sched);
                } else {
                    for (auto& name : names) {
                        auto it = vals.registration.ecs_system_providers.find(name);
                        if (it != vals.registration.ecs_system_providers.end())
                            it->second->register_systems(sched);
                    }
                }
            }
        );
    }

    void plugin_management_system::auto_register() {
        for (auto& [name, plugin] : __internal__::registration_list()) {
            if (!api::configuration::get().disabled_plugins.contains(name))
                register_plugin(plugin->construct());
        }
        __internal__::registration_list().clear();
    }

    void plugin_management_system::call_initialization() {
        std::unordered_map<std::string, plugin_registration_ptr> plugins;
        protected_values.get(
            [&](const protected_values_t& vals) {
                plugins = vals.plugins;
            }
        );
        for (auto& [name, plugin] : plugins)
            plugin->on_initialization(plugin);
    }

    void plugin_management_system::call_load() {
        std::unordered_map<std::string, plugin_registration_ptr> plugins;
        protected_values.get(
            [&](const protected_values_t& vals) {
                plugins = vals.plugins;
            }
        );

        fast_task::future_tool::for_each(plugins, [](auto& plugin) {
            if (!plugin.second->is_loaded)
                plugin.second->on_load(plugin.second);
        })->wait();

        fast_task::future_tool::for_each(plugins, [](auto& plugin) {
            if (!plugin.second->is_loaded)
                plugin.second->on_post_load(plugin.second);
        })->wait();

        fast_task::future_tool::for_each(plugins, [](auto& plugin) {
            if (!plugin.second->is_loaded)
                plugin.second->on_load_complete(plugin.second);
            plugin.second->is_loaded = true;
        })->wait();
    }

    void plugin_management_system::call_unload() {
        std::unordered_map<std::string, plugin_registration_ptr> plugins;
        protected_values.get(
            [&](const protected_values_t& vals) {
                plugins = vals.plugins;
            }
        );
        for (auto& [name, plugin] : plugins)
            if (plugin->is_loaded)
                plugin->on_unload(plugin);

        for (auto& [name, plugin] : plugins)
            if (plugin->is_loaded)
                plugin->on_post_unload(plugin);

        for (auto& [name, plugin] : plugins) {
            if (plugin->is_loaded) {
                plugin->on_unload_complete(plugin);
                plugin->clean_up_registered_events();
                plugin->is_loaded = false;
            }
        }
    }

    void plugin_management_system::call_fault_unload() {
        std::unordered_map<std::string, plugin_registration_ptr> plugins;
        protected_values.get(
            [&](const protected_values_t& vals) {
                plugins = vals.plugins;
            }
        );
        for (auto& [name, plugin] : plugins) {
            if (plugin->is_loaded) {
                plugin->is_loaded = false;
                plugin->on_fault_unload(plugin);
                plugin->clean_up_registered_events();
            }
        }
    }

    void plugin_management_system::unregister_all() {
        protected_values.set(
            [&](protected_values_t& vals) {
                vals.plugins.clear();
            }
        );
    }

    plugin_management_system plugin_management;
}