#ifndef SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#define SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#include <src/api/new_packets.hpp>
#include <src/base_objects/shared_client_data.hpp>

namespace copper_server::base_objects {
    struct virtual_client {
        client_data_holder client;

        virtual_client(client_data_holder allocated, const std::string& name, const std::string& brand);

        std::function<void(const api::new_packets::client_bound::play_packet&)> packet_processor;
        std::function<void()> requested_disconnect;

        void send(api::new_packets::server_bound::play_packet&&);
    };

    inline virtual_client& operator<<(virtual_client& client, api::new_packets::server_bound::play_packet&& packet) {
        client.send(std::move(packet));
        return client;
    }
}
#endif /* SRC_BASE_OBJECTS_VIRTUAL_CLIENT */
