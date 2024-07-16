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
            std::shared_ptr<Commandline> cmd;
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

            fast_task::task_rw_mutex console_mutex;

            std::string color(uint8_t r, uint8_t g, uint8_t b, const std::string& message) {
                return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + message + "\033[0m";
            }

            void print(log::level level, const std::string& source, const std::string& message) {
                if (!log_levels_switch[(int)level] || !cmd)
                    return;
                auto [r, g, b] = log_levels_colors[(int)level];
                auto line = "[" + log_levels_names[(int)level] + "] [" + source + "] ";
                std::string alignment(line.size() + 1, ' ');
                alignment[0] = '\n';
                auto aligned_message = list_array<char>(message)
                                           .replace('\t', "    ", 4)
                                           .replace('\n', alignment.data(), line.size() + 1)
                                           .to_container<std::string>();
                fast_task::read_lock lock(console_mutex);
                if (cmd)
                    cmd->write(color(r, g, b, line + aligned_message));
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
            bool is_inited() {
                fast_task::read_lock lock(console::console_mutex);
                return (bool)console::cmd;
            }
            void init() {
                fast_task::write_lock lock(console::console_mutex);
                if (!console::cmd)
                    console::cmd = std::make_shared<Commandline>();
                console::cmd->enable_history();
                console::cmd->set_prompt(">");
                console::cmd->on_command = [](Commandline& cmd) {
                    on_command.async_notify(cmd.get_command());
                };
            }

            void deinit() {
                fast_task::write_lock lock(console::console_mutex);
                console::cmd.reset();
            }

            void registerCommandSuggestion(const std::function<std::vector<std::string>(const std::string&, int)>& callback) {
                fast_task::write_lock lock(console::console_mutex);
                if (console::cmd)
                    console::cmd->on_autocomplete = [callback](Commandline&, const std::string& line, int position) {
                        return callback(line, position);
                    };
            }

            void unloadCommandSuggestion() {
                fast_task::write_lock lock(console::console_mutex);
                if (console::cmd)
                    console::cmd->on_autocomplete = nullptr;
            }
        }
    }
}
