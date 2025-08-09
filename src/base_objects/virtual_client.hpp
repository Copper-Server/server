#ifndef SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#define SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#include <src/base_objects/shared_client_data.hpp>

namespace copper_server::api::packets {
    namespace client_bound {
        struct play_packet;
    }

    namespace server_bound {
        struct play_packet;
    }
}

namespace copper_server::base_objects {
    struct virtual_client {
        client_data_holder client;

        virtual_client(client_data_holder allocated, const std::string& name, const std::string& brand);

        std::function<void(const api::packets::client_bound::play_packet&)> packet_processor;
        std::function<void()> requested_disconnect;

        void send(api::packets::server_bound::play_packet&&);
    };

    inline virtual_client& operator<<(virtual_client& client, api::packets::server_bound::play_packet&& packet) {
        client.send(std::move(packet));
        return client;
    }
}
#endif /* SRC_BASE_OBJECTS_VIRTUAL_CLIENT */
