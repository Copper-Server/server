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
                bool instant_granted = false;
                bool has_incompatible = false;

                item->second.for_each([&](const std::string& tag) {
                    auto permission = values.permissions.find(tag);
                    if (permission == values.permissions.end())
                        return;
                    auto& perm = permission->second;
                    auto client_perm = client_data.permissions.find(perm.permission_tag);
                    if (client_data.permissions.npos == client_perm && perm.permission_level == -1) {
                        has_incompatible = true;
                        return;
                    }
                    instant_granted |= perm.instant_grant;

                    if (perm.permission_level != -1) {
                        has_incompatible |= perm.permission_level > client_data.op_level;
                    }
                });

                return !has_incompatible || instant_granted;
            });
        }

        bool permissions_manager::has_action(const std::string& action_name) {
            return protected_values.get([&](const protected_values_t& values) {
                return values.actions.contains(action_name);
            });
        }

        bool permissions_manager::has_permissions(const std::string& action_name) {
            return protected_values.get([&](const protected_values_t& values) {
                return values.permissions.contains(action_name);
            });
        }

        void permissions_manager::register_action(const std::string& action_name) {
            protected_values.set([&](protected_values_t& values) {
                auto it = values.actions.find(action_name);
                if (it != values.actions.end())
                    throw std::runtime_error("This action already registered.");
                values.actions[action_name].clear();
            });
        }

        void permissions_manager::register_action(const std::string& action_name, const list_array<std::string>& required_perm) {
            protected_values.set([&](protected_values_t& values) {
                auto it = values.actions.find(action_name);
                if (it != values.actions.end())
                    throw std::runtime_error("This action already registered.");
                values.actions[action_name] = required_perm;
            });
        }

        void permissions_manager::register_action(const std::string& action_name, list_array<std::string>&& required_perm) {
            protected_values.set([&](protected_values_t& values) {
                auto it = values.actions.find(action_name);
                if (it != values.actions.end())
                    throw std::runtime_error("This action already registered.");
                values.actions[action_name] = std::move(required_perm);
            });
        }

        void permissions_manager::unregister_action(const std::string& action_name) {
            protected_values.set([&](protected_values_t& values) {
                auto it = values.actions.find(action_name);
                if (it == values.actions.end())
                    throw std::runtime_error("This action already unregistered.");
                values.actions.erase(it);
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

        void permissions_manager::enum_actions(const std::function<void(const std::string&)>& callback) {
            return protected_values.get([&](const protected_values_t& values) {
                for (auto&& [action, data] : values.actions)
                    callback(action);
            });
        }

        void permissions_manager::enum_action_requirements(const std::string& action_name, const std::function<void(const std::string&)>& callback) {
            return protected_values.get([&](const protected_values_t& values) {
                auto item = values.actions.find(action_name);
                if (item == values.actions.end())
                    throw std::runtime_error("This action not registered.");
                item->second.for_each(callback);
            });
        }

        void permissions_manager::enum_permissions(const std::function<void(const std::string&)>& callback) {
            return protected_values.get([&](const protected_values_t& values) {
                for (auto&& [perm, data] : values.permissions)
                    callback(perm);
            });
        }


        void permissions_manager::sync() {
            using namespace util;

            auto config_holder = try_read_json_file(base_path);
            if (!config_holder.has_value()) {
                log::error("server", "Failed to load permissions file");
                return;
            }
            auto root = js_object::get_object(*config_holder);

            list_array<base_objects::permissions_object> readden_permissions_tag;
            auto permissions_obj = js_object::get_object(root["permissions"]);
            for (auto&& [permission_tag, value] : permissions_obj) {
                auto perm = js_object::get_object(value);
                std::string description = perm["description"].or_apply("");
                bool instant_grant = perm["instant_grant"].or_apply(false);
                int8_t permission_level = perm["permission_level"].or_apply(0);

                readden_permissions_tag.push_back(base_objects::permissions_object(
                    std::string(permission_tag.data(), permission_tag.size()),
                    std::move(description),
                    instant_grant,
                    permission_level
                ));
            }
            protected_values.set([&](protected_values_t& values) {
                list_array<std::string> declared_actions;
                list_array<std::string> declared_permissions;

                auto actions_obj = js_object::get_object(root["actions"]);
                declared_actions.reserve(actions_obj.size());

                //updating actions from json
                for (auto&& [key, value] : actions_obj) {
                    std::string cast_key = std::string(key.c_str(), key.size());
                    auto& read = values.actions[cast_key];
                    for (auto item : js_array::get_array(value))
                        read.push_back((std::string)item);
                    read.commit();
                    declared_actions.push_back(std::move(cast_key));
                }

                //adding new actions to json
                for (auto&& [key, value] : values.actions) {
                    if (!declared_actions.contains(key)) {
                        boost::json::array arr;
                        arr.reserve(value.size());
                        for (auto& item : value)
                            arr.push_back(boost::json::string(item));
                        actions_obj[key] = std::move(arr);
                    }
                }

                if (readden_permissions_tag.empty() && values.permissions.empty()) {
                    values.permissions["operator_1"] = base_objects::permissions_object{
                        .permission_tag = "operator_1",
                        .description = "The default permission tag for operator level 1.",
                        .permission_level = 1,
                        .instant_grant = false
                    };
                    values.permissions["operator_2"] = base_objects::permissions_object{
                        .permission_tag = "operator_2",
                        .description = "The default permission tag for operator level 2.",
                        .permission_level = 1,
                        .instant_grant = false
                    };
                    values.permissions["operator_3"] = base_objects::permissions_object{
                        .permission_tag = "operator_3",
                        .description = "The default permission tag for operator level 3.",
                        .permission_level = 1,
                        .instant_grant = false
                    };
                    values.permissions["permission_op_4"] = base_objects::permissions_object{
                        .permission_tag = "permission_op_4",
                        .description = "The default permission tag for operator level 4.",
                        .permission_level = 1,
                        .instant_grant = false
                    };
                    values.permissions["operator_4"] = base_objects::permissions_object{
                        .permission_tag = "operator_4",
                        .description = "The default permission tag for operator level 4.",
                        .permission_level = 1,
                        .instant_grant = false
                    };
                    values.permissions["console"] = base_objects::permissions_object{
                        .permission_tag = "console",
                        .description = "The default permission tag for console commands.",
                        .permission_level = 1,
                        .instant_grant = false
                    };
                }

                //updating permissions from json
                for (auto& perm : readden_permissions_tag) {
                    values.permissions[perm.permission_tag] = std::move(perm);
                    declared_permissions.push_back(perm.permission_tag);
                }

                //adding new permissions to json
                for (auto&& [permission_tag, value] : values.permissions) {
                    if (!declared_permissions.contains(permission_tag)) {
                        permissions_obj[permission_tag] =
                            value.description.empty()
                                ? boost::json::object{
                                      {"instant_grant", value.instant_grant},
                                      {"permission_level", value.permission_level},
                                  }
                                : boost::json::object{
                                      {"description", value.description},
                                      {"instant_grant", value.instant_grant},
                                      {"permission_level", value.permission_level},
                                  };
                    }
                }

                return values;
            });
            {
                std::ofstream file(base_path, std::ofstream::trunc);
                if (!file.is_open()) {
                    log::warn("server", "Failed to save permissions file. Can not open file.");
                    return;
                }
                util::pretty_print(file, *config_holder);
            }
        }
    }
}
