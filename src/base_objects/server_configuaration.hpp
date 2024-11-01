#ifndef SRC_BASE_OBJECTS_SERVER_CONFIGUARATION
#define SRC_BASE_OBJECTS_SERVER_CONFIGUARATION
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace crafted_craft {
    namespace base_objects {
        struct ServerConfiguration {
            struct Query {
                uint16_t port = 25545;
                bool enabled = false;
            } query;

            struct World {
                std::string name = "overworld";
                std::string seed = "0";
                std::string type = "default";
                size_t unload_speed = 10; //max 1000 chunks per tick

                struct {
                    int64_t x = 0;
                    int64_t y = 64;
                    int64_t z = 0;
                    float yaw = 0;
                } spawn;

                std::unordered_map<std::string, std::string> generator_settings = {};
            } world;

            struct GamePlay {
                std::string difficulty = "normal";
                std::string gamemode = "survival";
                uint64_t max_chained_neighbor_updates = 4000;
                uint32_t max_tick_time = 70; //tick time
                uint32_t view_distance = 10;
                uint32_t simulation_distance = 5;
                uint32_t max_word_size = UINT32_MAX;
                uint32_t spawn_protection = 1;
                uint32_t player_idle_timeout = 2000; //ms
                bool hardcore = false;
                bool pvp = true;
                bool spawn_animals = true;
                bool spawn_monsters = true;
                bool allow_flight = true;
                bool sync_chunk_writes = false;
                bool enable_command_block = false;
            } game_play;

            struct Protocol {
                int32_t compression_threshold = -1;
                uint32_t max_players = 0; //0 for unlimited
                uint32_t rate_limit = 0;

                bool prevent_proxy_connections = false; //	If the ISP/AS sent from the server is different from the one from Mojang Studios' authentication server, the player is kicked.
                bool offline_mode = false;
                bool enable_encryption = true;

                enum class connection_conflict_t {
                    kick_connected,
                    prevent_join
                } connection_conflict = connection_conflict_t::kick_connected;
            } protocol;

            //in this struct everything can be disabled by setting to zero
            struct AntiCheat {
                struct Fly {
                    bool prevent_illegal_flying = true; //when player not have fly attribute then these checks will be performed for player
                    std::chrono::milliseconds allow_flying_time = std::chrono::milliseconds(200);
                } fly;

                struct Speed {
                    bool prevent_illegal_speed = true;
                    float max_speed = 0.3; //overspeed treeshold
                } speed;

                struct XRay {
                    uint32_t visibility_distance = 0; //hides blocks that are farther than this distance(in blocks)
                    bool send_fake_blocks = false;
                    bool hide_surrounded_blocks = false;
                    std::unordered_set<std::string> block_ids = {};
                } xray;

                bool check_block_breaking_time = true;

                float reach_threshold = 0.2;

                struct KillAura {
                    float angle_threshold = 5.0f;
                    bool enable = true;
                } killaura;

                bool nofall = true;

                struct Scaffold {
                    //list of checks
                    bool enable_all = true;
                } scaffold;

                struct FastPlace {
                    uint32_t max_clicks = 4;
                } fastplace;

                struct Movement {
                    bool detect_baritone = true;
                    bool no_slow_down = true;
                } movement;
            } anti_cheat;

            struct Mojang {
                bool enforce_secure_profile = true;


            } mojang;

            struct Status {
                std::string server_name = "CraftedCraft";
                std::string description = "The C++ Minecraft server!";

                //can be empty
                std::string favicon_path;
                /*[[computed_from(favicon_path)]] [runtime]*/ std::vector<uint8_t> favicon; //icon must be 64x64 and png format, can be empty
                size_t sample_players_count = 20; //how many players to show in the list, 0 to disable
                bool enable = true;
                bool show_players = true;
            } status;

            struct Server {
                /*[runtime]*/ const std::filesystem::path base_path = std::filesystem::current_path();

                std::string storage_folder = "storage";
                std::string worlds_folder = "storage/worlds";
                std::string ip = "localhost";
                uint16_t port = 25565;
                size_t accepting_threads = 0; //0 == auto, optional
                size_t working_threads = 0;   //0 == auto, optional
                size_t ssl_key_length = 1024; //1024, 2048, 4096, optional


                std::filesystem::path get_storage_path() const {
                    return base_path / storage_folder;
                }

                std::filesystem::path get_worlds_path() const {
                    return base_path / worlds_folder;
                }
            } server;

            //allowed dimensions to visit to player without the `action.world.transfer.disallowed` permission
            //if empty then this setting ignored
            std::unordered_set<std::string> allowed_dimensions = {"overworld"};


            void load(const std::filesystem::path& config_file_path, bool fill_default_values = true);
            void set(const std::filesystem::path& config_file_path, const std::string& config_item_path, const std::string& value);
            std::string get(const std::string& config_item_path);
        };
    }
}


#endif /* SRC_BASE_OBJECTS_SERVER_CONFIGUARATION */
