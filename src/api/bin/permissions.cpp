#include <src/base_objects/permissions.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/storage/permissions_manager.hpp>

namespace copper_server::api::permissions {
    storage::permissions_manager* perm = nullptr;

    void init_permissions(storage::permissions_manager& manager) {
        perm = &manager;
    }

    bool has_rights(const std::string& action_name, const base_objects::SharedClientData& client) {
        if (perm == nullptr)
            return true;
        return perm->has_rights(action_name, client);
    }

    bool has_action(const std::string& action_name) {
        if (perm == nullptr)
            return true;
        return perm->has_action(action_name);
    }

    bool has_action_limits(const std::string& action_name) {
        if (perm == nullptr)
            return true;
        return perm->has_action_limits(action_name);
    }

    bool has_permission(const std::string& permission_name) {
        if (perm == nullptr)
            return true;
        return perm->has_permission(permission_name);
    }

    bool has_group(const std::string& group_name) {
        if (perm == nullptr)
            return true;
        return perm->has_group(group_name);
    }

    void register_action(const std::string& action_name) {
        if (perm == nullptr)
            return;
        perm->register_action(action_name);
    }

    void register_action(const std::string& action_name, const list_array<std::string>& required_perm) {
        if (perm == nullptr)
            return;
        perm->register_action(action_name, required_perm);
    }

    void register_action(const std::string& action_name, list_array<std::string>&& required_perm) {
        if (perm == nullptr)
            return;
        perm->register_action(action_name, std::move(required_perm));
    }

    void unregister_action(const std::string& action_name) {
        if (perm == nullptr)
            return;
        perm->unregister_action(action_name);
    }

    void add_requirement(const std::string& action_name, const std::string& permission_tag) {
        if (perm == nullptr)
            return;
        perm->add_requirement(action_name, permission_tag);
    }

    void remove_requirement(const std::string& action_name, const std::string& permission_tag) {
        if (perm == nullptr)
            return;
        perm->remove_requirement(action_name, permission_tag);
    }

    void add_permission(base_objects::permissions_object permission) {
        if (perm == nullptr)
            return;
        perm->add_permission(permission);
    }

    void remove_permission(const std::string& permission_tag) {
        if (perm == nullptr)
            return;
        perm->remove_permission(permission_tag);
    }

    bool is_in_group(const std::string& group_name, const base_objects::SharedClientData& client) {
        if (perm == nullptr)
            return true;
        return perm->is_in_group(group_name, client);
    }
    
    void add_group(const base_objects::permission_group& group) {
        if (perm == nullptr)
            return;
        perm->add_group(group);
    }

    void remove_group(const std::string& group_name) {
        if (perm == nullptr)
            return;
        perm->remove_group(group_name);
    }

    //permission_tags can also accept direct actions name with prefix 'action:'
    void add_group_value(const std::string& group_name, const std::string& permission_tag) {
        if (perm == nullptr)
            return;
        perm->add_group_value(group_name, permission_tag);
    }

    //permission_tags can also accept direct actions name with prefix 'action:'
    void remove_group_value(const std::string& group_name, const std::string& permission_tag) {
        if (perm == nullptr)
            return;
        perm->remove_group_value(group_name, permission_tag);
    }

    void enum_actions(const std::function<void(const std::string&)>& callback) {
        if (perm == nullptr)
            return;
        perm->enum_actions(callback);
    }

    void enum_actions(const std::function<void(const std::string&, const list_array<std::string>&)>& callback) {
        if (perm == nullptr)
            return;
        perm->enum_actions(callback);
    }

    void enum_action_requirements(const std::string& action_name, const std::function<void(const std::string&)>& callback) {
        if (perm == nullptr)
            return;
        perm->enum_action_requirements(action_name, callback);
    }

    void enum_permissions(const std::function<void(const base_objects::permissions_object&)>& callback) {
        if (perm == nullptr)
            return;
        perm->enum_permissions(callback);
    }

    void enum_groups(const std::function<void(const base_objects::permission_group&)>& callback) {
        if (perm == nullptr)
            return;
        perm->enum_groups(callback);
    }

    void enum_group_values(const std::string& group_name, const std::function<void(const std::string&)>& callback) {
        if (perm == nullptr)
            return;
        perm->enum_group_values(group_name, callback);
    }

    void view_permission(const std::string& perm_name, const std::function<void(const base_objects::permissions_object&)>& callback) {
        if (perm == nullptr)
            return;
        perm->view_permission(perm_name, callback);
    }

    void view_group(const std::string& group_name, const std::function<void(const base_objects::permission_group&)>& callback) {
        if (perm == nullptr)
            return;
        perm->view_group(group_name, callback);
    }

    void set_check_mode(storage::permissions_manager::permission_check_mode mode) {
        if (perm == nullptr)
            return;
        perm->set_check_mode(mode);
    }

    storage::permissions_manager::permission_check_mode get_check_mode() {
        if (perm == nullptr)
            return storage::permissions_manager::permission_check_mode::all_or_noting;
        return perm->get_check_mode();
    }

    void make_sync() {
        if (perm == nullptr)
            return;
        perm->sync();
    }
}