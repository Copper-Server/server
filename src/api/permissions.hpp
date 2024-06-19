#ifndef SRC_API_PERMISSIONS
#define SRC_API_PERMISSIONS
#include "../base_objects/shared_client_data.hpp"
#include "../storage/permissions_manager.hpp"

namespace crafted_craft {
    namespace api {
        namespace permissions {
            void init_permissions(storage::permissions_manager& manager);

            bool has_rights(const std::string& action_name, const base_objects::client_data_holder& client);


            void add_requirement(const std::string& action_name, const std::string& permission_tag);
            void remove_requirement(const std::string& action_name, const std::string& permission_tag);

            void add_permission(base_objects::permissions_object permission);
            void remove_permission(const std::string& permission_tag);
        };
    }
}

#endif /* SRC_API_PERMISSIONS */
