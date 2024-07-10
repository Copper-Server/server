#ifndef SRC_LOG
#define SRC_LOG
#include "base_objects/event.hpp"
#include <string>

namespace crafted_craft {
    namespace log {
        enum class level {
            info,
            warn,
            error,
            fatal,
            debug_error,
            debug,

            __max
        };
        void info(const std::string& source, const std::string& message);

        void error(const std::string& source, const std::string& message);

        void warn(const std::string& source, const std::string& message);

        void debug(const std::string& source, const std::string& message);

        void debug_error(const std::string& source, const std::string& message);

        void fatal(const std::string& source, const std::string& message);

        void disable_log_level(level);
        void enable_log_level(level);
        bool is_enabled(level);

        namespace commands {
            extern base_objects::event<std::string> on_command;
            bool is_inited();
            void init();
            void deinit();
            void registerCommandSuggestion(const std::function<std::vector<std::string>(const std::string&, int)>& callback);
            void unloadCommandSuggestion();
        }
    }
}


#endif /* SRC_LOG */
