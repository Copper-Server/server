#ifndef SRC_STORAGE_PERMISSIONS_MANAGER
#define SRC_STORAGE_PERMISSIONS_MANAGER
#include "../base_objects/permissions.hpp"
#include "../base_objects/shared_client_data.hpp"
#include "../library/fast_task.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>

namespace crafted_craft {

    namespace storage {
        class permissions_manager {
            std::filesystem::path base_path;

            struct protected_values_t {
                std::unordered_map<std::string, list_array<std::string>> actions;
                std::unordered_map<std::string, base_objects::permissions_object> permissions;
            };

            fast_task::protected_value<protected_values_t> protected_values;

        public:
            permissions_manager(const std::filesystem::path& base_path);
            bool has_rights(const std::string& action_name, const base_objects::client_data_holder& client);
            bool has_action(const std::string& action_name);
            bool has_permissions(const std::string& action_name);


            void register_action(const std::string& action_name);
            void register_action(const std::string& action_name, const list_array<std::string>& required_perm);
            void register_action(const std::string& action_name, list_array<std::string>&& required_perm);


            void unregister_action(const std::string& action_name);
            void add_requirement(const std::string& action_name, const std::string& permission_tag);
            void remove_requirement(const std::string& action_name, const std::string& permission_tag);

            void add_permission(base_objects::permissions_object permission);
            void remove_permission(const std::string& permission_tag);

            void enum_actions(const std::function<void(const std::string&)>& callback);
            void enum_action_requirements(const std::string& action_name, const std::function<void(const std::string&)>& callback);
            void enum_permissions(const std::function<void(const std::string&)>& callback);

            void sync();
        };
    }
}
#endif /* SRC_STORAGE_PERMISSIONS_MANAGER */
