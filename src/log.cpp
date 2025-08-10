/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <commandline.h>
#include <iostream>
#include <library/fast_task.hpp>
#include <library/fast_task/include/files.hpp>
#include <library/list_array.hpp>
#include <mutex>
#include <src/api/console.hpp>
#include <src/base_objects/events/event.hpp>
#include <src/log.hpp>
#include <src/util/task_management.hpp>

namespace copper_server::log {

    std::string log_levels_names[(int)level::__max] = {
        "info",
        "warn",
        "error",
        "fatal",
        "debug_error",
        "debug",
    };

    namespace file {
        fast_task::protected_value<std::shared_ptr<fast_task::files::async_iofstream>> handle;

        void set_handle(std::filesystem::path path) {
            handle.set(
                [&path](auto& value) {
                    value = std::make_shared<fast_task::files::async_iofstream>(
                        path,
                        fast_task::files::open_mode::append,
                        fast_task::files::on_open_action::open,
                        fast_task::files::_sync_flags{}
                    );
                }
            );
        }

        void clear_handle() {
            handle.set([](auto& value) { value = nullptr; });
        }

        std::atomic_bool log_levels_switch[(int)level::__max]{
            true, //info
            true, //warn
            true, //error
            true, //fatal
            true, //debug_error
            true, //debug
        };

        void print(log::level level, std::string_view source, std::string_view message) {
            auto duration = std::chrono::system_clock::now().time_since_epoch();

            auto years = std::chrono::duration_cast<std::chrono::years>(duration);
            duration -= years;
            auto months = std::chrono::duration_cast<std::chrono::months>(duration);
            duration -= months;
            auto days = std::chrono::duration_cast<std::chrono::days>(duration);
            duration -= days;
            auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
            duration -= hours;
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
            duration -= minutes;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
            duration -= seconds;
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            duration -= milliseconds;
            auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            duration -= microseconds;
            auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

            handle.get([&](auto& handle) {
                if (!log_levels_switch[(int)level] || !handle)
                    return;
                auto line = "[" + std::to_string(days.count()) + "/" + std::to_string(months.count()) + "/" + std::to_string(years.count()) + "] ["
                            + std::to_string(hours.count()) + ":" + std::to_string(minutes.count()) + ":" + std::to_string(seconds.count()) + " " + std::to_string(milliseconds.count()) + "." + std::to_string(microseconds.count()) + "." + std::to_string(nanoseconds.count()) + "] ["
                            + log_levels_names[(int)level] + "] ["
                            + std::string(source) + "] ";
                std::string alignment(line.size(), ' ');
                alignment[0] = '\n';
                auto aligned_message = list_array<char>(message)
                                           .replace('\t', "    ", 4)
                                           .replace('\n', alignment.data(), line.size())
                                           .push_front(list_array<char>(line))
                                           .push_back('\n')
                                           .to_container<std::string>();

                *handle << aligned_message << std::flush;
            });
        }
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
            {128, 128, 0},   //warn
            {128, 0, 0},     //error
            {128, 0, 0},     //fatal
            {170, 128, 0},   //debug_error
            {0, 128, 0},     //debug
        };


        fast_task::task_rw_mutex console_mutex;

        std::string color(uint8_t r, uint8_t g, uint8_t b, const std::string& message) {
            return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + message + "\033[0m";
        }

        void print(log::level level, std::string_view source, std::string_view message) {
            if (!log_levels_switch[(int)level] || !cmd)
                return;
            auto [r, g, b] = log_levels_colors[(int)level];
            auto line = "[" + log_levels_names[(int)level] + "] [" + std::string(source) + "] ";
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

        void direct_print(std::string_view message) {
            fast_task::read_lock lock(console_mutex);
            if (cmd)
                cmd->write(std::string(message));
        }
    } // namespace console

    void info(std::string_view source, std::string_view message) {
        console::print(level::info, source, message);
        file::print(level::info, source, message);
    }

    void error(std::string_view source, std::string_view message) {
        console::print(level::error, source, message);
        file::print(level::error, source, message);
    }

    void warn(std::string_view source, std::string_view message) {
        console::print(level::warn, source, message);
        file::print(level::warn, source, message);
    }

    void debug(std::string_view source, std::string_view message) {
        console::print(level::debug, source, message);
        file::print(level::debug, source, message);
    }

    void debug_error(std::string_view source, std::string_view message) {
        console::print(level::debug_error, source, message);
        file::print(level::debug_error, source, message);
    }

    void fatal(std::string_view source, std::string_view message) {
        console::print(level::fatal, source, message);
        file::print(level::fatal, source, message);
    }

    void clear() {
        console::direct_print("\033[2J");
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

    void disable_log_file_level(level level) {
        file::log_levels_switch[(int)level] = false;
    }

    void enable_log_file_level(level level) {
        file::log_levels_switch[(int)level] = true;
    }

    bool is_enabled_file(level level) {
        return file::log_levels_switch[(int)level];
    }

    void set_log_folder(std::filesystem::path path) {
        std::filesystem::create_directories(path);
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        auto years = std::chrono::duration_cast<std::chrono::years>(duration);
        duration -= years;
        auto months = std::chrono::duration_cast<std::chrono::months>(duration);
        duration -= months;
        auto days = std::chrono::duration_cast<std::chrono::days>(duration);
        duration -= days;
        auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
        duration -= hours;
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
        duration -= minutes;
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
        std::filesystem::path file;
        do {
            file = path
                   / ("log_"
                      + std::to_string(years.count())
                      + "_" + std::to_string(months.count())
                      + "_" + std::to_string(days.count())
                      + "_" + std::to_string(hours.count())
                      + "_" + std::to_string(minutes.count())
                      + "_" + std::to_string(seconds.count())
                      + ".txt");
        } while (std::filesystem::exists(file));

        file::set_handle(file);
    }

    void disable_log_folder() {
        file::clear_handle();
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
                auto command = cmd.get_command();
                if (command.ends_with('\r'))
                    command = command.substr(0, command.size() - 1);
                api::console::on_command.async_notify(command);
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

        void set_history(const std::vector<std::string>& history) {
            fast_task::write_lock lock(console::console_mutex);
            if (console::cmd)
                console::cmd->set_history(history);
        }

        std::vector<std::string> load_history() {
            fast_task::write_lock lock(console::console_mutex);
            if (console::cmd)
                return console::cmd->history();
            return {};
        }
    }
}