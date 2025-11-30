/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_CONFIGURATION
#define SRC_API_CONFIGURATION
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <library/list_array.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <src/base_objects/events/event.hpp>
#include <src/util/nbt.hpp>

namespace copper_server::api::configuration {
    struct server_configuration {
        struct World {
            std::string name = "overworld";
            std::string seed = "0";
            std::string type = "minecraft:overworld";
            std::string generator_type = "default";
            std::string saving_mode = "zstd"; //allowed modes is 'zstd' and 'raw'

            size_t unload_speed = 10; //max 10 chunks at once
            size_t load_speed = 10;   //max 10 chunks at once
            size_t auto_save = 6000;  //0 to disable

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
            bool allow_flight = true;
            bool sync_chunk_writes = false;
            bool enable_command_block = false;
            bool reduced_debug_screen = false;
            bool enable_code_of_conduct = false;

            struct {
                bool spawn_animals = true;
                bool spawn_monsters = true;
                double enable_spawners = 16;
                double spawn_mobs_in_range = 24;
                double tick_mobs_in_range = 32;
                double despawn_mobs_outside = 128;

                struct {
                    uint32_t despawn_after_inactivity = 30 * 20;
                    uint32_t high_light_penalty = 2; //1 + 2
                    float despawn_chance = 1.0f / 800;
                    uint8_t high_light_value = 12;
                } despawn{};

                struct { //
                    double enable_spawners = 16 * 16;
                    double spawn_mobs_in_range = 24 * 24;
                    double tick_mobs_in_range = 32 * 32;
                    double despawn_mobs_outside = 128 * 128;
                } /*[[computed_from(entity)]] [runtime]*/ squared_values{};
            } entity{};
        } game_play;

        struct Protocol {
            int32_t compression_threshold = -1;
            uint32_t rate_limit = 0; //0 for unlimited, in bytes per second
            uint32_t max_unacknowledged_chunk_batches = 10;
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
            bool skip_unregistered_packets = false; //if set, the player could send packets and would'nt be kicked, could be useful for disabling some functionality. For example disabling specific base/play_engine/* plugin

            enum class connection_conflict_t {
                kick_connected,
                prevent_join
            } connection_conflict
                = connection_conflict_t::kick_connected;
        } protocol;


        struct Mojang {
            static constexpr std::string_view session_server = "sessionserver.mojang.com";
            static constexpr std::string_view services_server = "api.minecraftservices.com";
            static constexpr std::string_view snoop_server = "snoop.minecraft.net";
            bool enforce_secure_profile = true; //enables signature signing for chat messages using mojang's service
            bool enable_snoop_stats = false;    //should server send server stats
            bool prevent_proxy_connections = false; //sends player ip to session server to verify ip
        } mojang;

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
            bool frozen_config : 1 = false;        //disables the config file and uses default values set at compile time(still modifable on runtime)
            bool enable_debug_task_thread_naming : 1 = false; //optional


            std::filesystem::path get_storage_path() const {
                return (base_path / storage_folder).lexically_normal();
            }

            std::filesystem::path get_worlds_path() const {
                return (base_path / worlds_folder).lexically_normal();
            }
        } server;


        std::unordered_set<std::string> disabled_plugins;

        //allowed dimensions to visit to player without the `action.world.transfer.disallowed` permission
        //if empty then this setting ignored
        std::unordered_set<std::string> allowed_dimensions = {"overworld"};

        class plugin_actions {
            util::nbt& it;
            plugin_actions(util::nbt& it);
            friend struct server_configuration;

        public:
            struct get_value {};

            plugin_actions operator^(std::string_view name);
            const util::nbt& operator^(get_value);
            plugin_actions& operator^=(const util::nbt& value);
            plugin_actions& operator|=(const util::nbt& value);
            operator const util::nbt&() const;
        };

        plugin_actions operator^(std::string_view name);

        std::string get(const std::string& config_item_path);


    private:
        util::nbt plugins;

        friend util::nbt& _get_plugins_(server_configuration& cfg);
    };

    server_configuration& get();

    void load(bool fill_default_values = true);

    void set_item(const std::string& config_item_path, const std::string& value); //accepts json
    std::string get_item(const std::string& config_item_path);                    //returns json
    void apply_preset(const std::string& preset);

    extern base_objects::events::event<void> updated;
}

static constexpr inline auto get_conf = copper_server::api::configuration::server_configuration::plugin_actions::get_value{};

#endif /* SRC_API_CONFIGURATION */
