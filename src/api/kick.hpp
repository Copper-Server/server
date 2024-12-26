#ifndef SRC_API_KICK
#define SRC_API_KICK
#include <src/base_objects/events/event.hpp>

namespace copper_server::api::kick {
    struct ban_data {
        std::string who;
        std::string by;
        std::string reason;
    };

    extern base_objects::events::event<ban_data> on_kick;
}

#endif /* SRC_API_KICK */
