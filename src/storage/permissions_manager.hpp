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
                std::unordered_map<base_objects::shared_string, list_array<base_objects::shared_string>> actions;
                std::unordered_map<base_objects::shared_string, base_objects::permissions_object> permissions;
                std::unordered_map<base_objects::shared_string, base_objects::permission_group> permissions_group;
                permission_check_mode check_mode = permission_check_mode::all_or_noting;
                bool check_mode_changed = false;
            };

            fast_task::protected_value<protected_values_t> protected_values;

        public:
            permissions_manager(const std::filesystem::path& base_path);
            bool has_rights(const base_objects::shared_string& action_name, const base_objects::client_data_holder& client);
            bool has_action(const base_objects::shared_string& action_name) const;
            bool has_permission(const base_objects::shared_string& permission_name) const;
            bool has_group(const base_objects::shared_string& group_name) const;


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

            void enum_actions(const std::function<void(const base_objects::shared_string&)>& callback) const;
            void enum_actions(const std::function<void(const base_objects::shared_string&, const list_array<base_objects::shared_string>&)>& callback) const;
            void enum_action_requirements(const base_objects::shared_string& action_name, const std::function<void(const base_objects::shared_string&)>& callback) const;
            void enum_permissions(const std::function<void(const base_objects::permissions_object&)>& callback) const;
            void enum_groups(const std::function<void(const base_objects::permission_group&)>& callback) const;
            void enum_group_values(const base_objects::shared_string& group_name, const std::function<void(const base_objects::shared_string&)>& callback) const;

            void view_permission(const base_objects::shared_string& perm_name, const std::function<void(const base_objects::permissions_object&)>& callback) const;
            void view_group(const base_objects::shared_string& group_name, const std::function<void(const base_objects::permission_group&)>& callback) const;

            void set_check_mode(permissions_manager::permission_check_mode mode);
            permission_check_mode get_check_mode() const;
            void sync();
        };
    }
}
#endif /* SRC_STORAGE_PERMISSIONS_MANAGER */
