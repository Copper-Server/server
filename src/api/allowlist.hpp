#ifndef SRC_API_ALLOWLIST
#define SRC_API_ALLOWLIST
#include <src/base_objects/event.hpp>

namespace copper_server::api::allowlist {
    enum class allowlist_mode {
        allow,
        block,
        off
    };
    extern base_objects::event<allowlist_mode> on_mode_change;
    // std::string is the player name
    extern base_objects::event<std::string> on_kick;
    extern base_objects::event<std::string> on_add;
    extern base_objects::event<std::string> on_remove;
}
#endif /* SRC_API_ALLOWLIST */
