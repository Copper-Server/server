#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
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

        void merge_configs(ServerConfiguration& cfg, js_object& data) {
            bool calculate_favicon = false;
            {
                auto query = js_object::get_object(data["query"]);
                cfg.query.enabled = query["enabled"].or_apply(cfg.query.enabled);
                cfg.query.port = query["port"].or_apply(cfg.query.port);
            }
            {
                auto world = js_object::get_object(data["world"]);
                cfg.world.name = world["name"].or_apply(cfg.world.name.get());
                cfg.world.seed = world["seed"].or_apply(cfg.world.seed.get());
                cfg.world.type = world["type"].or_apply(cfg.world.type.get());
                cfg.world.unload_speed = world["unload_speed"].or_apply(cfg.world.unload_speed);
                cfg.world.generate_structures = world["generate_structures"].or_apply(cfg.world.generate_structures);
                {
                    auto generator_settings = js_object::get_object(world["generator_settings"]);
                    if (generator_settings.empty())
                        for (auto&& [key, value] : cfg.world.generator_settings)
                            generator_settings[key.get()] = value;
                    else {
                        cfg.world.generator_settings.clear();
                        for (auto&& [key, value] : generator_settings)
                            cfg.world.generator_settings[(std::string)key] = generator_settings[(std::string)key];
                    }
                }
            }
            {
                auto game_play = js_object::get_object(data["game_play"]);
                cfg.game_play.difficulty = game_play["difficulty"].or_apply(cfg.game_play.difficulty.get());
                cfg.game_play.gamemode = game_play["gamemode"].or_apply(cfg.game_play.gamemode.get());
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
                cfg.protocol.enable_encryption = protocol["enable_encryption"].or_apply(cfg.protocol.enable_encryption);
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
                calculate_favicon = (boost::json::string)status["favicon_path"].or_apply(std::string()) != cfg.status.favicon_path;
                cfg.status.favicon_path = (std::string)status["favicon_path"].or_apply(std::string());
                cfg.status.sample_players_count = status["sample_players_count"].or_apply(cfg.status.sample_players_count);
                cfg.status.enable = status["enable"].or_apply(cfg.status.enable);
                cfg.status.show_players = status["show_players"].or_apply(cfg.status.show_players);
            }
            {
                auto server = js_object::get_object(data["server"]);
                auto& folder = (boost::json::string&)server["storage_folder"].or_apply(cfg.server.storage_folder);

                if (folder.find_first_of(".,/\\#$%^&*()`~'\":;|?!<>") != folder.npos)
                    log::warn("server", "server config: root.server.storage_folder contains special symbol .,/\\#$%^&*()`~'\":;|?!<>, item has been ignored");
                else {
                    cfg.server.storage_folder = (std::string)folder;
                    std::filesystem::create_directories(cfg.server.get_storage_path());
                }
            }
            {
                auto allowed_dimensions = js_array::get_array(data["allowed_dimensions"]);
                if (allowed_dimensions.empty()) {
                    for (auto& id : cfg.allowed_dimensions)
                        allowed_dimensions.push_back(id.get().c_str());
                } else {
                    size_t arr_siz = allowed_dimensions.size();
                    for (size_t i = 0; i < arr_siz; i++)
                        cfg.allowed_dimensions.emplace((std::string)allowed_dimensions[i]);
                }
            }

            if (cfg.status.favicon_path != "" && calculate_favicon) {
                std::ifstream file(cfg.status.favicon_path, std::ifstream::in | std::ifstream::binary);
                if (file.is_open()) {
                    file.seekg(0, std::ifstream::end);
                    size_t file_size = file.tellg();
                    if (file_size < 28) {
                        log::error("server", "Failed to read favicon, icon too small, skipping...");
                        return;
                    }
                    file.seekg(0, std::ifstream::beg);
                    std::vector<uint8_t> res;
                    res.resize(file_size);
                    file.read((char*)res.data(), res.size());
                    uint32_t width = 0, height = 0;
                    width = ENBT::ConvertEndian(std::endian::big, *(uint32_t*)&res[16]);
                    height = ENBT::ConvertEndian(std::endian::big, *(uint32_t*)&res[20]);

                    if (width != 64 || height != 64) {
                        log::error("server", "Failed to read favicon, icon resolution not equal to 64x64, skipping...");
                        return;
                    }
                    cfg.status.favicon = std::move(res);
                }
            } else if (calculate_favicon)
                cfg.status.favicon.clear();
        }

        void save_config(const std::filesystem::path& config_file_path, boost::json::object& config_data) {
            std::ofstream file(config_file_path / "config.json", std::ofstream::trunc);
            if (!file.is_open()) {
                log::warn("server", "Failed to save config file. Can not open file.");
                return;
            }
            util::pretty_print(file, config_data);
        }

        void ServerConfiguration::load(const std::filesystem::path& config_file_path, bool fill_default_values) {
            auto config_data = try_read_json_file(config_file_path / "config.json");
            if (!config_data.has_value() && !fill_default_values) {
                log::warn("server", "Failed to read config file. Using default values.");
                return;
            }
            auto config = js_object::get_object(*config_data);
            //if (fill_default_values) {

            try {
                merge_configs(*this, config);
            } catch (const std::exception& ex) {
                log::error("server", ex.what());
                throw;
            }
            save_config(config_file_path, *config_data);
            //}
        }

        [[noreturn]] void decorated_exception(const std::string& desc, const std::string& part_path, const std::string& full_path) {
            assert(full_path.ends_with(part_path) && "The part path must belong to full path");

            std::string msq = desc + ", in path: " + full_path + "\n";
            size_t where_point = msq.size() - part_path.size();
            std::string point(where_point + 3, ' ');
            point[where_point + 2] = '^';
            throw std::runtime_error(msq + point);
        }

        boost::json::value& get_value_by_path(boost::json::value& value, std::string& path, const std::string& full_path) {
            if (path.empty())
                return value;
            auto pos = path.find_first_of(".[");
            if (pos == std::string::npos) {
                if (value.is_object()) {
                    auto& obj = value.get_object();
                    auto it = obj.find(path);
                    path.clear();
                    if (it != obj.end())
                        return it->value();
                    else
                        decorated_exception("The element not found", path, full_path);
                } else {
                    path.clear();
                    decorated_exception("Type miss match, excepted object but received: " + util::to_string(value.kind()), path, full_path);
                }
            }
            if (path[pos] == '[') {
                auto next = path.find_first_of(']', pos);
                if (next == std::string::npos)
                    decorated_exception("Incomplete expression. not found ] in after [", path, full_path);

                auto index_str = path.substr(pos + 1, next - pos - 1);
                path = path.substr(next + 1);

                if (value.is_array()) {
                    unsigned long pos = 0;
                    try {
                        pos = std::stoul(index_str);
                    } catch (const std::invalid_argument&) {
                        decorated_exception("The index is not integer", path, full_path);
                    } catch (const std::out_of_range&) {
                        decorated_exception("Index is too big", path, full_path);
                    }
                    if (value.get_array().size() <= pos)
                        decorated_exception("Index out of range", path, full_path);
                    return value.get_array()[pos];
                } else
                    decorated_exception("Type miss match, excepted array but received: " + util::to_string(value.kind()), path, full_path);
            } else {
                auto key = path.substr(0, pos);
                path = path.substr(pos + 1);
                if (key.empty())
                    return get_value_by_path(value, path, full_path);
                else {
                    if (value.is_object()) {
                        auto& obj = value.get_object();
                        auto it = obj.find(key);
                        if (it != obj.end()) {
                            if (path.empty())
                                return it->value();
                            else
                                return get_value_by_path(it->value(), path, full_path);
                        } else
                            decorated_exception("The element not found", path, full_path);
                    } else
                        decorated_exception("Type miss match, excepted object but received: " + util::to_string(value.kind()), path, full_path);
                }
            }
        }

        boost::json::value& get_value_by_path_(boost::json::value& entry, const std::string& path) {
            std::string tmp = path;
            return get_value_by_path(entry, tmp, path);
        }

        void ServerConfiguration::set(const std::filesystem::path& config_file_path, const std::string& config_item_path, const std::string& value) {
            boost::json::value config_data = boost::json::object();
            auto config = js_object::get_object(config_data.get_object());
            merge_configs(*this, config);
            auto& val = get_value_by_path_(config_data, config_item_path);
            boost::system::error_code ec;
            val = boost::json::parse(value, ec);
            if (ec)
                throw std::runtime_error("Failed to parse value, strings must be in \" scope and constants must be in lowercase");
            merge_configs(*this, config);
            save_config(config_file_path, config_data.get_object());
        }

        std::string ServerConfiguration::get(const std::string& config_item_path) {
            boost::json::value config_data = boost::json::object();
            auto config = js_object::get_object(config_data.get_object());
            merge_configs(*this, config);
            return util::pretty_print(get_value_by_path_(config_data, config_item_path));
        }
    }
}