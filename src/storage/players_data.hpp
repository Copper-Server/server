#ifndef SRC_STORAGE_PLAYERS_DATA
#define SRC_STORAGE_PLAYERS_DATA
#include <filesystem>
#include <fstream>
#include <library/enbt.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/slot.hpp>
#include <string>

namespace copper_server {
    namespace storage {
        class player_data {
            std::filesystem::path path;

        public:
            player_data(const std::filesystem::path& path);
            base_objects::player player;

            void load();
            void save();
        };

        class players_data {
            std::filesystem::path base_path;

        public:
            players_data(const std::filesystem::path& base_path);

            player_data get_player_data(const std::string& player_uuid);
        };

    } // namespace storage
} // namespace copper_server


#endif /* SRC_STORAGE_PLAYERS_DATA */
