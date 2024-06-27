#include "../base_objects/permissions.hpp"
#include "../base_objects/shared_client_data.hpp"
#include "../storage/permissions_manager.hpp"

namespace crafted_craft {
    namespace api {
        namespace permissions {
            storage::permissions_manager* perm = nullptr;

            void init_permissions(storage::permissions_manager& manager) {
                perm = &manager;
            }

            bool has_rights(const std::string& action_name, const base_objects::client_data_holder& client) {
                if (perm == nullptr)
                    return false;
                return perm->has_rights(action_name, client);
            }

            bool has_action(const std::string& action_name) {
                if (perm == nullptr)
                    return false;
                return perm->has_action(action_name);
            }

            bool has_permissions(const std::string& action_name) {
                if (perm == nullptr)
                    return false;
                return perm->has_permissions(action_name);
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

            void enum_actions(const std::function<void(const std::string&)>& callback) {
                if (perm == nullptr)
                    return;
                perm->enum_actions(callback);
            }

            void enum_action_requirements(const std::string& action_name, const std::function<void(const std::string&)>& callback) {
                if (perm == nullptr)
                    return;
                perm->enum_action_requirements(action_name, callback);
            }

            void enum_permissions(const std::function<void(const std::string&)>& callback) {
                if (perm == nullptr)
                    return;
                perm->enum_permissions(callback);
            }
        };
    }
}