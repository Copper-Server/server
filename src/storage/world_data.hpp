#ifndef SRC_STORAGE_WORLD_DATA
#define SRC_STORAGE_WORLD_DATA
#include "../base_objects/atomic_holder.hpp"
#include "../base_objects/block.hpp"
#include "../base_objects/bounds.hpp"
#include "../base_objects/entity.hpp"
#include "../base_objects/event.hpp"
#include "../base_objects/ptr_optional.hpp"
#include "../base_objects/server_configuaration.hpp"
#include "../base_objects/shared_client_data.hpp"
#include "../calculations.hpp"
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include "../util/task_management.hpp"
#include <atomic>
#include <bitset>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

namespace crafted_craft {
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

        class world_data;
        class worlds_data;
        struct sub_chunk_data;

        struct light_data {
            union light_item {
                struct {
                    uint8_t light_point : 4;
                    bool lighted : 1;
                    bool _unused : 3;
                };

                uint8_t raw;
            };

            light_item light_map[16][16][16];
        };

        union block_data {
            struct {
                base_objects::block_id_t block_id : 15;
                bool is_tickable : 1;
                uint16_t block_state_data : 15;
            };

            uint32_t raw;

            void tick(world_data&, sub_chunk_data& sub_chunk, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z);
        };

        struct sub_chunk_data {
            block_data blocks[16][16][16];
            std::vector<base_objects::block_entity> block_entities;
            list_array<base_objects::entity_ref> stored_entities;
            list_array<base_objects::local_block_pos> queried_for_tick;


            light_data sky_light;
            light_data block_light;
            bool has_tickable_blocks = false;
            bool need_to_recalculate_light = false;

            sub_chunk_data() {
                for (int i = 0; i < 16; i++) {
                    for (int j = 0; j < 16; j++) {
                        for (int k = 0; k < 16; k++) {
                            blocks[i][j][k] = {0, false, 0};
                            sky_light.light_map[i][j][k].light_point = 0;
                            sky_light.light_map[i][j][k].lighted = false;
                            block_light.light_map[i][j][k].light_point = 0;
                            block_light.light_map[i][j][k].lighted = false;
                        }
                    }
                }
            }
        };

        class chunk_data {
            friend world_data;
            int64_t chunk_x, chunk_z;
            bool load(const std::filesystem::path& chunk_z);
            bool load(const ENBT& chunk_data);
            bool save(const std::filesystem::path& chunk_z);

        public:
            std::chrono::high_resolution_clock::time_point last_usage;
            std::vector<sub_chunk_data> sub_chunks;
            bool marked_for_tick = false;

            chunk_data(int64_t chunk_x, int64_t chunk_z);

            void for_each_entity(std::function<void(base_objects::entity_ref& entity)> func);
            void for_each_entity(std::function<void(base_objects::entity_ref& entity)> func, uint64_t sub_chunk_y);

            void for_each_sub_chunk(std::function<void(sub_chunk_data& sub_chunk)> func);
            void get_sub_chunk(std::function<void(sub_chunk_data& sub_chunk)> func, uint64_t sub_chunk_y);

            void query_for_tick(uint8_t local_x, uint64_t global_y, uint8_t local_z);

            void tick(world_data& world, size_t max_random_tick_for_chunk, std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time);
        };

        class chunk_generator {
        public:
            virtual ENBT generate_chunk(world_data& world, int64_t chunk_x, int64_t chunk_z) = 0;
            virtual ENBT generate_sub_chunk(world_data& world, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z) = 0;
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
            virtual void block_placed(world_data& world, int64_t block_x, uint64_t block_y, int64_t block_z) = 0;
            virtual void block_removed(world_data& world, int64_t block_x, uint64_t block_y, int64_t block_z) = 0;

            virtual void block_changed(world_data& world, int64_t block_x, uint64_t block_y, int64_t block_z) {
                block_removed(world, block_x, block_y, block_z);
                block_placed(world, block_x, block_y, block_z);
            }
        };

        class world_data {

            using chunk_row = std::unordered_map<uint64_t, base_objects::atomic_holder<chunk_data>>;
            using chunk_column = std::unordered_map<uint64_t, chunk_row>;

            fast_task::task_recursive_mutex mutex;
            chunk_column chunks;
            base_objects::atomic_holder<chunk_generator> generator;
            base_objects::atomic_holder<chunk_light_processor> light_processor;


            friend class worlds_data;
            void load();
            void save();
            std::string preview_world_name();
            std::filesystem::path path;

            std::unordered_map<calc::XY<int64_t>, FuturePtr<base_objects::atomic_holder<chunk_data>>> on_load_process;
            std::unordered_map<calc::XY<int64_t>, FuturePtr<bool>> on_save_process;
            std::unordered_map<uint64_t, base_objects::client_data_holder> clients;
            uint64_t local_client_id_generator = 0;


            std::chrono::high_resolution_clock::time_point last_usage;
            void make_save(int64_t chunk_x, int64_t chunk_z, bool also_erase);
            void make_save(int64_t chunk_x, int64_t chunk_z, chunk_row::iterator, bool also_erase);
            base_objects::atomic_holder<chunk_data> load_chunk_sync(int64_t chunk_x, int64_t chunk_z);

        public:
            enbt::compound general_world_data;
            enbt::compound world_game_rules;
            enbt::compound world_generator_data;
            enbt::compound world_records;

            ENBT::UUID world_seed;
            ENBT::UUID wandering_trader_id;
            float wandering_trader_spawn_chance = 0;
            int32_t wandering_trader_spawn_delay = 0;
            std::string world_name;
            std::string world_type;
            std::string generator_id;
            std::vector<std::string> enabled_datapacks;
            std::vector<std::string> enabled_plugins;
            std::vector<std::string> enabled_features;
            std::vector<base_objects::cubic_bounds_chunk> load_points;
            std::vector<base_objects::spherical_bounds_chunk> load_points_sphere;

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
            size_t max_random_tick_for_chunk = 100;
            std::chrono::milliseconds tick_speed = std::chrono::milliseconds(1000) / 20;
            std::chrono::milliseconds chunk_lifetime = std::chrono::seconds(1);
            std::chrono::milliseconds world_lifetime = std::chrono::seconds(50);


            int32_t clear_weather_time = 0;


            int32_t internal_version = 0;
            uint16_t chunk_y_count = 20; // == (320 / 16), do not change this config
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

            bool exists(int64_t chunk_x, int64_t chunk_z);
            //returns std::nullopt if chunk already queried for async load
            std::optional<base_objects::atomic_holder<chunk_data>> request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z);
            FuturePtr<base_objects::atomic_holder<chunk_data>> request_chunk_data(int64_t chunk_x, int64_t chunk_z);
            void save_chunks();
            void await_save_chunks();
            void save_and_unload_chunks();
            void save_and_unload_chunk(int64_t chunk_x, int64_t chunk_z);
            void unload_chunk(int64_t chunk_x, int64_t chunk_z);
            void save_chunk(int64_t chunk_x, int64_t chunk_z);
            void erase_chunk(int64_t chunk_x, int64_t chunk_z);
            void regenerate_chunk(int64_t chunk_x, int64_t chunk_z);

            void for_each_chunk(std::function<void(chunk_data& chunk)> func);
            void for_each_chunk(base_objects::square_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func);
            void for_each_chunk(base_objects::spherical_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func);
            void for_each_sub_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(sub_chunk_data& chunk)> func);
            void get_sub_chunk(int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, std::function<void(sub_chunk_data& chunk)> func);


            void for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(base_objects::square_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func);
            void for_each_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func);
            void for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func);


            void query_for_tick(int64_t global_x, uint64_t global_y, int64_t global_z);
            bool set_block(base_objects::block block, int64_t global_x, uint64_t global_y, int64_t global_z);
            bool place_block(base_objects::block block, int64_t global_x, uint64_t global_y, int64_t global_z);
            bool remove_block(int64_t global_x, uint64_t global_y, int64_t global_z);
            bool break_block(int64_t global_x, uint64_t global_y, int64_t global_z);
            void query_block(int64_t global_x, uint64_t global_y, int64_t global_z, std::function<void(base_objects::block block)> func);
            void query_block_entity(int64_t global_x, uint64_t global_y, int64_t global_z, std::function<void(base_objects::block_entity& block)> func);
            uint64_t register_client(const base_objects::client_data_holder& client);
            void unregister_client(uint64_t);


            void tick(std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time, bool update_tps);
            //unloads unused chunks and check themselves lifetime and active operations, if expired and there no active operations, then function will return true
            bool collect_unused_data(std::chrono::high_resolution_clock::time_point current_time, size_t& unload_limit);


            struct profiling_data {
                std::chrono::milliseconds tick_time = std::chrono::milliseconds(0);
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

                std::function<void(world_data& world, int64_t chunk_x, int64_t chunk_z, std::chrono::milliseconds tick_time)> slow_chunk_tick_callback = nullptr;
                std::function<void(world_data& world, std::chrono::milliseconds tick_time)> slow_world_tick_callback = nullptr;
                std::function<void(world_data& world)> got_tps_update = nullptr;
            } profiling;
        };

        class worlds_data {
            base_objects::ServerConfiguration& configuration;
            fast_task::task_mutex mutex;
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
            double tps = 0;


            worlds_data(base_objects::ServerConfiguration& configuration, const std::filesystem::path& base_path);

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


            void for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func);

            void for_each_entity(uint64_t world_id, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(uint64_t world_id, int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func);
            void for_each_entity(uint64_t world_id, int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func);

            //use only on tick task, returns nanoseconds to sleep
            std::chrono::nanoseconds apply_tick(std::mt19937& random_engine);
        };
    } // namespace storage
} // namespace crafted_craft


#endif /* SRC_STORAGE_WORLD_DATA */
