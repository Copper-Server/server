#include <src/api/packets.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/virtual_client.hpp>
#include <src/registers.hpp>

namespace copper_server::base_objects {

    virtual_client::virtual_client(client_data_holder allocated, const std::string& name, const std::string& brand)
        : client(allocated) {

        client->name = name;
        client->ip = "";
        client->client_brand = brand;
        client->locale = "en_US";
        client->data = {};

        client->player_data.local_data["virtual_client"] = name;
        client->player_data.gamemode = (uint8_t)-1;
        client->player_data.op_level = 4;
        client->player_data.world_id = "virtual_client astral space";
        client->is_virtual = true;
        client->packets_state.protocol_version = registers::current_protocol_id;
        client->packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::play;
        client->special_callback = [this](base_objects::SharedClientData& self, network::response&& resp) {
            resp.data.for_each([&](base_objects::network::response::item& it) {
                if (it.data.empty())
                    return;
                ArrayStream arr(it.data.data(), it.data.size());
                packet_processor(api::packets::decode_client_play(*client, arr));
            });
            if (resp.is_disconnect())
                requested_disconnect();
        };
    }

    void virtual_client::send(api::packets::server_bound::play_packet&& packet) {
        api::packets::make_process(*client, std::move(packet));
    }
}
