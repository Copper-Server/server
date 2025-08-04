#include <src/api/network/tcp.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
namespace copper_server::base_objects {

    SharedClientData::SharedClientData(api::network::tcp::session* ss, void* assigned_data, std::function<void(base_objects::SharedClientData& self, base_objects::network::response&&)> special_callback)
        : assigned_data(assigned_data), ss(ss), special_callback(special_callback), player_data(reinterpret_cast<player&>(*new player())) {}

    void SharedClientData::send_indirect(base_objects::network::response&& resp) {
        ss->send_indirect(std::move(resp));
    }

    SharedClientData::~SharedClientData() {
        delete &player_data;
    }
}