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

    std::unordered_map<std::string, static_block_data*> block::named_full_block_data;
    list_array<std::unique_ptr<static_block_data>> block::full_block_data_;
    list_array<static_block_data*> block::general_block_data_;
    list_array<static_block_data*> block::block_entity_data_;

    bit_list_array<> block::cached_is_air;
    bit_list_array<> block::cached_is_solid;
    bit_list_array<> block::cached_is_liquid;
    bit_list_array<> block::cached_is_burnable;
    bit_list_array<> block::cached_is_emits_redstone;
    bit_list_array<> block::cached_is_full_cube;
    bit_list_array<> block::cached_is_tool_required;
    bit_list_array<> block::cached_is_replaceable;
    bit_list_array<> block::cached_is_block_entity;
    bit_list_array<> block::cached_is_default_state;
    bit_list_array<> block::cached_has_random_ticks;
    bit_list_array<> block::cached_has_comparator_output;
    list_array<static_block_data::transparent_sides_t> block::cached_transparent_sides;
    list_array<float> block::cached_slipperiness;
    list_array<float> block::cached_velocity_multiplier;
    list_array<float> block::cached_jump_velocity_multiplier;
    list_array<float> block::cached_hardness;
    list_array<float> block::cached_blast_resistance;
    list_array<int32_t> block::cached_map_color_rgb;
    list_array<int32_t> block::cached_block_entity_id;
    list_array<int32_t> block::cached_default_drop_item_id;
    list_array<int32_t> block::cached_experience;
    list_array<block_id_t> block::cached_general_block_id;
    list_array<block_id_t> block::cached_default_state;
    list_array<uint8_t> block::cached_luminance;
    list_array<uint8_t> block::cached_opacity;

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

    void block::initialize() {
        {
            list_array<static_block_data*> data;
            size_t max_ids = 0;
            data.resize(full_block_data_.size());
            for (auto& it : full_block_data_) {
                data[it->general_block_id] = it.get();
                max_ids = std::max<size_t>(it->general_block_id, max_ids);
            }
            data.resize(max_ids).commit();
            general_block_data_ = std::move(data);
        }
        {
            list_array<static_block_data*> data;
            size_t max_ids = 0;
            data.resize(full_block_data_.size());
            for (auto& it : full_block_data_) {
                if (!it->is_block_entity)
                    continue;
                data[it->block_entity_id] = it.get();
                max_ids = std::max<size_t>(it->block_entity_id, max_ids);
            }
            data.resize(max_ids).commit();
            block_entity_data_ = std::move(data);
        }

        {
            cached_is_air.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_air.set(i++, it->is_air);
        }
        {
            cached_is_solid.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_solid.set(i++, it->is_solid);
        }
        {
            cached_is_liquid.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_liquid.set(i++, it->is_liquid);
        }
        {
            cached_is_burnable.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_burnable.set(i++, it->is_burnable);
        }
        {
            cached_is_emits_redstone.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_emits_redstone.set(i++, it->is_emits_redstone);
        }
        {
            cached_is_full_cube.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_full_cube.set(i++, it->is_full_cube);
        }
        {
            cached_is_tool_required.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_tool_required.set(i++, it->is_tool_required);
        }
        {
            cached_is_replaceable.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_replaceable.set(i++, it->is_replaceable);
        }
        {
            cached_is_block_entity.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_block_entity.set(i++, it->is_block_entity);
        }
        {
            cached_is_default_state.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_is_default_state.set(i++, it->is_default_state);
        }
        {
            cached_has_random_ticks.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_has_random_ticks.set(i++, it->has_random_ticks);
        }
        {
            cached_has_comparator_output.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_has_comparator_output.set(i++, it->has_comparator_output);
        }
        {
            cached_transparent_sides.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_transparent_sides[i++] = it->transparent_sides;
        }
        {
            cached_slipperiness.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_slipperiness[i++] = it->slipperiness;
        }
        {
            cached_velocity_multiplier.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_velocity_multiplier[i++] = it->velocity_multiplier;
        }
        {
            cached_jump_velocity_multiplier.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_jump_velocity_multiplier[i++] = it->jump_velocity_multiplier;
        }
        {
            cached_hardness.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_hardness[i++] = it->hardness;
        }
        {
            cached_blast_resistance.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_blast_resistance[i++] = it->blast_resistance;
        }
        {
            cached_map_color_rgb.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_map_color_rgb[i++] = it->map_color_rgb;
        }
        {
            cached_block_entity_id.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_block_entity_id[i++] = it->block_entity_id;
        }
        {
            cached_default_drop_item_id.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_default_drop_item_id[i++] = it->default_drop_item_id;
        }
        {
            cached_experience.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_experience[i++] = it->experience;
        }
        {
            cached_general_block_id.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_general_block_id[i++] = it->general_block_id;
        }
        {
            cached_default_state.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_default_state[i++] = it->default_state;
        }
        {
            cached_luminance.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_luminance[i++] = it->luminance;
        }
        {
            cached_opacity.resize(full_block_data_.size()).commit();
            size_t i = 0;
            for (auto& it : full_block_data_)
                cached_opacity[i++] = it->opacity;
        }
    }

    void static_block_data::reset_blocks() {
        block::access_full_block_data([](auto& i0, auto& i1) {
            i0.clear();
            i1.clear();
        });
    }
}
