#ifndef SRC_API_BAN
#define SRC_API_BAN
#include <src/base_objects/event.hpp>

namespace copper_server::api::ban {
    struct ban_data {
        std::string who;
        std::string by;
        std::string reason;
    };

    extern base_objects::event<ban_data> on_ban;
    extern base_objects::event<ban_data> on_pardon;
    extern base_objects::event<ban_data> on_ban_ip;
    extern base_objects::event<ban_data> on_pardon_ip;
}
#endif /* SRC_API_BAN */
