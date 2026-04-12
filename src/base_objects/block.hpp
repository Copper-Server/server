/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#pragma once
#include <cstdint>
#include <library/list_array.hpp>
#include <map>
#include <mutex>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/number_provider.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

namespace copper_server {
    namespace storage {
        class world_data;
    }

    namespace base_objects {
        namespace world {
            struct sub_chunk_data;
        }
        struct block;
        typedef uint32_t block_id_t;

        struct shape_data final {
            double min_x, min_y, min_z;
            double max_x, max_y, max_z;
        };

        class static_block_data final {
            struct block_state_hash {
                size_t operator()(const std::unordered_map<std::string, std::string>& value) const noexcept {
                    size_t result = 0;
                    std::hash<std::string> string_hasher;
                    for (auto& [key, val] : value) {
                        result ^= string_hasher(key) & string_hasher(val);
                    }
                    return result;
                }
            };

        public:
            base_objects::chat display_name;
            std::shared_ptr<util::nbt> loot_table;
            std::vector<shape_data*> collision_shapes;
            std::vector<shape_data*> outline_shapes;
            std::string instrument;
            std::string piston_behavior;
            std::string name;
            std::string translation_key;
            float slipperiness = 0;
            float velocity_multiplier = 0;
            float jump_velocity_multiplier = 0;
            float hardness = 0;
            float blast_resistance = 0;
            int32_t map_color_rgb = 0;
            int32_t block_entity_id = 0; //used only when is_block_entity == true, like interact and storage
            int32_t item_id = 0;
            int32_t experience = 0;
            block_id_t general_block_id = 0; //does not represent state
            block_id_t default_state = 0;
            block_id_t current_state = 0;
            uint8_t luminance = 0;
            uint8_t opacity = 0; //255 not opaque
            bool is_air : 1 = true;
            bool is_solid : 1 = false;
            bool is_liquid : 1 = false;
            bool is_burnable : 1 = false;
            bool is_emits_redstone : 1 = false;
            bool is_full_cube : 1 = false;
            bool is_tool_required : 1 = false;
            bool is_replaceable : 1 = false;
            bool is_block_entity : 1 = false;
            bool is_default_state : 1 = false;
            bool has_random_ticks : 1 = false;
            bool has_comparator_output : 1 = false;
            bool is_tickable : 1 = false;

            struct transparent_sides_t {
                bool down_side_solid : 1 = false;
                bool up_side_solid : 1 = false;
                bool north_side_solid : 1 = false;
                bool south_side_solid : 1 = false;
                bool west_side_solid : 1 = false;
                bool east_side_solid : 1 = false;
                bool down_center_solid : 1 = false;
                bool up_center_solid : 1 = false;
            } transparent_sides;

            struct flammable_t {
                float spread_chance;
                float burn_chance;
            };

            std::optional<flammable_t> flammable;

            struct ore_data_t {
                std::shared_ptr<number_provider> experience;
            };

            std::optional<ore_data_t> ore_data;


            bool can_explode(float explode_strength) const {
                return blast_resistance < explode_strength;
            }

            bool can_break(float break_strength) const { //use when in client side player break, server need calculate break tick long
                return hardness < break_strength;
            }

            //on tick checks `on_tick`, if undefined then do nothing, otherwise call the callback
            std::function<void(storage::world_data&, world::sub_chunk_data&, block& data, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked)> on_tick;

            //used to check properties usage
            std::vector<int32_t> allowed_properties;


            using map_of_states = boost::bimaps::bimap<
                boost::bimaps::unordered_set_of<block_id_t, std::hash<block_id_t>>,
                boost::bimaps::unordered_set_of<std::unordered_map<std::string, std::string>, block_state_hash>>;
            std::shared_ptr<map_of_states> assigned_states_to_properties;
            std::unordered_map<std::string, std::string> current_properties;

            list_array<std::string> block_aliases; //string block ids(checks from first to last, if none found in `initialize_blocks()` throws) implicitly uses id first

            static_block_data() {}

            static_block_data(const static_block_data& copy)
                : loot_table(copy.loot_table),
                  collision_shapes(copy.collision_shapes),
                  instrument(copy.instrument),
                  piston_behavior(copy.piston_behavior),
                  name(copy.name),
                  translation_key(copy.translation_key),
                  slipperiness(copy.slipperiness),
                  velocity_multiplier(copy.velocity_multiplier),
                  jump_velocity_multiplier(copy.jump_velocity_multiplier),
                  hardness(copy.hardness),
                  blast_resistance(copy.blast_resistance),
                  map_color_rgb(copy.map_color_rgb),
                  block_entity_id(copy.block_entity_id),
                  item_id(copy.item_id),
                  experience(copy.experience),
                  general_block_id(copy.general_block_id),
                  default_state(copy.default_state),
                  current_state(copy.current_state),
                  luminance(copy.luminance),
                  opacity(copy.opacity),
                  is_air(copy.is_air),
                  is_solid(copy.is_solid),
                  is_liquid(copy.is_liquid),
                  is_burnable(copy.is_burnable),
                  is_emits_redstone(copy.is_emits_redstone),
                  is_full_cube(copy.is_full_cube),
                  is_tool_required(copy.is_tool_required),
                  is_replaceable(copy.is_replaceable),
                  is_block_entity(copy.is_block_entity),
                  is_default_state(copy.is_default_state),
                  has_random_ticks(copy.has_random_ticks),
                  has_comparator_output(copy.has_comparator_output),
                  transparent_sides(copy.transparent_sides),
                  on_tick(copy.on_tick),
                  allowed_properties(copy.allowed_properties),
                  assigned_states_to_properties(copy.assigned_states_to_properties),
                  current_properties(copy.current_properties),
                  block_aliases(copy.block_aliases) {}

            static_block_data(static_block_data&& copy) noexcept
                : loot_table(std::move(copy.loot_table)),
                  collision_shapes(std::move(copy.collision_shapes)),
                  instrument(std::move(copy.instrument)),
                  piston_behavior(std::move(copy.piston_behavior)),
                  name(std::move(copy.name)),
                  translation_key(std::move(copy.translation_key)),
                  slipperiness(std::move(copy.slipperiness)),
                  velocity_multiplier(std::move(copy.velocity_multiplier)),
                  jump_velocity_multiplier(std::move(copy.jump_velocity_multiplier)),
                  hardness(std::move(copy.hardness)),
                  blast_resistance(std::move(copy.blast_resistance)),
                  map_color_rgb(std::move(copy.map_color_rgb)),
                  block_entity_id(std::move(copy.block_entity_id)),
                  item_id(std::move(copy.item_id)),
                  experience(std::move(copy.experience)),
                  general_block_id(std::move(copy.general_block_id)),
                  default_state(std::move(copy.default_state)),
                  current_state(std::move(copy.current_state)),
                  luminance(std::move(copy.luminance)),
                  opacity(std::move(copy.opacity)),
                  is_air(copy.is_air),
                  is_solid(copy.is_solid),
                  is_liquid(copy.is_liquid),
                  is_burnable(copy.is_burnable),
                  is_emits_redstone(copy.is_emits_redstone),
                  is_full_cube(copy.is_full_cube),
                  is_tool_required(copy.is_tool_required),
                  is_replaceable(copy.is_replaceable),
                  is_block_entity(copy.is_block_entity),
                  is_default_state(copy.is_default_state),
                  has_random_ticks(copy.has_random_ticks),
                  has_comparator_output(copy.has_comparator_output),
                  transparent_sides(std::move(copy.transparent_sides)),
                  on_tick(std::move(copy.on_tick)),
                  allowed_properties(std::move(copy.allowed_properties)),
                  assigned_states_to_properties(std::move(copy.assigned_states_to_properties)),
                  current_properties(std::move(copy.current_properties)),
                  block_aliases(std::move(copy.block_aliases)) {}

            //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
            static void reset_blocks(); //INTERNAL

            static list_array<shape_data> all_shapes;
            static list_array<std::string> block_entity_types;
            static std::unordered_map<int32_t, std::unordered_set<std::string>> all_properties;


            static boost::bimaps::bimap<
                boost::bimaps::unordered_set_of<int32_t, std::hash<int32_t>>,
                boost::bimaps::unordered_set_of<std::string, std::hash<std::string>>>
                assigned_property_name;

            inline static const std::unordered_set<std::string>& get_allowed_property_values(int32_t property_id) {
                static std::unordered_set<std::string> local;
                auto it = all_properties.find(property_id);
                if (it != all_properties.end())
                    return it->second;
                else
                    return local;
            }

            inline static const std::unordered_set<std::string>& get_allowed_property_values(const std::string& property_id) {
                static std::unordered_set<std::string> local;
                auto it = all_properties.find(assigned_property_name.right.at(property_id));
                if (it != all_properties.end())
                    return it->second;
                else
                    return local;
            }
        };

        struct block final {
            base_objects::block_id_t id = 0;


            static void initialize();

            static block_id_t addNewStatelessBlock(static_block_data&& new_block) {
                if (named_full_block_data.contains(new_block.name))
                    throw std::runtime_error("Block with " + new_block.name + " name already defined.");


                auto new_id = full_block_data_.size();
                if (full_block_data_.size() >= INT32_MAX)
                    throw std::out_of_range("Blocks count out of range, block can't added");
                auto block_ = std::make_unique<static_block_data>(std::move(new_block));
                auto& new_loc = full_block_data_.emplace_back(std::move(block_));

                named_full_block_data[new_loc->name] = new_loc.get();
                return (int32_t)new_id;
            }

            static void access_full_block_data(std::function<void(list_array<std::unique_ptr<static_block_data>>&, std::unordered_map<std::string, static_block_data*>&)> access) {
                access(full_block_data_, named_full_block_data);
            }

            static block_id_t addNewStatelessBlock(const static_block_data& new_block) {
                return addNewStatelessBlock(static_block_data(new_block));
            }

            const static_block_data& getStaticData() const {
                return *full_block_data_.at(id);
            }

            static const static_block_data& getStaticData(block_id_t id) {
                return *full_block_data_.at(id);
            }

            inline bool operator==(const block& b) const {
                return id == b.id;
            }

            inline bool operator!=(const block& b) const {
                return id != b.id;
            }

            void tick(storage::world_data&, base_objects::world::sub_chunk_data& sub_chunk, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked);

            inline const std::vector<shape_data*>& collision_shapes() const {
                return getStaticData().collision_shapes;
            }

            inline const base_objects::chat& display_name() const {
                return getStaticData().display_name;
            }

            inline const std::string& instrument() const {
                return getStaticData().instrument;
            }

            inline const std::string& piston_behavior() const {
                return getStaticData().piston_behavior;
            }

            inline const std::string& name() const {
                return getStaticData().name;
            }

            inline const std::string& translation_key() const {
                return getStaticData().translation_key;
            }

            inline block_id_t general_block_id() const {
                return cached_general_block_id[id];
            }

            inline float slipperiness() const {
                return cached_slipperiness[id];
            }

            inline float velocity_multiplier() const {
                return cached_velocity_multiplier[id];
            }

            inline float jump_velocity_multiplier() const {
                return cached_jump_velocity_multiplier[id];
            }

            inline float hardness() const {
                return cached_hardness[id];
            }

            inline float blast_resistance() const {
                return cached_blast_resistance[id];
            }

            inline int32_t map_color_rgb() const {
                return cached_map_color_rgb[id];
            }

            inline int32_t block_entity_id() const {
                return cached_block_entity_id[id];
            }

            inline int32_t item_id() const {
                return cached_item_id[id];
            }

            inline int32_t experience() const {
                return cached_experience[id];
            }

            inline block_id_t default_state() const {
                return cached_default_state[id];
            }

            inline uint8_t luminance() const {
                return cached_luminance[id];
            }

            inline uint8_t opacity() const {
                return cached_opacity[id];
            }

            inline bool is_air() const {
                return cached_is_air.get_unchecked(id);
            }

            inline bool is_liquid() const {
                return cached_is_liquid.get_unchecked(id);
            }

            inline bool is_burnable() const {
                return cached_is_burnable.get_unchecked(id);
            }

            inline bool is_emits_redstone() const {
                return cached_is_emits_redstone.get_unchecked(id);
            }

            inline bool is_full_cube() const {
                return cached_is_full_cube.get_unchecked(id);
            }

            inline bool is_tool_required() const {
                return cached_is_tool_required.get_unchecked(id);
            }

            inline bool is_sided_transparency() const {
                auto sides = cached_transparent_sides[id];
                return sides.down_side_solid
                       && sides.up_side_solid
                       && sides.north_side_solid
                       && sides.south_side_solid
                       && sides.west_side_solid
                       && sides.east_side_solid
                       && sides.down_center_solid
                       && sides.up_center_solid;
            }

            inline bool is_replaceable() const {
                return cached_is_replaceable.get_unchecked(id);
            }

            inline bool is_block_entity() const {
                return cached_is_block_entity.get_unchecked(id);
            }

            inline bool is_tickable() const {
                return cached_is_tickable.get_unchecked(id);
            }

            inline bool is_solid() const {
                return cached_is_solid.get_unchecked(id);
            }

            inline static size_t block_states_size() {
                return full_block_data_.size();
            }

            inline static_block_data::transparent_sides_t transparent_sides() const {
                return cached_transparent_sides[id];
            }

            static static_block_data& get_block(const std::string& name) {
                return *named_full_block_data.at(name);
            }

            static static_block_data& get_block(block_id_t block_state_id) {
                return *full_block_data_.at(block_state_id);
            }

            static static_block_data& get_general_block(block_id_t general_id) {
                return *general_block_data_.at(general_id);
            }

            static static_block_data& get_block_entity(int32_t block_entity_id) {
                return *block_entity_data_.at(block_entity_id);
            }

            static list_array<int32_t> get_block_states() {
                return full_block_data_.convert_fn([](auto& block) {
                    return block->current_state;
                });
            }

            static list_array<int32_t> get_block_entities() {
                return block_entity_data_.convert_fn([](auto& block) {
                    return block->block_entity_id;
                });
            }

            static list_array<int32_t> get_block_generals() {
                return general_block_data_.convert_fn([](auto& block) {
                    return block->general_block_id;
                });
            }

            static block make_block(const std::string& name) {
                return block(get_block(name).default_state);
            }

            inline static block make_block(block_id_t id) {
                return block(id);
            }


        private:
            static std::unordered_map<std::string, static_block_data*> named_full_block_data;
            static list_array<std::unique_ptr<static_block_data>> full_block_data_;
            static list_array<static_block_data*> general_block_data_;
            static list_array<static_block_data*> block_entity_data_;

            static bit_list_array<> cached_is_air;
            static bit_list_array<> cached_is_solid;
            static bit_list_array<> cached_is_liquid;
            static bit_list_array<> cached_is_burnable;
            static bit_list_array<> cached_is_emits_redstone;
            static bit_list_array<> cached_is_full_cube;
            static bit_list_array<> cached_is_tool_required;
            static bit_list_array<> cached_is_replaceable;
            static bit_list_array<> cached_is_block_entity;
            static bit_list_array<> cached_is_default_state;
            static bit_list_array<> cached_has_random_ticks;
            static bit_list_array<> cached_has_comparator_output;
            static bit_list_array<> cached_is_tickable;
            static list_array<static_block_data::transparent_sides_t> cached_transparent_sides;
            static list_array<float> cached_slipperiness;
            static list_array<float> cached_velocity_multiplier;
            static list_array<float> cached_jump_velocity_multiplier;
            static list_array<float> cached_hardness;
            static list_array<float> cached_blast_resistance;
            static list_array<int32_t> cached_map_color_rgb;
            static list_array<int32_t> cached_block_entity_id;
            static list_array<int32_t> cached_item_id;
            static list_array<int32_t> cached_experience;
            static list_array<block_id_t> cached_general_block_id;
            static list_array<block_id_t> cached_default_state;
            static list_array<uint8_t> cached_luminance;
            static list_array<uint8_t> cached_opacity;
        };

        struct local_block_pos {
            uint8_t x : 4;
            uint8_t y : 4;
            uint8_t z : 4;
        };

        struct chunk_block_pos {
            uint8_t x : 4;
            uint32_t y : 21;
            uint8_t z : 4;
        };

        struct compressed_block_state {
            uint64_t blockStateId : 52;
            uint64_t blockLocalX : 4;
            uint64_t blockLocalZ : 4;
            uint64_t blockLocalY : 4;

            inline void set(uint64_t raw) {
                union u_t {
                    compressed_block_state state;
                    uint64_t r;
                } u{.r = raw};

                *this = u.state;
            }

            inline uint64_t get() const {
                union u_t {
                    compressed_block_state state;
                    uint64_t r;
                } u{.state = *this};

                return u.r;
            }
        };

        struct block_hash {
            std::size_t operator()(const block& k) const {
                using std::hash;
                return hash<block_id_t>()(k.id);
            }
        };
    }
}
