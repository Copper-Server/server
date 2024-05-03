#ifndef SRC_STORAGE_WORLD_DATA
#define SRC_STORAGE_WORLD_DATA
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include <atomic>
#include <string>
#include <vector>

namespace crafted_craft {
    namespace storage {
        class world_data {
            std::string path;

        public:
            ENBT::UUID world_seed;
            std::string world_name;
            std::string world_type;
            std::string generator_id;
            uint16_t chunk_y_count = 15;

            double border_center_x = 0;
            double border_center_z = 0;
            double border_size = 0;
            double border_safe_zone = 5;
            double border_damage_per_block = 0;
            double border_lerp_target = 0;
            uint64_t border_lerp_time = 0;
            double border_warning_blocks = 5;
            double border_warning_time = 15;
            int32_t clear_weather_time = 0;

            std::vector<std::string> enabled_datapacks;
            std::vector<std::string> enabled_plugins;
            std::vector<std::string> enabled_features;

            int32_t internal_version = 0;
            long long day_time = 0;
            int8_t difficulty = 0;
            bool difficulty_locked = false;
            ENBT general_world_data;
            ENBT world_game_rules;
            ENBT world_generator_data;
            uint8_t default_gamemode = 0;
            bool is_hardcore = false;
            bool initialized = false;
            bool raining = false;
            bool thundering = false;
            long long time = 0;
            ENBT::UUID wandering_trader_id;
            float wandering_trader_spawn_chance = 0;
            int32_t wandering_trader_spawn_delay = 0;


            ENBT world_records;

            world_data(const std::string& path);


            void load();
            void save();
        };

        class worlds_data {
            std::string base_path;

        public:
            worlds_data(const std::string& base_path);

            world_data get_world_data(const std::string& world_id);
        };


    } // namespace storage

} // namespace crafted_craft


#endif /* SRC_STORAGE_WORLD_DATA */
