#ifndef SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#define SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#include <src/api/new_packets.hpp>
#include <src/base_objects/shared_client_data.hpp>

namespace copper_server::base_objects {
    struct virtual_client {
        client_data_holder client;

        virtual_client(client_data_holder allocated, const std::string& name, const std::string& brand);

        std::function<void(const api::new_packets::client_bound::play_packet&)> packet_processor;
    };
}
#endif /* SRC_BASE_OBJECTS_VIRTUAL_CLIENT */
