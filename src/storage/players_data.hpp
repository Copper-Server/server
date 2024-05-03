#ifndef SRC_STORAGE_PLAYERS_DATA
#define SRC_STORAGE_PLAYERS_DATA
#include "../base_objects/player.hpp"
#include "../base_objects/slot.hpp"
#include "../library/enbt.hpp"
#include <fstream>
#include <string>

namespace crafted_craft {
    namespace storage {
        class player_data {
            std::string path;

        public:
            player_data(const std::string& path);
            base_objects::player player;

            void load();
            void save();
        };

        class players_data {
            std::string base_path;

        public:
            players_data(const std::string& base_path);

            player_data get_player_data(const std::string& player_uuid);
        };

    } // namespace storage


} // namespace crafted_craft


#endif /* SRC_STORAGE_PLAYERS_DATA */
