/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/ecs/base_components.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/generated/entity/components.hpp>
#include <src/generated/entity/components_to_packets.hpp>
#include <src/plugin/main.hpp>
#include <src/util/templates.hpp>

namespace copper_server::build_in_plugins::base::ecs {
    struct metadata_sync_system : public api::ecs::system_interface {
        using reads = util::apply_tuple_to<api::ecs::dependent, generated::com::all_metadata_components>;
        using writes = api::ecs::dependent<>;

        void tick(api::ecs::world_local_registry& world) override {
            std::unordered_map<api::ecs::entity, api::packets::client_bound::play::set_entity_data> data_updates;
            util::for_each_type<generated::com::all_mark_metadata_components>::each([&world, &data_updates]<class T> {
                for (auto&& [it] : world.view().with_changes<T>()) {
                    auto state = it.get_change_state<T>();
                    bool present = api::ecs::structural_changes::removed != state;
                    T tt;
                    auto res = generated::com::to_metadata(tt, present);
                    data_updates[it.current_entity()].metadata.emplace_back(res.first, std::move(res.second));
                }
            });
            util::for_each_type<generated::com::all_simple_metadata_components>::each([&world, &data_updates]<class T> {
                for (auto&& [it, item] : world.view().reads<T>().with_dirty<T>()) {
                    auto res = generated::com::to_metadata(item);
                    data_updates[it.current_entity()].metadata.emplace_back(res.first, std::move(res.second));
                }
            });
            for (auto [entity, packet] : data_updates)
                packet.id = entity.get<api::ecs::com::protocol_id>().value;
            //TODO send updates
        }
    };

    struct metadata_sync : public plugin_auto_register<"base/ecs/metadata_sync", metadata_sync> {
        void register_systems(api::ecs::scheduler& sched) override {
            sched.add_system<metadata_sync_system>(api::ecs::tick_phase::mobile_entity);
        }
    };
}
