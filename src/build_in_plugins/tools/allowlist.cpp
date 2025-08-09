#include <src/api/allowlist.hpp>
#include <src/api/client.hpp>
#include <src/api/configuration.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/storage/unordered_list_storage.hpp>

namespace copper_server::build_in_plugins {
    struct allow_list_plugin : public PluginAutoRegister<"tools/allow_list", allow_list_plugin> {
        storage::unordered_list_storage allow_list{api::configuration::get().server.get_storage_path() / "allow_list.txt"};
        api::allowlist::allowlist_mode mode = api::allowlist::allowlist_mode::off;

        ~allow_list_plugin() noexcept {};

        void OnInitialization(const PluginRegistrationPtr& self) {
            api::configuration::get() ^ "allow_list" ^ "on_kick_message" |= enbt::compound{{"text", "You are not in allowlist."}, {"color", "red"}};
        }

        void OnPostLoad(const PluginRegistrationPtr& self) override {
            register_event(api::allowlist::on_mode_change, base_objects::events::priority::high, [this](api::allowlist::allowlist_mode mode) {
                if (mode == api::allowlist::allowlist_mode::block) {
                    bool reached = false;
                    auto set = allow_list.entrys((size_t)-1, reached);
                    api::players::get_players().for_each([&set](auto& client) {
                        if (set.contains(client->name))
                            api::allowlist::on_kick(client);
                        return false;
                    });
                }
                this->mode = mode;
                return false;
            });
            register_event(api::allowlist::on_kick, base_objects::events::priority::low, [this](const base_objects::client_data_holder& client) {
                api::players::calls::on_player_kick({client, Chat::fromEnbt(api::configuration::get() ^ "allow_list" ^ "on_kick_message")});
                return false;
            });
            register_event(api::allowlist::on_add, base_objects::events::priority::low, [this](const std::string name) {
                switch (mode) {
                case api::allowlist::allowlist_mode::block:
                    if (allow_list.contains(name))
                        api::allowlist::on_kick(api::players::get_player(base_objects::SharedClientData::packets_state_t::protocol_state::play, name));
                    break;
                case api::allowlist::allowlist_mode::allow:
                    if (!allow_list.contains(name))
                        api::allowlist::on_kick(api::players::get_player(base_objects::SharedClientData::packets_state_t::protocol_state::play, name));
                    break;
                default:
                    break;
                }
                return false;
            });
        }

        void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
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
                        if (api::allowlist::on_add(player_name)) {
                            context.executor << api::client::play::system_chat{.content = {"Player " + player_name + " is not added to allowlist"}};
                            return;
                        }

                        allow_list.add(player_name);
                        context.executor << api::client::play::system_chat{.content = {"Player " + player_name + " added to allowlist"}};
                    });
                allowlist.add_child({"remove", "", ""})
                    .add_child({"<player>", "remove player from allowlist", "/allowlist remove <player>"}, cmd_pred_string::quotable_phrase)
                    .set_callback("command.allowlist.remove", [this](const list_array<predicate>& args, base_objects::command_context& context) -> void {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        if (player_name.contains("\n"))
                            throw std::runtime_error("Player name contains newline character");
                        if (api::allowlist::on_remove(player_name)) {
                            context.executor << api::client::play::system_chat{.content = {"Player " + player_name + " is not removed from allowlist"}};
                            return;
                        }
                        allow_list.remove(player_name);
                        context.executor << api::client::play::system_chat{.content = {"Player " + player_name + " removed from allowlist"}};
                    });
                allowlist.add_child({"list", "list all players in allowlist", "/allowlist list"})
                    .set_callback("command.allowlist.list", [this](const list_array<predicate>&, base_objects::command_context& context) -> void {
                        bool max_reached = false;
                        auto listed = allow_list.entrys(100, max_reached);
                        if (listed.size() == 0) {
                            context.executor << api::client::play::system_chat{.content = {"There are no listed player."}};
                        } else if (listed.size() == 1) {
                            context.executor << api::client::play::system_chat{.content = {"There is only one player in the list:" + *listed.begin()}};
                        } else {
                            std::string message = "There a total of " + std::to_string(listed.size()) + " listed players:\n";
                            size_t i = 0;
                            size_t m = listed.size();
                            for (auto& player : listed) {
                                if (++i == m) {
                                    if (max_reached)
                                        message += "and " + player + '.';
                                    else
                                        message += player + ", ...";
                                    break;
                                } else
                                    message += player + ", ";
                            }
                            context.executor << api::client::play::system_chat{.content = {message}};
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
                            context.executor << api::client::play::system_chat{.content = {"Usage: /allowlist mode block|allow|off"}};
                            return;
                        }
                        context.executor << api::client::play::system_chat{.content = {"Allowlist mode set to " + mode}};
                    });
            }
        }

        void OnPlay_initialize(base_objects::SharedClientData& client) override {
            switch (mode) {
            case api::allowlist::allowlist_mode::block:
                if (allow_list.contains(client.name))
                    api::allowlist::on_kick(api::players::get_player(client));
                break;
            case api::allowlist::allowlist_mode::allow:
                if (!allow_list.contains(client.name))
                    api::allowlist::on_kick(api::players::get_player(client));
                break;
            default:
                break;
            }
        }
    };
}
