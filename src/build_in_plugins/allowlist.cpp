#include "allowlist.hpp"
#include "../ClientHandleHelper.hpp"
#include "../api/players.hpp"
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/main.hpp"

namespace crafted_craft {
    class Server;

    namespace build_in_plugins {
        AllowListPlugin::AllowListPlugin()
            : allow_list(Server::instance().config.server.get_storage_path() / "allow_list.txt"), server(Server::instance()) {}

        void AllowListPlugin::OnPostLoad(const PluginRegistrationPtr& self) {
            register_event(api::allowlist::on_mode_change, base_objects::event_priority::heigh, [this](api::allowlist::allowlist_mode mode) {
                if (mode == api::allowlist::allowlist_mode::block)
                    allow_list.for_each(-1, [&](const auto& entry) {
                        api::allowlist::on_kick(entry);
                    });
                this->mode = mode;
                return false;
            });
        }

        void AllowListPlugin::OnLoad(const PluginRegistrationPtr& self) {
            pluginManagement.registerPluginOn(self, PluginManagement::registration_on::play);
        }

        void AllowListPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            {
                auto allowlist = browser.add_child({"allowlist", "", ""});
                allowlist.add_child({"add", "", ""})
                    .add_child({"<player>", "add player to allowlist", "/allowlist add <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback("command.allowlist.add", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) -> void {
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /allowlist add <player>"}});
                            return;
                        }
                        const std::string& player = args[0];
                        if (player.contains("\n"))
                            throw std::runtime_error("Player name contains newline character");
                        if (api::allowlist::on_add(player))
                            return;

                        allow_list.add(args[0]);
                        api::players::calls::on_system_message({client, {"Player " + args[0] + " added to allowlist"}});
                    });
                allowlist.add_child({"remove", "", ""})
                    .add_child({"<player>", "remove player from allowlist", "/allowlist remove <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback("command.allowlist.remove", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) -> void {
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /allowlist remove <player>"}});
                            return;
                        }
                        const std::string& player = args[0];
                        if (player.contains("\n"))
                            throw std::runtime_error("Player name contains newline character");
                        if (api::allowlist::on_remove(player))
                            return;
                        allow_list.remove(args[0]);
                        api::players::calls::on_system_message({client, {"Player " + args[0] + " removed from allowlist"}});
                    });
                allowlist.add_child({"list", "list all players in allowlist", "/allowlist list"})
                    .set_callback("command.allowlist.list", [this](const list_array<std::string>&, base_objects::client_data_holder& client) -> void {
                        bool max_reached = false;
                        auto listed = allow_list.entrys(100, max_reached);
                        if (listed.size() == 0) {
                            api::players::calls::on_system_message({client, {"There are no listed player."}});
                        } else if (listed.size() == 1) {
                            api::players::calls::on_system_message({client, {"There is only one player in the list:" + listed.back()}});
                        } else {
                            std::string last_item = listed.back();
                            listed.pop_back();
                            std::string message = "There a total of " + std::to_string(listed.size() + 1) + " listed players:\n";
                            for (auto& player : listed)
                                message += player + ", ";

                            if (!max_reached) {
                                message.erase(message.size() - 2, 2);
                                message += "and " + last_item + '.';
                            } else
                                message += last_item + ", ...";
                            api::players::calls::on_system_message({client, {message}});
                        }
                    });
                allowlist.add_child({"mode"})
                    .add_child({"<mode>", "set allowlist mode", "/allowlist mode block|allow|off"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback("command.allowlist.mode", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) -> void {
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /allowlist mode <mode>"}});
                            return;
                        }
                        if (args[0] == "block")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::block);
                        else if (args[0] == "allow")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::allow);
                        else if (args[0] == "off")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::off);
                        else {
                            api::players::calls::on_system_message({client, {"Usage: /allowlist mode block|allow|off"}});
                            return;
                        }
                        api::players::calls::on_system_message({client, {"Allowlist mode set to " + args[0]}});
                    });
            }
        }

        AllowListPlugin::plugin_response AllowListPlugin::OnPlay_initialize(base_objects::client_data_holder& client) {
            switch (mode) {
            case api::allowlist::allowlist_mode::block:
                if (allow_list.contains(client->name))
                    api::allowlist::on_kick(client->name);
                break;
            case api::allowlist::allowlist_mode::allow:
                if (!allow_list.contains(client->name))
                    api::allowlist::on_kick(client->name);
                break;
            default:
                break;
            }
            return false;
        }
    }
}
