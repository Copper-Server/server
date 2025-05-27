#ifndef SRC_API_NETWORK
#define SRC_API_NETWORK
#include <library/fast_task/src/networking/networking.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <src/base_objects/events/sync_event.hpp>

namespace copper_server::base_objects {
    namespace network::tcp {
        class client;
    }
    struct SharedClientData;
    using client_data_holder = atomic_holder<SharedClientData>;
}

namespace copper_server::api::network {
    extern base_objects::events::sync_event<const fast_task::networking::address&> ip_filter;
}

#endif /* SRC_API_NETWORK */
