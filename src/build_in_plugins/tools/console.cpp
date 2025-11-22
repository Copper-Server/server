/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/command.hpp>
#include <src/api/configuration.hpp>
#include <src/api/console.hpp>
#include <src/api/internal/console.hpp>
#include <src/api/log.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/virtual_client.hpp>
#include <src/util/readers.hpp>

#include <src/plugin/main.hpp>
#include <src/storage/list_storage.hpp>

namespace copper_server::build_in_plugins {
    void add_log_type_suggestion(base_objects::command_browser& browser) {
        browser.set_suggestion_callback([](const std::string& current, base_objects::command_context&) {
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

    struct console : public plugin_auto_register<"tools/console", console> {
        base_objects::virtual_client console_data{api::players::allocate_player(), "console", "console"};

        void on_load(const plugin_registration_ptr&) override {
            api::console::register_virtual_client(console_data);

            api::log::commands::registerCommandSuggestion([this](const std::string& line, int position) {
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
                    .to_container<std::vector>();
            });

            console_data.set_special_callback([](base_objects::virtual_client& _, base_objects::shared_client_data& client, base_objects::network::response&& resp) {
                resp.data.for_each([&](base_objects::network::response_item& data) {
                    if (data.data.empty())
                        return;
                    ArrayStream arr(data.data.data(), data.data.size());

                    api::packets::client_bound_play_ops::client_decode(client, arr, []<class T>(auto& client, T&& it) {
                        if constexpr (std::is_same_v<T, api::packets::client_bound::play::disguised_chat>) {
                            if (!it.target_name)
                                api::log::info("message", "[" + it.sender.to_ansi_console() + "] " + it.message.to_ansi_console());
                            else
                                api::log::info("message", "[" + it.sender.to_ansi_console() + " -> " + it.target_name->to_ansi_console() + "] " + it.message.to_ansi_console());
                        } else if constexpr (std::is_same_v<T, api::packets::client_bound::play::system_chat>) {
                            if (!it.is_overlay)
                                api::log::info("console", it.content.to_ansi_console());
                            else
                                api::log::info("console [overlay]", it.content.to_ansi_console());
                        }
                    });
                });
            });

            console_data.client->player_data.permission_groups = {"console", "operator"};

            register_event(api::console::on_command, base_objects::events::priority::high, [&](const std::string& command) {
                if (command.empty())
                    return false;
                try {
                    api::console::execute_as_console(command);
                } catch (const base_objects::command_exception& ex) {
                    std::string error_message = command;
                    std::string error_place(command.size() + 4, ' ');
                    error_place[0] = '\n';
                    error_place[error_place.size() - 2] = '\n';
                    error_place[error_place.size() - 1] = '\t';
                    if (ex.pos != -1)
                        error_place[ex.pos] = '^';
                    api::log::error("console", error_message + error_place + ex.what);
                    return false;
                } catch (const std::exception& ex) {
                    api::log::error("console", command + "\n Failed to execute command, reason:\n\t" + ex.what());
                    return false;
                }
                api::log::info("console", command);
                return true;
            });

            api::log::info("console", "console registered.");
        }

        void on_unload(const plugin_registration_ptr&) override {
            api::log::commands::unloadCommandSuggestion();
            clean_up_registered_events();
            api::console::unregister_virtual_client();
        }

        void on_commands_load(const plugin_registration_ptr&, base_objects::command_root_browser& browser) override {
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
                              .add_child({"log level"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                              .set_callback({"command.console.log.enable", {"console"}}, [](const list_array<predicate>& args, base_objects::command_context&) {
                                  auto& level = std::get<pred_string>(args[0]).value;
                                  if (level == "info")
                                      api::log::enable_log_level(api::log::level::info);
                                  else if (level == "warn")
                                      api::log::enable_log_level(api::log::level::warn);
                                  else if (level == "error")
                                      api::log::enable_log_level(api::log::level::error);
                                  else if (level == "fatal")
                                      api::log::enable_log_level(api::log::level::fatal);
                                  else if (level == "debug_error")
                                      api::log::enable_log_level(api::log::level::debug_error);
                                  else if (level == "debug")
                                      api::log::enable_log_level(api::log::level::debug);
                                  else {
                                      api::log::error("console", "log level " + level + " is undefined.");
                                      return false;
                                  }
                                  api::log::info("console", "Log level " + level + " is now enabled.");
                                  return true;
                              });

                    auto& disable
                        = _log
                              .add_child("disable")
                              .add_child({"log level"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                              .set_callback({"command.console.log.disable", {"console"}}, [](const list_array<predicate>& args, base_objects::command_context&) {
                                  auto& level = std::get<pred_string>(args[0]).value;
                                  if (level == "info")
                                      api::log::disable_log_level(api::log::level::info);
                                  else if (level == "warn")
                                      api::log::disable_log_level(api::log::level::warn);
                                  else if (level == "error")
                                      api::log::disable_log_level(api::log::level::error);
                                  else if (level == "fatal")
                                      api::log::disable_log_level(api::log::level::fatal);
                                  else if (level == "debug_error")
                                      api::log::disable_log_level(api::log::level::debug_error);
                                  else if (level == "debug")
                                      api::log::disable_log_level(api::log::level::debug);
                                  else {
                                      api::log::error("console", "log level " + level + " is undefined.");
                                      return false;
                                  }
                                  api::log::info("console", "Log level " + level + " is now disabled.");
                                  return true;
                              });

                    add_log_type_suggestion(enable);
                    add_log_type_suggestion(disable);
                }
                _console.add_child("clear")
                    .set_callback({"command.console.clear", {"console"}}, [](const list_array<predicate>&, base_objects::command_context&) {
                        api::log::clear();
                        return true;
                    });
            }
        }
    };
}