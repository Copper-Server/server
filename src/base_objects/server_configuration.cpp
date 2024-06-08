#include "../log.hpp"
#include "../util/json_helpers.hpp"
#include "server_configuaration.hpp"
#include <filesystem>
#include <fstream>

namespace crafted_craft {
    using namespace util;

    namespace base_objects {

        std::string to_string(ServerConfiguration::Protocol::connection_conflict_t conflict_type) {
            switch (conflict_type) {
                using t = ServerConfiguration::Protocol::connection_conflict_t;
            case t::kick_connected:
                return "kick_connected";
            case t::prevent_join:
                return "prevent_join";
            }
            throw std::runtime_error("Stack corruption or incomplete to_string code");
        }

        void set_from_string(ServerConfiguration::Protocol::connection_conflict_t& conflict_type, const std::string& val) {
            using t = ServerConfiguration::Protocol::connection_conflict_t;
            if (val == "kick_connected")
                conflict_type = t::kick_connected;
            else if (val == "prevent_join")
                conflict_type = t::prevent_join;
        }

        void load_fill(ServerConfiguration& cfg, js_object& data) {
            {
                auto rcon = js_object::get_object(data["rcon"]);
                cfg.rcon.password = rcon["password"].or_apply(cfg.rcon.password);
                cfg.rcon.port = rcon["port"].or_apply(cfg.rcon.port);
                cfg.rcon.enabled = rcon["enabled"].or_apply(cfg.rcon.enabled);
                cfg.rcon.broadcast_to_ops = rcon["broadcast_to_ops"].or_apply(cfg.rcon.broadcast_to_ops);
            }
            {
                auto query = js_object::get_object(data["query"]);
                cfg.query.enabled = query["enabled"].or_apply(cfg.query.enabled);
                cfg.query.port = query["port"].or_apply(cfg.query.port);
            }
            {
                auto world = js_object::get_object(data["world"]);
                cfg.world.name = world["name"].or_apply(cfg.world.name);
                cfg.world.seed = world["seed"].or_apply(cfg.world.seed);
                cfg.world.type = world["type"].or_apply(cfg.world.type);
                cfg.world.unload_speed = world["unload_speed"].or_apply(cfg.world.unload_speed);
                cfg.world.generate_structures = world["generate_structures"].or_apply(cfg.world.generate_structures);
                {
                    auto generator_settings = js_object::get_object(world["generator_settings"]);
                    if (generator_settings.empty())
                        for (auto&& [key, value] : cfg.world.generator_settings)
                            generator_settings[key] = value;
                    else {
                        cfg.world.generator_settings.clear();
                        for (auto&& [key, value] : generator_settings)
                            cfg.world.generator_settings[key] = generator_settings[key];
                    }
                }
            }
            {
                auto game_play = js_object::get_object(data["game_play"]);
                cfg.game_play.difficulty = game_play["difficulty"].or_apply(cfg.game_play.difficulty);
                cfg.game_play.gamemode = game_play["gamemode"].or_apply(cfg.game_play.gamemode);
                cfg.game_play.max_chained_neighbor_updates = game_play["max_chained_neighbor_updates"].or_apply(cfg.game_play.max_chained_neighbor_updates);
                cfg.game_play.max_tick_time = game_play["max_tick_time"].or_apply(cfg.game_play.max_tick_time);
                cfg.game_play.view_distance = game_play["view_distance"].or_apply(cfg.game_play.view_distance);
                cfg.game_play.simulation_distance = game_play["simulation_distance"].or_apply(cfg.game_play.simulation_distance);
                cfg.game_play.max_word_size = game_play["max_word_size"].or_apply(cfg.game_play.max_word_size);
                cfg.game_play.spawn_protection = game_play["spawn_protection"].or_apply(cfg.game_play.spawn_protection);
                cfg.game_play.player_idle_timeout = game_play["player_idle_timeout"].or_apply(cfg.game_play.player_idle_timeout);
                cfg.game_play.hardcore = game_play["hardcore"].or_apply(cfg.game_play.hardcore);
                cfg.game_play.pvp = game_play["pvp"].or_apply(cfg.game_play.pvp);
                cfg.game_play.spawn_animals = game_play["spawn_animals"].or_apply(cfg.game_play.spawn_animals);
                cfg.game_play.spawn_monsters = game_play["spawn_monsters"].or_apply(cfg.game_play.spawn_monsters);
                cfg.game_play.allow_flight = game_play["allow_flight"].or_apply(cfg.game_play.allow_flight);
                cfg.game_play.sync_chunk_writes = game_play["sync_chunk_writes"].or_apply(cfg.game_play.sync_chunk_writes);
                cfg.game_play.enable_command_block = game_play["enable_command_block"].or_apply(cfg.game_play.enable_command_block);
            }
            {
                auto protocol = js_object::get_object(data["protocol"]);
                cfg.protocol.compression_threshold = protocol["compression_threshold"].or_apply(cfg.protocol.compression_threshold);
                cfg.protocol.max_players = protocol["max_players"].or_apply(cfg.protocol.max_players);
                cfg.protocol.rate_limit = protocol["rate_limit"].or_apply(cfg.protocol.rate_limit);
                cfg.protocol.prevent_proxy_connections = protocol["prevent_proxy_connections"].or_apply(cfg.protocol.prevent_proxy_connections);
                cfg.protocol.offline_mode = protocol["offline_mode"].or_apply(cfg.protocol.offline_mode);
                set_from_string(cfg.protocol.connection_conflict, protocol["connection_conflict"].or_apply(to_string(cfg.protocol.connection_conflict)));
            }
            {
                auto anti_cheat = js_object::get_object(data["anti_cheat"]);
                {
                    auto fly = js_object::get_object(anti_cheat["fly"]);
                    cfg.anti_cheat.fly.prevent_illegal_flying = fly["prevent_illegal_flying"].or_apply(cfg.anti_cheat.fly.prevent_illegal_flying);
                    cfg.anti_cheat.fly.allow_flying_time = std::chrono::milliseconds((int64_t)fly["allow_flying_time"].or_apply(cfg.anti_cheat.fly.allow_flying_time.count()));
                }
                {
                    auto speed = js_object::get_object(anti_cheat["speed"]);
                    cfg.anti_cheat.speed.prevent_illegal_speed = speed["prevent_illegal_speed"].or_apply(cfg.anti_cheat.speed.prevent_illegal_speed);
                    cfg.anti_cheat.speed.max_speed = speed["max_speed"].or_apply(cfg.anti_cheat.speed.max_speed);
                }
                {
                    auto xray = js_object::get_object(anti_cheat["xray"]);
                    cfg.anti_cheat.xray.visibility_distance = xray["visibility_distance"].or_apply(cfg.anti_cheat.xray.visibility_distance);
                    cfg.anti_cheat.xray.send_fake_blocks = xray["send_fake_blocks"].or_apply(cfg.anti_cheat.xray.send_fake_blocks);
                    cfg.anti_cheat.xray.hide_surrounded_blocks = xray["hide_surrounded_blocks"].or_apply(cfg.anti_cheat.xray.hide_surrounded_blocks);
                    {
                        auto block_ids = js_array::get_array(anti_cheat["block_ids"]);
                        if (block_ids.empty()) {
                            for (auto& id : cfg.anti_cheat.xray.block_ids)
                                block_ids.push_back(id.c_str());
                        } else {
                            size_t arr_siz = block_ids.size();
                            for (size_t i = 0; i < arr_siz; i++)
                                cfg.anti_cheat.xray.block_ids.emplace((std::string)block_ids[i]);
                        }
                    }
                }
                cfg.anti_cheat.check_block_breaking_time = anti_cheat["check_block_breaking_time"].or_apply(cfg.anti_cheat.check_block_breaking_time);
                cfg.anti_cheat.reach_threshold = anti_cheat["reach_threshold"].or_apply(cfg.anti_cheat.reach_threshold);

                {
                    auto killaura = js_object::get_object(anti_cheat["killaura"]);
                    cfg.anti_cheat.killaura.angle_threshold = killaura["angle_threshold"].or_apply(cfg.anti_cheat.killaura.angle_threshold);
                    cfg.anti_cheat.killaura.enable = killaura["enable"].or_apply(cfg.anti_cheat.killaura.enable);
                }
                cfg.anti_cheat.nofall = anti_cheat["nofall"].or_apply(cfg.anti_cheat.nofall);
                {
                    auto scaffold = js_object::get_object(anti_cheat["scaffold"]);
                    cfg.anti_cheat.scaffold.enable_all = scaffold["enable_all"].or_apply(cfg.anti_cheat.scaffold.enable_all);
                }
                {
                    auto fastplace = js_object::get_object(anti_cheat["fastplace"]);
                    cfg.anti_cheat.fastplace.max_clicks = fastplace["max_clicks"].or_apply(cfg.anti_cheat.fastplace.max_clicks);
                }
                {
                    auto movement = js_object::get_object(anti_cheat["movement"]);
                    cfg.anti_cheat.movement.detect_baritone = movement["detect_baritone"].or_apply(cfg.anti_cheat.movement.detect_baritone);
                    cfg.anti_cheat.movement.no_slow_down = movement["no_slow_down"].or_apply(cfg.anti_cheat.movement.no_slow_down);
                }
            }
            {
                auto mojang = js_object::get_object(data["mojang"]);
                cfg.mojang.enforce_secure_profile = mojang["enforce_secure_profile"].or_apply(cfg.mojang.enforce_secure_profile);
            }
            {
                auto status = js_object::get_object(data["status"]);
                cfg.status.server_name = status["server_name"].or_apply(cfg.status.server_name);
                cfg.status.description = status["description"].or_apply(cfg.status.description);
                if (status.contains("favicon")) {
                    //TODO
                }
                cfg.status.sample_players_count = status["sample_players_count"].or_apply(cfg.status.sample_players_count);
                cfg.status.enable = status["enable"].or_apply(cfg.status.enable);
                cfg.status.show_players = status["show_players"].or_apply(cfg.status.show_players);
            }
            {
                auto allowed_dimensions = js_array::get_array(data["allowed_dimensions"]);
                if (allowed_dimensions.empty()) {
                    for (auto& id : cfg.allowed_dimensions)
                        allowed_dimensions.push_back(id.c_str());
                } else {
                    size_t arr_siz = allowed_dimensions.size();
                    for (size_t i = 0; i < arr_siz; i++)
                        cfg.allowed_dimensions.emplace((std::string)allowed_dimensions[i]);
                }
            }
        }

        void ServerConfiguration::load(const std::filesystem::path& config_file_path, bool fill_default_values) {
            boost::json::object config_data;
            {
                std::ifstream file(config_file_path / "config.json");
                if (file.is_open()) {
                    boost::system::error_code ec;
                    config_data = boost::json::parse(file, ec).as_object();
                    file.close();
                    if (ec) {
                        std::string err_string =
                            ec.has_location()
                                ? std::format("Failed to read config file because:\n{}\n On:\n{}", ec.message(), ec.location().to_string())
                                : std::format("Failed to read config file because:\n{}", ec.message());
                        log::warn("server", err_string);

                        if (fill_default_values)
                            config_data = boost::json::object();
                        else
                            return;
                    }
                } else if (fill_default_values)
                    config_data = boost::json::object();
                else {
                    log::warn("server", "Failed to open config file. Using default values.");
                    return;
                }
            }
            auto config = js_object::get_object(config_data);
            //if (fill_default_values) {

            try {
                load_fill(*this, config);
            } catch (const std::exception& ex) {
                log::error("server", ex.what());
                throw;
            }
            {
                std::ofstream file(config_file_path / "config.json", std::ofstream::trunc);
                if (!file.is_open()) {
                    log::warn("server", "Failed to save config file. Can not open file.");
                    return;
                }
                util::pretty_print(file, config);
            }
            //}
        }
    }
}