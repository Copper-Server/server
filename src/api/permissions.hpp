#ifndef SRC_API_PERMISSIONS
#define SRC_API_PERMISSIONS
#include <src/base_objects/event.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/storage/permissions_manager.hpp>

namespace copper_server::api::permissions {
    template <class T>
    struct event_data {
        T data;
        base_objects::client_data_holder client_data;
    };

    bool has_rights(const std::string& action_name, const base_objects::client_data_holder& client);
    bool has_action(const std::string& action_name);
    bool has_permission(const std::string& permission_name);
    bool has_group(const std::string& group_name);


    void register_action(const std::string& action_name);
    void register_action(const std::string& action_name, const list_array<std::string>& required_perm);
    void register_action(const std::string& action_name, list_array<std::string>&& required_perm);


    void unregister_action(const std::string& action_name);
    void add_requirement(const std::string& action_name, const std::string& permission_tag);
    void remove_requirement(const std::string& action_name, const std::string& permission_tag);

    void add_permission(base_objects::permissions_object permission);
    void remove_permission(const std::string& permission_tag);

    void add_group(const base_objects::permission_group& group);
    void remove_group(const std::string& group_name);
    //permission_tags can also accept direct actions name with prefix 'action:'
    void add_group_value(const std::string& group_name, const std::string& permission_tag);
    //permission_tags can also accept direct actions name with prefix 'action:'
    void remove_group_value(const std::string& group_name, const std::string& permission_tag);

    void enum_actions(const std::function<void(const std::string&)>& callback);
    void enum_actions(const std::function<void(const std::string&, const list_array<std::string>&)>& callback);
    void enum_action_requirements(const std::string& action_name, const std::function<void(const std::string&)>& callback);
    void enum_permissions(const std::function<void(const base_objects::permissions_object&)>& callback);
    void enum_groups(const std::function<void(const base_objects::permission_group&)>& callback);
    void enum_group_values(const std::string& group_name, const std::function<void(const std::string&)>& callback);


    void view_permission(const std::string& perm_name, const std::function<void(const base_objects::permissions_object&)>& callback);
    void view_group(const std::string& group_name, const std::function<void(const base_objects::permission_group&)>& callback);

    void set_check_mode(storage::permissions_manager::permission_check_mode mode);
    storage::permissions_manager::permission_check_mode get_check_mode();
    void make_sync();
};

#endif /* SRC_API_PERMISSIONS */
