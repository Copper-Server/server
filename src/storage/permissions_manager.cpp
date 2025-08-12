/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/json.hpp>
#include <format>
#include <library/fast_task/include/files.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/log.hpp>
#include <src/storage/permissions_manager.hpp>
#include <src/util/json_helpers.hpp>

namespace copper_server::storage {
    permissions_manager::permissions_manager(const std::filesystem::path& base_path)
        : base_path(base_path) {}

    bool permissions_manager::has_rights(const std::string& action_name, const base_objects::SharedClientData& client) {
        return protected_values.get([&](const protected_values_t& values) {
            if (client.player_data.instant_granted_actions.find(action_name) != client.player_data.instant_granted_actions.npos)
                return true;

            bool pass_if_noting = values.check_mode == permission_check_mode::all_or_noting || values.check_mode == permission_check_mode::any_or_noting;

            auto item = values.actions.find(action_name);
            if (item == values.actions.end())
                return pass_if_noting;

            auto& client_data = client.player_data;
            bool instant_granted = false;
            bool has_not_found = false;
            bool is_incompatible = false;
            bool is_compatible = false;

            bool check_all = values.check_mode == permission_check_mode::all || values.check_mode == permission_check_mode::all_or_noting;
            bool pass_any = values.check_mode == permission_check_mode::any || values.check_mode == permission_check_mode::any_or_noting;


            item->second.for_each([&](const std::string& tag) {
                if (is_incompatible || is_compatible)
                    return;
                auto permission = values.permissions.find(tag);
                if (permission == values.permissions.end())
                    return;

                auto& perm = permission->second;
                auto client_perm = client_data.permissions.find(tag);
                if (!perm.reverse_mode) {
                    if (client_data.permissions.npos == client_perm && perm.permission_level == -1) {
                        if (perm.important || check_all)
                            is_incompatible = true;
                        else
                            has_not_found = true;
                        return;
                    } else if (client_data.permissions.npos != client_perm && perm.permission_level != -1) {
                        if (perm.permission_level > client_data.op_level) {
                            if (perm.important || check_all)
                                is_incompatible = true;
                            else
                                has_not_found = true;
                            return;
                        }
                    }
                    if (pass_any)
                        is_compatible = true;
                    else
                        instant_granted |= perm.instant_grant;
                } else {
                    if (client_data.permissions.npos != client_perm && perm.permission_level == -1) {
                        if (perm.important || check_all)
                            is_incompatible = true;
                        else
                            has_not_found = true;
                        return;
                    } else if (client_data.permissions.npos != client_perm && perm.permission_level != -1) {
                        if (perm.permission_level <= client_data.op_level) {
                            if (perm.important || check_all)
                                is_incompatible = true;
                            else
                                has_not_found = true;
                            return;
                        }
                    }
                    if (pass_any)
                        is_compatible = true;
                    else
                        instant_granted |= perm.instant_grant;
                }
            });

            if (item->second.empty() && pass_if_noting)
                return true;
            else
                return is_compatible || (!is_incompatible && (!has_not_found || instant_granted));
        });
    }

    bool permissions_manager::has_action(const std::string& action_name) const {
        return protected_values.get([&](const protected_values_t& values) {
            return values.actions.contains(action_name);
        });
    }

    bool permissions_manager::has_action_limits(const std::string& action_name) const {
        return protected_values.get([&](const protected_values_t& values) {
            auto item = values.actions.find(action_name);
            if (item == values.actions.end())
                return !(values.check_mode == permission_check_mode::all_or_noting || values.check_mode == permission_check_mode::any_or_noting);
            else
                return !item->second.empty();
        });
    }

    bool permissions_manager::has_permission(const std::string& permission_name) const {
        return protected_values.get([&](const protected_values_t& values) {
            return values.permissions.contains(permission_name);
        });
    }

    bool permissions_manager::is_in_group(const std::string& group_name, const base_objects::SharedClientData& client) {
        return client.player_data.permission_groups.contains(group_name);
    }

    bool permissions_manager::has_group(const std::string& group_name) const {
        return protected_values.get([&](const protected_values_t& values) {
            return values.permissions.contains(group_name);
        });
    }

    void permissions_manager::register_action(const std::string& action_name) {
        protected_values.set([&](protected_values_t& values) {
            auto it = values.actions.find(action_name);
            if (it != values.actions.end())
                throw std::runtime_error("This action already registered.");
            values.actions[action_name].clear();
        });
    }

    void permissions_manager::register_action(const std::string& action_name, const list_array<std::string>& required_perm) {
        protected_values.set([&](protected_values_t& values) {
            auto it = values.actions.find(action_name);
            if (it != values.actions.end())
                throw std::runtime_error("This action already registered.");
            values.actions[action_name] = required_perm;
        });
    }

    void permissions_manager::register_action(const std::string& action_name, list_array<std::string>&& required_perm) {
        protected_values.set([&](protected_values_t& values) {
            auto it = values.actions.find(action_name);
            if (it != values.actions.end())
                throw std::runtime_error("This action already registered.");
            values.actions[action_name] = std::move(required_perm);
        });
    }

    void permissions_manager::unregister_action(const std::string& action_name) {
        protected_values.set([&](protected_values_t& values) {
            auto it = values.actions.find(action_name);
            if (it == values.actions.end())
                throw std::runtime_error("This action already unregistered.");
            values.actions.erase(it);
        });
    }

    void permissions_manager::add_requirement(const std::string& action_name, const std::string& permission_tag) {
        protected_values.set([&](protected_values_t& values) {
            auto& action = values.actions[action_name];
            if (action.find(action_name) == action.npos) {
                action.push_back(permission_tag);
                action.commit();
            }
        });
    }

    void permissions_manager::remove_requirement(const std::string& action_name, const std::string& permission_tag) {
        protected_values.set([&](protected_values_t& values) {
            values.actions[action_name].remove_if([permission_tag](const std::string& tag) {
                return tag == permission_tag;
            });
        });
    }

    void permissions_manager::add_permission(base_objects::permissions_object permission) {
        protected_values.set([&](protected_values_t& values) {
            values.permissions[permission.permission_tag] = std::move(permission);
        });
    }

    void permissions_manager::remove_permission(const std::string& permission_tag) {
        protected_values.set([&](protected_values_t& values) {
            values.permissions.erase(permission_tag);
        });
    }

    void permissions_manager::add_group(const base_objects::permission_group& group) {
        protected_values.set([&](protected_values_t& values) {
            values.permissions_group[group.group_name] = group;
        });
    }

    void permissions_manager::remove_group(const std::string& group_name) {
        protected_values.set([&](protected_values_t& values) {
            values.permissions_group.erase(group_name);
        });
    }

    void permissions_manager::add_group_value(const std::string& group_name, const std::string& permission_tag) {
        protected_values.set([&](protected_values_t& values) {
            auto& ref = values.permissions_group.at(group_name).permissions_tags;
            ref.push_back(permission_tag);
            if (ref.need_commit())
                ref.commit();
        });
    }

    void permissions_manager::remove_group_value(const std::string& group_name, const std::string& permission_tag) {
        protected_values.set([&](protected_values_t& values) {
            auto& ref = values.permissions_group.at(group_name).permissions_tags;
            ref.remove_if([permission_tag](const std::string& tag) {
                return tag == permission_tag;
            });
            if (ref.need_commit())
                ref.commit();
        });
    }

    void permissions_manager::enum_actions(const std::function<void(const std::string&)>& callback) const {
        return protected_values.get([&](const protected_values_t& values) {
            for (auto&& [action, data] : values.actions)
                callback(action);
        });
    }

    void permissions_manager::enum_actions(const std::function<void(const std::string&, const list_array<std::string>&)>& callback) const {
        return protected_values.get([&](const protected_values_t& values) {
            for (auto&& [action, data] : values.actions)
                callback(action, data);
        });
    }

    void permissions_manager::enum_action_requirements(const std::string& action_name, const std::function<void(const std::string&)>& callback) const {
        return protected_values.get([&](const protected_values_t& values) {
            auto item = values.actions.find(action_name);
            if (item == values.actions.end())
                throw std::runtime_error("This action not registered.");
            item->second.for_each(callback);
        });
    }

    void permissions_manager::enum_permissions(const std::function<void(const base_objects::permissions_object&)>& callback) const {
        return protected_values.get([&](const protected_values_t& values) {
            for (auto&& [perm, data] : values.permissions)
                callback(data);
        });
    }

    void permissions_manager::enum_groups(const std::function<void(const base_objects::permission_group&)>& callback) const {
        return protected_values.get([&](const protected_values_t& values) {
            for (auto&& [perm, data] : values.permissions_group)
                callback(data);
        });
    }

    void permissions_manager::enum_group_values(const std::string& group_name, const std::function<void(const std::string&)>& callback) const {
        return protected_values.get([&](const protected_values_t& values) {
            auto item = values.permissions_group.find(group_name);
            if (item == values.permissions_group.end())
                throw std::runtime_error("This group not registered.");
            item->second.permissions_tags.for_each(callback);
        });
    }

    void permissions_manager::view_permission(const std::string& perm_name, const std::function<void(const base_objects::permissions_object&)>& callback) const {
        return protected_values.get([&](const protected_values_t& values) {
            auto item = values.permissions.find(perm_name);
            if (item == values.permissions.end())
                throw std::runtime_error("This group not registered.");
            callback(item->second);
        });
    }

    void permissions_manager::view_group(const std::string& group_name, const std::function<void(const base_objects::permission_group&)>& callback) const {
        return protected_values.get([&](const protected_values_t& values) {
            auto item = values.permissions_group.find(group_name);
            if (item == values.permissions_group.end())
                throw std::runtime_error("This group not registered.");
            callback(item->second);
        });
    }

    void permissions_manager::set_check_mode(permissions_manager::permission_check_mode mode) {
        protected_values.set([&](protected_values_t& values) {
            values.check_mode = mode;
            values.check_mode_changed = true;
            return values;
        });
    }

    permissions_manager::permission_check_mode permissions_manager::get_check_mode() const {
        return protected_values.get([&](const protected_values_t& values) {
            return values.check_mode;
        });
    }

    std::string from_check_mode(permissions_manager::permission_check_mode mode) {
        switch (mode) {
        case permissions_manager::permission_check_mode::all:
            return "all";
        case permissions_manager::permission_check_mode::any:
            return "any";
        case permissions_manager::permission_check_mode::all_or_noting:
            return "all_or_noting";
        case permissions_manager::permission_check_mode::any_or_noting:
            return "any_or_noting";
        default:
            return "unknown";
        }
    }

    permissions_manager::permission_check_mode to_check_mode(std::string_view mode) {
        if (mode == "all")
            return permissions_manager::permission_check_mode::all;
        if (mode == "any")
            return permissions_manager::permission_check_mode::any;
        if (mode == "all_or_noting")
            return permissions_manager::permission_check_mode::all_or_noting;
        if (mode == "any_or_noting")
            return permissions_manager::permission_check_mode::any_or_noting;
        return permissions_manager::permission_check_mode::all_or_noting;
    }


    void permissions_manager::sync() {
        using namespace util;

        auto config_holder = try_read_json_file(base_path);
        if (!config_holder.has_value()) {
            log::error("server", "Failed to load permissions file");
            return;
        }
        auto root = js_object::get_object(*config_holder);
        protected_values.set([&](protected_values_t& values) {
            {
                auto check_mode_obj = root["check_mode"];
                if (values.check_mode_changed)
                    check_mode_obj = from_check_mode(values.check_mode);
                else
                    values.check_mode = to_check_mode((boost::json::string)check_mode_obj.or_apply("all_or_noting"));
                values.check_mode_changed = false;
            }
            {
                auto actions_obj = js_object::get_object(root["actions"]);
                if (actions_obj.empty() && values.permissions.empty()) {
                    values.permissions["operator_1"] = base_objects::permissions_object{
                        .permission_tag = "operator_1",
                        .description = "The default permission tag for operator level 1.",
                        .permission_level = 1,
                        .instant_grant = false
                    };
                    values.permissions["operator_2"] = base_objects::permissions_object{
                        .permission_tag = "operator_2",
                        .description = "The default permission tag for operator level 2.",
                        .permission_level = 2,
                        .instant_grant = false
                    };
                    values.permissions["operator_3"] = base_objects::permissions_object{
                        .permission_tag = "operator_3",
                        .description = "The default permission tag for operator level 3.",
                        .permission_level = 3,
                        .instant_grant = false
                    };
                    values.permissions["operator_4"] = base_objects::permissions_object{
                        .permission_tag = "operator_4",
                        .description = "The default permission tag for operator level 4.",
                        .permission_level = 4,
                        .instant_grant = false
                    };
                    values.permissions["console"] = base_objects::permissions_object{
                        .permission_tag = "console",
                        .description = "The default permission tag for console commands.",
                        .permission_level = 5,
                        .instant_grant = false
                    };
                }

                list_array<std::string> declared_actions;
                declared_actions.reserve(actions_obj.size());
                for (auto&& [key, value] : actions_obj) {
                    std::string shared_tag(key.data(), key.size());
                    auto& read = values.actions[shared_tag];
                    for (auto item : js_array::get_array(value))
                        read.push_back((std::string)item);
                    read.commit();
                    declared_actions.push_back(std::move(shared_tag));
                }
                for (auto&& [key, value] : values.actions) {
                    if (!declared_actions.contains(key)) {
                        boost::json::array arr;
                        arr.reserve(value.size());
                        for (auto& item : value)
                            arr.push_back(boost::json::string(item));
                        actions_obj[key] = std::move(arr);
                    }
                }
            }
            {
                list_array<std::string> declared_permissions;
                auto permissions_obj = js_object::get_object(root["permissions"]);
                for (auto&& [permission_tag, value] : permissions_obj) {
                    auto perm = js_object::get_object(value);
                    std::string description = perm["description"].or_apply("");
                    int8_t permission_level = perm["permission_level"].or_apply(0);
                    bool instant_grant = perm["instant_grant"].or_apply(false);
                    bool reverse_mode = perm["reverse_mode"].or_apply(false);
                    bool important = perm["important"].or_apply(false);

                    std::string shared_tag(permission_tag.data(), permission_tag.size());
                    values.permissions[shared_tag] = base_objects::permissions_object(
                        {permission_tag.data(), permission_tag.size()},
                        std::move(description),
                        permission_level,
                        instant_grant,
                        reverse_mode,
                        important
                    );
                    declared_permissions.push_back(shared_tag);
                }
                for (auto&& [permission_tag, value] : values.permissions) {
                    if (!declared_permissions.contains(permission_tag)) {
                        permissions_obj[permission_tag]
                            = value.description.empty()
                                  ? boost::json::object{
                                        {"instant_grant", (bool)value.instant_grant},
                                        {"permission_level", (bool)value.permission_level},
                                        {"instant_grant", (bool)value.instant_grant},
                                        {"reverse_mode", (bool)value.reverse_mode},
                                        {"important", (bool)value.important},
                                    }
                                  : boost::json::object{
                                        {"description", value.description},
                                        {"permission_level", value.permission_level},
                                        {"instant_grant", (bool)value.instant_grant},
                                        {"reverse_mode", (bool)value.reverse_mode},
                                        {"important", (bool)value.important},
                                    };
                    }
                }
            }
            {
                auto group_obj = js_object::get_object(root["permission_group"]);
                if (group_obj.empty() && values.permissions_group.empty()) {
                    values.permissions_group["operator"] = base_objects::permission_group{
                        .group_name = "operator",
                        .permissions_tags = {
                            "operator_4",
                            "operator_3",
                            "operator_2",
                            "operator_1",
                        }
                    };
                    values.permissions_group["console"] = base_objects::permission_group{
                        .group_name = "console",
                        .permissions_tags = {
                            "console",
                            "operator_4",
                            "operator_3",
                            "operator_2",
                            "operator_1",
                        }
                    };
                }


                list_array<std::string> declared_groups;
                declared_groups.reserve(group_obj.size());
                for (auto&& [group_tag, value] : group_obj) {
                    auto group_list = js_array::get_array(value);
                    list_array<std::string> perm_tags;
                    perm_tags.reserve(group_list.size());
                    for (auto item : group_list) {
                        auto& res = (boost::json::string&)item;
                        perm_tags.push_back({res.data(), res.size()});
                    }
                    std::string shared_tag(group_tag.data(), group_tag.size());
                    values.permissions_group[shared_tag] = base_objects::permission_group(
                        shared_tag,
                        std::move(perm_tags)
                    );
                    declared_groups.push_back(shared_tag);
                }
                for (auto&& [group_tag, value] : values.permissions_group) {
                    if (!declared_groups.contains(group_tag)) {
                        boost::json::array arr;
                        arr.reserve(value.permissions_tags.size());
                        for (auto& it : value.permissions_tags)
                            arr.push_back(boost::json::string(it));
                        group_obj[group_tag] = std::move(arr);
                    }
                }
            }
            return values;
        });
        {
            fast_task::files::async_iofstream file(
                base_path,
                fast_task::files::open_mode::write,
                fast_task::files::on_open_action::always_new,
                fast_task::files::_sync_flags{}
            );
            if (!file.is_open()) {
                log::warn("server", "Failed to save permissions file. Can not open file.");
                return;
            }
            file << util::pretty_print(*config_holder);
        }
    }
}
