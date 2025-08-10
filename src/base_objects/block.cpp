/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/block.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::base_objects {
    list_array<shape_data> static_block_data::all_shapes;
    list_array<std::string> static_block_data::block_entity_types;
    std::unordered_map<int32_t, std::unordered_set<std::string>> static_block_data::all_properties;
    boost::bimaps::bimap<
        boost::bimaps::unordered_set_of<int32_t, std::hash<int32_t>>,
        boost::bimaps::unordered_set_of<std::string, std::hash<std::string>>>
        static_block_data::assigned_property_name;

    std::unordered_map<std::string, std::shared_ptr<static_block_data>> block::named_full_block_data;
    list_array<std::shared_ptr<static_block_data>> block::full_block_data_;
    list_array<std::shared_ptr<static_block_data>> block::general_block_data_;
    list_array<std::shared_ptr<static_block_data>> block::block_entity_data_;

    void block::tick(storage::world_data& world, base_objects::world::sub_chunk_data& sub_chunk, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked) {
    retry:
        auto& static_data = getStaticData();
        switch (tickable) {
        case tick_opt::block_tickable:
            static_data.on_tick(world, sub_chunk, *this, chunk_x, sub_chunk_y, chunk_z, local_x, local_y, local_z, random_ticked);
            return;
        case tick_opt::entity_tickable:
            static_data.as_entity_on_tick(world, sub_chunk, *this, sub_chunk.get_block_entity_data(local_x, local_y, local_z), chunk_x, sub_chunk_y, chunk_z, local_x, local_y, local_z, random_ticked);
            return;
        case tick_opt::undefined:
            tickable = static_data.resolve_tickable();
            goto retry;
            break;
        default:
        case tick_opt::no_tick:
            break;
        }
    }

    block::tick_opt block::resolve_tickable(base_objects::block_id_t block_id) {
        return base_objects::block(block_id).getStaticData().resolve_tickable();
    }

    bool block::is_tickable() {
        switch (tickable) {
        case tick_opt::block_tickable:
        case tick_opt::entity_tickable:
            return true;
        case tick_opt::undefined:
            tickable = getStaticData().resolve_tickable();
            return tickable != tick_opt::no_tick;
        default:
        case tick_opt::no_tick:
            return false;
        }
    }

    bool block::is_tickable() const {
        switch (tickable) {
        case tick_opt::block_tickable:
        case tick_opt::entity_tickable:
        case tick_opt::undefined:
            return true;
        default:
        case tick_opt::no_tick:
            return false;
        }
    }

    bool block::is_solid() const {
        return getStaticData().is_solid;
    }

    const std::vector<shape_data*>& block::collision_shapes() const {
        return getStaticData().collision_shapes;
    }

    const std::string& block::instrument() const {
        return getStaticData().instrument;
    }

    const std::string& block::piston_behavior() const {
        return getStaticData().piston_behavior;
    }

    const std::string& block::name() const {
        return getStaticData().name;
    }

    const std::string& block::translation_key() const {
        return getStaticData().translation_key;
    }

    block_id_t block::general_block_id() const {
        return getStaticData().general_block_id;
    }

    float block::slipperiness() const {
        return getStaticData().slipperiness;
    }

    float block::velocity_multiplier() const {
        return getStaticData().velocity_multiplier;
    }

    float block::jump_velocity_multiplier() const {
        return getStaticData().jump_velocity_multiplier;
    }

    float block::hardness() const {
        return getStaticData().hardness;
    }

    float block::blast_resistance() const {
        return getStaticData().blast_resistance;
    }

    int32_t block::map_color_rgb() const {
        return getStaticData().map_color_rgb;
    }

    int32_t block::block_entity_id() const {
        return getStaticData().block_entity_id;
    }

    int32_t block::default_drop_item_id() const {
        return getStaticData().default_drop_item_id;
    }

    int32_t block::experience() const {
        return getStaticData().experience;
    }

    block_id_t block::default_state() const {
        return getStaticData().default_state;
    }

    uint8_t block::luminance() const {
        return getStaticData().luminance;
    }

    uint8_t block::opacity() const {
        return getStaticData().opacity;
    }

    bool block::is_air() const {
        return getStaticData().is_air;
    }

    bool block::is_liquid() const {
        return getStaticData().is_liquid;
    }

    bool block::is_burnable() const {
        return getStaticData().is_burnable;
    }

    bool block::is_emits_redstone() const {
        return getStaticData().is_emits_redstone;
    }

    bool block::is_full_cube() const {
        return getStaticData().is_full_cube;
    }

    bool block::is_tool_required() const {
        return getStaticData().is_tool_required;
    }

    bool block::is_sided_transparency() const {
        auto sides = getStaticData().transparent_sides;
        return sides.down_side_solid
               && sides.up_side_solid
               && sides.north_side_solid
               && sides.south_side_solid
               && sides.west_side_solid
               && sides.east_side_solid
               && sides.down_center_solid
               && sides.up_center_solid;
    }

    bool block::is_replaceable() const {
        return getStaticData().is_replaceable;
    }

    bool block::is_block_entity() const {
        return getStaticData().is_block_entity;
    }

    void block::initialize() {
        {
            list_array<std::shared_ptr<static_block_data>> data;
            size_t max_ids = 0;
            data.resize(full_block_data_.size());
            for (auto& it : full_block_data_) {
                data[it->general_block_id] = it;
                max_ids = std::max<size_t>(it->general_block_id, max_ids);
            }
            data.resize(max_ids);
            general_block_data_ = data;
        }
        {
            list_array<std::shared_ptr<static_block_data>> data;
            size_t max_ids = 0;
            data.resize(full_block_data_.size());
            for (auto& it : full_block_data_) {
                if (!it->is_block_entity)
                    continue;
                data[it->block_entity_id] = it;
                max_ids = std::max<size_t>(it->block_entity_id, max_ids);
            }
            data.resize(max_ids);
            block_entity_data_ = data;
        }
    }

    size_t block::block_states_size() {
        return full_block_data_.size();
    }

    void static_block_data::reset_blocks() {
        block::access_full_block_data([](auto& i0, auto& i1) {
            i0.clear();
            i1.clear();
        });
    }
}
