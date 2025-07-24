#ifndef SRC_API_CONFIGURATION
#define SRC_API_CONFIGURATION
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <src/base_objects/events/event.hpp>

namespace copper_server::api::configuration {
    struct ServerConfiguration {

        struct World {
            std::string name = "overworld";
            std::string seed = "0";
            std::string type = "default";
            std::string saving_mode = "zstd"; //allowed modes is 'zstd' and 'raw'

            size_t unload_speed = 10; //max 10 chunks per tick and per world
            size_t auto_save = 6000;

            struct {
                int64_t x = 0;
                int64_t y = 64;
                int64_t z = 0;
                float yaw = 0;
            } spawn;

            std::unordered_map<std::string, std::string> generator_settings = {};
            enum class world_not_found_for_client_e {
                kick,
                transfer_to_default,
                request_plugin_or_default
            } world_not_found_for_client
                = world_not_found_for_client_e::transfer_to_default;
        } world;

        struct GamePlay {
            std::unordered_set<std::string> enabled_features;
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
            bool reduced_debug_screen = false;
        } game_play;

        struct Protocol {
            int32_t compression_threshold = -1;
            uint32_t rate_limit = 0; //0 for unlimited, in bytes per second
            bool handle_legacy = false;
            uint16_t new_client_buffer = 100;       //buffer for new connections, in bytes, used to prevent DoS attacks
            uint16_t buffer = 8192;                 //buffer for connections, in bytes
            uint16_t max_accept_packet_size = 8192; //8192 bytes, maximum packet size, compressed, if packet is too large then client will be disconnected
            uint16_t max_send_packet_size = 8192;   //8192 bytes, maximum packet size, if packet is too large then client will be disconnected
            float timeout_seconds = 30;
            float keep_alive_send_each_seconds = 20;
            float all_connections_timeout_seconds = 30000; //30 sec


            bool prevent_proxy_connections = false; //	If the ISP/AS sent from the server is different from the one from Mojang Studios' authentication server, the player is kicked.
            bool enable_encryption = true;
            bool send_nbt_data_in_chunk = true; //enabled by default to be same as vanilla server, this option exists to allow 'fix' chunk ban and reduce network consumption, should not affect gameplay for regular players

            enum class connection_conflict_t {
                kick_connected,
                prevent_join
            } connection_conflict
                = connection_conflict_t::kick_connected;
        } protocol;

        //in this struct everything can be disabled by setting to zero
        struct AntiCheat {
            struct Fly {
                bool prevent_illegal_flying = true; //when player not have fly attribute then these checks will be performed for player
                std::chrono::milliseconds allow_flying_time = std::chrono::milliseconds(200);
            } fly;

            struct Speed {
                bool prevent_illegal_speed = true;
                float max_speed = 0.3f; //overspeed threshold
            } speed;

            struct XRay {
                uint32_t visibility_distance = 0; //hides blocks that are farther than this distance(in blocks)
                bool send_fake_blocks = false;
                bool hide_surrounded_blocks = false;
                std::unordered_set<std::string> block_ids = {};
            } xray;

            bool check_block_breaking_time = true;

            float reach_threshold = 0.2f;

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
            bool enforce_secure_profile = true; //enables signature signing for chat messages using mojang's service
        } mojang;

        struct Query {
            uint16_t port = 25545;
            bool enabled = false;
        } query;

        struct Status {
            std::string server_name = "Copper Server";
            std::string description = "The C++ Minecraft server!";

            //can be empty
            std::string favicon_path;
            /*[[computed_from(favicon_path)]] [runtime]*/ std::vector<uint8_t> favicon; //icon must be 64x64 and png format, can be empty
            size_t sample_players_count = 20;                                           //how many players to show in the list, 0 to disable
            bool enable = true;
            bool show_players = true;
        } status;

        struct Server {
            /*[runtime]*/ const std::filesystem::path base_path = std::filesystem::current_path();

            std::string storage_folder = "storage";
            std::string worlds_folder = "storage/worlds";
            std::string ip = "localhost";
            size_t working_threads = 0;   //0 == auto, optional
            size_t ssl_key_length = 1024; //1024, 2048, 4096, optional
            uint32_t max_players = 0;     //0 for unlimited
            uint16_t port = 25565;
            bool offline_mode : 1 = false;
            bool prevent_chat_reports : 1 = false; //if true then chat reports will be prevented despite `mojang.enforce_secure_profile` setting
            bool world_debug_mode : 1 = false;     //disables disk usage for worlds
            bool frozen_config : 1 = false;        //disables file and uses default values


            std::filesystem::path get_storage_path() const {
                return (base_path / storage_folder).lexically_normal();
            }

            std::filesystem::path get_worlds_path() const {
                return (base_path / worlds_folder).lexically_normal();
            }
        } server;

        //allows custom plugin settings
        enbt::compound plugins;

        list_array<std::string> disabled_plugins;

        //allowed dimensions to visit to player without the `action.world.transfer.disallowed` permission
        //if empty then this setting ignored
        std::unordered_set<std::string> allowed_dimensions = {"overworld"};

        class plugin_actions {
            enbt::value& it;
            plugin_actions(enbt::value& it);
            friend struct ServerConfiguration;

        public:
            plugin_actions operator^(std::string_view name);
            plugin_actions& operator^=(const enbt::value& value);
            plugin_actions& operator|=(const enbt::value& value);
            operator const enbt::value&() const;
        };

        plugin_actions operator^(std::string_view name);

        std::string get(const std::string& config_item_path);
    };

    ServerConfiguration& get();

    void load(bool fill_default_values = true);

    void set_item(const std::string& config_item_path, const std::string& value); //accepts json
    std::string get_item(const std::string& config_item_path);                    //returns json

    extern base_objects::events::event<void> updated;
}

#endif /* SRC_API_CONFIGURATION */
