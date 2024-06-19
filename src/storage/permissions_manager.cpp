#include "permissions_manager.hpp"
#include "../base_objects/shared_client_data.hpp"
#include "../log.hpp"
#include "../util/json_helpers.hpp"
#include <boost/json.hpp>
#include <format>
#include <fstream>

namespace crafted_craft {
    namespace storage {
        permissions_manager::permissions_manager(const std::filesystem::path& base_path)
            : base_path(base_path / "permissions.json") {}

        bool permissions_manager::has_rights(const std::string& action_name, const base_objects::client_data_holder& client) {
            return protected_values.get([&](const protected_values_t& values) {
                auto item = values.actions.find(action_name);
                if (item == values.actions.end())
                    return true;

                auto& client_data = client->player_data;
                bool instant_granted = true;
                bool has_incompatible = item->second.find_if([&client_data, &instant_granted, &values](const std::string& tag) {
                    auto permission = values.permissions.find(tag);
                    if (permission == values.permissions.end())
                        return false;
                    auto& perm = permission->second;
                    return client_data
                               .permissions
                               .find_if([perm, client_data, &instant_granted](const std::string& client_perm) {
                                   bool compatible = perm.permission_tag == client_perm && perm.permission_level != -1 ? perm.permission_level <= client_data.op_level : true;
                                   if (compatible && perm.instant_grant) {
                                       instant_granted = true;
                                       return true;
                                   }
                                   return !compatible;
                               }) != client_data.permissions.npos;
                }) != item->second.npos;
                return !has_incompatible || instant_granted;
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
                return values;
            });
        }

        void permissions_manager::add_permission(base_objects::permissions_object permission) {
            protected_values.set([&](protected_values_t& values) {
                values.permissions[permission.permission_tag] = std::move(permission);
                return values;
            });
        }

        void permissions_manager::remove_permission(const std::string& permission_tag) {
            protected_values.set([&](protected_values_t& values) {
                values.permissions.erase(permission_tag);
                return values;
            });
        }

        void permissions_manager::sync() {
            using namespace util;

            auto config_holder = try_read_json_file(base_path);
            if (!config_holder.empty()) {
                log::error("server", "Failed to load permissions file");
                return;
            }
            auto root = js_object::get_object(config_holder);

            list_array<base_objects::permissions_object> readden_permissions_tag;
            for (auto value : js_array::get_array(root["actions"])) {
                auto perm = js_object::get_object(value);

                auto permission = base_objects::permissions_object(
                    perm["permission_tag"],
                    perm["description"].or_apply(""),
                    perm["instant_grant"].or_apply(false),
                    perm["permission_level"].or_apply(0)
                );
                readden_permissions_tag.push_back(permission);
            }
            protected_values.set([&](protected_values_t& values) {
                for (auto&& [key, value] : js_object::get_object(root["permissions"])) {
                    auto& read = values.actions[std::string(key.c_str(), key.size())];
                    for (auto item : js_array::get_array(value))
                        read.push_back((std::string)item);
                    read.commit();
                }
                for (auto& perm : readden_permissions_tag)
                    values.permissions[perm.permission_tag] = std::move(perm);
                return values;
            });
            {
                std::ofstream file(base_path, std::ofstream::trunc);
                if (!file.is_open()) {
                    log::warn("server", "Failed to save permissions file. Can not open file.");
                    return;
                }
                util::pretty_print(file, config_holder);
            }
        }
    }
}
