/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <filesystem>
#include <library/fast_task/include/files.hpp>
#include <library/list_array.hpp>
#include <src/api/configuration.hpp>
#include <src/api/log.hpp>
#include <src/base_objects/events/event.hpp>
#include <src/util/conversions.hpp>
#include <src/util/endian.hpp>
#include <src/util/json_helpers.hpp>
#include <thread>

namespace copper_server::api::configuration {
    server_configuration config;
    bool loaded = false;

    std::string to_string(server_configuration::Protocol::connection_conflict_t conflict_type) {
        switch (conflict_type) {
            using t = server_configuration::Protocol::connection_conflict_t;
        case t::kick_connected:
            return "kick_connected";
        case t::prevent_join:
            return "prevent_join";
        }
        throw std::runtime_error("Stack corruption or incomplete to_string code");
    }

    void set_from_string(server_configuration::Protocol::connection_conflict_t& conflict_type, const std::string& val) {
        using t = server_configuration::Protocol::connection_conflict_t;
        if (val == "kick_connected")
            conflict_type = t::kick_connected;
        else if (val == "prevent_join")
            conflict_type = t::prevent_join;
    }

    void merge_configs_world(server_configuration& cfg, util::js_object& data) {
        auto world = util::js_object::get_object(data["world"]);
        cfg.world.name = (std::string)world["name"].or_apply(cfg.world.name);
        cfg.world.seed = world["seed"].or_apply(cfg.world.seed).to_string();
        cfg.world.type = (std::string)world["type"].or_apply(cfg.world.type);
        if (!cfg.world.type.contains(':'))
            cfg.world.type = "minecraft:" + cfg.world.type;
        cfg.world.generator_type = (std::string)world["generator_type"].or_apply(cfg.world.generator_type);
        cfg.world.unload_speed = world["unload_speed"].or_apply(cfg.world.unload_speed);
        cfg.world.load_speed = world["load_speed"].or_apply(cfg.world.load_speed);
        cfg.world.auto_save = world["auto_save"].or_apply(cfg.world.auto_save);
        {
            auto generator_settings = util::js_object::get_object(world["generator_settings"]);
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
            cfg.world.saving_mode = std::move(saving_mode);
        }
        {
            static std::unordered_map<std::string, server_configuration::World::world_not_found_for_client_e> world_not_found_for_client_from_str = {
                {"kick", server_configuration::World::world_not_found_for_client_e::kick},
                {"transfer_to_default", server_configuration::World::world_not_found_for_client_e::transfer_to_default},
                {"request_plugin_or_default", server_configuration::World::world_not_found_for_client_e::request_plugin_or_default},
            };
            static std::unordered_map<int, std::string> world_not_found_for_client_to_str = {
                {(int)server_configuration::World::world_not_found_for_client_e::kick, "kick"},
                {(int)server_configuration::World::world_not_found_for_client_e::transfer_to_default, "transfer_to_default"},
                {(int)server_configuration::World::world_not_found_for_client_e::request_plugin_or_default, "request_plugin_or_default"},
            };
            auto world_not_found_for_client = util::js_object::get_object(world["world_not_found_for_client"]);
            cfg.world.world_not_found_for_client = world_not_found_for_client_from_str.at(
                world_not_found_for_client["world_not_found_for_client"].or_apply(
                    world_not_found_for_client_to_str.at((int)cfg.world.world_not_found_for_client)
                )
            );
        }
    }

    void merge_configs_game_play(server_configuration& cfg, util::js_object& data) {
        auto game_play = util::js_object::get_object(data["game_play"]);
        cfg.game_play.difficulty = (std::string)game_play["difficulty"].or_apply(cfg.game_play.difficulty);
        cfg.game_play.gamemode = (std::string)game_play["gamemode"].or_apply(cfg.game_play.gamemode);
        cfg.game_play.max_chained_neighbor_updates = game_play["max_chained_neighbor_updates"].or_apply(cfg.game_play.max_chained_neighbor_updates);
        cfg.game_play.max_tick_time = game_play["max_tick_time"].or_apply(cfg.game_play.max_tick_time);
        cfg.game_play.view_distance = game_play["view_distance"].or_apply(cfg.game_play.view_distance);
        cfg.game_play.simulation_distance = game_play["simulation_distance"].or_apply(cfg.game_play.simulation_distance);
        cfg.game_play.max_word_size = game_play["max_word_size"].or_apply(cfg.game_play.max_word_size);
        cfg.game_play.spawn_protection = game_play["spawn_protection"].or_apply(cfg.game_play.spawn_protection);
        cfg.game_play.player_idle_timeout = game_play["player_idle_timeout"].or_apply(cfg.game_play.player_idle_timeout);
        cfg.game_play.hardcore = game_play["hardcore"].or_apply(cfg.game_play.hardcore);
        cfg.game_play.pvp = game_play["pvp"].or_apply(cfg.game_play.pvp);
        cfg.game_play.allow_flight = game_play["allow_flight"].or_apply(cfg.game_play.allow_flight);
        cfg.game_play.sync_chunk_writes = game_play["sync_chunk_writes"].or_apply(cfg.game_play.sync_chunk_writes);
        cfg.game_play.enable_command_block = game_play["enable_command_block"].or_apply(cfg.game_play.enable_command_block);
        cfg.game_play.reduced_debug_screen = game_play["reduced_debug_screen"].or_apply(cfg.game_play.reduced_debug_screen);
        cfg.game_play.enable_code_of_conduct = game_play["enable_code_of_conduct"].or_apply(cfg.game_play.enable_code_of_conduct);
        if (game_play.contains("enabled_features")) {
            cfg.game_play.enabled_features.clear();
            auto enabled_features = util::js_array::get_array(game_play["enabled_features"]);
            cfg.game_play.enabled_features.reserve(enabled_features.size());
            for (auto&& it : enabled_features)
                cfg.game_play.enabled_features.emplace((std::string)it);
        } else {
            auto& enabled_features = (game_play["enabled_features"] = boost::json::array()).get().get_array();
            enabled_features.reserve(cfg.game_play.enabled_features.size());
            for (auto& it : cfg.game_play.enabled_features)
                enabled_features.push_back((boost::json::string)it);
        }
        cfg.game_play.entity.spawn_animals = game_play["spawn_animals"].or_apply(cfg.game_play.entity.spawn_animals);
        cfg.game_play.entity.spawn_monsters = game_play["spawn_monsters"].or_apply(cfg.game_play.entity.spawn_monsters);
    }

    void merge_configs_protocol(server_configuration& cfg, util::js_object& data) {
        auto protocol = util::js_object::get_object(data["protocol"]);
        cfg.protocol.compression_threshold = protocol["compression_threshold"].or_apply(cfg.protocol.compression_threshold);
        cfg.protocol.rate_limit = protocol["rate_limit"].or_apply(cfg.protocol.rate_limit);
        cfg.protocol.max_unacknowledged_chunk_batches = protocol["max_unacknowledged_chunk_batches"].or_apply(cfg.protocol.max_unacknowledged_chunk_batches);
        cfg.protocol.handle_legacy = protocol["handle_legacy"].or_apply(cfg.protocol.handle_legacy);
        cfg.protocol.new_client_buffer = protocol["new_client_buffer"].or_apply(cfg.protocol.new_client_buffer);
        cfg.protocol.buffer = protocol["buffer"].or_apply(cfg.protocol.buffer);
        cfg.protocol.max_accept_packet_size = protocol["max_accept_packet_size"].or_apply(cfg.protocol.max_accept_packet_size);
        cfg.protocol.max_send_packet_size = protocol["max_send_packet_size"].or_apply(cfg.protocol.max_send_packet_size);
        cfg.protocol.max_send_packet_size = protocol["max_send_packet_size"].or_apply(cfg.protocol.max_send_packet_size);
        cfg.protocol.timeout_seconds = protocol["timeout_seconds"].or_apply(cfg.protocol.timeout_seconds);
        cfg.protocol.keep_alive_send_each_seconds = protocol["keep_alive_send_each_seconds"].or_apply(cfg.protocol.keep_alive_send_each_seconds);
        cfg.protocol.all_connections_timeout_seconds = protocol["all_connections_timeout_seconds"].or_apply(cfg.protocol.all_connections_timeout_seconds);

        cfg.protocol.prevent_proxy_connections = protocol["prevent_proxy_connections"].or_apply(cfg.protocol.prevent_proxy_connections);
        cfg.protocol.enable_encryption = protocol["enable_encryption"].or_apply(cfg.protocol.enable_encryption);
        cfg.protocol.send_nbt_data_in_chunk = protocol["send_nbt_data_in_chunk"].or_apply(cfg.protocol.send_nbt_data_in_chunk);
        cfg.protocol.skip_unregistered_packets = protocol["skip_unregistered_packets"].or_apply(cfg.protocol.skip_unregistered_packets);
        set_from_string(cfg.protocol.connection_conflict, protocol["connection_conflict"].or_apply(to_string(cfg.protocol.connection_conflict)));
    }

    void merge_configs_mojang(server_configuration& cfg, util::js_object& data) {
        auto mojang = util::js_object::get_object(data["mojang"]);
        cfg.mojang.enforce_secure_profile = mojang["enforce_secure_profile"].or_apply(cfg.mojang.enforce_secure_profile);
        cfg.mojang.enable_snoop_stats = mojang["enable_snoop_stats"].or_apply(cfg.mojang.enable_snoop_stats);
        cfg.mojang.prevent_proxy_connections = mojang["prevent_proxy_connections"].or_apply(cfg.mojang.prevent_proxy_connections);
    }

    void merge_configs_status(server_configuration& cfg, util::js_object& data) {
        auto status = util::js_object::get_object(data["status"]);
        cfg.status.server_name = (std::string)status["server_name"].or_apply(cfg.status.server_name);
        cfg.status.description = (std::string)status["description"].or_apply(cfg.status.description);
        cfg.status.favicon_path = (std::string)status["favicon_path"].or_apply(std::string());
        cfg.status.sample_players_count = status["sample_players_count"].or_apply(cfg.status.sample_players_count);
        cfg.status.enable = status["enable"].or_apply(cfg.status.enable);
        cfg.status.show_players = status["show_players"].or_apply(cfg.status.show_players);
    }

    void merge_configs_server(server_configuration& cfg, util::js_object& data) {
        auto server = util::js_object::get_object(data["server"]);
        auto& folder = (boost::json::string&)server["storage_folder"].or_apply(cfg.server.storage_folder);


        if (folder.find_first_of(".,\\#$%^&*()`~'\":;|?!<>") != folder.npos)
            api::log::warn("server", "server config: root.server.storage_folder contains special symbol .,\\#$%^&*()`~'\":;|?!<>, item has been ignored");
        else {
            cfg.server.storage_folder = (std::string)folder;
            std::filesystem::create_directories(cfg.server.get_storage_path());
        }

        auto& worlds = (boost::json::string&)server["worlds_folder"].or_apply(cfg.server.worlds_folder);
        if (worlds.find_first_of(".,\\#$%^&*()`~'\":;|?!<>") != worlds.npos)
            api::log::warn("server", "server config: root.server.worlds_folder contains special symbol .,\\#$%^&*()`~'\":;|?!<>, item has been ignored");
        else {
            cfg.server.worlds_folder = (std::string)worlds;
            std::filesystem::create_directories(cfg.server.get_storage_path());
        }
        cfg.server.ip = (std::string)server["ip"].or_apply(cfg.server.ip);
        cfg.server.port = server["port"].or_apply(cfg.server.port);
        cfg.server.offline_mode = server["offline_mode"].or_apply(cfg.server.offline_mode);
        cfg.server.max_players = server["max_players"].or_apply(cfg.server.max_players);
        cfg.server.world_debug_mode = server["world_debug_mode"].or_apply(cfg.server.world_debug_mode);
        cfg.server.frozen_config = server["frozen_config"].or_apply(cfg.server.frozen_config);
        if (server.contains("enable_debug_task_thread_naming"))
            cfg.server.enable_debug_task_thread_naming = server["enable_debug_task_thread_naming"];
        if (server.contains("working_threads"))
            cfg.server.working_threads = server["working_threads"];
        if (server.contains("ssl_key_length"))
            cfg.server.ssl_key_length = server["ssl_key_length"];


        if (cfg.server.working_threads == 0)
            cfg.server.working_threads = std::thread::hardware_concurrency();
    }

    void merge_configs_allowed_dimensions(server_configuration& cfg, util::js_object& data) {
        auto allowed_dimensions = util::js_array::get_array(data["allowed_dimensions"]);
        if (allowed_dimensions.empty()) {
            for (auto& id : cfg.allowed_dimensions)
                allowed_dimensions.push_back(id);
        } else {
            size_t arr_siz = allowed_dimensions.size();
            for (size_t i = 0; i < arr_siz; i++)
                cfg.allowed_dimensions.emplace((std::string)allowed_dimensions[i]);
        }
    }

    void merge_configs__process__status_favicon_path(server_configuration& cfg) {
        if (!cfg.status.favicon_path.empty()) {
            fast_task::files::async_iofstream file(
                cfg.status.favicon_path,
                fast_task::files::open_mode::read,
                fast_task::files::on_open_action::open,
                fast_task::files::_sync_flags{}
            );
            if (file.is_open()) {
                file.seekg(0, std::istream::end);
                size_t file_size = file.tellg();
                if (file_size < 28) {
                    api::log::error("server", "Failed to read favicon, icon too small, skipping...");
                    return;
                }
                file.seekg(0, std::istream::beg);
                std::vector<uint8_t> res;
                res.resize(file_size);
                file.read((char*)res.data(), res.size());
                uint32_t width = 0, height = 0;
                width = util::convert_endian(std::endian::big, *(uint32_t*)&res[16]);
                height = util::convert_endian(std::endian::big, *(uint32_t*)&res[20]);

                if (width != 64 || height != 64) {
                    api::log::error("server", "Failed to read favicon, icon resolution not equal to 64x64, skipping...");
                    return;
                }
                cfg.status.favicon = std::move(res);
            }
        } else
            cfg.status.favicon.clear();
    }

    void merge_compounds(std::unordered_map<std::string, util::nbt>& left, std::unordered_map<std::string, util::nbt>& right) {
        for (auto& [name, val] : right) {
            auto& it = left[name];
            if (it.is_compound() && val.is_compound())
                merge_compounds(it.get_compound(), val.get_compound());
            else
                it = std::move(val);
        }
    }

    util::nbt& _get_plugins_(server_configuration& cfg) {
        return cfg.plugins;
    }

    void merge_configs_plugins(server_configuration& cfg, util::js_object& data) {
        auto nbt = util::conversions::json::from_json(util::js_object::get_object(data["plugins"]).get());
        merge_compounds(_get_plugins_(cfg).get_compound(), nbt.get_compound());
        data["plugins"] = util::conversions::json::to_json(_get_plugins_(cfg));
    }

    void merge_configs_disabled_plugins(server_configuration& cfg, util::js_object& data, bool load) {
        if (load) {
            auto disabled_plugins = util::js_array::get_array(data["disabled_plugins"].or_apply(boost::json::array{}));
            cfg.disabled_plugins.clear();
            cfg.disabled_plugins.reserve(disabled_plugins.size());
            for (auto&& name : disabled_plugins)
                cfg.disabled_plugins.emplace((std::string)name);
        } else if (data.contains("disabled_plugins")) {
            auto& disabled_plugins = (data["disabled_plugins"]).get().get_array();
            disabled_plugins.clear();
            disabled_plugins.reserve(cfg.disabled_plugins.size());
            for (auto& it : cfg.disabled_plugins)
                disabled_plugins.push_back((boost::json::string)it);
        }
    }

    void merge_configs(server_configuration& cfg, util::js_object& data, bool load = false) {
        merge_configs_world(cfg, data);
        merge_configs_game_play(cfg, data);
        merge_configs_protocol(cfg, data);
        merge_configs_mojang(cfg, data);
        merge_configs_status(cfg, data);
        merge_configs_server(cfg, data);
        merge_configs_allowed_dimensions(cfg, data);
        merge_configs_plugins(cfg, data);
        merge_configs_disabled_plugins(cfg, data, load);

        if (load)
            merge_configs__process__status_favicon_path(cfg);
    }

    void save_config(const std::filesystem::path& config_file_path, boost::json::object& config_data) {
        if (config.server.frozen_config)
            return;
        fast_task::files::atomic_async_ofstream file(config_file_path / "config.json");

        if (!file.is_open()) {
            api::log::warn("server", "Failed to save config file. Can not open file.");
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
                unsigned long index = 0;
                try {
                    index = std::stoul(index_str);
                } catch (const std::invalid_argument&) {
                    decorated_exception("The index is not integer", path, full_path);
                } catch (const std::out_of_range&) {
                    decorated_exception("Index is too big", path, full_path);
                }
                if (value.get_array().size() <= index)
                    decorated_exception("Index out of range", path, full_path);
                return value.get_array()[index];
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

    std::string server_configuration::get(const std::string& config_item_path) {
        boost::json::value config_data = boost::json::object();
        auto js_config = util::js_object::get_object(config_data.get_object());
        merge_configs(*this, js_config);
        return util::pretty_print(get_value_by_path_(config_data, config_item_path));
    }


    base_objects::events::event<void> updated;

    server_configuration::plugin_actions::plugin_actions(util::nbt& it) : it(it) {}

    auto server_configuration::plugin_actions::operator^(std::string_view name) -> plugin_actions {
        return it.get_compound()[std::string(name)];
    }

    auto server_configuration::plugin_actions::operator^(get_value) -> const util::nbt& {
        return it;
    }

    auto server_configuration::plugin_actions::operator^=(const util::nbt& value) -> plugin_actions& {
        it = value;
        updated();
        if (config.server.frozen_config)
            return *this;
        boost::json::value config_data = boost::json::object();
        auto config_js = util::js_object::get_object(config_data.get_object());
        merge_configs(config, config_js);
        save_config(std::filesystem::current_path(), config_data.get_object());
        return *this;
    }

    auto server_configuration::plugin_actions::operator|=(const util::nbt& value) -> plugin_actions& {
        if (it.is_compound())
            if (it.get_compound().empty())
                return operator^=(value);
        return *this;
    }

    server_configuration::plugin_actions::operator const util::nbt&() const {
        return it;
    }

    auto server_configuration::operator^(std::string_view name) -> plugin_actions {
        return plugins.get_compound()[std::string(name)];
    }

    void load(bool fill_default_values) {
        if (config.server.frozen_config)
            return;
        {
            auto config_file_path = std::filesystem::current_path();
            auto config_data = util::try_read_json_file(config_file_path / "config.json");
            if (!config_data.has_value() && !fill_default_values) {
                api::log::warn("server", "Failed to read config file. Using default values.");
                return;
            } else if (!config_data.has_value())
                config_data = boost::json::object();
            auto config_js = util::js_object::get_object(*config_data);
            //if (fill_default_values) {

            try {
                merge_configs(config, config_js, true);
                save_config(config_file_path, *config_data);
            } catch (const std::filesystem::filesystem_error& ex) {
                api::log::error("server", ex.what());
                throw;
            } catch (const std::exception& ex) {
                api::log::error("server", ex.what());
                throw;
            }
            //}
        }
        loaded = true;
        updated();
    }

    server_configuration& get() {
        if (!loaded)
            load(true);
        return config;
    }

    void set_item(const std::string& config_item_path, const std::string& value) {
        boost::json::value config_data = boost::json::object();
        auto config_js = util::js_object::get_object(config_data.get_object());
        merge_configs(config, config_js);
        auto& val = get_value_by_path_(config_data, config_item_path);
        boost::system::error_code ec;
        val = boost::json::parse(value, ec);
        if (ec)
            throw std::runtime_error("Failed to parse value, strings must be in \" scope and constants must be in lowercase");
        merge_configs(config, config_js, true);
        save_config(std::filesystem::current_path(), config_data.get_object());
        updated();
    }

    std::string get_item(const std::string& config_item_path) {
        return get().get(config_item_path);
    }

    void apply_preset(const std::string& preset) {
    }
}