#include <src/api/command.hpp>
#include <src/api/configuration.hpp>
#include <src/api/console.hpp>
#include <src/api/internal/console.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/build_in_plugins/tools/console.hpp>
#include <src/log.hpp>
#include <src/plugin/registration.hpp>
#include <src/storage/list_storage.hpp>

namespace copper_server::build_in_plugins {
    void add_log_type_suggestion(base_objects::command_browser& browser) {
        browser.set_suggestion_callback([](const std::string& current, base_objects::command_context& context) {
            auto suggestions = list_array<std::string>{
                "info",
                "warn",
                "error",
                "fatal",
                "debug_error",
                "debug"
            };

            if (current.empty())
                return suggestions;
            else {
                return suggestions.where([&current](const std::string& suggestion) { return suggestion.starts_with(current); });
            }
        });
    }

    ConsolePlugin::ConsolePlugin()
        : console_data(api::players::allocate_player(), "Console", "Console") {
    }

    void ConsolePlugin::OnLoad(const PluginRegistrationPtr& self) {
        api::console::register_virtual_client(console_data);

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
            base_objects::command_context context(console_data.client);
            return api::command::get_manager()
                .request_suggestions(tmp, context)
                .transform([&insertion_](auto&& suggestion) { return insertion_ + suggestion; })
                .sort()
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

        register_event(log::commands::on_command, base_objects::events::priority::heigh, [&](const std::string& command) {
            if (command.empty())
                return false;
            log::info("command", command);
            try {
                api::console::execute_as_console(command);
            } catch (const base_objects::command_exception& ex) {
                try {
                    std::rethrow_exception(ex.exception);
                } catch (const std::exception& inner_ex) {
                    std::string error_message = command;
                    std::string error_place(command.size() + 4, ' ');
                    error_place[0] = '\n';
                    error_place[error_place.size() - 2] = '\n';
                    error_place[error_place.size() - 1] = '\t';
                    if (ex.pos != -1)
                        error_place[ex.pos] = '^';
                    log::error("command", error_message + error_place + inner_ex.what());
                }
                return false;
            } catch (const std::exception& ex) {
                log::error("command", command + "\n Failed to execute command, reason:\n\t" + ex.what());
                return false;
            }
            return true;
        });

        log::info("Console", "console registered.");
    }

    void ConsolePlugin::OnUnload(const PluginRegistrationPtr& self) {
        clean_up_registered_events();
        api::console::unregister_virtual_client();
    }

    void ConsolePlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
        using predicate = base_objects::parser;
        using pred_string = base_objects::parsers::string;
        using cmd_pred_string = base_objects::parsers::command::string;
        {
            auto _console = browser.add_child("console");
            auto _log = _console.add_child("log");
            {
                auto& enable
                    = _log
                          .add_child("enable")
                          .add_child({"<log level>"}, cmd_pred_string::quotable_phrase)
                          .set_callback({"command.console.log.enable", {"console"}}, [](const list_array<predicate>& args, base_objects::command_context& context) {
                              auto& level = std::get<pred_string>(args[0]).value;
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

                auto& disable
                    = _log
                          .add_child("disable")
                          .add_child({"<log level>"}, cmd_pred_string::quotable_phrase)
                          .set_callback({"command.console.log.disable", {"console"}}, [](const list_array<predicate>& args, base_objects::command_context& context) {
                              auto& level = std::get<pred_string>(args[0]).value;
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

                add_log_type_suggestion(enable);
                add_log_type_suggestion(disable);
            }
            _console.add_child("clear")
                .set_callback({"command.console.clear", {"console"}}, [](const list_array<predicate>& args, base_objects::command_context& context) {
                    log::clear();
                });
        }
    }
}