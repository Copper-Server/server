#include <src/ClientHandleHelper.hpp>
#include <src/api/configuration.hpp>
#include <src/api/internal/permissions.hpp>
#include <src/api/permissions.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/player.hpp>
#include <src/build_in_plugins/permissions.hpp>
#include <src/log.hpp>

namespace copper_server {
    namespace build_in_plugins {
        PermissionsPlugin::PermissionsPlugin()
            : op_list(api::configuration::get().server.base_path / "op_list.txt"),
              manager(std::filesystem::current_path()) {
        }

        void apply_group(const std::string& group_name, base_objects::player& pd) {
            api::permissions::enum_group_values(group_name, [&](const std::string& perm_tag) {
                if (perm_tag.starts_with("action.")) {
                    pd.instant_granted_actions.push_back(perm_tag.substr(7));
                } else
                    pd.permissions.push_back(perm_tag);
            });
        }

        void PermissionsPlugin::update_perm(base_objects::SharedClientData& client_ref) {
            auto& pd = client_ref.player_data;
            pd.permissions.clear();
            pd.instant_granted_actions.clear();

            for (const auto& group_name : pd.permission_groups)
                apply_group(group_name, pd);
            if (op_list.contains(client_ref.name))
                apply_group("operator", pd);
            pd.permissions.unify();

            int8_t op_level = 0;
            for (const auto& perm_name : pd.permissions) {
                api::permissions::view_permission(perm_name, [&op_level](const base_objects::permissions_object& perm) {
                    if (perm.permission_level != (int8_t)-1)
                        op_level = std::max(op_level, perm.permission_level);
                });
            }
            pd.op_level = op_level;


            pd.permissions.commit();
            pd.instant_granted_actions.commit();
        }

        void PermissionsPlugin::OnInitialization(const PluginRegistrationPtr& self) {
            api::permissions::init_permissions(manager);
        }

        void PermissionsPlugin::OnLoad(const PluginRegistrationPtr& self) {
            if (!op_list.is_loaded())
                log::error("permissions", "failed to load permissions");
        }

        void PermissionsPlugin::OnPostLoad(const PluginRegistrationPtr& self) {
            api::permissions::make_sync();
            api::players::iterate_online([&](base_objects::SharedClientData& client_ref) {
                update_perm(client_ref);
                return false;
            });
        }

        void PermissionsPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            auto permissions = browser.add_child("permissions");
            permissions.add_child("reload").set_callback("command.permissions.reload", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                api::players::calls::on_system_message({context.executor, "Reloading permissions..."});
                api::permissions::make_sync();
                api::players::iterate_online([&](base_objects::SharedClientData& client_ref) {
                    update_perm(client_ref);
                    return false;
                });
                api::players::calls::on_system_message({context.executor, "Permissions reload complete."});
            });
            {
                auto list = permissions.add_child("list");
                list.add_child("group").set_callback("command.permissions.list.group", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    list_array<std::string> enumerate;
                    bool overflow = false;
                    api::permissions::enum_groups([&](const base_objects::permission_group& group) {
                        if (enumerate.size() < 100)
                            enumerate.push_back(group.group_name);
                        else
                            overflow = true;
                    });

                    if (enumerate.empty())
                        api::players::calls::on_system_message({context.executor, "There no groups."});
                    else if (enumerate.size() == 1)
                        api::players::calls::on_system_message({context.executor, "There only one group: " + enumerate[0]});
                    else {
                        std::string buf;
                        buf.reserve(enumerate.sum([](const std::string& val) {
                            return val.size() + 1;
                        }) + 25);
                        buf += "There are " + std::to_string(enumerate.size()) + " groups\n";
                        for (auto& it : enumerate)
                            buf += it + "\n";
                        if (overflow)
                            buf += "...\n";
                        api::players::calls::on_system_message({context.executor, std::move(buf)});
                    }
                });
                list.add_child("permission").set_callback("command.permissions.list.permission", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    list_array<std::string> enumerate;
                    bool overflow = false;
                    api::permissions::enum_permissions([&](const base_objects::permissions_object& group) {
                        if (enumerate.size() < 100)
                            enumerate.push_back(group.permission_tag);
                        else
                            overflow = true;
                    });

                    if (enumerate.empty())
                        api::players::calls::on_system_message({context.executor, "There no permissions."});
                    else if (enumerate.size() == 1)
                        api::players::calls::on_system_message({context.executor, "There only one permission: " + enumerate[0]});
                    else {
                        std::string buf;
                        buf.reserve(enumerate.sum([](const std::string& val) {
                            return val.size() + 1;
                        }) + 30);
                        size_t calculate_reserve;

                        buf += "There are " + std::to_string(enumerate.size()) + " permissions\n";
                        for (auto& it : enumerate)
                            buf += it + "\n";
                        if (overflow)
                            buf += "...\n";
                        api::players::calls::on_system_message({context.executor, std::move(buf)});
                    }
                });
            }
            {
                auto group = permissions.add_child("group");
                group
                    .add_child("list")
                    .set_redirect(
                        "permissions list group",
                        [browser](base_objects::command& cmd, const list_array<predicate>& args, const std::string& left, base_objects::command_context& context) {
                            browser.get_manager().execute_command_from("", cmd, context);
                        }
                    );
                {
                    group
                        .add_child("add")
                        .add_child("<name>", cmd_pred_string::quotable_phrase)
                        .set_callback("command.permissions.group.add", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            api::permissions::add_group({std::get<pred_string>(args[0]).value});
                        })
                        .add_child("<values>", cmd_pred_string::greedy_phrase)
                        .set_callback("command.permissions.group.add:with_values", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto permissions = list_array<char>(std::get<pred_string>(args[1]).value).split_by(' ').convert<std::string>([](const list_array<char>& a) {
                                return std::string(a.data(), a.size());
                            });
                            api::permissions::add_group({std::get<pred_string>(args[0]).value, permissions});
                        });
                }
            }
            {
                browser.add_child("op")
                    .add_child({"<player>", "op player", "/op <player>"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.op", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        if (op_list.contains(player_name)) {
                            api::players::calls::on_system_message({context.executor, "This player already operator."});
                            return;
                        }
                        op_list.add(player_name);
                        api::players::calls::on_system_message_broadcast("Player " + player_name + " is now operator.");

                        auto target = api::players::get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            player_name
                        );
                        if (!target)
                            update_perm(*target);
                    });
            }
            {
                browser.add_child("deop")
                    .add_child({"<player>", "deop player", "/deop <player>"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.deop", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        op_list.remove(player_name);
                        auto target = api::players::get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            player_name
                        );
                        if (!target)
                            update_perm(*target);

                        api::players::calls::on_system_message_broadcast({"Player " + player_name + " is no more operator."});
                    });
            }
        }

        PermissionsPlugin::plugin_response PermissionsPlugin::OnPlay_initialize(base_objects::client_data_holder& client_ref) {
            update_perm(*client_ref);
            return false;
        }
    }
}