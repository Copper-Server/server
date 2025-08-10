/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_STORAGE_WORLD_DATA
#define SRC_STORAGE_WORLD_DATA
#include <atomic>
#include <bitset>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/bounds.hpp>
#include <src/base_objects/entity/animation.hpp>
#include <src/base_objects/entity/event.hpp>
#include <src/base_objects/events/event.hpp>
#include <src/base_objects/weather.hpp>
#include <src/base_objects/world/block_action.hpp>
#include <src/base_objects/world/height_maps.hpp>
#include <src/base_objects/world/loading_point_ticket.hpp>
#include <src/base_objects/world/sub_chunk_data.hpp>
#include <src/util/calculations.hpp>
#include <src/util/task_management.hpp>

namespace copper_server::base_objects {
    struct entity;
    using entity_ref = atomic_holder<entity>;
}

namespace copper_server::storage {


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

    struct on_tick_state {
        using block_callback = std::function<void(base_objects::block_id_t id, int64_t x, int64_t y, int64_t z)>;
        using entity_callback = std::function<void(const base_objects::entity_ref& entity, int64_t x, int64_t y, int64_t z)>;

        struct on_block_event {
            block_callback callback;
            uint8_t max_distance;
        };

        struct on_entity_event {
            entity_callback callback;
            uint8_t max_distance;
        };

        world_data& current_world;

        std::unordered_map<base_objects::block_id_t, list_array<on_block_event>> handle_on_block_tick;
        std::unordered_map<base_objects::block_id_t, list_array<on_block_event>> handle_on_block_placed;
        std::unordered_map<base_objects::block_id_t, list_array<on_block_event>> handle_on_block_exists;
        std::unordered_map<base_objects::block_id_t, list_array<block_callback>> handle_stand_on_active_block; //lava, water, magma, etc...


        void on_block_tick(base_objects::block_id_t id, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z);
    };


    class world_data;
    class worlds_data;

    class chunk_data {
        friend world_data;
        bool load(const std::filesystem::path& chunk_z, uint64_t tick_counter, world_data& world);
        bool load(const enbt::compound_const_ref& chunk_data, uint64_t tick_counter, world_data& world);
        bool save(const std::filesystem::path& chunk_z, uint64_t tick_counter, world_data& world);

    public:
        base_objects::world::height_maps height_maps;
        std::vector<base_objects::world::sub_chunk_data> sub_chunks;

        //instead of using negative values for priority, schedule ticks in reverse order
        // -1 == 1, -2 == 2, etc... means higher value == lower priority
        list_array<list_array<std::pair<uint64_t, base_objects::chunk_block_pos>>> queried_for_tick;
        list_array<std::pair<uint64_t, base_objects::chunk_block_pos>> queried_for_liquid_tick;
        const int64_t chunk_x, chunk_z;
        uint8_t load_level = 44;
        uint8_t resume_gen_level = 255; //if load_level would be lower or equal than this, then generation would be resumed, used by generators
        uint8_t generator_stage = 0xFF; //0xFF == the chunk is complete and accessible, should be managed by generator

        chunk_data(int64_t chunk_x, int64_t chunk_z);

        void update_height_map_on(uint8_t local_x, uint64_t local_y, uint8_t local_z);
        void update_height_map();

        void for_each_entity(std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(uint64_t local_y, std::function<void(base_objects::entity_ref& entity)> func);

        void for_each_block_entity(std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
        void for_each_block_entity(uint64_t local_y, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);

        void for_each_sub_chunk(std::function<void(base_objects::world::sub_chunk_data& sub_chunk)> func);
        void get_sub_chunk(uint64_t local_y, std::function<void(base_objects::world::sub_chunk_data& sub_chunk)> func);

        //priority accepts only negative values
        void query_for_tick(uint8_t local_x, uint64_t local_y, uint8_t local_z, uint64_t on_tick, int8_t priority = 0);
        void query_for_liquid_tick(uint8_t local_x, uint64_t local_y, uint8_t local_z, uint64_t on_tick);


        void tick(world_data& world, size_t random_tick_speed, std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time);


        //generator functions
        void gen_set_block(const base_objects::full_block_data& block, uint8_t local_x, uint64_t local_y, uint8_t local_z);
        void gen_set_block(base_objects::full_block_data&& block, uint8_t local_x, uint64_t local_y, uint8_t local_z);
        void gen_remove_block(uint8_t local_x, uint64_t local_y, uint8_t local_z);
        base_objects::full_block_data_ref gen_get_block(uint8_t local_x, uint64_t local_y, uint8_t local_z);
    };

    class chunk_generator {
    public:
        virtual ~chunk_generator() {}

        //Subchunk format:
        //{
        //  "blocks": array(16)[array(16)[sarray_ui32(16)[...]...]...], // x[y[z[]]]
        //  "block_entities": array[
        //      {
        //          "x" : int,//used ui8
        //          "y" : int,//used ui8
        //          "z" : int,//used ui8
        //          "id" : int,//used ui32
        //          "state" : int, //used 15 bits of ui16
        //          "nbt" : {any},
        //      },
        //      ...
        //  ],
        //  "entity": array[{entity data}...],
        //  "queried_for_tick" : array[{"x":int, "y":int, "z":int}]//used ui8
        //}
        //** Every item in subchunk structure is optional
        //
        //Chunk format:
        //{
        //  "sub_chunks": array[
        //      {Subchunk},
        //      ...
        //  ]
        //}

        //Chunk format:
        //  all fields is optional, the subchunk array size must match world.get_chunk_y_count(), if it does not match, the array would be resized
        //  the "height_maps" used only in files, for generators this is ignored
        //
        //{
        //  "sub_chunks" : [Subchunk],
        //  "queried_for_tick" : [
        //      {
        //          "x" : int,//used ui8
        //          "y" : int,//used ui32
        //          "z" : int,//used ui8
        //          "duration" : int,//used uint32_t
        //      }
        //  ],
        //  "generator_stage" : int,//used ui8
        //  "resume_gen_level" : int, //used ui8
        //  "height_maps": [internal]
        //}

        enum class preset_mode {
            sync, // default
            parallel
        };

        //generating pipeline:
        //  sync mode blocks current preset stage up till all previous queried tasks is completed and executes current in sequence, after completion next stage executed
        //  parallel mode used to parallelize ippendent changes to chunks and generate them accordingly, if the next stage is also parallel they also executed without awaiting
        //
        // when new or incomplete chunk requested, engine starts calling process_chunk function to process each chunk, when generation is complete the request is completes
        // for new chunk, generate_chunk is always called, and processed parallel

        //if not set, all stages is parallel
        //preset_it -> mode
        std::unordered_map<uint8_t, preset_mode> config;

        //this function also should increment generator_stage for chunk and set to 0xFF when complete
        //to access information from other chunk, the generator must use request_chunk_data_weak_* or request_chunk_data_weak
        //if chunk is not fully generated, it would not be accessible other way
        virtual void process_chunk([[maybe_unused]] world_data& world, chunk_data& chunk, [[maybe_unused]] uint8_t preset_stage) {
            chunk.generator_stage = 0xFF;
        };

        //Returns {Chunk}
        //If generator uses process_chunk this function should return compound with "generator_stage" and (optionally) "resume_gen_level"
        virtual enbt::compound generate_chunk(world_data& world, int64_t chunk_x, int64_t chunk_z) = 0;
        //Returns {Subchunk}
        virtual enbt::compound generate_sub_chunk(world_data& world, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z) = 0;


        static void register_it(const std::string& id, base_objects::atomic_holder<chunk_generator> gen);
        static void unregister_it(const std::string& id);
        static base_objects::atomic_holder<chunk_generator> get_it(const std::string& id);
    };

    class chunk_light_processor {
    public:
        bool enable_entity_light_source_updates : 1 = false;
        bool enable_entity_light_source_updates_include_rot : 1 = false;

        chunk_light_processor() {}

        virtual ~chunk_light_processor() {}

        //fired after entity_teleport and entity_move
        virtual void process_entity_light_source([[maybe_unused]] world_data& world, [[maybe_unused]] const base_objects::entity& entity, [[maybe_unused]] util::VECTOR new_pos) {}

        //fired after entity_look_changes
        virtual void process_entity_light_source_rot([[maybe_unused]] world_data& world, [[maybe_unused]] const base_objects::entity& entity, [[maybe_unused]] util::ANGLE_DEG new_rot) {}

        virtual void process_chunk(world_data& world, int64_t chunk_x, int64_t chunk_z) = 0;
        virtual void process_sub_chunk(world_data& world, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z) = 0;
        virtual void block_changed(world_data& world, int64_t global_x, int64_t global_y, int64_t global_z) = 0;

        static void register_it(const std::string& id, base_objects::atomic_holder<chunk_light_processor> processor);
        static void unregister_it(const std::string& id);
        static base_objects::atomic_holder<chunk_light_processor> get_it(const std::string& id);
    };

    enum class block_set_mode {
        replace,
        destroy,
        keep,
    };

    class world_data {
        using chunk_row = std::unordered_map<uint64_t, base_objects::atomic_holder<chunk_data>>;
        using chunk_column = std::unordered_map<uint64_t, chunk_row>;
        uint64_t hashed_seed_value = 0;

        fast_task::task_recursive_mutex mutex;
        chunk_column chunks;
        base_objects::atomic_holder<chunk_light_processor> light_processor;

        std::unordered_map<util::XY<int64_t>, FuturePtr<base_objects::atomic_holder<chunk_data>>> on_generate_process;

        struct {
            std::bitset<255> sync_modes;
            fast_task::task_mutex mutex;
            fast_task::task_condition_variable notifier;
            fast_task::task_limiter limiter;
            base_objects::atomic_holder<chunk_generator> process;
            uint32_t count = 0;
            uint32_t lock_count = 0;
            uint32_t stage_complete_count = 0;
            uint8_t chunks_next_blocking_stage = 0xFF;
            uint8_t lowest_stage = 0xFF;
            uint8_t lowest_sync_stage = 0xFF;
            bool sync_mode = false;

            void next_stage_sync() {
                auto old_stage = chunks_next_blocking_stage;
                for (uint16_t i = chunks_next_blocking_stage + 1; i < 255; i++)
                    if (sync_modes[i])
                        chunks_next_blocking_stage = (uint8_t)i;
                if (old_stage == chunks_next_blocking_stage)
                    chunks_next_blocking_stage = 0xFF;
                sync_mode = chunks_next_blocking_stage == 0xFF;
            }

            void calculate() {
                sync_modes.reset();
                lowest_sync_stage = 0xFF;
                for (auto& [stage, mode] : process->config) {
                    if (lowest_sync_stage > stage)
                        lowest_sync_stage = stage;
                    sync_modes[stage] = mode == chunk_generator::preset_mode::sync;
                }
            }
        } generator;

        friend class worlds_data;
        std::string preview_world_name();
        std::filesystem::path path;

        std::unordered_map<util::XY<int64_t>, FuturePtr<base_objects::atomic_holder<chunk_data>>> on_load_process;
        std::unordered_map<util::XY<int64_t>, FuturePtr<bool>> on_save_process;
        std::unordered_map<size_t, base_objects::entity_ref> entities;
        std::unordered_map<size_t, base_objects::entity_ref> to_load_entities;
        size_t local_entity_id_generator = 0;
        size_t world_spawn_ticket_id;

        std::chrono::high_resolution_clock::time_point last_usage;

        FuturePtr<base_objects::atomic_holder<chunk_data>> create_chunk_generate_future(int64_t chunk_x, int64_t chunk_z, base_objects::atomic_holder<chunk_data>& chunk);
        FuturePtr<base_objects::atomic_holder<chunk_data>> create_chunk_load_future(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback, std::function<void()> fault);
        FuturePtr<base_objects::atomic_holder<chunk_data>> create_chunk_load_future(int64_t chunk_x, int64_t chunk_z);
        void make_save(int64_t chunk_x, int64_t chunk_z, bool also_unload);
        void make_save(int64_t chunk_x, int64_t chunk_z, chunk_row::iterator, bool also_unload);
        base_objects::atomic_holder<chunk_data> load_chunk_sync(int64_t chunk_x, int64_t chunk_z);
        base_objects::atomic_holder<chunk_data> processed_load_chunk_sync(int64_t chunk_x, int64_t chunk_z, bool is_async_context = false);

        base_objects::atomic_holder<chunk_generator>& get_generator();
        base_objects::atomic_holder<chunk_light_processor>& get_light_processor();

        int32_t internal_version = 0;
        uint16_t chunk_y_count = 20;      // == (320 / 16), do not change this config
        int16_t world_y_chunk_offset = 0; //calculated_from(world_y_offset)
        int16_t world_y_offset = 0;


        void __set_block_silent(const base_objects::full_block_data& block, int64_t global_x, int64_t global_y, int64_t global_z, block_set_mode mode = block_set_mode::replace);
        void __set_block_silent(base_objects::full_block_data&& block, int64_t global_x, int64_t global_y, int64_t global_z, block_set_mode mode = block_set_mode::replace);

    public:
        uint16_t get_chunk_y_count() const {
            return chunk_y_count;
        }

        int16_t get_world_y_chunk_offset() const {
            return world_y_chunk_offset;
        }

        int16_t get_world_y_offset() const {
            return world_y_offset;
        }

        uint64_t get_hashed_seed() const {
            return hashed_seed_value;
        }

        void set_seed(int32_t seed);

        //metadata
        void load(const enbt::compound_const_ref& load_from_nbt);
        //metadata
        void load();
        //metadata
        void save();
        enbt::compound general_world_data;
        enbt::compound world_game_rules;
        enbt::compound world_generator_data;
        enbt::compound world_light_processor_data; //not saved
        enbt::compound world_records;

        enbt::raw_uuid wandering_trader_id = enbt::raw_uuid::as_null();
        float wandering_trader_spawn_chance = 0;
        int32_t wandering_trader_spawn_delay = 0;
        int32_t world_seed = 0;
        std::string world_name;
        std::string world_type;
        std::string light_processor_id;
        std::string generator_id;
        std::unordered_map<size_t, base_objects::world::loading_point_ticket> loading_tickets;

        struct {
            int64_t x = 0;
            int64_t z = 0;
            int64_t radius = 0;
            int64_t y = 0; //y is calculated by heightmap
            float angle = 0;
        } spawn_data;

        double border_center_x = 0;
        double border_center_z = 0;
        double border_size = 0;
        double border_safe_zone = 5;
        double border_damage_per_block = 0;
        double border_lerp_target = 0;
        uint64_t border_lerp_time = 0;
        double border_warning_blocks = 5;
        double border_warning_time = 15;
        int64_t day_time = 0;
        int64_t time = 0;
        uint64_t random_tick_speed = 3;
        uint64_t ticks_per_second = 20;
        int32_t portal_teleport_boundary = 29999984;

        std::chrono::milliseconds chunk_lifetime = std::chrono::seconds(1);
        std::chrono::milliseconds world_lifetime = std::chrono::seconds(50);


        int32_t clear_weather_time = 0;
        int32_t weather_time = 0;
        base_objects::weather current_weather = base_objects::weather::clear;


        int8_t difficulty = 0;
        uint8_t default_gamemode = 0;
        bool ticking_frozen : 1 = false;
        bool difficulty_locked : 1 = false;
        bool is_hardcore : 1 = false;
        bool initialized : 1 = false;
        bool increase_time : 1 = true;
        bool has_skylight : 1 = true;
        bool enable_entity_light_source_updates : 1 = false; //calculated from light processor
        bool enable_entity_light_source_updates_include_rot : 1 = false;

        const int32_t world_id;

        world_data(int32_t world_id, const std::filesystem::path& path);

        std::filesystem::path get_path() const {
            return path;
        }

        void update_spawn_data(int64_t x, int64_t z, int64_t radius, float angle);
        size_t add_loading_ticket(base_objects::world::loading_point_ticket&& ticket);
        void remove_loading_ticket(size_t id);
        size_t loaded_chunks_count();

        bool exists(int64_t chunk_x, int64_t chunk_z);
        base_objects::atomic_holder<chunk_data> request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z);
        FuturePtr<base_objects::atomic_holder<chunk_data>> request_chunk_data(int64_t chunk_x, int64_t chunk_z);
        std::optional<base_objects::atomic_holder<chunk_data>> request_chunk_data_weak_gen(int64_t chunk_x, int64_t chunk_z);  //if chunk does not exists then it will be generated, if chunk exists or not loaded then std::nullopt
        std::optional<base_objects::atomic_holder<chunk_data>> request_chunk_data_weak(int64_t chunk_x, int64_t chunk_z);      //if chunk loaded returns it, else - std::nullopt
        std::optional<base_objects::atomic_holder<chunk_data>> request_chunk_data_weak_sync(int64_t chunk_x, int64_t chunk_z); //if chunk exists returns it, else - std::nullopt
        void request_chunk_gen(int64_t chunk_x, int64_t chunk_z);                                                              //generates chunk if it does not exists
        bool request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback);
        void request_chunk_data(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback, std::function<void()> fault);

        void save_chunks(bool unload = false);
        void await_save_chunks();
        void save_and_unload_chunk(int64_t chunk_x, int64_t chunk_z);
        void unload_chunk(int64_t chunk_x, int64_t chunk_z);
        void save_chunk(int64_t chunk_x, int64_t chunk_z);
        void erase_chunk(int64_t chunk_x, int64_t chunk_z);
        void regenerate_chunk(int64_t chunk_x, int64_t chunk_z);
        void reset_light_data(int64_t chunk_x, int64_t chunk_z);


        void save_and_unload_chunk_at(int64_t global_x, int64_t global_z);
        void unload_chunk_at(int64_t global_x, int64_t global_z);
        void save_chunk_at(int64_t global_x, int64_t global_z);
        void erase_chunk_at(int64_t global_x, int64_t global_z);
        void regenerate_chunk_at(int64_t global_x, int64_t global_z);
        void reset_light_data_at(int64_t global_x, int64_t global_z);

        void for_each_chunk(std::function<void(chunk_data& chunk)> func);
        void for_each_chunk(base_objects::cubic_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func);
        void for_each_chunk(base_objects::spherical_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func);
        void for_each_sub_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(base_objects::world::sub_chunk_data& chunk)> func);
        void get_sub_chunk(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, std::function<void(base_objects::world::sub_chunk_data& chunk)> func);
        void get_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> func);


        void for_each_chunk(base_objects::cubic_bounds_block bounds, std::function<void(chunk_data& chunk)> func);
        void for_each_chunk(base_objects::spherical_bounds_block bounds, std::function<void(chunk_data& chunk)> func);
        void for_each_sub_chunk_at(int64_t global_x, int64_t global_z, std::function<void(base_objects::world::sub_chunk_data& chunk)> func);
        void get_sub_chunk_at(int64_t global_x, int64_t global_y, int64_t global_z, std::function<void(base_objects::world::sub_chunk_data& chunk)> func);
        void get_chunk_at(int64_t global_y, int64_t global_z, std::function<void(chunk_data& chunk)> func);


        void for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::cubic_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::cubic_bounds_chunk_radius bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::cubic_bounds_chunk_radius_out bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::spherical_bounds_chunk_out bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_entity(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_block_entity(base_objects::cubic_bounds_chunk bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
        void for_each_block_entity(base_objects::cubic_bounds_chunk_radius bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
        void for_each_block_entity(base_objects::cubic_bounds_chunk_radius_out bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
        void for_each_block_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
        void for_each_block_entity(base_objects::spherical_bounds_chunk_out bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
        void for_each_block_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
        void for_each_block_entity(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);


        void for_each_entity(base_objects::cubic_bounds_block bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::cubic_bounds_block_radius bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::cubic_bounds_block_radius_out bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::spherical_bounds_block bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity(base_objects::spherical_bounds_block_out bounds, std::function<void(base_objects::entity_ref& entity)> func);
        void for_each_entity_at(int64_t global_x, int64_t global_z, std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_entity_at(int64_t global_x, int64_t global_y, int64_t global_z, std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_block_entity_at(int64_t global_x, int64_t global_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
        void for_each_block_entity_at(int64_t global_x, int64_t global_y, int64_t global_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);


        //priority accepts only negative values, doesn't work for unloaded chunks
        void query_for_tick(int64_t global_x, int64_t global_y, int64_t global_z, uint64_t duration, int8_t priority = 0);
        void query_for_liquid_tick(int64_t global_x, int64_t global_y, int64_t global_z, uint64_t duration);

        void set_block(const base_objects::full_block_data& block, int64_t global_x, int64_t global_y, int64_t global_z, block_set_mode mode = block_set_mode::replace);
        void set_block(base_objects::full_block_data&& block, int64_t global_x, int64_t global_y, int64_t global_z, block_set_mode mode = block_set_mode::replace);
        void remove_block(int64_t global_x, int64_t global_y, int64_t global_z);
        void get_block(int64_t global_x, int64_t global_y, int64_t global_z, std::function<void(base_objects::block& block)> func, std::function<void(base_objects::block& block, enbt::value& extended_data)> block_entity);
        void query_block(int64_t global_x, int64_t global_y, int64_t global_z, std::function<void(base_objects::block& block)> func, std::function<void(base_objects::block& block, enbt::value& extended_data)> block_entity, std::function<void()> fault);

        void block_updated(int64_t global_x, int64_t global_y, int64_t global_z);
        void chunk_updated(int64_t chunk_x, int64_t chunk_z);
        void sub_chunk_updated(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z);


        void locked(std::function<void(world_data& self)> func);

        void set_block_range(base_objects::cubic_bounds_block bounds, const list_array<base_objects::full_block_data>& blocks, block_set_mode mode = block_set_mode::replace);
        void set_block_range(base_objects::cubic_bounds_block bounds, list_array<base_objects::full_block_data>&& blocks, block_set_mode mode = block_set_mode::replace);
        void set_block_range(base_objects::spherical_bounds_block bounds, const list_array<base_objects::full_block_data>& blocks, block_set_mode mode = block_set_mode::replace);
        void set_block_range(base_objects::spherical_bounds_block bounds, list_array<base_objects::full_block_data>&& blocks, block_set_mode mode = block_set_mode::replace);


        uint32_t get_biome(int64_t global_x, int64_t global_y, int64_t global_z);
        void set_biome(int64_t global_x, int64_t global_y, int64_t global_z, uint32_t id);
        void set_biome_range(base_objects::cubic_bounds_block bounds, const list_array<uint32_t>& blocks, block_set_mode mode = block_set_mode::replace);
        void set_biome_range(base_objects::cubic_bounds_block bounds, list_array<uint32_t>&& blocks, block_set_mode mode = block_set_mode::replace);
        void set_biome_range(base_objects::spherical_bounds_block bounds, const list_array<uint32_t>& blocks, block_set_mode mode = block_set_mode::replace);
        void set_biome_range(base_objects::spherical_bounds_block bounds, list_array<uint32_t>&& blocks, block_set_mode mode = block_set_mode::replace);

        void get_height_maps(int64_t chunk_x, int64_t chunk_z, std::function<void(base_objects::world::height_maps& height_maps)> func);
        void get_height_maps_at(int64_t chunk_x, int64_t chunk_z, std::function<void(base_objects::world::height_maps& height_maps)> func);

        void register_entity(base_objects::entity_ref& entity);
        void unregister_entity(base_objects::entity_ref& entity);


        void change_chunk_generator(const std::string& id);
        void change_light_processor(const std::string& id);


        void tick(std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time);
        //unloads unused chunks and check themselves lifetime and active operations, if expired and there no active operations, then function will return true
        bool collect_unused_data(std::chrono::high_resolution_clock::time_point current_time, size_t& unload_limit);


#pragma region Communication
        void entity_init(base_objects::entity&);

        void entity_teleport(base_objects::entity&, util::VECTOR new_pos);
        void entity_move(base_objects::entity&, util::VECTOR move);
        void entity_look_changes(base_objects::entity&, util::ANGLE_DEG new_rotation);
        void entity_rotation_changes(base_objects::entity&, util::ANGLE_DEG new_rotation);
        void entity_motion_changes(base_objects::entity&, util::VECTOR new_motion);

        void entity_rides(base_objects::entity&, size_t other_entity_id);
        void entity_leaves_ride(base_objects::entity&, size_t other_entity_id);

        void entity_attach(base_objects::entity&, size_t other_entity_id);
        void entity_detach(base_objects::entity&, size_t other_entity_id);

        void entity_damage(base_objects::entity&, float health, int32_t type_id, std::optional<util::VECTOR> pos);
        void entity_damage(base_objects::entity&, float health, int32_t type_id, base_objects::entity_ref& source, std::optional<util::VECTOR> pos);
        void entity_damage(base_objects::entity&, float health, int32_t type_id, base_objects::entity_ref& source, base_objects::entity_ref& source_direct, std::optional<util::VECTOR> pos);

        void entity_attack(base_objects::entity&, size_t other_entity_id);
        void entity_iteract(base_objects::entity&, size_t other_entity_id);
        void entity_iteract(base_objects::entity&, int64_t x, int64_t y, int64_t z);

        void entity_break(base_objects::entity&, int64_t x, int64_t y, int64_t z, uint8_t state); //form 0 to 9, other ignored
        void entity_cancel_break(base_objects::entity&, int64_t x, int64_t y, int64_t z);
        void entity_finish_break(base_objects::entity&, int64_t x, int64_t y, int64_t z);
        void entity_place(base_objects::entity&, bool is_main_hand, int64_t x, int64_t y, int64_t z, base_objects::block);
        void entity_place(base_objects::entity&, bool is_main_hand, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref);


        void entity_animation(base_objects::entity&, base_objects::entity_animation animation);
        void entity_event(base_objects::entity&, base_objects::entity_event status);


        void entity_add_effect(base_objects::entity&, uint32_t id, uint32_t duration, uint8_t amplifier = 1, bool ambient = false, bool show_particles = true, bool show_icon = true, bool use_blend = false);
        void entity_remove_effect(base_objects::entity&, uint32_t id);

        void entity_death(base_objects::entity&);
        void entity_deinit(base_objects::entity&);


        void notify_block_event(const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z);
        void notify_block_change(int64_t x, int64_t y, int64_t z, base_objects::block);
        void notify_block_change(int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref);
        void notify_block_destroy_change(int64_t x, int64_t y, int64_t z, base_objects::block);
        void notify_block_destroy_change(int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref);
        void notify_biome_change(int64_t x, int64_t y, int64_t z, uint32_t);

        void notify_sub_chunk(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z); //used after multiply changes
        void notify_chunk(int64_t chunk_x, int64_t chunk_z);                      //used after multiply changes or to init

        void notify_sub_chunk_light(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z); //used after multiply changes
        void notify_chunk_light(int64_t chunk_x, int64_t chunk_z);                      //used after multiply changes

        void notify_sub_chunk_blocks(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z); //used after multiply changes
        void notify_chunk_blocks(int64_t chunk_x, int64_t chunk_z);                      //used after multiply changes
#pragma endregion

        struct profiling_data {
            std::chrono::high_resolution_clock::time_point last_tick = std::chrono::high_resolution_clock::now();
            uint64_t got_ticks = 0;
            double tps_for_world = 0;

            //if set 0.1
            // total 1000 ticked chunks
            // tick_speed = 50ms
            // 50ms / 1000 = 0.05
            // 0.05 + 0.1 = 0.15ms per chunk
            //
            // if chunk got ticked more than 0.15ms, then callback will called after tick
            double slow_chunk_tick_callback_threshold = 0.1;

            //if set 1.5
            // tick_speed = 50ms
            // tick_speed * 1.5 = 75ms
            //
            // if world got ticked more than 75ms, then callback will be called after tick
            double slow_world_tick_callback_threshold = 1.5;


            bool enable_world_profiling = false; //calls profiling callbacks

            std::function<void(world_data& world, int64_t chunk_x, int64_t chunk_z, std::chrono::milliseconds tick_time)> chunk_speedometer_callback = nullptr;
            std::function<void(world_data& world, int64_t chunk_x, int64_t chunk_z, std::chrono::milliseconds tick_time)> slow_chunk_tick_callback = nullptr;
            std::function<void(world_data& world, std::chrono::milliseconds tick_time)> slow_world_tick_callback = nullptr;
            std::function<void(world_data& world)> got_tps_update = nullptr;
            std::function<void(world_data& world, chunk_data&)> chunk_loaded;
            std::function<void(world_data& world, int64_t chunk_x, int64_t chunk_z)> chunk_load_failed;
            std::function<void(world_data& world, int64_t chunk_x, int64_t chunk_z)> chunk_unloaded;

            std::atomic_size_t chunk_generator_counter = 0; //generating in process
            std::atomic_size_t chunk_load_counter = 0; //load in process
            size_t chunk_target_to_load = 0;
            size_t chunk_total_loaded = 0;
        } profiling;

        //tick sync
        std::chrono::nanoseconds accumulated_time{0};
        uint64_t tick_counter = 0;
    };

    class worlds_data {
        fast_task::task_recursive_mutex mutex;
        std::filesystem::path base_path;
        std::unordered_map<int32_t, base_objects::atomic_holder<world_data>> cached_worlds;
        std::chrono::high_resolution_clock::time_point last_tps_calculated = std::chrono::high_resolution_clock::now();
        uint64_t got_ticks = 0;

        base_objects::atomic_holder<world_data> load(int32_t world_id);

        list_array<int32_t> cached_ids;
        const list_array<int32_t>& get_ids();


    public:
        base_objects::events::event<int32_t> on_world_loaded;
        base_objects::events::event<int32_t> on_world_unloaded;
        base_objects::events::event<double> on_tps_changed;
        uint64_t ticks_per_second = 20;
        int32_t base_world_id = -1;

        //calculated
        double tps = (double)ticks_per_second;


        worlds_data(const std::filesystem::path& base_path);

        size_t loaded_chunks_count();
        size_t loaded_chunks_count(int32_t world_id);
        bool exists(int32_t world_id);
        bool exists(const std::string& name);
        const list_array<int32_t>& get_list();
        std::string get_name(int32_t world_id);
        int32_t get_id(const std::string& name);
        list_array<int32_t> get_all_ids();

        base_objects::atomic_holder<world_data> get(int32_t world_id);
        void save(int32_t world_id);
        void save_all();

        void save_and_unload(int32_t world_id);
        void save_and_unload_all();

        //be sure that world is not used by anything, otherwise will throw exception
        void unload(int32_t world_id);
        void unload_all();
        void erase(int32_t world_id);

        void locked(std::function<void()> func);
        void locked(std::function<void(worlds_data& self)> func);

        int32_t create(const std::string& name);
        int32_t create(const std::string& name, std::function<void(world_data& world)> init);

        void for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_entity(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);

        void for_each_entity(int32_t world_id, std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_entity(int32_t world_id, int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
        void for_each_entity(int32_t world_id, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);

        void for_each_world(std::function<void(int32_t id, world_data& world)> func);

        void apply_tick(std::chrono::high_resolution_clock::time_point current_time, std::chrono::nanoseconds elapsed);
    };
}
#endif /* SRC_STORAGE_WORLD_DATA */
