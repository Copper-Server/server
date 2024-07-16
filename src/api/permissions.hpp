#ifndef SRC_API_PERMISSIONS
#define SRC_API_PERMISSIONS
#include "../base_objects/event.hpp"
#include "../base_objects/shared_client_data.hpp"
#include "../storage/permissions_manager.hpp"

namespace crafted_craft {
    namespace api {
        namespace permissions {
            template <class T>
            struct event_data {
                T data;
                base_objects::client_data_holder client_data;
            };


            void init_permissions(storage::permissions_manager& manager);


            bool has_rights(const base_objects::shared_string& action_name, const base_objects::client_data_holder& client);
            bool has_action(const base_objects::shared_string& action_name);
            bool has_permission(const base_objects::shared_string& permission_name);
            bool has_group(const base_objects::shared_string& group_name);


            void register_action(const base_objects::shared_string& action_name);
            void register_action(const base_objects::shared_string& action_name, const list_array<base_objects::shared_string>& required_perm);
            void register_action(const base_objects::shared_string& action_name, list_array<base_objects::shared_string>&& required_perm);


            void unregister_action(const base_objects::shared_string& action_name);
            void add_requirement(const base_objects::shared_string& action_name, const base_objects::shared_string& permission_tag);
            void remove_requirement(const base_objects::shared_string& action_name, const base_objects::shared_string& permission_tag);

            void add_permission(base_objects::permissions_object permission);
            void remove_permission(const base_objects::shared_string& permission_tag);

            void add_group(const base_objects::permission_group& group);
            void remove_group(const base_objects::shared_string& group_name);
            //permission_tags can also accept direct actions name with prefix 'action:'
            void add_group_value(const base_objects::shared_string& group_name, const base_objects::shared_string& permission_tag);
            //permission_tags can also accept direct actions name with prefix 'action:'
            void remove_group_value(const base_objects::shared_string& group_name, const base_objects::shared_string& permission_tag);

            void enum_actions(const std::function<void(const base_objects::shared_string&)>& callback);
            void enum_actions(const std::function<void(const base_objects::shared_string&, const list_array<base_objects::shared_string>&)>& callback);
            void enum_action_requirements(const base_objects::shared_string& action_name, const std::function<void(const base_objects::shared_string&)>& callback);
            void enum_permissions(const std::function<void(const base_objects::permissions_object&)>& callback);
            void enum_groups(const std::function<void(const base_objects::permission_group&)>& callback);
            void enum_group_values(const base_objects::shared_string& group_name, const std::function<void(const base_objects::shared_string&)>& callback);


            void view_permission(const base_objects::shared_string& perm_name, const std::function<void(const base_objects::permissions_object&)>& callback);
            void view_group(const base_objects::shared_string& group_name, const std::function<void(const base_objects::permission_group&)>& callback);

            void set_check_mode(storage::permissions_manager::permission_check_mode mode);
            storage::permissions_manager::permission_check_mode get_check_mode();
            void make_sync();
        };
    }
}

#endif /* SRC_API_PERMISSIONS */
