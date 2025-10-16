/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/entity.hpp>
#include <src/api/entity_proxy.hpp>
#include <src/api/network/tcp.hpp>
#include <src/api/packets.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>

namespace copper_server::base_objects {

    SharedClientData::packets_state_t::play_data_t::screen::screen(base_objects::SharedClientData& client) : client(client) {}

    void SharedClientData::packets_state_t::play_data_t::screen::close() {
        client << api::packets::client_bound::play::container_close{.windows_id = get_windows_id()};
        client.packets_state.get_play_data([&](auto& play_data) { play_data.current_screen.reset(); });
    }

    void SharedClientData::packets_state_t::play_data_t::screen::set_data(int16_t prop, int16_t data) {
        client << api::packets::client_bound::play::container_set_data{.windows_id = get_windows_id(), .value = api::packets::client_bound::play::container_set_data::other{.property = prop, .value = data}};
    }

    void SharedClientData::packets_state_t::play_data_t::screen::set_slot(int32_t slot, const base_objects::slot& item) {
        client << api::packets::client_bound::play::container_set_slot{.windows_id = get_windows_id(), .slot = (int16_t)slot, .item = item.to_packet()};
    }

    void SharedClientData::packets_state_t::play_data_t::screen::set_slot(int32_t slot, const base_objects::slot_data& item) {
        client << api::packets::client_bound::play::container_set_slot{.windows_id = get_windows_id(), .slot = (int16_t)slot, .item = item.to_packet()};
    }

    void SharedClientData::packets_state_t::play_data_t::screen::update_content() {
        client << api::packets::client_bound::play::container_set_content{
            .windows_id = get_windows_id(),
            .state_id = state_id++,
            .inventory_data = [&]() {
                api::packets::list_array_no_size<api::packets::slot> res;
                res.resize(max_size());
                iterate([&](base_objects::slot_data& data, int32_t slot) {
                    if (slot >= 0 && slot < (int32_t)res.size())
                        res[slot] = data.to_packet();
                });
                return res;
            }(),
            .carried_item = client.packets_state.get_play_data([&](auto& play_data) { return play_data.main_screen->get_carried_item().to_packet(); })
        };
    }

    void SharedClientData::packets_state_t::play_data_t::screen::drop_item(const base_objects::slot& item) {
        if (!item)
            return;
        if (client.player_data.assigned_entity) {
            api::entity assigned_entity(*client.player_data.assigned_entity);
            if (assigned_entity.current_world()) {
                auto entity = api::entity::create("minecraft:item");
                api::entity_proxy::item proxy(entity);
                proxy.stack() = item;
                entity.set<api::ecs::com::position>(assigned_entity.handle.get<api::ecs::com::position>());
                *entity.modify<api::ecs::com::motion>() = util::moved<double>(assigned_entity.handle.get<api::ecs::com::rotation>(), 2);
                assigned_entity.current_world()->register_entity(entity);
            }
        }
    }

    void SharedClientData::packets_state_t::play_data_t::screen::set_held_item(const base_objects::slot_data& item) {
        client << api::packets::client_bound::play::set_cursor_item{
            .item = item.to_packet()
        };
    }

    SharedClientData::SharedClientData(api::network::tcp::session* ss, void* assigned_data, std::function<void(base_objects::SharedClientData& self, base_objects::network::response&&)> special_callback)
        : player_data(reinterpret_cast<player&>(*new player())), special_callback(special_callback), assigned_data(assigned_data), ss(ss), skin_parts() {}

    void SharedClientData::send_indirect(base_objects::network::response&& resp) {
        ss->send_indirect(std::move(resp));
    }

    SharedClientData::~SharedClientData() {
        delete &player_data;
    }

    bool SharedClientData::is_active() const {
        return special_callback ? true : ss ? ss->is_active()
                                            : false;
    }
}