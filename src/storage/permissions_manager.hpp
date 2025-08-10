/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_STORAGE_PERMISSIONS_MANAGER
#define SRC_STORAGE_PERMISSIONS_MANAGER
#include <filesystem>
#include <library/fast_task.hpp>
#include <src/base_objects/permissions.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <string>
#include <unordered_map>

namespace copper_server::storage {
    class permissions_manager {
    public:
        enum class permission_check_mode {
            all,           //every permission will be checked, if it is not present then check failed
            any,           //if at least one permission is present then check passed
            all_or_noting, //same as all, but when requirements empty then check passed
            any_or_noting  //same as any, but when requirements empty then check passed
        };

    private:
        std::filesystem::path base_path;

        struct protected_values_t {
            std::unordered_map<std::string, list_array<std::string>> actions;
            std::unordered_map<std::string, base_objects::permissions_object> permissions;
            std::unordered_map<std::string, base_objects::permission_group> permissions_group;
            permission_check_mode check_mode = permission_check_mode::all_or_noting;
            bool check_mode_changed = false;
        };

        fast_task::protected_value<protected_values_t> protected_values;

    public:
        permissions_manager(const std::filesystem::path& base_path);
        bool has_rights(const std::string& action_name, const base_objects::SharedClientData& client);
        bool has_action(const std::string& action_name) const;
        bool has_action_limits(const std::string& action_name) const;
        bool has_permission(const std::string& permission_name) const;
        bool has_group(const std::string& group_name) const;


        void register_action(const std::string& action_name);
        void register_action(const std::string& action_name, const list_array<std::string>& required_perm);
        void register_action(const std::string& action_name, list_array<std::string>&& required_perm);


        void unregister_action(const std::string& action_name);
        void add_requirement(const std::string& action_name, const std::string& permission_tag);
        void remove_requirement(const std::string& action_name, const std::string& permission_tag);

        void add_permission(base_objects::permissions_object permission);
        void remove_permission(const std::string& permission_tag);

        bool is_in_group(const std::string& group_name, const base_objects::SharedClientData& client);
        void add_group(const base_objects::permission_group& group);
        void remove_group(const std::string& group_name);
        //permission_tags can also accept direct actions name with prefix 'action:'
        void add_group_value(const std::string& group_name, const std::string& permission_tag);
        //permission_tags can also accept direct actions name with prefix 'action:'
        void remove_group_value(const std::string& group_name, const std::string& permission_tag);

        void enum_actions(const std::function<void(const std::string&)>& callback) const;
        void enum_actions(const std::function<void(const std::string&, const list_array<std::string>&)>& callback) const;
        void enum_action_requirements(const std::string& action_name, const std::function<void(const std::string&)>& callback) const;
        void enum_permissions(const std::function<void(const base_objects::permissions_object&)>& callback) const;
        void enum_groups(const std::function<void(const base_objects::permission_group&)>& callback) const;
        void enum_group_values(const std::string& group_name, const std::function<void(const std::string&)>& callback) const;

        void view_permission(const std::string& perm_name, const std::function<void(const base_objects::permissions_object&)>& callback) const;
        void view_group(const std::string& group_name, const std::function<void(const base_objects::permission_group&)>& callback) const;

        void set_check_mode(permissions_manager::permission_check_mode mode);
        permission_check_mode get_check_mode() const;
        void sync();
    };
}
#endif /* SRC_STORAGE_PERMISSIONS_MANAGER */
