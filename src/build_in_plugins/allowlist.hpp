#ifndef SRC_BUILD_IN_PLUGINS_ALLOWLIST
#define SRC_BUILD_IN_PLUGINS_ALLOWLIST
#include "../api/allowlist.hpp"
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/registration.hpp"
#include "../protocolHelper/state_play.hpp"
#include "../storage/list_storage.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class AllowListPlugin : public PluginRegistration {
            storage::list_storage allow_list;
            base_objects::event_register_id mode_change_id;

            api::allowlist::allowlist_mode mode = api::allowlist::allowlist_mode::off;

        public:
            AllowListPlugin(const std::string& storage_path)
                : allow_list(storage_path + "/allow_list.txt") {}

            void OnLoad(const PluginRegistrationPtr& self) override {
                mode_change_id = api::allowlist::on_mode_change.join(
                    base_objects::event_priority::heigh,
                    [this](api::allowlist::allowlist_mode mode) {
                        if (mode == api::allowlist::allowlist_mode::block)
                            for (const auto& entry : allow_list.entrys())
                                api::allowlist::on_kick(entry);

                        this->mode = mode;
                        return false;
                    }
                );
            }

            void OnUnload(const PluginRegistrationPtr& self) override {
                api::allowlist::on_mode_change.leave(mode_change_id, base_objects::event_priority::heigh, false);
            }

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
                {
                    auto allowlist = browser.add_child({"allowlist", "", ""});
                    allowlist.add_child({"add", "", ""})
                        .add_child({"<player>", "add player to allowlist", "/allowlist add <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) -> void {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                client->sendPacket(packets::play::systemChatMessage({"Usage: /allowlist add <player>"}));
                                return;
                            }
                            const std::string& player = args[0];
                            if (player.contains("\n"))
                                throw std::runtime_error("Player name contains newline character");
                            if (api::allowlist::on_add(player))
                                return;

                            allow_list.add(args[0]);
                            client->sendPacket(packets::play::systemChatMessage({"Player " + args[0] + " added to allowlist"}));
                        });
                    allowlist.add_child({"remove", "", ""})
                        .add_child({"<player>", "remove player from allowlist", "/allowlist remove <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) -> void {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                client->sendPacket(packets::play::systemChatMessage({"Usage: /allowlist remove <player>"}));
                                return;
                            }
                            const std::string& player = args[0];
                            if (player.contains("\n"))
                                throw std::runtime_error("Player name contains newline character");
                            if (api::allowlist::on_remove(player))
                                return;
                            allow_list.remove(args[0]);
                            client->sendPacket(packets::play::systemChatMessage({"Player " + args[0] + " removed from allowlist"}));
                        });
                    allowlist.add_child({"list", "list all players in allowlist", "/allowlist list"})
                        .set_callback([this](const list_array<std::string>&, base_objects::client_data_holder& client) -> void {
                            if (client->player_data.op_level < 4)
                                return;
                            //list all players in allowlist
                            auto entrys = allow_list.entrys();
                            std::string message = "Players in allowlist: ";
                            for (const auto& entry : entrys)
                                message += entry + ", ";
                            message.erase(message.size() - 2);
                            message[message.size() - 1] = '.';

                            client->sendPacket(packets::play::systemChatMessage({message}));
                        });
                    allowlist.add_child({"mode"})
                        .add_child({"<mode>", "set allowlist mode", "/allowlist mode block|allow|off"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback([](const list_array<std::string>& args, base_objects::client_data_holder& client) -> void {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                client->sendPacket(packets::play::systemChatMessage({"Usage: /allowlist mode <mode>"}));
                                return;
                            }
                            if (args[0] == "block")
                                api::allowlist::on_mode_change(api::allowlist::allowlist_mode::block);
                            else if (args[0] == "allow")
                                api::allowlist::on_mode_change(api::allowlist::allowlist_mode::allow);
                            else if (args[0] == "off")
                                api::allowlist::on_mode_change(api::allowlist::allowlist_mode::off);
                            else {
                                client->sendPacket(packets::play::systemChatMessage({"Usage: /allowlist mode block|allow|off"}));
                                return;
                            }
                            client->sendPacket(packets::play::systemChatMessage({"Allowlist mode set to " + args[0]}));
                        });
                }
            }

            plugin_response OnPlay_initialize(base_objects::client_data_holder& client) override {
                if (mode == api::allowlist::allowlist_mode::block && !allow_list.contains(client->name))
                    api::allowlist::on_kick(client->name);
                return false;
            }
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_ALLOWLIST */
