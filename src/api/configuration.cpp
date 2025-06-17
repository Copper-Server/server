
#include <filesystem>
#include <library/enbt/enbt.hpp>
#include <library/fast_task/src/files/files.hpp>
#include <library/list_array.hpp>
#include <src/api/configuration.hpp>
#include <src/base_objects/events/event.hpp>
#include <src/base_objects/packets.hpp>
#include <src/log.hpp>
#include <src/util/json_helpers.hpp>
#include <thread>

namespace copper_server::api::configuration {

    using namespace base_objects;
    using namespace util;

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
            cfg.world.name = world["name"].or_apply(cfg.world.name);
            cfg.world.seed = world["seed"].or_apply(cfg.world.seed);
            cfg.world.type = world["type"].or_apply(cfg.world.type);
            cfg.world.unload_speed = world["unload_speed"].or_apply(cfg.world.unload_speed);
            cfg.world.auto_save = world["auto_save"].or_apply(cfg.world.auto_save);
            {
                auto generator_settings = js_object::get_object(world["generator_settings"]);
                if (generator_settings.empty())
                    for (auto&& [key, value] : cfg.world.generator_settings)
                        generator_settings[key] = value;
                else {
                    cfg.world.generator_settings.clear();
                    for (auto&& [key, value] : generator_settings)
                        cfg.world.generator_settings[(std::string)key] = (std::string)generator_settings[key];
                }
            }
            {
                std::string saving_mode = world["saving_mode"].or_apply("zstd");
                static std::unordered_set<std::string> allowed_modes = {
                    "zstd",
                    "raw"
                };
                if (!allowed_modes.contains(saving_mode))
                    saving_mode = "zstd";
                cfg.world.saving_mode;
            }
            {
                static std::unordered_map<std::string, ServerConfiguration::World::world_not_found_for_client_e> world_not_found_for_client_from_str = {
                    {"kick", ServerConfiguration::World::world_not_found_for_client_e::kick},
                    {"transfer_to_default", ServerConfiguration::World::world_not_found_for_client_e::transfer_to_default},
                    {"request_plugin_or_default", ServerConfiguration::World::world_not_found_for_client_e::request_plugin_or_default},
                };
                static std::unordered_map<int, std::string> world_not_found_for_client_to_str = {
                    {(int)ServerConfiguration::World::world_not_found_for_client_e::kick, "kick"},
                    {(int)ServerConfiguration::World::world_not_found_for_client_e::transfer_to_default, "transfer_to_default"},
                    {(int)ServerConfiguration::World::world_not_found_for_client_e::request_plugin_or_default, "request_plugin_or_default"},
                };
                auto world_not_found_for_client = js_object::get_object(world["world_not_found_for_client"]);
                cfg.world.world_not_found_for_client = world_not_found_for_client_from_str.at(
                    world_not_found_for_client["world_not_found_for_client"].or_apply(
                        world_not_found_for_client_to_str.at((int)cfg.world.world_not_found_for_client)
                    )
                );
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
            cfg.game_play.reduced_debug_screen = game_play["reduced_debug_screen"].or_apply(cfg.game_play.reduced_debug_screen);
            if (game_play.contains("enabled_features")) {
                cfg.game_play.enabled_features.clear();
                auto enabled_features = js_array::get_array(game_play["enabled_features"]);
                cfg.game_play.enabled_features.reserve(enabled_features.size());
                for (auto&& it : enabled_features)
                    cfg.game_play.enabled_features.emplace((std::string)it);
            } else {
                auto& enabled_features = (game_play["enabled_features"] = boost::json::array()).get().get_array();
                enabled_features.reserve(cfg.game_play.enabled_features.size());
                for (auto& it : cfg.game_play.enabled_features)
                    enabled_features.push_back((boost::json::string)it);
            }
        }
        {
            auto protocol = js_object::get_object(data["protocol"]);
            cfg.protocol.compression_threshold = protocol["compression_threshold"].or_apply(cfg.protocol.compression_threshold);
            cfg.protocol.rate_limit = protocol["rate_limit"].or_apply(cfg.protocol.rate_limit);
            cfg.protocol.prevent_proxy_connections = protocol["prevent_proxy_connections"].or_apply(cfg.protocol.prevent_proxy_connections);
            cfg.protocol.enable_encryption = protocol["enable_encryption"].or_apply(cfg.protocol.enable_encryption);
            if (protocol.contains("allowed_versions")) {
                auto allowed_versions = js_array::get_array(protocol["allowed_versions"]);
                cfg.protocol.allowed_versions.clear();
                cfg.protocol.allowed_versions.reserve(allowed_versions.size());
                cfg.protocol.allowed_versions_processed.clear();
                cfg.protocol.allowed_versions_processed.reserve(allowed_versions.size());
                for (auto&& it : allowed_versions) {
                    std::string version = it;
                    cfg.protocol.allowed_versions.push_back(version);
                    cfg.protocol.allowed_versions_processed.push_back(packets::java_name_to_protocol(version));
                }
                cfg.protocol.allowed_versions_processed.unify();
                if (cfg.protocol.allowed_versions.empty())
                    cfg.protocol.allowed_versions_processed = {770};
            } else {
                auto allowed_versions = js_array::get_array(protocol["allowed_versions"] = boost::json::array());
                for (const auto& it : cfg.protocol.allowed_versions)
                    allowed_versions.push_back(it);
            }
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
                            block_ids.push_back(id);
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
            cfg.status.server_name = (std::string)status["server_name"].or_apply(cfg.status.server_name);
            cfg.status.description = (std::string)status["description"].or_apply(cfg.status.description);
            calculate_favicon = (boost::json::string)status["favicon_path"].or_apply(std::string()) != cfg.status.favicon_path;
            cfg.status.favicon_path = (std::string)status["favicon_path"].or_apply(std::string());
            cfg.status.sample_players_count = status["sample_players_count"].or_apply(cfg.status.sample_players_count);
            cfg.status.enable = status["enable"].or_apply(cfg.status.enable);
            cfg.status.show_players = status["show_players"].or_apply(cfg.status.show_players);
        }
        {
            auto server = js_object::get_object(data["server"]);
            auto& folder = (boost::json::string&)server["storage_folder"].or_apply(cfg.server.storage_folder);


            if (folder.find_first_of(".,\\#$%^&*()`~'\":;|?!<>") != folder.npos)
                log::warn("server", "server config: root.server.storage_folder contains special symbol .,\\#$%^&*()`~'\":;|?!<>, item has been ignored");
            else {
                cfg.server.storage_folder = (std::string)folder;
                std::filesystem::create_directories(cfg.server.get_storage_path());
            }

            auto& worlds = (boost::json::string&)server["worlds_folder"].or_apply(cfg.server.worlds_folder);
            if (worlds.find_first_of(".,\\#$%^&*()`~'\":;|?!<>") != worlds.npos)
                log::warn("server", "server config: root.server.worlds_folder contains special symbol .,\\#$%^&*()`~'\":;|?!<>, item has been ignored");
            else {
                cfg.server.worlds_folder = (std::string)worlds;
                std::filesystem::create_directories(cfg.server.get_storage_path());
            }
            cfg.server.ip = (std::string)server["ip"].or_apply(cfg.server.ip);
            cfg.server.port = server["port"].or_apply(cfg.server.port);
            cfg.server.offline_mode = server["offline_mode"].or_apply(cfg.server.offline_mode);
            cfg.server.max_players = server["max_players"].or_apply(cfg.server.max_players);
            if (server.contains("network_threads"))
                cfg.server.network_threads = server["network_threads"];
            if (server.contains("working_threads"))
                cfg.server.working_threads = server["working_threads"];
            if (server.contains("ssl_key_length"))
                cfg.server.ssl_key_length = server["ssl_key_length"];


            cfg.server.timeout_seconds = server["timeout_seconds"].or_apply(cfg.server.timeout_seconds);
            cfg.server.max_accept_buffer = server["max_accept_buffer"].or_apply(cfg.server.max_accept_buffer);
            cfg.server.all_connections_timeout = server["all_connections_timeout"].or_apply(cfg.server.all_connections_timeout);

            size_t total_threads_ = std::thread::hardware_concurrency();
            if (cfg.server.network_threads == 0)
                cfg.server.network_threads = (total_threads_ / 2) ? (total_threads_ / 2) : 1;
            if (cfg.server.working_threads == 0)
                cfg.server.working_threads = (total_threads_ / 2) ? (total_threads_ / 2) : 1;
        }
        {
            auto allowed_dimensions = js_array::get_array(data["allowed_dimensions"]);
            if (allowed_dimensions.empty()) {
                for (auto& id : cfg.allowed_dimensions)
                    allowed_dimensions.push_back(id);
            } else {
                size_t arr_siz = allowed_dimensions.size();
                for (size_t i = 0; i < arr_siz; i++)
                    cfg.allowed_dimensions.emplace((std::string)allowed_dimensions[i]);
            }
        }

        if (cfg.status.favicon_path != "" && calculate_favicon) {
            fast_task::files::async_iofstream file(cfg.status.favicon_path, std::istream::in | std::istream::binary);
            if (file.is_open()) {
                file.seekg(0, std::istream::end);
                size_t file_size = file.tellg();
                if (file_size < 28) {
                    log::error("server", "Failed to read favicon, icon too small, skipping...");
                    return;
                }
                file.seekg(0, std::istream::beg);
                std::vector<uint8_t> res;
                res.resize(file_size);
                file.read((char*)res.data(), res.size());
                uint32_t width = 0, height = 0;
                width = enbt::endian_helpers::convert_endian(std::endian::big, *(uint32_t*)&res[16]);
                height = enbt::endian_helpers::convert_endian(std::endian::big, *(uint32_t*)&res[20]);

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
        fast_task::files::async_iofstream file(config_file_path / "config.json", std::iostream::trunc);
        if (!file.is_open()) {
            log::warn("server", "Failed to save config file. Can not open file.");
            return;
        }
        file << util::pretty_print(config_data);
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

    std::string ServerConfiguration::get(const std::string& config_item_path) {
        boost::json::value config_data = boost::json::object();
        auto config = js_object::get_object(config_data.get_object());
        merge_configs(*this, config);
        return util::pretty_print(get_value_by_path_(config_data, config_item_path));
    }

    ServerConfiguration config;
    bool loaded = false;


    base_objects::events::event<void> updated;

    void load(bool fill_default_values) {
        {
            auto config_file_path = std::filesystem::current_path();
            auto config_data = try_read_json_file(config_file_path / "config.json");
            if (!config_data.has_value() && !fill_default_values) {
                log::warn("server", "Failed to read config file. Using default values.");
                return;
            } else if (!config_data.has_value())
                config_data = boost::json::object();
            auto config_js = js_object::get_object(*config_data);
            //if (fill_default_values) {

            try {
                merge_configs(config, config_js);
            } catch (const std::exception& ex) {
                log::error("server", ex.what());
                throw;
            }
            save_config(config_file_path, *config_data);
            //}
        }
        loaded = true;
        updated();
    }

    ServerConfiguration& get() {
        if (!loaded)
            load(true);
        return config;
    }

    void set_item(const std::string& config_item_path, const std::string& value) {
        boost::json::value config_data = boost::json::object();
        auto config_js = js_object::get_object(config_data.get_object());
        merge_configs(config, config_js);
        auto& val = get_value_by_path_(config_data, config_item_path);
        boost::system::error_code ec;
        val = boost::json::parse(value, ec);
        if (ec)
            throw std::runtime_error("Failed to parse value, strings must be in \" scope and constants must be in lowercase");
        merge_configs(config, config_js);
        save_config(std::filesystem::current_path(), config_data.get_object());
        updated();
    }

    std::string get_item(const std::string& config_item_path) {
        return get().get(config_item_path);
    }
}