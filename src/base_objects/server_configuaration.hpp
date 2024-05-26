#ifndef SRC_BASE_OBJECTS_SERVER_CONFIGUARATION
#define SRC_BASE_OBJECTS_SERVER_CONFIGUARATION
#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace crafted_craft {
    namespace base_objects {
        struct ServerConfiguration {
            struct RCON {
                std::string password;
                uint16_t port;
                bool enabled;
                bool broadcast_to_ops;
            } rcon;

            struct Query {
                uint16_t port;
                bool enabled;
            } query;

            struct World {
                std::string name;
                std::string seed;
                std::string type;
                size_t unload_speed = 10; //max 1000 chunks per tick

                bool generate_structures;

                std::unordered_map<std::string, std::string> generator_settings;
            } world;

            struct GamePlay {
                std::string difficulty;
                uint64_t max_chained_neighbor_updates;
                uint32_t max_tick_time;
                uint32_t view_distance;
                uint32_t simulation_distance;
                uint32_t max_word_size;
                uint32_t spawn_protection;
                uint32_t player_idle_timeout;
                bool hardcore;
                bool pvp;
                bool spawn_animals;
                bool spawn_monsters;
                bool allow_flight;
                bool sync_chunk_writes;
            } game_play;

            struct Protocol {
                uint32_t compression_threshold = -1;
                uint32_t max_players = 0; //0 for unlimited
                uint32_t rate_limit = 0;

                bool prevent_proxy_connections = false; //	If the ISP/AS sent from the server is different from the one from Mojang Studios' authentication server, the player is kicked.

                enum class connection_conflict_t {
                    kick_connected,
                    prevent_join
                } connection_conflict = connection_conflict_t::kick_connected;
            } protocol;

            //in this struct everything can be disabled by setting to zero
            struct AntiCheat {
                struct Fly {
                    bool prevent_illegal_flying; //when player not have fly attribute then these checks will be performed for player
                    std::chrono::milliseconds allow_flying_time;
                } fly;

                struct Speed {
                    bool prevent_illegal_speed;
                    float max_speed;
                } speed;

                struct XRay {
                    uint32_t visibility_distance; //hides blocks that are farther than this distance(in blocks)
                    bool send_fake_blocks;
                    bool hide_surrounded_blocks;
                    std::unordered_set<std::string> block_ids;
                } xray;

                bool check_block_breaking_time;

                float reach_threshold;

                struct KillAura {
                    float angle_threshold;
                } killaura;

                bool nofall;

                struct Scaffold {
                    //list of checks
                    bool enable_all;
                } scaffold;

                struct FastPlace {
                    uint32_t max_clicks;
                } fastplace;

                struct NoSlowDown {
                    bool detect_baritone;
                } movement;
            } anti_cheat;

            struct Mojang {
                bool enforce_secure_profile = true;


            } mojang;

            struct Status {
                std::string server_name = "CraftedCraft";
                std::string description = "The C++ Minecraft server!";

                std::vector<uint8_t> favicon;     //icon must be 64x64 and png format
                size_t sample_players_count = 20; //how many players to show in the list, 0 to disable
                bool enable = true;
                bool show_players = true;
            } status;

            std::unordered_set<std::string> allowed_dimensions;

            std::string gamemode = "survival";
            uint32_t max_players = 20;
            bool enable_command_block = false;

            bool offline_mode = false;
        };
    }
}


#endif /* SRC_BASE_OBJECTS_SERVER_CONFIGUARATION */
