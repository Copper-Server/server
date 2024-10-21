#include "allowlist.hpp"
#include "../ClientHandleHelper.hpp"
#include "../api/configuration.hpp"
#include "../api/players.hpp"
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/main.hpp"

namespace crafted_craft {
    class Server;

    namespace build_in_plugins {
        AllowListPlugin::AllowListPlugin()
            : allow_list(api::configuration::get().server.get_storage_path() / "allow_list.txt") {}

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

        void AllowListPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            {
                auto allowlist = browser.add_child({"allowlist", "", ""});
                allowlist.add_child({"add", "", ""})
                    .add_child({"<player>", "add player to allowlist", "/allowlist add <player>"}, cmd_pred_string::quotable_phrase)
                    .set_callback("command.allowlist.add", [this](const list_array<predicate>& args, base_objects::command_context& context) -> void {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        if (player_name.contains("\n"))
                            throw std::runtime_error("Player name contains newline character");
                        if (api::allowlist::on_add(player_name))
                            return;

                        allow_list.add(player_name);
                        api::players::calls::on_system_message({context.executor, {"Player " + player_name + " added to allowlist"}});
                    });
                allowlist.add_child({"remove", "", ""})
                    .add_child({"<player>", "remove player from allowlist", "/allowlist remove <player>"}, cmd_pred_string::quotable_phrase)
                    .set_callback("command.allowlist.remove", [this](const list_array<predicate>& args, base_objects::command_context& context) -> void {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        if (player_name.contains("\n"))
                            throw std::runtime_error("Player name contains newline character");
                        if (api::allowlist::on_remove(player_name))
                            return;
                        allow_list.remove(player_name);
                        api::players::calls::on_system_message({context.executor, {"Player " + player_name + " removed from allowlist"}});
                    });
                allowlist.add_child({"list", "list all players in allowlist", "/allowlist list"})
                    .set_callback("command.allowlist.list", [this](const list_array<predicate>&, base_objects::command_context& context) -> void {
                        bool max_reached = false;
                        auto listed = allow_list.entrys(100, max_reached);
                        if (listed.size() == 0) {
                            api::players::calls::on_system_message({context.executor, {"There are no listed player."}});
                        } else if (listed.size() == 1) {
                            api::players::calls::on_system_message({context.executor, {"There is only one player in the list:" + listed.back()}});
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
                            api::players::calls::on_system_message({context.executor, {message}});
                        }
                    });
                allowlist.add_child({"mode"})
                    .add_child({"<mode>", "set allowlist mode", "/allowlist mode block|allow|off"}, cmd_pred_string::quotable_phrase)
                    .set_callback("command.allowlist.mode", [this](const list_array<predicate>& args, base_objects::command_context& context) -> void {
                        auto& mode = std::get<pred_string>(args[0]).value;
                        if (mode == "block")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::block);
                        else if (mode == "allow")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::allow);
                        else if (mode == "off")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::off);
                        else {
                            api::players::calls::on_system_message({context.executor, {"Usage: /allowlist mode block|allow|off"}});
                            return;
                        }
                        api::players::calls::on_system_message({context.executor, {"Allowlist mode set to " + mode}});
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
