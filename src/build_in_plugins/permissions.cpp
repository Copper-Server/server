#include "permissions.hpp"
#include "../ClientHandleHelper.hpp"
#include "../api/permissions.hpp"
#include "../api/players.hpp"
#include "../log.hpp"
#include "../plugin/main.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        PermissionsPlugin::PermissionsPlugin()
            : server(Server::instance()), op_list(Server::instance().config.server.base_path / "op_list.txt") {
        }

        void apply_group(const base_objects::shared_string& group_name, base_objects::player& pd) {
            api::permissions::enum_group_values(group_name, [&](const base_objects::shared_string& perm_tag) {
                if (perm_tag.get().starts_with("action.")) {
                    pd.instant_granted_actions.push_back(perm_tag.get().substr(7));
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

        void PermissionsPlugin::OnLoad(const PluginRegistrationPtr& self) {
            if (op_list.is_loaded())
                pluginManagement.registerPluginOn(self, PluginManagement::registration_on::play);
            else
                log::error("permissions", "failed to load permissions");
        }

        void PermissionsPlugin::OnPostLoad(const PluginRegistrationPtr& self) {
            api::permissions::make_sync();
            server.online_players.iterate_online([&](base_objects::SharedClientData& client_ref) {
                update_perm(client_ref);
                return false;
            });
        }

        void PermissionsPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            auto permissions = browser.add_child({"permissions"});
            permissions.add_child({"reload"}).set_callback("command.permissions.reload", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                api::players::calls::on_system_message({client, "Reloading permissions..."});
                api::permissions::make_sync();
                server.online_players.iterate_online([&](base_objects::SharedClientData& client_ref) {
                    update_perm(client_ref);
                    return false;
                });
                api::players::calls::on_system_message({client, "Permissions reload complete."});
            });
            {
                auto list = permissions.add_child({"list"});
                list.add_child({"group"}).set_callback("command.permissions.list.group", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                    list_array<base_objects::shared_string> enumerate;
                    bool overflow = false;
                    api::permissions::enum_groups([&](const base_objects::permission_group& group) {
                        if (enumerate.size() < 100)
                            enumerate.push_back(group.group_name);
                        else
                            overflow = true;
                    });

                    if (enumerate.empty())
                        api::players::calls::on_system_message({client, "There no groups."});
                    else if (enumerate.size() == 1)
                        api::players::calls::on_system_message({client, "There only one group: " + enumerate[0].get()});
                    else {
                        std::string buf;
                        buf.reserve(enumerate.sum([](const base_objects::shared_string& val) {
                            return val.get().size() + 1;
                        }) + 25);
                        buf += "There are " + std::to_string(enumerate.size()) + " groups\n";
                        for (auto& it : enumerate)
                            buf += it.get() + "\n";
                        if (overflow)
                            buf += "...\n";
                        api::players::calls::on_system_message({client, std::move(buf)});
                    }
                });
                list.add_child({"permission"}).set_callback("command.permissions.list.permission", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                    list_array<base_objects::shared_string> enumerate;
                    bool overflow = false;
                    api::permissions::enum_permissions([&](const base_objects::permissions_object& group) {
                        if (enumerate.size() < 100)
                            enumerate.push_back(group.permission_tag);
                        else
                            overflow = true;
                    });

                    if (enumerate.empty())
                        api::players::calls::on_system_message({client, "There no permissions."});
                    else if (enumerate.size() == 1)
                        api::players::calls::on_system_message({client, "There only one permission: " + enumerate[0].get()});
                    else {
                        std::string buf;
                        buf.reserve(enumerate.sum([](const base_objects::shared_string& val) {
                            return val.get().size() + 1;
                        }) + 30);
                        size_t calculate_reserve;

                        buf += "There are " + std::to_string(enumerate.size()) + " permissions\n";
                        for (auto& it : enumerate)
                            buf += it.get() + "\n";
                        if (overflow)
                            buf += "...\n";
                        api::players::calls::on_system_message({client, std::move(buf)});
                    }
                });
            }
            {
                auto group = permissions.add_child({"group"});
                group
                    .add_child({"list"})
                    .set_redirect(
                        "permissions list group",
                        [browser](const list_array<std::string>& args, const std::string& left, base_objects::client_data_holder& client) {
                            browser.get_manager().execute_command("permissions list group", client);
                        }
                    );
                {
                    auto add = group
                                   .add_child({"add"})
                                   .add_child({"<name>"}, base_objects::packets::command_node::parsers::brigadier_string, {.flags = 1});

                    add.set_callback("command.permissions.group.add", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        api::permissions::add_group({args[0]});
                    });
                    add
                        .add_child({"<values>"}, base_objects::packets::command_node::parsers::brigadier_string, {.flags = 2})
                        .set_callback("command.permissions.group.add:with_values", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            auto permissions = list_array<char>(args[0]).split_by(' ').convert<base_objects::shared_string>([](const list_array<char>& a) {
                                return base_objects::shared_string(a.data(), a.size());
                            });
                            api::permissions::add_group({args[0]});
                        });
                }
            }
            {
                browser.add_child({"op"})
                    .add_child({"<player>", "op player", "/op <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback("command.op", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (op_list.contains(args[0])) {
                            api::players::calls::on_system_message({client, "This player already operator."});
                            return;
                        }
                        op_list.add(args[0]);
                        api::players::calls::on_system_message_broadcast("Player " + args[0] + " is now operator.");

                        auto target = server.online_players.get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            args[0]
                        );
                        if (!target)
                            update_perm(*target);
                    });
            }
            {
                browser.add_child({"deop"})
                    .add_child({"<player>", "deop player", "/deop <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback("command.deop", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        op_list.remove(args[0]);
                        auto target = server.online_players.get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            args[0]
                        );
                        if (!target)
                            update_perm(*target);

                        api::players::calls::on_system_message_broadcast({"Player " + args[0] + " is no more operator."});
                    });
            }
        }

        PermissionsPlugin::plugin_response PermissionsPlugin::OnPlay_initialize(base_objects::client_data_holder& client_ref) {
            update_perm(*client_ref);
            return false;
        }
    }
}