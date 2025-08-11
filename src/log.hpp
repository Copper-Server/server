/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_LOG
#define SRC_LOG
#include <filesystem>
#include <src/base_objects/events/event.hpp>
#include <string>

namespace copper_server::log {
    enum class level {
        info,
        warn,
        error,
        fatal,
        debug_error,
        debug,

        __max
    };
    void info(std::string_view source, std::string_view message);

    void error(std::string_view source, std::string_view message);

    void warn(std::string_view source, std::string_view message);

    void debug(std::string_view source, std::string_view message);

    void debug_error(std::string_view source, std::string_view message);

    void fatal(std::string_view source, std::string_view message);

    void clear();

    void disable_log_level(level);
    void enable_log_level(level);
    bool is_enabled(level);

    void disable_log_file_level(level);
    void enable_log_file_level(level);
    bool is_enabled_file(level);

    void set_log_folder(std::filesystem::path);
    void disable_log_folder();

    namespace commands {
        bool is_inited();
        void init();
        void deinit();
        void registerCommandSuggestion(const std::function<std::vector<std::string>(const std::string&, int)>& callback);
        void unloadCommandSuggestion();
        void set_history(const std::vector<std::string>& history);
        std::vector<std::string> load_history();
    }
}
#endif /* SRC_LOG */
