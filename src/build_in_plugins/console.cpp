#include "console.hpp"
#include "../api/command.hpp"
#include "../api/console.hpp"
#include "../api/players.hpp"
#include "../log.hpp"
#include "../plugin/registration.hpp"
#include "../protocolHelper/state_play.hpp"

namespace crafted_craft {
    class Server;

    namespace build_in_plugins {
        ConsolePlugin::ConsolePlugin()
            : server(Server::instance()), console_data(server.online_players.allocate_player(), "Console", "Console", Server::instance()) {
        }

        void ConsolePlugin::OnLoad(const PluginRegistrationPtr& self) {
            api::console::register_virtual_client(console_data);
            register_event(log::commands::on_command, base_objects::event_priority::heigh, [&](const std::string& command) {
                if (command.empty())
                    return false;
                log::info("command", command);
                try {
                    api::console::execute_as_console(command);
                } catch (const std::exception& ex) {
                    log::error("command", command + "\n Failed to execute command, reason:\n\t" + ex.what());
                    return false;
                }
                return true;
            });

            log::commands::registerCommandSuggestion([this](const std::string& line, int position) {
                auto tmp = line;
                if (uint32_t(position) <= line.size())
                    tmp.resize(position);
                else
                    tmp += ' ';
                auto insertion_ = tmp;

                if (!insertion_.starts_with(' ')) {
                    auto it = insertion_.find_last_of(' ');
                    if (insertion_.npos == it)
                        insertion_.clear();
                    else
                        insertion_ = insertion_.substr(0, it) + ' ';
                }
                return api::command::get_manager()
                    .request_suggestions(tmp, console_data.client)
                    .convert<std::string>([&insertion_](auto&& suggestion) { return insertion_ + suggestion.insertion; })
                    .to_container<std::vector<std::string>>();
            });
            console_data.systemChatMessage = [this](const Chat& message) {
                log::info("system", message.to_ansi_console());
            };
            console_data.systemChatMessageOverlay = [this](const Chat& message) {
                log::info("overlay", message.to_ansi_console());
            };
            console_data.disguisedChatMessage = [this](const Chat& message, int32_t chat_type, const Chat& sender, std::optional<Chat> target_name) {
                if (!target_name)
                    log::info("message", "[" + sender.to_ansi_console() + "] " + message.to_ansi_console());
                else
                    log::info("message", "[" + sender.to_ansi_console() + " -> " + target_name->to_ansi_console() + "] " + message.to_ansi_console());
            };

            console_data.client->player_data.permission_groups = {"console", "operator"};
        }

        void ConsolePlugin::OnUnload(const PluginRegistrationPtr& self) {
            api::console::unregister_virtual_client();
        }

        void ConsolePlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            {
                auto _console = browser.add_child({"console"});
                auto _log = _console.add_child({"log"});
                {
                    _log
                        .add_child({"enable"})
                        .add_child({"<log level>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback({"command.console.log.enable", {"console"}}, [](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            auto& level = args[0];
                            if (level == "info")
                                log::enable_log_level(log::level::info);
                            else if (level == "warn")
                                log::enable_log_level(log::level::warn);
                            else if (level == "error")
                                log::enable_log_level(log::level::error);
                            else if (level == "fatal")
                                log::enable_log_level(log::level::fatal);
                            else if (level == "debug_error")
                                log::enable_log_level(log::level::debug_error);
                            else if (level == "debug")
                                log::enable_log_level(log::level::debug);
                            else {
                                log::error("console", "log level " + level + " is undefined.");
                                return;
                            }
                            log::info("console", "Log level " + level + " is now enabled.");
                        });
                    _log
                        .add_child({"disable"})
                        .add_child({"<log level>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback({"command.console.log.disable", {"console"}}, [](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            auto& level = args[0];
                            if (level == "info")
                                log::disable_log_level(log::level::info);
                            else if (level == "warn")
                                log::disable_log_level(log::level::warn);
                            else if (level == "error")
                                log::disable_log_level(log::level::error);
                            else if (level == "fatal")
                                log::disable_log_level(log::level::fatal);
                            else if (level == "debug_error")
                                log::disable_log_level(log::level::debug_error);
                            else if (level == "debug")
                                log::disable_log_level(log::level::debug);
                            else {
                                log::error("console", "log level " + level + " is undefined.");
                                return;
                            }
                            log::info("console", "Log level " + level + " is now disabled.");
                        });
                }
            }
        }
    }
}