#include "world_data.hpp"
#include "../log.hpp"
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace crafted_craft {
    namespace storage {

        template <class T>
        T convert_chunk_global_pos(T pos) {
            if (pos == 0)
                return 0;
            if (pos < 0)
                return (pos + 1) / 16 - 1;
            return pos / 16;
        }

        template <class T>
        T convert_chunk_local_pos(T pos) {
            if (pos == 0)
                return 0;
            if (pos < 0)
                return 16 + (pos % 16);
            return pos % 16;
        }


        class world_data;
        class worlds_data;

        enbt::value& sub_chunk_data::get_block_entity_data(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
            return block_entities[local_z | (local_y << 4) | (local_x << 8)];
        }

        void sub_chunk_data::get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, std::function<void(base_objects::block& block)> on_normal, std::function<void(base_objects::block& block, enbt::value& entity_data)> on_entity) {
            auto& block = blocks[local_x][local_y][local_z];
            if (block.is_block_entity())
                on_entity(block, get_block_entity_data(local_x, local_y, local_z));
            else
                on_normal(block);
        }

        void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::full_block_data& block) {
            std::visit(
                [&](auto& block) {
                    using T = std::decay_t<decltype(block)>;
                    if constexpr (std::is_same_v<T, base_objects::block>) {
                        blocks[local_x][local_y][local_z] = block;
                        block_entities.erase(local_z | (local_y << 4) | (local_x << 8));
                    } else {
                        blocks[local_x][local_y][local_z] = block.block;
                        get_block_entity_data(local_x, local_y, local_z) = block.data;
                    }
                },
                block
            );
        }

        void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::full_block_data&& block) {
            std::visit(
                [&](auto& block) {
                    using T = std::decay_t<decltype(block)>;
                    if constexpr (std::is_same_v<T, base_objects::block>) {
                        blocks[local_x][local_y][local_z] = block;
                        block_entities.erase(local_z | (local_y << 4) | (local_x << 8));
                    } else {
                        blocks[local_x][local_y][local_z] = block.block;
                        get_block_entity_data(local_x, local_y, local_z) = std::move(block.data);
                    }
                },
                block
            );
        }

        uint32_t sub_chunk_data::get_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
            return biomes[2 >> local_x][2 >> local_y][2 >> local_z];
        }

        void sub_chunk_data::set_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z, uint32_t id) {
            biomes[2 >> local_x][2 >> local_y][2 >> local_z] = id;
        }

        void sub_chunk_data::for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block)> func) {
            for (uint8_t x = 0; x < 16; x++)
                for (uint8_t y = 0; y < 16; y++)
                    for (uint8_t z = 0; z < 16; z++)
                        func(x, y, z, blocks[x][y][z]);
        }

        bool chunk_data::load(const std::filesystem::path& chunk_z) {
            if (!std::filesystem::exists(chunk_z))
                return false;
            std::ifstream file(chunk_z, std::ios::binary);
            if (std::filesystem::file_size(chunk_z) == 0)
                return false;
            if (!file.is_open())
                return false;
            boost::iostreams::filtering_istream filter;
            filter.push(boost::iostreams::zstd_decompressor());
            filter.push(file);

            enbt::value chunk_data_file = enbt::io_helper::read_token(filter);
            file.close();
            return load(chunk_data_file.as_compound());
        }

        bool valid_sub_chunk_size(const enbt::value& chunk) {
            try {
                auto dim_0 = chunk.as_fixed_array();
                if (dim_0.size() != 16)
                    return false;
                for (auto& x : dim_0) {
                    auto dim_1 = x.as_fixed_array();
                    if (dim_1.size() != 16)
                        return false;
                    for (auto& y : dim_1) {
                        if (y.size() != 16)
                            return false;
                    }
                }
            } catch (...) {
                return false;
            }

            return true;
        }

        void load_light_data(const enbt::value& chunk, light_data& data, bool& need_to_recalculate_light) {
            if (!valid_sub_chunk_size(chunk)) {
                need_to_recalculate_light = true;
                return;
            }

            size_t x_ = 0;
            for (auto& x : chunk.as_fixed_array()) {
                size_t y_ = 0;
                for (auto& y : x.as_fixed_array()) {
                    size_t z_ = 0;
                    for (auto& z : y.as_ui8_array())
                        data.light_map[x_][y_][z_++].raw = z;
                    ++y_;
                }
                ++x_;
            }
        }

        void load_block_data(const enbt::value& chunk, base_objects::block (&data)[16][16][16], bool& has_tickable_blocks) {
            size_t x_ = 0;
            for (auto& x : chunk.as_fixed_array()) {
                size_t y_ = 0;
                for (auto& y : x.as_fixed_array()) {
                    size_t z_ = 0;
                    for (auto z : y.as_ui32_array()) {
                        data[x_][y_][z_].raw = z;
                        has_tickable_blocks = data[x_][y_][z_].is_tickable();
                        ++z_;
                    }
                    ++y_;
                }
                ++x_;
            }
        }

        bool chunk_data::load(const enbt::compound_const_ref& chunk_data) {
            if (!chunk_data.contains("sub_chunks"))
                return false;
            auto sub_chunks_ref = chunk_data["sub_chunks"].as_fixed_array();
            sub_chunks.reserve(sub_chunks_ref.size());
            for (auto& sub_chunk : sub_chunks_ref) {
                std::shared_ptr<storage::sub_chunk_data> sub_chunk_data = std::make_shared<storage::sub_chunk_data>();
                if (sub_chunk.contains("blocks")) {
                    auto& blocks = sub_chunk["blocks"];
                    if (!valid_sub_chunk_size(blocks))
                        return false;
                    load_block_data(blocks, sub_chunk_data->blocks, sub_chunk_data->has_tickable_blocks);
                }
                if (sub_chunk.contains("entities")) {
                    for (auto& entity : sub_chunk["entities"].as_fixed_array()) {
                        sub_chunk_data->stored_entities.push_back(base_objects::entity::load_from_enbt(entity.as_compound()));
                    }
                }
                if (sub_chunk.contains("block_entities")) {
                    for (auto& block_entity : sub_chunk["block_entities"].as_fixed_array()) {
                        base_objects::local_block_pos local_pos;
                        const enbt::value& nbt = block_entity["data"];
                        local_pos.x = block_entity["x"];
                        local_pos.y = block_entity["y"];
                        local_pos.z = block_entity["z"];
                        sub_chunk_data->blocks[local_pos.x][local_pos.y][local_pos.z] = base_objects::block{
                            (base_objects::block_id_t)block_entity["id"]
                        };
                        sub_chunk_data->block_entities[local_pos.z | (local_pos.y << 4) | (local_pos.x << 8)] = nbt;
                    }
                }
                if (sub_chunk.contains("block_light")) {
                    load_light_data(sub_chunk["block_light"], sub_chunk_data->block_light, sub_chunk_data->need_to_recalculate_light);
                } else
                    sub_chunk_data->need_to_recalculate_light = true;

                if (sub_chunk.contains("queried_for_tick"))
                    for (auto& query : sub_chunk["queried_for_tick"].as_fixed_array())
                        sub_chunk_data->queried_for_tick.push_back({query["x"], query["y"], query["z"]});
                sub_chunks.push_back(std::move(*sub_chunk_data));
            }
            return true;
        }

        bool chunk_data::save(const std::filesystem::path& chunk_z) {
            std::filesystem::create_directories(chunk_z.parent_path());
            std::ofstream file(chunk_z, std::ios::binary);
            if (!file.is_open())
                return false;
            enbt::compound chunk_data_file;
            enbt::fixed_array enbt_sub_chunks;
            for (auto& sub_chunk : sub_chunks) {
                enbt::compound sub_chunk_data;
                {
                    enbt::fixed_array x_axis(16);
                    for (size_t x = 0; x < 16; x++) {
                        enbt::fixed_array y_axis(16);
                        for (size_t y = 0; y < 16; y++) {
                            enbt::simple_array_ui32 z_axis(16);
                            for (size_t z = 0; z < 16; z++)
                                z_axis[z] = sub_chunk.blocks[x][y][z].raw;
                            y_axis.set(y, std::move(z_axis));
                        }
                        x_axis.set(x, std::move(y_axis));
                    }
                    sub_chunk_data["blocks"] = std::move(x_axis);
                }
                {
                    enbt::fixed_array entities;
                    for (auto& entity : sub_chunk.stored_entities)
                        entities.push_back(entity->copy_to_enbt());
                    sub_chunk_data["entities"] = std::move(entities);
                }
                {
                    enbt::fixed_array block_entities(sub_chunk.block_entities.size());
                    for (auto& [_pos, data] : sub_chunk.block_entities) {
                        base_objects::local_block_pos pos;
                        pos.x = _pos >> 8;
                        pos.y = (_pos >> 4) & 0xF;
                        pos.z = _pos & 0xF;

                        enbt::compound be;
                        be["id"] = sub_chunk.blocks[pos.x][pos.y][pos.z].id;
                        be["state"] = sub_chunk.blocks[pos.x][pos.y][pos.z].block_state_data;
                        be["nbt"] = data;
                        be["x"] = pos.x;
                        be["y"] = pos.y;
                        be["z"] = pos.z;
                        block_entities.push_back(std::move(be));
                    }
                    sub_chunk_data["block_entities"] = std::move(block_entities);
                }
                if (!sub_chunk.need_to_recalculate_light) {
                    {
                        enbt::fixed_array x_axis(16);
                        for (size_t x = 0; x < 16; x++) {
                            enbt::fixed_array y_axis(16);
                            for (size_t y = 0; y < 16; y++) {
                                enbt::simple_array_ui8 z_axis(16);
                                for (size_t z = 0; z < 16; z++)
                                    z_axis[z] = sub_chunk.block_light.light_map[x][y][z].raw;
                                y_axis.set(y, std::move(z_axis));
                            }
                            x_axis.set(x, std::move(y_axis));
                        }
                        sub_chunk_data["block_light"] = std::move(x_axis);
                    }
                    {
                        enbt::fixed_array x_axis(16);
                        for (size_t x = 0; x < 16; x++) {
                            enbt::fixed_array y_axis(16);
                            for (size_t y = 0; y < 16; y++) {
                                enbt::simple_array_ui8 z_axis(16);
                                for (size_t z = 0; z < 16; z++)
                                    z_axis[z] = sub_chunk.sky_light.light_map[x][y][z].raw;
                                y_axis.set(y, std::move(z_axis));
                            }
                            x_axis.set(x, std::move(y_axis));
                        }
                        sub_chunk_data["sky_light"] = std::move(x_axis);
                    }
                }
                if (!sub_chunk.queried_for_tick.empty()) {
                    enbt::fixed_array queried_for_tick;
                    for (auto& query : sub_chunk.queried_for_tick) {
                        enbt::compound q;
                        q["x"] = query.x;
                        q["y"] = query.y;
                        q["z"] = query.z;
                        queried_for_tick.push_back(std::move(q));
                    }
                    sub_chunk_data["queried_for_tick"] = std::move(queried_for_tick);
                }
                enbt_sub_chunks.push_back(std::move(sub_chunk_data));
            }
            chunk_data_file["sub_chunks"] = std::move(enbt_sub_chunks);
            boost::iostreams::filtering_ostream filter;
            filter.push(boost::iostreams::zstd_compressor());
            filter.push(file);
            enbt::io_helper::write_token(filter, (const enbt::value&)chunk_data_file);
            return true;
        }

        chunk_data::chunk_data(int64_t chunk_x, int64_t chunk_z)
            : chunk_x(chunk_x), chunk_z(chunk_z) {}

        void chunk_data::for_each_entity(std::function<void(base_objects::entity_ref& entity)> func) {
            for (auto& sub_chunk : sub_chunks)
                for (auto& entity : sub_chunk.stored_entities)
                    func(entity);
        }

        void chunk_data::for_each_block_entity(std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
            for (auto& sub_chunk : sub_chunks)
                for (auto& [_pos, data] : sub_chunk.block_entities) {
                    base_objects::local_block_pos pos;
                    pos.x = _pos >> 8;
                    pos.y = (_pos >> 4) & 0xF;
                    pos.z = _pos & 0xF;
                    func(sub_chunk.blocks[pos.x][pos.y][pos.z], data);
                }
        }

        void chunk_data::for_each_block_entity(uint64_t sub_chunk_y, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
            if (sub_chunk_y < sub_chunks.size())
                for (auto& [_pos, data] : sub_chunks[sub_chunk_y].block_entities) {
                    base_objects::local_block_pos pos;
                    pos.x = _pos >> 8;
                    pos.y = (_pos >> 4) & 0xF;
                    pos.z = _pos & 0xF;
                    func(sub_chunks[sub_chunk_y].blocks[pos.x][pos.y][pos.z], data);
                }
        }

        void chunk_data::for_each_entity(uint64_t sub_chunk_y, std::function<void(base_objects::entity_ref& entity)> func) {
            if (sub_chunk_y < sub_chunks.size())
                for (auto& entity : sub_chunks[sub_chunk_y].stored_entities)
                    func(entity);
        }

        void chunk_data::for_each_sub_chunk(std::function<void(sub_chunk_data& sub_chunk)> func) {
            for (auto& sub_chunk : sub_chunks)
                func(sub_chunk);
        }

        void chunk_data::get_sub_chunk(uint64_t sub_chunk_y, std::function<void(sub_chunk_data& sub_chunk)> func) {
            if (sub_chunk_y < sub_chunks.size())
                func(sub_chunks[sub_chunk_y]);
        }

        void chunk_data::query_for_tick(uint8_t local_x, uint64_t global_y, uint8_t local_z) {
            uint64_t sub_chunk = global_y >> 4;
            if (sub_chunk < sub_chunks.size())
                sub_chunks[sub_chunk].queried_for_tick.push_back({local_x, uint8_t(global_y & 15), local_z});
        }

        void chunk_data::tick(world_data& world, size_t max_random_tick_for_chunk, std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time) {
            last_usage = current_time;


            uint64_t sub_chunk_y = 0;
            for (auto& sub_chunk : sub_chunks) {
                for (auto& entity : sub_chunk.stored_entities)
                    entity->tick();

                for (auto& query : sub_chunk.queried_for_tick)
                    sub_chunk.blocks[query.x][query.y][query.z].tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, query.x, query.y, query.z, false);
                sub_chunk.queried_for_tick.clear();


                if (sub_chunk.has_tickable_blocks && max_random_tick_for_chunk) {
                    for (size_t x = 0; x < 16; x++)
                        for (size_t y = 0; y < 16; y++)
                            for (size_t z = 0; z < 16; z++)
                                if (sub_chunk.blocks[x][y][z].tickable != base_objects::block::tick_opt::no_tick)
                                    if (max_random_tick_for_chunk > 0) {
                                        if (random_engine() & 1) {
                                            sub_chunk.blocks[x][y][z].tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, x, y, z, true);
                                            --max_random_tick_for_chunk;
                                        }
                                    }
                }
                sub_chunk_y++;
            }
        }

        fast_task::protected_value<std::unordered_map<std::string, base_objects::atomic_holder<chunk_generator>>> chunk_generators;

        void chunk_generator::register_it(const std::string& id, base_objects::atomic_holder<chunk_generator> gen) {
            chunk_generators.set([&](auto& map) {
                map[id] = std::move(gen);
            });
        }

        void chunk_generator::unregister_it(const std::string& id) {
            chunk_generators.set([&](auto& map) {
                map.erase(id);
            });
        }

        base_objects::atomic_holder<chunk_generator> chunk_generator::get_it(const std::string& id) {
            return chunk_generators.set([&](auto& map) {
                return map.at(id);
            });
        }

        fast_task::protected_value<std::unordered_map<std::string, base_objects::atomic_holder<chunk_light_processor>>> light_orocessors;

        void chunk_light_processor::register_it(const std::string& id, base_objects::atomic_holder<chunk_light_processor> processor) {
            light_orocessors.set([&](auto& map) {
                map[id] = std::move(processor);
            });
        }

        void chunk_light_processor::unregister_it(const std::string& id) {
            light_orocessors.set([&](auto& map) {
                map.erase(id);
            });
        }

        base_objects::atomic_holder<chunk_light_processor> chunk_light_processor::get_it(const std::string& id) {
            return light_orocessors.set([&](auto& map) {
                return map.at(id);
            });
        }

        void world_data::make_save(int64_t chunk_x, int64_t chunk_z, bool also_unload) {
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                    make_save(chunk_x, chunk_z, y_axis, also_unload);
        }

        void world_data::make_save(int64_t chunk_x, int64_t chunk_z, chunk_row::iterator item, bool also_unload) {
            if (auto process = on_save_process.find({chunk_x, chunk_z}); process == on_save_process.end()) {
                auto& chunk = item->second;
                on_save_process[{chunk_x, chunk_z}] = Future<bool>::start(
                    [this, chunk, chunk_x, chunk_z, also_unload] {
                        try {
                            chunk->save(path / "chunks" / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"));
                        } catch (...) {
                            return false;
                        }
                        std::unique_lock lock(mutex);
                        on_save_process.erase({chunk_x, chunk_z});

                        if (also_unload)
                            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                                x_axis->second.erase(chunk_z);
                        return true;
                    }
                );
            }
        }

        base_objects::atomic_holder<chunk_data> world_data::load_chunk_sync(int64_t chunk_x, int64_t chunk_z) {
            auto chunk = base_objects::atomic_holder<chunk_data>(new chunk_data(chunk_x, chunk_z));
            if (!chunk->load(path / "chunks" / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"))) {
                if (!chunk->load(get_generator()->generate_chunk(*this, chunk_x, chunk_z)))
                    return nullptr;
            }
            get_light_processor();
            uint64_t y = 0;
            chunk->for_each_sub_chunk([&](sub_chunk_data& sub_chunk) {
                if (sub_chunk.need_to_recalculate_light)
                    light_processor->process_sub_chunk(*this, chunk_x, y, chunk_z);
                ++y;
            });
            return chunk;
        }

        base_objects::atomic_holder<chunk_generator>& world_data::get_generator() {
            if (!generator)
                generator = chunk_generator::get_it(light_processor_id);
            return generator;
        }

        base_objects::atomic_holder<chunk_light_processor>& world_data::get_light_processor() {
            if (!light_processor)
                light_processor = chunk_light_processor::get_it(light_processor_id);
            return light_processor;
        }

        template <class T>
        enbt::fixed_array to_fixed_array(const std::vector<T>& vec) {
            enbt::fixed_array _enabled_datapacks(vec.size());
            size_t i = 0;
            for (auto& item : vec)
                _enabled_datapacks.set(i++, item);
            return _enabled_datapacks;
        }

        template <class T>
        std::vector<T> from_fixed_array(const enbt::value& abstract) {
            auto arr = abstract.as_fixed_array();
            std::vector<T> res;
            res.reserve(arr.size());
            for (auto& item : arr)
                res.push_back((T)item);
            return res;
        }

        template <class T, class _FN>
        enbt::fixed_array to_fixed_array(const std::vector<T>& vec, _FN&& cast_fn) {
            enbt::fixed_array _enabled_datapacks(vec.size());
            size_t i = 0;
            for (auto& item : vec)
                _enabled_datapacks.set(i++, cast_fn(item));
            return _enabled_datapacks;
        }

        template <class T, class _FN>
        std::vector<T> from_fixed_array(const enbt::value& abstract, _FN&& cast_fn) {
            auto arr = abstract.as_fixed_array();
            std::vector<T> res;
            res.reserve(arr.size());
            for (auto& item : arr)
                res.push_back(cast_fn(item));
            return res;
        }

        void world_data::load(const enbt::compound_const_ref& load_from_nbt) {
            general_world_data = load_from_nbt["general_world_data"];
            world_game_rules = load_from_nbt["world_game_rules"];
            world_generator_data = load_from_nbt["world_generator_data"];
            world_records = load_from_nbt["world_records"];
            world_seed = load_from_nbt["world_seed"];
            wandering_trader_id = load_from_nbt["wandering_trader_id"];
            wandering_trader_spawn_chance = load_from_nbt["wandering_trader_spawn_chance"];
            wandering_trader_spawn_delay = load_from_nbt["wandering_trader_spawn_delay"];
            world_name = (std::string)(std::string)load_from_nbt["world_name"];
            light_processor_id = (std::string)(std::string)load_from_nbt["light_processor_id"];
            world_type = (std::string)(std::string)load_from_nbt["world_type"];
            generator_id = (std::string)(std::string)load_from_nbt["generator_id"];
            enabled_datapacks = from_fixed_array<std::string>(load_from_nbt["enabled_datapacks"]);
            enabled_plugins = from_fixed_array<std::string>(load_from_nbt["enabled_plugins"]);
            enabled_features = from_fixed_array<std::string>(load_from_nbt["enabled_features"]);
            load_points = from_fixed_array<base_objects::cubic_bounds_chunk>(load_from_nbt["load_points"], [](const enbt::value& enbt) {
                return base_objects::cubic_bounds_chunk(enbt["x1"], enbt["z1"], enbt["x2"], enbt["z2"]);
            });
            load_points_sphere = from_fixed_array<base_objects::spherical_bounds_chunk>(load_from_nbt["load_points_sphere"], [](const enbt::value& enbt) {
                return base_objects::spherical_bounds_chunk(enbt["x"], enbt["z"], enbt["radius"]);
            });

            border_center_x = load_from_nbt["border_center_x"];
            border_center_z = load_from_nbt["border_center_z"];
            border_size = load_from_nbt["border_size"];
            border_safe_zone = load_from_nbt["border_safe_zone"];
            border_damage_per_block = load_from_nbt["border_damage_per_block"];
            border_lerp_target = load_from_nbt["border_lerp_target"];
            border_lerp_time = load_from_nbt["border_lerp_time"];
            border_warning_blocks = load_from_nbt["border_warning_blocks"];
            border_warning_time = load_from_nbt["border_warning_time"];
            day_time = load_from_nbt["day_time"];
            time = load_from_nbt["time"];
            max_random_tick_for_chunk = load_from_nbt["max_random_tick_for_chunk"];
            ticks_per_second = (uint64_t)load_from_nbt["ticks_per_second"];
            chunk_lifetime = std::chrono::milliseconds((long long)load_from_nbt["chunk_lifetime"]);
            world_lifetime = std::chrono::milliseconds((long long)load_from_nbt["world_lifetime"]);
            clear_weather_time = load_from_nbt["clear_weather_time"];
            internal_version = load_from_nbt["internal_version"];
            chunk_y_count = load_from_nbt["chunk_y_count"];
            difficulty = load_from_nbt["difficulty"];
            default_gamemode = load_from_nbt["default_gamemode"];
            difficulty_locked = load_from_nbt["difficulty_locked"];
            is_hardcore = load_from_nbt["is_hardcore"];
            initialized = load_from_nbt["initialized"];
            raining = load_from_nbt["raining"];
            thundering = load_from_nbt["thundering"];
            has_skylight = load_from_nbt["has_skylight"];
            enable_entity_updates_when_hold_light_source = load_from_nbt["enable_entity_updates_when_hold_light_source"];
            enable_entity_light_source_updates = load_from_nbt["enable_entity_light_source_updates"];
        }

        void world_data::load() {
            std::unique_lock lock(mutex);
            std::ifstream file(path / "world.senbt", std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("Can't open world file");
            std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            load(senbt::parse(res).as_compound());
        }

        void world_data::save() {
            std::unique_lock lock(mutex);
            std::filesystem::create_directories(path);
            std::ofstream file(path / "world.senbt", std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("Can't open world file");
            enbt::compound world_data_file;
            world_data_file["general_world_data"] = general_world_data;
            world_data_file["world_game_rules"] = world_game_rules;
            world_data_file["world_generator_data"] = world_generator_data;
            world_data_file["world_records"] = world_records;
            world_data_file["world_seed"] = world_seed;
            world_data_file["wandering_trader_id"] = wandering_trader_id;
            world_data_file["wandering_trader_spawn_chance"] = wandering_trader_spawn_chance;
            world_data_file["wandering_trader_spawn_delay"] = wandering_trader_spawn_delay;
            world_data_file["world_name"] = world_name;
            world_data_file["world_type"] = world_type;
            world_data_file["light_processor_id"] = light_processor_id;
            world_data_file["generator_id"] = generator_id;
            world_data_file["enabled_datapacks"] = to_fixed_array(enabled_datapacks);
            world_data_file["enabled_plugins"] = to_fixed_array(enabled_plugins);
            world_data_file["enabled_features"] = to_fixed_array(enabled_features);
            world_data_file["load_points"] = to_fixed_array(load_points, [](const base_objects::cubic_bounds_chunk& chunk) {
                enbt::compound res;
                res["x1"] = chunk.x1;
                res["z1"] = chunk.z1;
                res["x2"] = chunk.x2;
                res["z2"] = chunk.z2;
                return std::move(res);
            });
            world_data_file["load_points_sphere"] = to_fixed_array(load_points_sphere, [](const base_objects::spherical_bounds_chunk& chunk) {
                enbt::compound res;
                res["x"] = chunk.x;
                res["z"] = chunk.z;
                res["radius"] = chunk.radius;
                return res;
            });
            world_data_file["border_center_x"] = border_center_x;
            world_data_file["border_center_z"] = border_center_z;
            world_data_file["border_size"] = border_size;
            world_data_file["border_safe_zone"] = border_safe_zone;
            world_data_file["border_damage_per_block"] = border_damage_per_block;
            world_data_file["border_lerp_target"] = border_lerp_target;
            world_data_file["border_lerp_time"] = border_lerp_time;
            world_data_file["border_warning_blocks"] = border_warning_blocks;
            world_data_file["border_warning_time"] = border_warning_time;
            world_data_file["day_time"] = day_time;
            world_data_file["time"] = time;
            world_data_file["max_random_tick_for_chunk"] = max_random_tick_for_chunk;
            world_data_file["ticks_per_second"] = ticks_per_second;
            world_data_file["chunk_lifetime"] = chunk_lifetime.count();
            world_data_file["world_lifetime"] = world_lifetime.count();
            world_data_file["clear_weather_time"] = clear_weather_time;
            world_data_file["internal_version"] = internal_version;
            world_data_file["chunk_y_count"] = chunk_y_count;
            world_data_file["difficulty"] = difficulty;
            world_data_file["default_gamemode"] = default_gamemode;
            world_data_file["difficulty_locked"] = difficulty_locked;
            world_data_file["is_hardcore"] = is_hardcore;
            world_data_file["initialized"] = initialized;
            world_data_file["raining"] = raining;
            world_data_file["thundering"] = thundering;
            world_data_file["has_skylight"] = has_skylight;

            if (enable_entity_updates_when_hold_light_source)
                world_data_file["enable_entity_updates_when_hold_light_source"] = true;
            if (enable_entity_light_source_updates)
                world_data_file["enable_entity_light_source_updates"] = true;

            auto stringized = senbt::serialize(world_data_file, false, true);
            file.write(stringized.data(), stringized.size());
        }

        std::string world_data::preview_world_name() {
            std::unique_lock lock(mutex);
            std::ifstream file(path / "world.senbt", std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("Can't open world file");
            std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            return (std::string)(std::string)senbt::parse(res)["world_name"];
        }

        world_data::world_data(const std::filesystem::path& path)
            : path(path) {
            if (!std::filesystem::exists(path))
                std::filesystem::create_directories(path);
        }

        bool world_data::exists(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                    return true;
            return std::filesystem::exists(path / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"));
        }

        std::optional<base_objects::atomic_holder<chunk_data>> world_data::request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                    return y_axis->second;

            if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
                return chunks[chunk_x][chunk_z] = load_chunk_sync(chunk_x, chunk_z);
            else {
                auto res = process->second;
                lock.unlock();
                res->wait();
                lock.lock();
                return request_chunk_data_sync(chunk_x, chunk_z);
            }
        }

        FuturePtr<base_objects::atomic_holder<chunk_data>> world_data::request_chunk_data(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                    return make_ready_future(y_axis->second);

            if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
                return on_load_process[{chunk_x, chunk_z}] = Future<base_objects::atomic_holder<chunk_data>>::start(
                           [this, chunk_x, chunk_z]() -> base_objects::atomic_holder<chunk_data> {
                               auto chunk = load_chunk_sync(chunk_x, chunk_z);
                               if (!chunk)
                                   return nullptr;
                               std::unique_lock lock(mutex);
                               on_load_process.erase({chunk_x, chunk_z});
                               return chunks[chunk_x][chunk_z] = chunk;
                           }
                       );
            else
                return process->second;
        }

        bool world_data::request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end()) {
                    callback(*y_axis->second);
                    return true;
                }

            if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
                auto res = load_chunk_sync(chunk_x, chunk_z);
                if (res)
                    callback(*(chunks[chunk_x][chunk_z] = res));
                else
                    return false;
                return true;
            } else {
                auto res = process->second;
                lock.unlock();
                res->wait();
                lock.lock();
                return request_chunk_data_sync(chunk_x, chunk_z, callback);
            }
        }

        void world_data::request_chunk_data(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback, std::function<void()> fault) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end()) {
                    callback(*y_axis->second);
                    return;
                }

            if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
                on_load_process[{chunk_x, chunk_z}] = Future<base_objects::atomic_holder<chunk_data>>::start(
                    [this, chunk_x, chunk_z, callback, fault]() -> base_objects::atomic_holder<chunk_data> {
                        auto chunk = load_chunk_sync(chunk_x, chunk_z);
                        std::unique_lock lock(mutex);
                        on_load_process.erase({chunk_x, chunk_z});
                        chunks[chunk_x][chunk_z] = chunk;
                        if (chunk)
                            callback(*chunk);
                        else
                            fault();
                        return chunk;
                    }
                );
            else
                return process->second->when_ready([this, chunk_x, chunk_z, callback, fault](base_objects::atomic_holder<chunk_data> chunk) {
                    if (!request_chunk_data_sync(chunk_x, chunk_z, callback))
                        fault();
                });
        }

        void world_data::await_save_chunks() {
            std::unique_lock lock(mutex);
            list_array<FuturePtr<bool>> to_await;
            for (auto& [location, future] : on_save_process)
                to_await.push_back(future);
            lock.unlock();
            to_await.for_each([](FuturePtr<bool>& i) { i->wait(); });
        }

        void world_data::save_chunks() {
            std::unique_lock lock(mutex);
            for (auto& [x, x_axis] : chunks)
                for (auto& [z, chunk] : x_axis)
                    make_save(x, z, false);
        }

        void world_data::save_and_unload_chunks() {
            std::unique_lock lock(mutex);
            save_chunks();
            chunks.clear();
        }

        void world_data::save_and_unload_chunk(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            make_save(chunk_x, chunk_z, true);
        }

        void world_data::unload_chunk(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto z_axis = x_axis->second.find(chunk_z); z_axis != x_axis->second.end())
                    x_axis->second.erase(z_axis);
        }

        void world_data::save_chunk(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            make_save(chunk_x, chunk_z, false);
        }

        void world_data::erase_chunk(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto z_axis = x_axis->second.find(chunk_z); z_axis != x_axis->second.end())
                    x_axis->second.erase(z_axis);
            std::filesystem::remove(path / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"));
        }

        void world_data::regenerate_chunk(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto z_axis = x_axis->second.find(chunk_z); z_axis != x_axis->second.end())
                    x_axis->second.erase(z_axis);
            std::filesystem::remove(path / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"));
            if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
                on_load_process[{chunk_x, chunk_z}] = Future<base_objects::atomic_holder<chunk_data>>::start(
                    [this, chunk_x, chunk_z]() -> base_objects::atomic_holder<chunk_data> {
                        auto chunk = load_chunk_sync(chunk_x, chunk_z);
                        std::unique_lock lock(mutex);
                        on_load_process.erase({chunk_x, chunk_z});
                        return chunks[chunk_x][chunk_z] = chunk;
                    }
                );
        }

        void world_data::reset_light_data(int64_t chunk_x, uint64_t chunk_z) {
            std::unique_lock lock(mutex);
            get_light_processor()->process_chunk(*this, chunk_x, chunk_z);
        }

        void world_data::for_each_chunk(std::function<void(chunk_data& chunk)> func) {
            std::unique_lock lock(mutex);
            for (auto& [x, x_axis] : chunks)
                for (auto& [z, chunk] : x_axis)
                    func(*chunk);
        }

        void world_data::for_each_chunk(base_objects::cubic_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func) {
            std::unique_lock lock(mutex);
            for (int64_t x = bounds.x1; x <= bounds.x2; x++)
                for (int64_t z = bounds.z1; z <= bounds.z2; z++)
                    if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                        if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                            func(*chunk->second);
        }

        void world_data::for_each_chunk(base_objects::spherical_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func) {
            std::unique_lock lock(mutex);
            bounds.enum_points([&](int64_t x, int64_t z) {
                if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                    if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                        func(*chunk->second);
            });
        }

        void world_data::for_each_sub_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(sub_chunk_data& chunk)> func) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                    chunk->second->for_each_sub_chunk(func);
        }

        void world_data::get_sub_chunk(int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, std::function<void(sub_chunk_data& chunk)> func) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                    chunk->second->get_sub_chunk(sub_chunk_y, func);
        }

        void world_data::get_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> func) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                    func(*chunk->second);
        }

        void world_data::for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            for (auto& [x, x_axis] : chunks)
                for (auto& [z, chunk] : x_axis)
                    chunk->for_each_entity(func);
        }

        void world_data::for_each_entity(base_objects::cubic_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            bounds.enum_points([&](int64_t x, int64_t z) {
                if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                    if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                        chunk->second->for_each_entity(func);
            });
        }

        void world_data::for_each_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            bounds.enum_points([&](int64_t x, int64_t z) {
                if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                    if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                        chunk->second->for_each_entity(func);
            });
        }

        void world_data::for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end()) {
                    chunk->second->for_each_entity(func);
                }
        }

        void world_data::for_each_entity(int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                    chunk->second->for_each_entity(sub_chunk_y, func);
        }

        void world_data::for_each_block_entity(base_objects::cubic_bounds_chunk bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
            std::unique_lock lock(mutex);
            bounds.enum_points([&](int64_t x, int64_t z) {
                if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                    if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                        chunk->second->for_each_block_entity(func);
            }
            );
        }

        void world_data::for_each_block_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
            std::unique_lock lock(mutex);
            bounds.enum_points([&](int64_t x, int64_t z) {
                if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                    if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                        chunk->second->for_each_block_entity(func);
            });
        }

        void world_data::for_each_block_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
            std::unique_lock lock(mutex);
            get_chunk(chunk_x, chunk_z, [&](auto& chunk) { chunk.for_each_block_entity(func); });
        }

        void world_data::for_each_block_entity(int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
            std::unique_lock lock(mutex);
            get_chunk(chunk_x, chunk_z, [&](auto& chunk) { chunk.for_each_block_entity(sub_chunk_y, func); });
        }

        void world_data::query_for_tick(int64_t global_x, uint64_t global_y, int64_t global_z) {
            std::unique_lock lock(mutex);
            auto chunk_x = global_x >> 4;
            auto chunk_z = global_z >> 4;
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                    chunk->second->query_for_tick(global_x & 15, global_y, global_z & 15);
        }

        void world_data::set_block(const base_objects::full_block_data& block, int64_t global_x, uint64_t global_y, int64_t global_z, block_set_mode mode) {
            get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
                sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, block);
                get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            });
        }

        void world_data::set_block(base_objects::full_block_data&& block, int64_t global_x, uint64_t global_y, int64_t global_z, block_set_mode mode) {
            get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
                sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, std::move(block));
                get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            });
        }

        void world_data::remove_block(int64_t global_x, uint64_t global_y, int64_t global_z) {
            get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
                sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, base_objects::block());
                get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            });
        }

        void world_data::get_block(int64_t global_x, uint64_t global_y, int64_t global_z, std::function<void(base_objects::block& block)> func, std::function<void(base_objects::block& block, enbt::value& extended_data)> block_entity) {
            get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
                sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15, func, block_entity);
            });
        }

        void world_data::query_block(int64_t global_x, uint64_t global_y, int64_t global_z, std::function<void(base_objects::block& block)> func, std::function<void(base_objects::block& block, enbt::value& extended_data)> block_entity, std::function<void()> fault) {
            request_chunk_data(
                global_x >> 4,
                global_z >> 4,
                [&](chunk_data& chunk) {
                    chunk.get_sub_chunk(
                        global_y >> 4,
                        [&, block_entity, func](sub_chunk_data& sub_chunk) {
                            sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15, func, block_entity);
                        }
                    );
                },
                fault
            );
        }

        void world_data::block_updated(int64_t global_x, uint64_t global_y, int64_t global_z) {
            std::unique_lock lock(mutex);
            get_light_processor()->block_changed(*this, global_x, global_y, global_z);
        }

        void world_data::chunk_updated(int64_t chunk_x, uint64_t chunk_z) {
            std::unique_lock lock(mutex);
            get_light_processor()->process_chunk(*this, chunk_x, chunk_z);
        }

        void world_data::sub_chunk_updated(int64_t chunk_x, uint64_t chunk_z, uint64_t sub_chunk_y) {
            std::unique_lock lock(mutex);
            get_light_processor()->process_sub_chunk(*this, chunk_x, sub_chunk_y, chunk_z);
        }

        void world_data::locked(std::function<void(world_data& self)> func) {
            std::unique_lock lock(mutex);
            func(*this);
        }

        void world_data::set_block_range(base_objects::cubic_bounds_block bounds, const list_array<base_objects::full_block_data>& blocks, block_set_mode mode) {
            if (blocks.size() == bounds.count()) {
                size_t i = 0;
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_block(blocks[i++], x, y, z, mode);
                    });
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        get_light_processor()->block_changed(*this, x, y, z);
                    });
                });
            } else {
                size_t i = 0;
                size_t max = blocks.size();
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_block(blocks[i++], x, y, z, mode);
                        if (i == max)
                            i = 0;
                    });
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        get_light_processor()->block_changed(*this, x, y, z);
                    });
                });
            }
        }

        void world_data::set_block_range(base_objects::cubic_bounds_block bounds, list_array<base_objects::full_block_data>&& blocks, block_set_mode mode) {
            if (blocks.size() == bounds.count()) {
                size_t i = 0;
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_block(std::move(blocks[i++]), x, y, z, mode);
                    });
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        get_light_processor()->block_changed(*this, x, y, z);
                    });
                });
            } else {
                size_t i = 0;
                size_t max = blocks.size();
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_block(std::move(blocks[i++]), x, y, z, mode);
                        if (++i == max)
                            i = 0;
                    });
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        get_light_processor()->block_changed(*this, x, y, z);
                    });
                });
            }
        }

        void world_data::set_block_range(base_objects::spherical_bounds_block bounds, const list_array<base_objects::full_block_data>& blocks, block_set_mode mode) {
            if (blocks.size() == bounds.count()) {
                size_t i = 0;
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_block(blocks[i++], x, y, z, mode);
                    });
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        get_light_processor()->block_changed(*this, x, y, z);
                    });
                });
            } else {
                size_t i = 0;
                size_t max = blocks.size();
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_block(blocks[i++], x, y, z, mode);
                    });
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        get_light_processor()->block_changed(*this, x, y, z);
                    });
                    if (++i == max)
                        i = 0;
                });
            }
        }

        void world_data::set_block_range(base_objects::spherical_bounds_block bounds, list_array<base_objects::full_block_data>&& blocks, block_set_mode mode) {
            if (blocks.size() == bounds.count()) {
                size_t i = 0;
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_block(std::move(blocks[i++]), x, y, z, mode);
                    });
                });
            } else {
                size_t i = 0;
                size_t max = blocks.size();
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_block(std::move(blocks[i++]), x, y, z, mode);
                        if (i++ == max)
                            i = 0;
                    });
                });
            }
        }

        uint32_t world_data::get_biome(int64_t global_x, uint64_t global_y, int64_t global_z) {
            uint32_t res = 0;
            get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
                res = sub_chunk.get_biome(global_x & 15, global_y & 15, global_z & 15);
            });
            return res;
        }

        void world_data::set_biome(int64_t global_x, uint64_t global_y, int64_t global_z, uint32_t id) {
            get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
                sub_chunk.set_biome(global_x & 15, global_y & 15, global_z & 15, id);
            });
        }

        void world_data::set_biome_range(base_objects::cubic_bounds_block bounds, const list_array<uint32_t>& blocks, block_set_mode mode) {
            if (blocks.size() == bounds.count()) {
                size_t i = 0;
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_biome(x, y, z, blocks[i++]);
                    });
                });
            } else {
                size_t i = 0;
                size_t max = blocks.size();
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_biome(x, y, z, blocks[i++]);
                        if (i == max)
                            i = 0;
                    });
                });
            }
        }

        void world_data::set_biome_range(base_objects::cubic_bounds_block bounds, list_array<uint32_t>&& blocks, block_set_mode mode) {
            if (blocks.size() == bounds.count()) {
                size_t i = 0;
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_biome(x, y, z, blocks[i++]);
                    });
                });
            } else {
                size_t i = 0;
                size_t max = blocks.size();
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_biome(x, y, z, blocks[i++]);
                        if (i == max)
                            i = 0;
                    });
                });
            }
        }

        void world_data::set_biome_range(base_objects::spherical_bounds_block bounds, const list_array<uint32_t>& blocks, block_set_mode mode) {
            if (blocks.size() == bounds.count()) {
                size_t i = 0;
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_biome(x, y, z, blocks[i++]);
                    });
                });
            } else {
                size_t i = 0;
                size_t max = blocks.size();
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_biome(x, y, z, blocks[i++]);
                        if (i == max)
                            i = 0;
                    });
                });
            }
        }

        void world_data::set_biome_range(base_objects::spherical_bounds_block bounds, list_array<uint32_t>&& blocks, block_set_mode mode) {
            if (blocks.size() == bounds.count()) {
                size_t i = 0;
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_biome(x, y, z, blocks[i++]);
                    });
                });
            } else {
                size_t i = 0;
                size_t max = blocks.size();
                locked([&](storage::world_data& world) {
                    bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                        world.set_biome(x, y, z, blocks[i++]);
                        if (i == max)
                            i = 0;
                    });
                });
            }
        }

        uint64_t world_data::register_client(const base_objects::client_data_holder& client) {
            std::unique_lock lock(mutex);
            uint64_t id = local_client_id_generator++;
            clients[id] = client;
            //TODO NOTIFY CLIENTS AND ENTITIES
            return id;
        }

        void world_data::unregister_client(uint64_t id) {
            std::unique_lock lock(mutex);
            clients.erase(id);
            //TODO NOTIFY CLIENTS AND ENTITIES
        }

        void world_data::register_entity(base_objects::entity_ref& entity) {
            std::unique_lock lock(mutex);
            entity->world = this;
            //TODO NOTIFY CLIENTS AND ENTITIES
        }

        void world_data::unregister_entity(base_objects::entity_ref& entity) {
            std::unique_lock lock(mutex);
            entity->world = nullptr;
            //TODO NOTIFY CLIENTS AND ENTITIES
        }

        void world_data::change_chunk_generator(const std::string& id) {
            std::unique_lock lock(mutex);
            light_processor = nullptr;
            light_processor_id = id;
        }

        void world_data::change_light_processor(const std::string& id) {
            std::unique_lock lock(mutex);
            light_processor = nullptr;
            light_processor_id = id;
        }

        void world_data::tick(std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time) {
            std::unique_lock lock(mutex);

            last_usage = current_time;
            list_array<base_objects::atomic_holder<chunk_data>> to_tick_chunks;

            for (auto& bound_init : load_points)
                bound_init.enum_points([this](int64_t x, int64_t z) {
                    auto res = request_chunk_data(x, z);
                    if (res->is_ready())
                        res->get()->marked_for_tick = true;
                });

            for (auto& bound_init : load_points_sphere)
                bound_init.enum_points([this](int64_t x, int64_t z) {
                    auto res = request_chunk_data(x, z);
                    if (res->is_ready())
                        res->get()->marked_for_tick = true;
                });


            for (auto& [id, client] : clients) {
                auto& pos = client->player_data.position;
                auto distance = client->simulation_distance;
                base_objects::cubic_bounds_chunk_radius(
                    pos.x / 16,
                    pos.z / 16,
                    distance
                )
                    .enum_points([this](int64_t x, int64_t z) {
                        auto res = request_chunk_data(x, z);
                        if (res->is_ready())
                            res->get()->marked_for_tick = true;
                    });
            }

            for (auto& [x, x_axis] : chunks)
                for (auto& [z, chunk] : x_axis)
                    if (chunk->marked_for_tick) {
                        to_tick_chunks.push_back(chunk);
                        chunk->marked_for_tick = false;
                    }
            lock.unlock();
            if (!profiling.enable_world_profiling) {
                to_tick_chunks.take().for_each([&](auto&& chunk) {
                    auto local_ticks = max_random_tick_for_chunk;
                    chunk->tick(*this, local_ticks, random_engine, current_time);
                });
            } else {
                const auto tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(1) / ticks_per_second);
                auto tick_local_time = current_time;
                to_tick_chunks.take().for_each([&](auto&& chunk) {
                    auto local_ticks = max_random_tick_for_chunk;
                    chunk->tick(*this, local_ticks, random_engine, current_time);

                    auto actual_time = std::chrono::high_resolution_clock::now();
                    auto current_tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    auto slow_chunk_threshold = tick_speed * profiling.slow_chunk_tick_callback_threshold;
                    if (slow_chunk_threshold < current_tick_speed)
                        if (profiling.slow_chunk_tick_callback)
                            profiling.slow_chunk_tick_callback(*this, chunk->chunk_x, chunk->chunk_z, current_tick_speed);
                    tick_local_time = actual_time;
                });
                ++profiling.got_ticks;
                auto current_tick_speed = tick_local_time - current_time;
                if (tick_local_time - profiling.last_tick >= std::chrono::seconds(1)) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(tick_local_time - profiling.last_tick).count();
                    profiling.tps_for_world = profiling.got_ticks / elapsed;
                    if (profiling.got_tps_update)
                        profiling.got_tps_update(*this);
                    profiling.last_tick = tick_local_time;
                    profiling.got_ticks = 0;
                }

                auto slow_world_threshold = tick_speed * profiling.slow_world_tick_callback_threshold;
                if (slow_world_threshold < current_tick_speed)
                    if (profiling.slow_world_tick_callback)
                        profiling.slow_world_tick_callback(*this, std::chrono::duration_cast<std::chrono::milliseconds>(current_tick_speed));
            }
        }

        bool world_data::collect_unused_data(std::chrono::high_resolution_clock::time_point current_time, size_t& unload_limit) {
            std::unique_lock lock(mutex);
            if (last_usage + world_lifetime < current_time)
                if (on_load_process.empty() && on_save_process.empty())
                    return true;

            for (auto& [x, x_axis] : chunks) {
                bool chunk_unloaded = false;
                do {
                    chunk_unloaded = false;
                    auto begin = x_axis.begin();
                    auto end = x_axis.end();
                    while (begin != end && unload_limit) {
                        if (!begin->second->last_usage.time_since_epoch().count()) {
                            if (begin->second->last_usage + chunk_lifetime < current_time) {
                                make_save(x, begin->first, begin, true);
                                --unload_limit;
                                chunk_unloaded = true;
                                break;
                            }
                        }
                        ++begin;
                    }
                    if (!unload_limit)
                        break;
                } while (chunk_unloaded);
            }
            return false;
        }

#pragma region worlds_data

        base_objects::atomic_holder<world_data> worlds_data::load(uint64_t world_id) {
            std::unique_lock lock(mutex);
            if (cached_worlds.find(world_id) == cached_worlds.end()) {
                auto path = base_path / std::to_string(world_id);
                if (!std::filesystem::exists(path))
                    throw std::runtime_error("World not found");

                auto world = base_objects::atomic_holder<world_data>(new world_data(path.string()));
                world->load();
                auto& res = cached_worlds[world_id] = world;
                on_world_loaded(world_id);
                return res;
            }
            return cached_worlds[world_id];
        }

        worlds_data::worlds_data(base_objects::ServerConfiguration& configuration, const std::filesystem::path& base_path)
            : base_path(base_path), configuration(configuration) {
            if (!std::filesystem::exists(base_path))
                std::filesystem::create_directories(base_path);
        }

        const list_array<uint64_t>& worlds_data::get_ids() {
            std::unique_lock lock(mutex);
            if (!cached_ids.empty())
                return cached_ids;
            list_array<uint64_t> result;
            for (auto& entry : std::filesystem::directory_iterator{base_path}) {
                if (entry.is_directory()) {
                    auto& path = entry.path();
                    try {
                        result.push_back(std::stoi(path.filename().string()));
                    } catch (const std::exception&) {
                        log::warn("storage:worlds_data", "Got corrupted file path: " + path.string());
                    }
                }
            }
            result.commit();
            return cached_ids = result;
        }

        bool worlds_data::exists(uint64_t world_id) {
            std::unique_lock lock(mutex);
            if (cached_ids.empty()) {
                lock.unlock();
                get_list();
                lock.lock();
            }
            return cached_ids.contains(world_id);
        }

        bool worlds_data::exists(const std::string& name) {
            return get_id(name) != -1;
        }

        const list_array<uint64_t>& worlds_data::get_list() {
            std::unique_lock lock(mutex);
            return get_ids();
        }

        std::string worlds_data::get_name(uint64_t world_id) {
            std::unique_lock lock(mutex);
            if (auto on_load = cached_worlds.find(world_id); on_load != cached_worlds.end())
                return on_load->second->world_name;

            auto world_path = base_path / std::to_string(world_id);
            if (std::filesystem::exists(world_path))
                return world_data(world_path).preview_world_name();
            else
                throw std::runtime_error("World with id " + std::to_string(world_id) + " not found.");
        }

        uint64_t worlds_data::get_id(const std::string& name) {
            std::unique_lock lock(mutex);
            for (auto& world : cached_worlds)
                if (world.second->world_name == name)
                    return world.first;

            if (cached_ids.empty())
                get_ids();
            size_t found = cached_ids.find_if([this, &name](uint64_t id) {
                return world_data(base_path / std::to_string(id)).preview_world_name() == name;
            });
            return found == list_array<uint64_t>::npos ? -1 : cached_ids[found];
        }

        base_objects::atomic_holder<world_data> worlds_data::get(uint64_t world_id) {
            std::unique_lock lock(mutex);
            if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
                return load(world_id);
            else
                return world->second;
        }

        void worlds_data::save(uint64_t world_id) {
            std::unique_lock lock(mutex);
            if (auto item = cached_worlds.find(world_id); item == cached_worlds.end())
                throw std::runtime_error("World not found");
            else {
                item->second->save();
                item->second->save_chunks();
            }
        }

        void worlds_data::save_all() {
            std::unique_lock lock(mutex);
            for (auto& [id, world] : cached_worlds) {
                world->save();
                world->save_chunks();
            }
        }

        void worlds_data::save_and_unload(uint64_t world_id) {
            std::unique_lock lock(mutex);
            if (auto item = cached_worlds.find(world_id); item == cached_worlds.end())
                throw std::runtime_error("World not found");
            else {
                auto& world = item->second;
                on_world_unloaded(world_id);
                world->save();
                world->save_chunks();
                world->await_save_chunks();
                cached_worlds.erase(item);
            }
        }

        void worlds_data::save_and_unload_all() {
            std::unique_lock lock(mutex);
            for (auto& [id, world] : cached_worlds) {
                on_world_unloaded(id);
                world->save();
                world->save_chunks();
                world->await_save_chunks();
            }
            cached_worlds.clear();
        }

        //be sure that world is not used by anything, otherwise will throw exception
        void worlds_data::unload(uint64_t world_id) {
            std::unique_lock lock(mutex);
            on_world_unloaded(world_id);
            cached_worlds.erase(world_id);
        }

        void worlds_data::unload_all() {
            std::unique_lock lock(mutex);
            for (auto&& [id, world] : cached_worlds)
                on_world_unloaded(id);
            cached_worlds.clear();
        }

        void worlds_data::erase(uint64_t world_id) {
            std::unique_lock lock(mutex);
            std::filesystem::remove_all(std::filesystem::path(base_path) / std::to_string(world_id));
            cached_worlds.erase(world_id);
            on_world_unloaded(world_id);
        }

        uint64_t worlds_data::create(const std::string& name) {
            std::unique_lock lock(mutex);
            if (get_id(name) != -1)
                throw std::runtime_error("World with name " + name + " already exists.");
            uint64_t id = 0;
            while (exists(id))
                id++;
            cached_ids.push_back(id);
            cached_worlds[id] = new world_data(base_path / std::to_string(id));
            cached_worlds[id]->world_name = name;
            cached_worlds[id]->save();
            on_world_loaded(id);
            return id;
        }

        uint64_t worlds_data::create(const std::string& name, std::function<void(world_data& world)> init) {
            std::unique_lock lock(mutex);
            if (get_id(name) != -1)
                throw std::runtime_error("World with name " + name + " already exists.");
            uint64_t id = 0;
            while (exists(id))
                id++;
            base_objects::atomic_holder<world_data> world = new world_data(base_path / std::to_string(id));
            init(*world);
            cached_ids.push_back(id);
            cached_worlds[id] = world;
            cached_worlds[id]->world_name = name;
            cached_worlds[id]->save();
            on_world_loaded(id);
            return id;
        }

        void worlds_data::locked(std::function<void()> func) {
            std::unique_lock lock(mutex);
            func();
        }

        void worlds_data::locked(std::function<void(worlds_data& self)> func) {
            std::unique_lock lock(mutex);
            func(*this);
        }

        void worlds_data::for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            for (auto& [id, world] : cached_worlds)
                world->for_each_entity(func);
        }

        void worlds_data::for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            for (auto& [id, world] : cached_worlds)
                world->for_each_entity(chunk_x, chunk_z, func);
        }

        void worlds_data::for_each_entity(int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            for (auto& [id, world] : cached_worlds)
                world->for_each_entity(chunk_x, chunk_z, sub_chunk_y, func);
        }

        void worlds_data::for_each_entity(uint64_t world_id, std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
                load(world_id)->for_each_entity(func);
            else
                world->second->for_each_entity(func);
        }

        void worlds_data::for_each_entity(uint64_t world_id, int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
                load(world_id)->for_each_entity(chunk_x, chunk_z, func);
            else
                world->second->for_each_entity(chunk_x, chunk_z, func);
        }

        void worlds_data::for_each_entity(uint64_t world_id, int64_t chunk_x, int64_t chunk_z, uint64_t sub_chunk_y, std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
                load(world_id)->for_each_entity(chunk_x, chunk_z, sub_chunk_y, func);
            else
                world->second->for_each_entity(chunk_x, chunk_z, sub_chunk_y, func);
        }

        void worlds_data::for_each_world(std::function<void(uint64_t id, world_data& world)> func) {
            std::unique_lock lock(mutex);
            for (auto& [id, world] : cached_worlds)
                func(id, *world);
        }

        void worlds_data::apply_tick(std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time, std::chrono::nanoseconds ns) {
            std::unique_lock lock(mutex);
            list_array<std::pair<int64_t, base_objects::atomic_holder<world_data>>> worlds_to_tick;
            for (auto& [id, world] : cached_worlds) {
                if (!world->last_usage.time_since_epoch().count())
                    world->last_usage = current_time;
                if (ticks_per_second > world->ticks_per_second) {
                    auto tick_interval = std::chrono::nanoseconds(1'000'000'000 / world->ticks_per_second);
                    world->accumulated_time += current_time - world->last_usage;
                    world->last_usage = current_time;
                    if (world->accumulated_time / tick_interval) {
                        world->accumulated_time -= tick_interval;
                        worlds_to_tick.push_back({id, world});
                    }
                } else
                    worlds_to_tick.push_back({id, world});
            }
            lock.unlock();

            list_array<uint64_t> to_unload_worlds;
            for (auto& [id, world] : worlds_to_tick) {
                world->tick(random_engine, current_time);

                size_t unload_speed = configuration.world.unload_speed;
                if (world->collect_unused_data(current_time, unload_speed))
                    to_unload_worlds.push_back(id);
            }
            to_unload_worlds.for_each([this](uint64_t id) { save_and_unload(id); });

            lock.lock();
            got_ticks++;
            auto new_current_time = std::chrono::high_resolution_clock::now();
            if (current_time - last_tps_calculated >= std::chrono::seconds(1)) {
                auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(new_current_time - last_tps_calculated).count();
                tps = double(got_ticks) / elapsed;
                on_tps_changed.async_notify(tps);
                last_tps_calculated = new_current_time;
                got_ticks = 0;
            }
        }

#pragma endregion
    } // namespace storage

} // namespace crafted_craft
