#ifndef SRC_API_KICK
#define SRC_API_KICK
#include "../base_objects/event.hpp"

namespace crafted_craft::api::kick {
    struct ban_data {
        std::string who;
        std::string by;
        std::string reason;
    };

    extern base_objects::event<ban_data> on_kick;
}

#endif /* SRC_API_KICK */
