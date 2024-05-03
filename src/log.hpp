#ifndef SRC_CONSOLE
#define SRC_CONSOLE
#include "base_objects/event.hpp"
#include <string>

namespace crafted_craft {
    namespace log {
        void info(const std::string& source, const std::string& message);

        void error(const std::string& source, const std::string& message);

        void warn(const std::string& source, const std::string& message);

        void critical(const std::string& source, const std::string& message);

        void debug(const std::string& source, const std::string& message);

        namespace commands {
            extern base_objects::event<std::string> on_command;
            void init();
        }
    }
}


#endif /* SRC_CONSOLE */
