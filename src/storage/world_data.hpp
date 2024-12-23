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
#include <src/base_objects/entity.hpp>
#include <src/base_objects/entity_animation.hpp>
#include <src/base_objects/event.hpp>
#include <src/base_objects/ptr_optional.hpp>
#include <src/base_objects/server_configuaration.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/base_objects/slot.hpp>
#include <src/base_objects/weather.hpp>
#include <src/calculations.hpp>
#include <src/util/task_management.hpp>

namespace copper_server {
    namespace storage {

        struct on_tick_state {
            using block_callback = std::function<void(base_objects::block_id_t id, int64_t x, uint64_t y, int64_t z)>;
            using entity_callback = std::function<void(const base_objects::entity_ref& entity, int64_t x, uint64_t y, int64_t z)>;

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


            void on_block_tick(base_objects::block_id_t id, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z);
        };

        struct loading_point_ticket {
            std::variant<uint16_t, std::function<bool(world_data&, size_t, loading_point_ticket&)>> expiration;
            base_objects::cubic_bounds_chunk_radius point;
            std::string name;
            int8_t level;

            //sets the whole cubic point to specified level and propagates to neighbors by default
        };

        class world_data;
        class worlds_data;
        struct sub_chunk_data;

        struct light_data {
            union light_item {
                struct {
                    //compact two values in one
                    uint8_t light_point : 4;
                    uint8_t _unused : 4;
                };

                uint8_t raw;
            };

            light_item light_map[16][16][16];

            light_data()
                : light_map() {}
        };

        struct sub_chunk_data {
            base_objects::block blocks[16][16][16];
            uint32_t biomes[4][4][4];
            std::unordered_map<uint16_t, enbt::value> block_entities; //0xXYZ => block_entity
            list_array<base_objects::entity_ref> stored_entities;


            light_data sky_light;
            light_data block_light;
            bool has_tickable_blocks = false;
            bool need_to_recalculate_light = false;
            bool sky_lighted = false;   //set true if at least one block is lighted in this sub_chunk
            bool block_lighted = false; //set true if at least one block is lighted in this sub_chunk

            sub_chunk_data() {}

            enbt::value& get_block_entity_data(uint8_t local_x, uint8_t local_y, uint8_t local_z);
            void get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, std::function<void(base_objects::block& block)> on_normal, std::function<void(base_objects::block& block, enbt::value& entity_data)> on_entity);
            void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::full_block_data& block);
            void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::full_block_data&& block);
            uint32_t get_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z);
            void set_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z, uint32_t id);
            void for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block)> func);
        };

        struct height_maps {
            uint64_t surface[16][16];
            uint64_t ocean_floor[16][16];
            uint64_t motion_blocking[16][16];
            uint64_t motion_blocking_no_leaves[16][16];

            height_maps() {
                for (int i = 0; i < 16; i++) {
                    for (int j = 0; j < 16; j++) {
                        surface[i][j] = 0;
                        ocean_floor[i][j] = 0;
                        motion_blocking[i][j] = 0;
                        motion_blocking_no_leaves[i][j] = 0;
                    }
                }
            }
        };

        class chunk_data {
            friend world_data;
            bool load(const std::filesystem::path& chunk_z, uint64_t tick_counter);
            bool load(const enbt::compound_const_ref& chunk_data, uint64_t tick_counter);
            bool save(const std::filesystem::path& chunk_z, uint64_t tick_counter);

        public:
            height_maps height_maps;
            std::vector<sub_chunk_data> sub_chunks;

            //instead of using negative values for priority, schedule ticks in reverse order
            // -1 == 1, -2 == 2, etc... means higher value == lower priority
            list_array<list_array<std::pair<uint64_t, base_objects::chunk_block_pos>>> queried_for_tick;
            list_array<std::pair<uint64_t, base_objects::chunk_block_pos>> queried_for_liquid_tick;
            const int64_t chunk_x, chunk_z;
            uint8_t load_level = 44;

            chunk_data(int64_t chunk_x, int64_t chunk_z);

            void for_each_entity(std::function<void(base_objects::entity_ref& entity)> func);
            void for_each_entity(uint64_t sub_chunk_y, std::function<void(base_objects::entity_ref& entity)> func);

            void for_each_block_entity(std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
            void for_each_block_entity(uint64_t sub_chunk_y, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);

            void for_each_sub_chunk(std::function<void(sub_chunk_data& sub_chunk)> func);
            void get_sub_chunk(uint64_t sub_chunk_y, std::function<void(sub_chunk_data& sub_chunk)> func);

            //priority accepts only negative values
            void query_for_tick(uint8_t local_x, uint64_t global_y, uint8_t local_z, uint64_t on_tick, int8_t priority = 0);
            void query_for_liquid_tick(uint8_t local_x, uint64_t global_y, uint8_t local_z, uint64_t on_tick);


            void tick(world_data& world, size_t random_tick_speed, std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time);
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


            //Returns {Chunk}
            virtual enbt::compound generate_chunk(world_data& world, int64_t chunk_x, int64_t chunk_z) = 0;
            //Returns {Subchunk}
            virtual enbt::compound generate_sub_chunk(world_data& world, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z) = 0;


            static void register_it(const std::string& id, base_objects::atomic_holder<chunk_generator> gen);
            static void unregister_it(const std::string& id);
            static base_objects::atomic_holder<chunk_generator> get_it(const std::string& id);
        };

        class chunk_light_processor {
        public:
            bool enable_entity_updates_when_hold_light_source : 1 = false;
            bool enable_entity_light_source_updates : 1 = false;

            chunk_light_processor() {}

            virtual ~chunk_light_processor() {}

            //updates every time when entity moved
            virtual void process_entity_light_source(world_data& world, const base_objects::entity_ref& entity) {}

            //updates every time when entity moved
            virtual void process_entity_hold_light_source(world_data& world, const base_objects::entity_ref& entity) {}

            virtual void process_chunk(world_data& world, int64_t chunk_x, int64_t chunk_z) = 0;
            virtual void process_sub_chunk(world_data& world, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z) = 0;
            virtual void block_changed(world_data& world, int64_t global_x, uint64_t global_y, int64_t global_z) = 0;

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

            fast_task::task_recursive_mutex mutex;
            chunk_column chunks;
            base_objects::atomic_holder<chunk_generator> generator;
            base_objects::atomic_holder<chunk_light_processor> light_processor;


            friend class worlds_data;
            std::string preview_world_name();
            std::filesystem::path path;

            std::unordered_map<calc::XY<int64_t>, FuturePtr<base_objects::atomic_holder<chunk_data>>> on_load_process;
            std::unordered_map<calc::XY<int64_t>, FuturePtr<bool>> on_save_process;
            std::unordered_map<uint64_t, base_objects::client_data_holder> clients;
            uint64_t local_client_id_generator = 0;
            size_t world_spawn_ticket_id;

            std::chrono::high_resolution_clock::time_point last_usage;
            void make_save(int64_t chunk_x, int64_t chunk_z, bool also_unload);
            void make_save(int64_t chunk_x, int64_t chunk_z, chunk_row::iterator, bool also_unload);
            base_objects::atomic_holder<chunk_data> load_chunk_sync(int64_t chunk_x, int64_t chunk_z);

            base_objects::atomic_holder<chunk_generator>& get_generator();
            base_objects::atomic_holder<chunk_light_processor>& get_light_processor();

        public:
            //metadata
            void load(const enbt::compound_const_ref& load_from_nbt);
            //metadata
            void load();
            //metadata
            void save();
            enbt::compound general_world_data;
            enbt::compound world_game_rules;
            enbt::compound world_generator_data;
            enbt::compound world_light_processor_data;
            enbt::compound world_records;

            enbt::raw_uuid world_seed = enbt::raw_uuid::generate_v4();
            enbt::raw_uuid wandering_trader_id = enbt::raw_uuid::as_null();
            float wandering_trader_spawn_chance = 0;
            int32_t wandering_trader_spawn_delay = 0;
            std::string world_name;
            std::string world_type;
            std::string light_processor_id;
            std::string generator_id;
            std::vector<std::string> enabled_datapacks;
            std::vector<std::string> enabled_plugins;
            std::unordered_map<size_t, loading_point_ticket> loading_tickets;

            struct {
                int64_t x = 0;
                int64_t z = 0;
                int64_t radius = 0;
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
            size_t random_tick_speed = 3;
            uint64_t ticks_per_second = 20;

            std::chrono::milliseconds chunk_lifetime = std::chrono::seconds(1);
            std::chrono::milliseconds world_lifetime = std::chrono::seconds(50);


            int32_t clear_weather_time = 0;


            int32_t internal_version = 0;
            uint16_t chunk_y_count = 20; // == (320 / 16), do not change this config
            int16_t world_y_offset = 0;
            int8_t difficulty = 0;
            uint8_t default_gamemode = 0;
            bool difficulty_locked : 1 = false;
            bool is_hardcore : 1 = false;
            bool initialized : 1 = false;
            bool raining : 1 = false;
            bool thundering : 1 = false;
            bool has_skylight : 1 = true;
            bool enable_entity_updates_when_hold_light_source : 1 = false;
            bool enable_entity_light_source_updates : 1 = false;

            world_data(const std::filesystem::path& path);

            std::filesystem::path get_path() const {
                return path;
            }

            void update_spawn_data(int64_t x, int64_t z, int64_t radius);
            size_t add_loading_ticket(loading_point_ticket&& ticket);
            void remove_loading_ticket(size_t id);
            size_t loaded_chunks_count();

            bool exists(int64_t chunk_x, int64_t chunk_z);
            //returns std::nullopt if chunk already queried for async load
            std::optional<base_objects::atomic_holder<chunk_data>> request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z);
            FuturePtr<base_objects::atomic_holder<chunk_data>> request_chunk_data(int64_t chunk_x, int64_t chunk_z);
            std::optional<base_objects::atomic_holder<chunk_data>> request_chunk_data_weak_gen(int64_t chunk_x, int64_t chunk_z); //if chunk does not exists then it will be generated, if chunk exists or not loaded then std::nullopt
            void request_chunk_gen(int64_t chunk_x, int64_t chunk_z);                                                             //generates chunk if it does not exists
            bool request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback);
            void request_chunk_data(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback, std::function<void()> fault);

            void save_chunks(bool unload = false);
            void await_save_chunks();
            void save_and_unload_chunk(int64_t chunk_x, int64_t chunk_z);
            void unload_chunk(int64_t chunk_x, int64_t chunk_z);
            void save_chunk(int64_t chunk_x, int64_t chunk_z);
            void erase_chunk(int64_t chunk_x, int64_t chunk_z);
            void regenerate_chunk(int64_t chunk_x, int64_t chunk_z);
            void reset_light_data(int64_t chunk_x, uint64_t chunk_z);


            void save_and_unload_chunk_at(int64_t global_x, int64_t global_z);
            void unload_chunk_at(int64_t global_x, int64_t global_z);
            void save_chunk_at(int64_t global_x, int64_t global_z);
            void erase_chunk_at(int64_t global_x, int64_t global_z);
            void regenerate_chunk_at(int64_t global_x, int64_t global_z);
            void reset_light_data_at(int64_t global_x, uint64_t global_z);

            void for_each_chunk(std::function<void(chunk_data& chunk)> func);
            void for_each_chunk(base_objects::cubic_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func);
            void for_each_chunk(base_objects::spherical_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func);
            void for_each_sub_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(sub_chunk_data& chunk)> func);
            void get_sub_chunk(int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, std::function<void(sub_chunk_data& chunk)> func);
            void get_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> func);


            void for_each_chunk(base_objects::cubic_bounds_block bounds, std::function<void(chunk_data& chunk)> func);
            void for_each_chunk(base_objects::spherical_bounds_block bounds, std::function<void(chunk_data& chunk)> func);
            void for_each_sub_chunk_at(int64_t global_x, int64_t global_z, std::function<void(sub_chunk_data& chunk)> func);
            void get_sub_chunk_at(int64_t global_x, uint64_t global_y, int64_t global_z, std::function<void(sub_chunk_data& chunk)> func);
            void get_chunk_at(int64_t global_y, int64_t global_z, std::function<void(chunk_data& chunk)> func);


            void for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(base_objects::cubic_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func);
            void for_each_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func);
            void for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_block_entity(base_objects::cubic_bounds_chunk bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
            void for_each_block_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
            void for_each_block_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
            void for_each_block_entity(int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);


            void for_each_entity(base_objects::cubic_bounds_block bounds, std::function<void(base_objects::entity_ref& entity)> func);
            void for_each_entity(base_objects::spherical_bounds_block bounds, std::function<void(base_objects::entity_ref& entity)> func);
            void for_each_block_entity(base_objects::cubic_bounds_block bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
            void for_each_block_entity(base_objects::spherical_bounds_block bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
            void for_each_entity_at(int64_t global_x, int64_t global_z, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity_at(int64_t global_x, int64_t global_z, uint64_t global_y, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_block_entity_at(int64_t global_x, int64_t global_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);
            void for_each_block_entity_at(int64_t global_x, int64_t global_z, uint64_t global_y, std::function<void(base_objects::block& block, enbt::value& extended_data)> func);


            //priority accepts only negative values, doesn't work for unloaded chunks
            void query_for_tick(int64_t global_x, uint64_t global_y, int64_t global_z, uint64_t duration, int8_t priority = 0);
            void query_for_liquid_tick(int64_t global_x, uint64_t global_y, int64_t global_z, uint64_t duration);

            void set_block(const base_objects::full_block_data& block, int64_t global_x, uint64_t global_y, int64_t global_z, block_set_mode mode = block_set_mode::replace);
            void set_block(base_objects::full_block_data&& block, int64_t global_x, uint64_t global_y, int64_t global_z, block_set_mode mode = block_set_mode::replace);
            void remove_block(int64_t global_x, uint64_t global_y, int64_t global_z);
            void get_block(int64_t global_x, uint64_t global_y, int64_t global_z, std::function<void(base_objects::block& block)> func, std::function<void(base_objects::block& block, enbt::value& extended_data)> block_entity);
            void query_block(int64_t global_x, uint64_t global_y, int64_t global_z, std::function<void(base_objects::block& block)> func, std::function<void(base_objects::block& block, enbt::value& extended_data)> block_entity, std::function<void()> fault);

            void block_updated(int64_t global_x, uint64_t global_y, int64_t global_z);
            void chunk_updated(int64_t chunk_x, uint64_t chunk_z);
            void sub_chunk_updated(int64_t chunk_x, uint64_t chunk_z, uint64_t sub_chunk_y);


            void locked(std::function<void(world_data& self)> func);

            void set_block_range(base_objects::cubic_bounds_block bounds, const list_array<base_objects::full_block_data>& blocks, block_set_mode mode = block_set_mode::replace);
            void set_block_range(base_objects::cubic_bounds_block bounds, list_array<base_objects::full_block_data>&& blocks, block_set_mode mode = block_set_mode::replace);
            void set_block_range(base_objects::spherical_bounds_block bounds, const list_array<base_objects::full_block_data>& blocks, block_set_mode mode = block_set_mode::replace);
            void set_block_range(base_objects::spherical_bounds_block bounds, list_array<base_objects::full_block_data>&& blocks, block_set_mode mode = block_set_mode::replace);


            uint32_t get_biome(int64_t global_x, uint64_t global_y, int64_t global_z);
            void set_biome(int64_t global_x, uint64_t global_y, int64_t global_z, uint32_t id);
            void set_biome_range(base_objects::cubic_bounds_block bounds, const list_array<uint32_t>& blocks, block_set_mode mode = block_set_mode::replace);
            void set_biome_range(base_objects::cubic_bounds_block bounds, list_array<uint32_t>&& blocks, block_set_mode mode = block_set_mode::replace);
            void set_biome_range(base_objects::spherical_bounds_block bounds, const list_array<uint32_t>& blocks, block_set_mode mode = block_set_mode::replace);
            void set_biome_range(base_objects::spherical_bounds_block bounds, list_array<uint32_t>&& blocks, block_set_mode mode = block_set_mode::replace);

            void get_height_maps(int64_t chunk_x, int64_t chunk_z, std::function<void(storage::height_maps& height_maps)> func);
            void get_height_maps_at(int64_t chunk_x, int64_t chunk_z, std::function<void(storage::height_maps& height_maps)> func);

            uint64_t register_client(const base_objects::client_data_holder& client);
            void unregister_client(uint64_t);
            void register_entity(base_objects::entity_ref& client);
            void unregister_entity(base_objects::entity_ref& client);


            void change_chunk_generator(const std::string& id);
            void change_light_processor(const std::string& id);


            void tick(std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time);
            //unloads unused chunks and check themselves lifetime and active operations, if expired and there no active operations, then function will return true
            bool collect_unused_data(std::chrono::high_resolution_clock::time_point current_time, size_t& unload_limit);

            //interface
            //virtual void broadcast_attach_entity(const base_objects::entity& target, const base_objects::entity& vehicle);
            //virtual void broadcast_block_action(calc::VECTOR block_pos, uint8_t byte1, uint8_t byte2, uint8_t block_type, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_block_break_animation(uint32_t entity_id, calc::VECTOR block_pos, int8_t stage, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_block_entity(calc::VECTOR block_pos, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_boss_bar_update_health(const base_objects::entity& entity, uint32_t unique_id, float fraction_filled);
            //virtual void broadcast_chat(const Chat& message, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_collect_entity(const base_objects::entity& collected, const base_objects::entity& collector, uint32_t count, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_destroy_entity(const base_objects::entity& entity, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_detach_entity(const base_objects::entity& target, const base_objects::entity& vehicle);
            //virtual void broadcast_entity_effect(const base_objects::entity& entity, int effect_id, int amplifier, int duration, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_entity_equipment(const base_objects::entity& entity, int16_t slot_num, const base_objects::slot& item, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_entity_head_look(const base_objects::entity& entity, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_entity_look(const base_objects::entity& entity, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_entity_metadata(const base_objects::entity& entity, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_entity_position(const base_objects::entity& entity, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_entity_velocity(const base_objects::entity& entity, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_entity_animation(const base_objects::entity& entity, base_objects::entity_animation animation, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_leash_entity(const base_objects::entity& entity, const base_objects::entity& entity_leashed_to);
            //virtual void broadcast_particle_effect(const std::string& particle_name, calc::VECTOR src, calc::VECTOR offset, float particle_data, int particle_amount, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_particle_effect(const std::string& particle_name, calc::VECTOR src, calc::VECTOR offset, float particle_data, int particle_amount, std::array<int, 2> data, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_player_list_add_player(const base_objects::player& player, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_player_list_header_footer(const Chat& header, const Chat& footer);
            //virtual void broadcast_player_list_remove_player(const base_objects::player& player, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_player_list_update_display_name(const base_objects::player& player, const std::string& custom_name, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_player_list_update_game_mode(const base_objects::player& player, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_player_list_update_ping();
            //virtual void broadcast_remove_entity_effect(const base_objects::entity& entity, int effect_id, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_scoreboard_objective(const std::string& name, const std::string& display_name, uint8_t mode);
            //virtual void broadcast_score_update(const std::string& objective, const std::string& player_name, int64_t score, uint8_t mode);
            //virtual void broadcast_display_objective(const std::string& objective, base_objects::scoreboard::display_slot display);
            //virtual void broadcast_sound_effect(const std::string& sound_name, calc::VECTOR position, float volume, float pitch, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_spawn_entity(base_objects::entity& entity, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_thunderbolt(calc::VECTOR block_pos, const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_time_update(const base_objects::client_data_holder& exclude = nullptr);
            //virtual void broadcast_unleash_entity(const base_objects::entity& entity);
            //virtual void broadcast_weather(base_objects::weather weather, const base_objects::client_data_holder& exclude = nullptr);


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
            } profiling;

            //tick sync
            std::chrono::nanoseconds accumulated_time{0};
            uint64_t tick_counter = 0;
        };

        class worlds_data {
            base_objects::ServerConfiguration& configuration;
            fast_task::task_recursive_mutex mutex;
            std::filesystem::path base_path;
            std::unordered_map<uint64_t, base_objects::atomic_holder<world_data>> cached_worlds;
            std::chrono::high_resolution_clock::time_point last_tps_calculated = std::chrono::high_resolution_clock::now();
            uint64_t got_ticks = 0;

            base_objects::atomic_holder<world_data> load(uint64_t world_id);

            list_array<uint64_t> cached_ids;
            const list_array<uint64_t>& get_ids();


        public:
            base_objects::event<uint64_t> on_world_loaded;
            base_objects::event<uint64_t> on_world_unloaded;
            base_objects::event<double> on_tps_changed;
            uint64_t ticks_per_second = 20;
            uint64_t base_world_id = -1;

            //calculated
            double tps = 1;


            worlds_data(base_objects::ServerConfiguration& configuration, const std::filesystem::path& base_path);

            size_t loaded_chunks_count();
            size_t loaded_chunks_count(uint64_t world_id);
            bool exists(uint64_t world_id);
            bool exists(const std::string& name);
            const list_array<uint64_t>& get_list();
            std::string get_name(uint64_t world_id);
            uint64_t get_id(const std::string& name);


            base_objects::atomic_holder<world_data> get(uint64_t world_id);
            void save(uint64_t world_id);
            void save_all();

            void save_and_unload(uint64_t world_id);
            void save_and_unload_all();

            //be sure that world is not used by anything, otherwise will throw exception
            void unload(uint64_t world_id);
            void unload_all();
            void erase(uint64_t world_id);

            void locked(std::function<void()> func);
            void locked(std::function<void(worlds_data& self)> func);

            uint64_t create(const std::string& name);
            uint64_t create(const std::string& name, std::function<void(world_data& world)> init);

            void for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func);

            void for_each_entity(uint64_t world_id, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(uint64_t world_id, int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(uint64_t world_id, int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func);

            void for_each_world(std::function<void(uint64_t id, world_data& world)> func);

            void apply_tick(std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time, std::chrono::nanoseconds elapsed);
        };
    } // namespace storage
} // namespace copper_server


#endif /* SRC_STORAGE_WORLD_DATA */
