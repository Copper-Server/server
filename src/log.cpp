#include "log.hpp"
#include "base_objects/event.hpp"
#include "library/fast_task.hpp"
#include "library/list_array.hpp"
#include "util/task_management.hpp"
#include <commandline.h>
#include <iostream>
#include <mutex>

namespace crafted_craft {
    namespace log {
        namespace commands {
            base_objects::event<std::string> on_command;
        }

        namespace console {
            Commandline cmd;
            std::atomic_bool log_levels_switch[(int)level::__max]{
                true, //info
                true, //warn
                true, //error
                true, //fatal
                true, //debug_error
                true, //debug
            };
            std::tuple<uint8_t, uint8_t, uint8_t> log_levels_colors[(int)level::__max] = {
                {128, 128, 128}, //info
                {128, 0, 0},     //warn
                {128, 128, 0},   //error
                {128, 0, 0},     //fatal
                {170, 128, 0},   //debug_error
                {0, 128, 0},     //debug
            };
            std::string log_levels_names[(int)level::__max] = {
                "info",
                "warn",
                "error",
                "fatal",
                "debug_error",
                "debug",
            };

            std::string current_line;
            list_array<std::tuple<log::level, std::string, std::string>> ouput_queue;
            fast_task::task_mutex console_mutex;
            fast_task::task_condition_variable output_mutex;

            std::string color(uint8_t r, uint8_t g, uint8_t b, const std::string& message) {
                return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + message + "\033[0m";
            }

            void print(log::level level, const std::string& source, const std::string& message) {
                if (!log_levels_switch[(int)level])
                    return;
                auto [r, g, b] = log_levels_colors[(int)level];
                auto line = "[" + log_levels_names[(int)level] + "] [" + source + "] ";
                std::string aligned_message;
                std::string alignment(line.size(), ' ');
                aligned_message.reserve(message.size() + alignment.size() + 1);
                for (char c : message) {
                    if (c == '\n')
                        aligned_message += "\n" + alignment;
                    else
                        aligned_message += c;
                }
                cmd.write(color(r, g, b, line + aligned_message));
            }
        } // namespace console

        void info(const std::string& source, const std::string& message) {
            console::print(level::info, source, message);
        }

        void error(const std::string& source, const std::string& message) {
            console::print(level::error, source, message);
        }

        void warn(const std::string& source, const std::string& message) {
            console::print(level::warn, source, message);
        }

        void debug(const std::string& source, const std::string& message) {
            console::print(level::debug, source, message);
        }

        void debug_error(const std::string& source, const std::string& message) {
            console::print(level::debug_error, source, message);
        }

        void fatal(const std::string& source, const std::string& message) {
            console::print(level::fatal, source, message);
            std::exit(EXIT_FAILURE);
        }

        void disable_log_level(level level) {
            console::log_levels_switch[(int)level] = false;
        }

        void enable_log_level(level level) {
            console::log_levels_switch[(int)level] = true;
        }

        bool is_enabled(level level) {
            return console::log_levels_switch[(int)level];
        }
        namespace commands {
            void init() {
                console::cmd.enable_history();
                console::cmd.on_command = [](Commandline& cmd) {
                    Task::start([command = cmd.get_command()]() {
                        if (!on_command.await_notify(command))
                            error("Server", "Undefined command: " + command);
                    });
                };
            }

            void registerCommandSuggestion(const std::function<std::vector<std::string>(const std::string&, int)>& callback) {
                console::cmd.on_autocomplete = [callback](Commandline&, const std::string& line, int position) {
                    return callback(line, position);
                };
            }

            void unloadCommandSuggestion() {
                console::cmd.on_autocomplete = nullptr;
            }
        }
    }
}
