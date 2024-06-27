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

            bool has_rights(const std::string& action_name, const base_objects::client_data_holder& client);
            bool has_action(const std::string& action_name);
            bool has_permissions(const std::string& action_name);


            void add_requirement(const std::string& action_name, const std::string& permission_tag);
            void remove_requirement(const std::string& action_name, const std::string& permission_tag);

            void add_permission(base_objects::permissions_object permission);
            void remove_permission(const std::string& permission_tag);

            void enum_actions(const std::function<void(const std::string&)>& callback);
            void enum_action_requirements(const std::string& action_name, const std::function<void(const std::string&)>& callback);
            void enum_permissions(const std::function<void(const std::string&)>& callback);

            extern base_objects::event<event_data<std::string>> on_action;
            extern base_objects::event<event_data<std::string>> on_command_action;
        };
    }
}

#endif /* SRC_API_PERMISSIONS */
