#include "base_objects/event.hpp"
#include "library/list_array.hpp"
#include <iostream>
#include <mutex>
#include <string>

namespace crafted_craft {
    namespace log {
        namespace commands {
            base_objects::event<std::string> on_command;
        }

        namespace console {
            std::string current_line;
            fast_task::task_mutex console_mutex;

            //use ascii escape codes to clear the line
            void clean_line() {
                std::cout << "\033[2K\r" << std::flush;
            }

            std::string color(uint8_t r, uint8_t g, uint8_t b, const std::string& message) {
                return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + message + "\033[0m";
            }

            std::string read_buffer() {
                size_t to_read = std::cin.rdbuf()->in_avail();
                std::string buffer;
                buffer.resize(to_read);
                std::cin.readsome(buffer.data(), to_read);
                return buffer;
            }

            void check_console() {
                current_line += read_buffer();
                std::string_view view(current_line);
                for (auto i = view.find('\n'); i != std::string::npos; i = view.find('\n')) {
                    commands::on_command((std::string)current_line.substr(0, i));
                    view.remove_prefix(i + 1);
                }
                current_line = std::string(view);
            }

            void print(const std::string& message) {
                std::lock_guard<fast_task::task_mutex> lock(console_mutex);
                check_console();
                clean_line();

                std::cout << message << std::endl
                          << current_line << std::flush;
            }

            void check_console_task() {
                while (true) {
                    {
                        std::lock_guard<fast_task::task_mutex> lock(console_mutex);
                        check_console();
                    }
                    fast_task::task::sleep(100);
                }
            }
        } // namespace console

        void info(const std::string& source, const std::string& message) {
            console::print(console::color(128, 128, 128, "[info] [" + source + "] " + message));
        }

        void error(const std::string& source, const std::string& message) {
            console::print(console::color(128, 0, 0, "[error] [" + source + "] " + message));
        }

        void warn(const std::string& source, const std::string& message) {
            console::print(console::color(128, 128, 0, "[warn] [" + source + "] " + message));
        }

        void critical(const std::string& source, const std::string& message) {
            console::print(console::color(128, 0, 128, "[critical] [" + source + "] " + message));
            std::exit(EXIT_FAILURE);
        }

        void debug(const std::string& source, const std::string& message) {
            console::print(console::color(0, 128, 0, "[debug] [" + source + "] " + message));
        }

        namespace commands {
            void init() {
                fast_task::task::start(std::make_shared<fast_task::task>(console::check_console_task));
            }
        }
    }
}
