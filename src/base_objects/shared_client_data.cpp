#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>

namespace copper_server::base_objects {

    SharedClientData::SharedClientData(void* assigned_data, std::function<void(SharedClientData& self)> special_callback)
        : assigned_data(assigned_data), special_callback(special_callback), player_data(reinterpret_cast<player&>(*new player())) {}

    SharedClientData::~SharedClientData() {
        delete &player_data;
    }
}