#include "world_data.hpp"
#include "../log.hpp"

namespace crafted_craft {
    namespace storage {

        class world_data;
        class worlds_data;

        void block_data::tick(world_data& world, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z) {
            if (is_tickable) {
                auto& tick = base_objects::block(block_id).getStaticData().on_tick;
                if (tick) {
                    chunk_x *= 16;
                    chunk_z *= 16;
                    tick(world, chunk_x + local_x, sub_chunk_y * 16 + local_y, chunk_z + local_z);
                } else
                    is_tickable = false;
            }
        }

        bool chunk_data::load(const std::filesystem::path& chunk_z) {
            std::ifstream file(chunk_z, std::ios::binary);
            if (!file.is_open())
                return false;
            ENBT chunk_data_file = ENBTHelper::ReadToken(file);
            file.close();
            return load(chunk_data_file);
        }

        bool valid_sub_chunk_size(const ENBT& x) {
            if (x.size() != 16)
                return false;
            else
                for (auto [empty__, y] : x)
                    if (y.size() != 16)
                        return false;
                    else
                        for (auto [empty__, z] : y)
                            if (z.size() != 16)
                                return false;
            return true;
        }

        void load_light_data(const ENBT& x, light_data& data, bool& need_to_recalculate_light) {
            if (!valid_sub_chunk_size(x)) {
                need_to_recalculate_light = true;
                return;
            }

            size_t x_ = 0;
            for (auto [empty__, y] : x) {
                size_t y_ = 0;
                for (auto [empty__, z] : y) {
                    size_t z_ = 0;
                    for (auto [empty__, z] : y)
                        data.light_map[x_][y_][z_++].raw = z;
                    ++y_;
                }
                ++x_;
            }
        }

        void load_block_data(const ENBT& x, block_data (&data)[16][16][16], bool& has_tickable_blocks) {
            size_t x_ = 0;
            for (auto [empty__, y] : x) {
                size_t y_ = 0;
                for (auto [empty__, z] : y) {
                    size_t z_ = 0;
                    for (auto [empty__, z] : y) {
                        data[x_][y_][z_].raw = z;
                        has_tickable_blocks |= data[x_][y_][z_].is_tickable;
                        ++z_;
                    }
                    ++y_;
                }
                ++x_;
            }
        }

        bool chunk_data::load(const ENBT& chunk_data) {
            std::lock_guard lock(mutex);
            for (auto [empty_name, sub_chunk] : chunk_data["sub_chunks"]) {
                sub_chunk_data sub_chunk_data;
                auto blocks = sub_chunk["blocks"];
                if (!valid_sub_chunk_size(blocks))
                    return false;
                load_block_data(blocks, sub_chunk_data.blocks, sub_chunk_data.has_tickable_blocks);
                for (auto [empty_, entity] : sub_chunk["entities"])
                    sub_chunk_data.stored_entities.push_back(base_objects::entity::load_from_enbt(entity));
                for (auto [empty_, block_entity] : sub_chunk["block_entities"]) {
                    base_objects::block_entity be;
                    be.block_id = base_objects::block(block_entity["id"]);
                    be.nbt = block_entity["nbt"];
                    be.type = block_entity["type"];
                    be.x = block_entity["x"];
                    be.y = block_entity["y"];
                    be.z = block_entity["z"];
                    sub_chunk_data.block_entities.push_back(std::move(be));
                }
                load_light_data(sub_chunk["block_light"], sub_chunk_data.block_light, sub_chunk_data.need_to_recalculate_light);
                load_light_data(sub_chunk["sky_light"], sub_chunk_data.sky_light, sub_chunk_data.need_to_recalculate_light);
                for (auto [empty_, query] : sub_chunk["queried_for_tick"])
                    sub_chunk_data.queried_for_tick.push_back({query["x"], query["y"], query["z"]});
                sub_chunks.push_back(std::move(sub_chunk_data));
            }
            return true;
        }

        bool chunk_data::save(const std::filesystem::path& chunk_z) {
            std::lock_guard lock(mutex);
            std::ofstream file(chunk_z, std::ios::binary);
            if (!file.is_open())
                return false;
            ENBT chunk_data_file = ENBT::compound();
            for (auto& sub_chunk : sub_chunks) {
                ENBT sub_chunk_data = ENBT::compound();
                {
                    ENBT blocks = ENBT::fixed_array(16);
                    for (size_t x = 0; x < 16; x++) {
                        ENBT x_axis = ENBT::fixed_array(16);
                        for (size_t y = 0; y < 16; y++) {
                            ENBT y_axis = ENBT::fixed_array(16);
                            for (size_t z = 0; z < 16; z++)
                                y_axis[z] = (sub_chunk.blocks[x][y][z].raw);
                            x_axis[y] = y_axis;
                        }
                        blocks[x] = x_axis;
                    }
                    sub_chunk_data["blocks"] = std::move(blocks);
                }
                {
                    ENBT entities = ENBT::dynamic_array();
                    for (auto& entity : sub_chunk.stored_entities)
                        entities.push(entity->copy_to_enbt());
                    sub_chunk_data["entities"] = std::move(entities);
                }
                {
                    ENBT block_entities = ENBT::dynamic_array();
                    for (auto& block_entity : sub_chunk.block_entities) {
                        ENBT be;
                        be["id"] = block_entity.block_id;
                        be["nbt"] = block_entity.nbt;
                        be["type"] = block_entity.type;
                        be["x"] = block_entity.x;
                        be["y"] = block_entity.y;
                        be["z"] = block_entity.z;
                        block_entities.push(std::move(be));
                    }
                    sub_chunk_data["block_entities"] = std::move(block_entities);
                }
                if (!sub_chunk.need_to_recalculate_light) {
                    {
                        ENBT block_light = ENBT::fixed_array(16);
                        for (size_t x = 0; x < 16; x++) {
                            ENBT x_axis = ENBT::fixed_array(16);
                            for (size_t y = 0; y < 16; y++) {
                                ENBT y_axis = ENBT::fixed_array(16);
                                for (size_t z = 0; z < 16; z++)
                                    y_axis[z] = sub_chunk.block_light.light_map[x][y][z].raw;
                                x_axis[y] = std::move(y_axis);
                            }
                            block_light[x] = std::move(x_axis);
                        }
                        sub_chunk_data["block_light"] = std::move(block_light);
                    }
                    {
                        ENBT sky_light = ENBT::fixed_array(16);
                        for (size_t x = 0; x < 16; x++) {
                            ENBT x_axis = ENBT::fixed_array(16);
                            for (size_t y = 0; y < 16; y++) {
                                ENBT y_axis = ENBT::fixed_array(16);
                                for (size_t z = 0; z < 16; z++)
                                    y_axis[z] = sub_chunk.sky_light.light_map[x][y][z].raw;
                                x_axis[y] = std::move(y_axis);
                            }
                            sky_light[x] = std::move(x_axis);
                        }
                        sub_chunk_data["sky_light"] = std::move(sky_light);
                    }
                }
                if (!sub_chunk.queried_for_tick.empty()) {
                    ENBT queried_for_tick = ENBT::dynamic_array();
                    for (auto& query : sub_chunk.queried_for_tick) {
                        ENBT q;
                        q["x"] = query.x;
                        q["y"] = query.y;
                        q["z"] = query.z;
                        queried_for_tick.push(std::move(q));
                    }
                    sub_chunk_data["queried_for_tick"] = std::move(queried_for_tick);
                }
                chunk_data_file["sub_chunks"].push(std::move(sub_chunk_data));
            }
            ENBTHelper::WriteToken(file, chunk_data_file);
            file.close();
            return true;
        }

        chunk_data::chunk_data(world_data& world, int64_t chunk_x, int64_t chunk_z)
            : world(world), chunk_x(chunk_x), chunk_z(chunk_z) {}

        void chunk_data::for_each_entity(std::function<void(base_objects::entity_ref& entity)> func) {
            std::lock_guard lock(mutex);
            for (auto& sub_chunk : sub_chunks)
                for (auto& entity : sub_chunk.stored_entities)
                    func(entity);
        }

        void chunk_data::for_each_entity(std::function<void(base_objects::entity_ref& entity)> func, uint64_t sub_chunk_y) {
            std::lock_guard lock(mutex);
            if (sub_chunk_y < sub_chunks.size())
                for (auto& entity : sub_chunks[sub_chunk_y].stored_entities)
                    func(entity);
        }

        void chunk_data::for_each_sub_chunk(std::function<void(sub_chunk_data& sub_chunk)> func) {
            std::lock_guard lock(mutex);
            for (auto& sub_chunk : sub_chunks)
                func(sub_chunk);
        }

        void chunk_data::get_sub_chunk(std::function<void(sub_chunk_data& sub_chunk)> func, uint64_t sub_chunk_y) {
            std::lock_guard lock(mutex);
            if (sub_chunk_y < sub_chunks.size())
                func(sub_chunks[sub_chunk_y]);
        }

        void chunk_data::query_for_tick(uint8_t local_x, uint64_t global_y, uint8_t local_z) {
            std::lock_guard lock(mutex);
            uint64_t sub_chunk = global_y >> 4;
            if (sub_chunk < sub_chunks.size())
                sub_chunks[sub_chunk].queried_for_tick.push_back({local_x, global_y & 15, local_z});
        }

        void chunk_data::tick(size_t max_random_tick_for_chunk, std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time) {
            std::lock_guard lock(mutex);
            if (!marked_for_tick)
                return;
            last_usage = current_time;


            uint64_t sub_chunk_y = 0;
            for (auto& sub_chunk : sub_chunks) {
                for (auto& entity : sub_chunk.stored_entities)
                    entity->tick();

                for (auto& query : sub_chunk.queried_for_tick)
                    sub_chunk.blocks[query.x][query.y][query.z].tick(world, chunk_x, sub_chunk_y, chunk_z, query.x, query.y, query.z);
                sub_chunk.queried_for_tick.clear();


                if (sub_chunk.has_tickable_blocks && max_random_tick_for_chunk) {
                    for (size_t x = 0; x < 16; x++)
                        for (size_t y = 0; y < 16; y++)
                            for (size_t z = 0; z < 16; z++)
                                if (sub_chunk.blocks[x][y][z].is_tickable) {
                                    if (max_random_tick_for_chunk > 0) {
                                        if (random_engine() & 1) {
                                            sub_chunk.blocks[x][y][z].tick(world, chunk_x, sub_chunk_y, chunk_z, x, y, z);
                                            --max_random_tick_for_chunk;
                                        }
                                    }
                                }
                }
                sub_chunk_y++;
            }
        }

        void world_data::make_save(int64_t chunk_x, int64_t chunk_z, bool also_erase) {
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                    make_save(chunk_x, chunk_z, y_axis, also_erase);
        }

        void world_data::make_save(int64_t chunk_x, int64_t chunk_z, chunk_row::iterator item, bool also_erase) {
            if (auto process = on_save_process.find({chunk_x, chunk_z}); process == on_save_process.end()) {
                auto& chunk = item->second;
                on_save_process[{chunk_x, chunk_z}] = Future<bool>::start(
                    [this, chunk, chunk_x, chunk_z] {
                        try {
                            chunk->save(path / "chunks" / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"));
                        } catch (...) {
                            return false;
                        }
                        std::unique_lock lock(mutex);
                        on_save_process.erase({chunk_x, chunk_z});
                        return true;
                    }
                );
            }
            if (also_erase) {
                if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                    x_axis->second.erase(chunk_z);
            }
        }

        base_objects::atomic_holder<chunk_data> world_data::load_chunk_sync(int64_t chunk_x, int64_t chunk_z) {
            auto chunk = base_objects::atomic_holder<chunk_data>(new chunk_data(*this, chunk_x, chunk_z));
            if (!chunk->load(path / "chunks" / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"))) {
                if (generator) {
                    if (!chunk->load(generator->generate_chunk(*this, chunk_x, chunk_z)))
                        return nullptr;
                } else
                    return nullptr;
            }
            uint64_t y = 0;
            chunk->for_each_sub_chunk([&](sub_chunk_data& sub_chunk) {
                if (sub_chunk.need_to_recalculate_light)
                    light_processor->process_sub_chunk(*this, chunk_x, y, chunk_z);
                ++y;
            });
            return chunk;
        }

        void world_data::load() {
            std::unique_lock lock(mutex);
            std::ifstream file(path / "world.dat", std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("Can't open world file");
            ENBT world_data_file = ENBTHelper::ReadToken(file);
            file.close();
            general_world_data = world_data_file["general_world_data"];
            world_game_rules = world_data_file["world_game_rules"];
            world_generator_data = world_data_file["world_generator_data"];
            world_records = world_data_file["world_records"];
            world_seed = world_data_file["world_seed"];
            wandering_trader_id = world_data_file["wandering_trader_id"];
            wandering_trader_spawn_chance = world_data_file["wandering_trader_spawn_chance"];
            wandering_trader_spawn_delay = world_data_file["wandering_trader_spawn_delay"];
            world_name = (std::string)world_data_file["world_name"];
            world_type = (std::string)world_data_file["world_type"];
            generator_id = (std::string)world_data_file["generator_id"];
            enabled_datapacks = (std::vector<std::string>)world_data_file["enabled_datapacks"];
            enabled_plugins = (std::vector<std::string>)world_data_file["enabled_plugins"];
            enabled_features = (std::vector<std::string>)world_data_file["enabled_features"];
            border_center_x = world_data_file["border_center_x"];
            border_center_z = world_data_file["border_center_z"];
            border_size = world_data_file["border_size"];
            border_safe_zone = world_data_file["border_safe_zone"];
            border_damage_per_block = world_data_file["border_damage_per_block"];
            border_lerp_target = world_data_file["border_lerp_target"];
            border_lerp_time = world_data_file["border_lerp_time"];
            border_warning_blocks = world_data_file["border_warning_blocks"];
            border_warning_time = world_data_file["border_warning_time"];
            day_time = world_data_file["day_time"];
            time = world_data_file["time"];
            clear_weather_time = world_data_file["clear_weather_time"];
            internal_version = world_data_file["internal_version"];
            chunk_y_count = world_data_file["chunk_y_count"];
            difficulty = world_data_file["difficulty"];
            default_gamemode = world_data_file["default_gamemode"];
            difficulty_locked = world_data_file["difficulty_locked"];
            is_hardcore = world_data_file["is_hardcore"];
            initialized = world_data_file["initialized"];
            raining = world_data_file["raining"];
            thundering = world_data_file["thundering"];
            has_skylight = world_data_file["has_skylight"];
            chunk_lifetime = std::chrono::milliseconds((long long)world_data_file["chunk_lifetime"]);
            world_lifetime = std::chrono::milliseconds((long long)world_data_file["world_lifetime"]);
            enable_entity_updates_when_hold_light_source = world_data_file["enable_entity_updates_when_hold_light_source"];
            enable_entity_light_source_updates = world_data_file["enable_entity_light_source_updates"];
        }

        void world_data::save() {
            std::unique_lock lock(mutex);
            std::ofstream file(path / "world.dat", std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("Can't open world file");
            ENBT world_data_file = ENBT::compound();
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
            world_data_file["generator_id"] = generator_id;
            world_data_file["enabled_datapacks"] = enabled_datapacks;
            world_data_file["enabled_plugins"] = enabled_plugins;
            world_data_file["enabled_features"] = enabled_features;
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
            world_data_file["chunk_lifetime"] = chunk_lifetime.count();
            world_data_file["world_lifetime"] = world_lifetime.count();

            if (enable_entity_updates_when_hold_light_source)
                world_data_file["enable_entity_updates_when_hold_light_source"] = true;
            if (enable_entity_light_source_updates)
                world_data_file["enable_entity_light_source_updates"] = true;
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

        FuturePtr<base_objects::atomic_holder<chunk_data>> world_data::request_chunk_data(int64_t chunk_x, int64_t chunk_z) {
            std::unique_lock lock(mutex);
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                    return make_ready_future(y_axis->second);

            if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
                return on_load_process[{chunk_x, chunk_z}] = Future<base_objects::atomic_holder<chunk_data>>::start(
                           [this, chunk_x, chunk_z]() -> base_objects::atomic_holder<chunk_data> {
                               auto chunk = load_chunk_sync(chunk_x, chunk_z);
                               std::unique_lock lock(mutex);
                               on_load_process.erase({chunk_x, chunk_z});
                               return chunks[chunk_x][chunk_z] = chunk;
                           }
                       );
            else
                return process->second;
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

        void world_data::for_each_chunk(std::function<void(chunk_data& chunk)> func) {
            std::unique_lock lock(mutex);
            for (auto& [x, x_axis] : chunks)
                for (auto& [z, chunk] : x_axis)
                    func(*chunk);
        }

        void world_data::for_each_chunk(base_objects::square_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func) {
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
                    chunk->second->get_sub_chunk(func, sub_chunk_y);
        }

        void world_data::for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            for (auto& [x, x_axis] : chunks)
                for (auto& [z, chunk] : x_axis)
                    chunk->for_each_entity(func);
        }

        void world_data::for_each_entity(base_objects::square_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func) {
            std::unique_lock lock(mutex);
            for (int64_t x = bounds.x1; x <= bounds.x2; x++)
                for (int64_t z = bounds.z1; z <= bounds.z2; z++)
                    if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                        if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                            chunk->second->for_each_entity(func);
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
                    chunk->second->for_each_entity(func, sub_chunk_y);
        }

        void world_data::query_for_tick(int64_t global_x, uint64_t global_y, int64_t global_z) {
            std::unique_lock lock(mutex);
            auto chunk_x = global_x >> 4;
            auto chunk_z = global_z >> 4;
            if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                    chunk->second->query_for_tick(global_x & 15, global_y, global_z & 15);
        }

        std::shared_ptr<Future<void>> world_data::tick(size_t max_random_tick_for_chunk, std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time, bool update_tps) {
            std::unique_lock lock(mutex);

            last_usage = current_time;
            if (!max_random_tick_for_chunk)
                return;

            list_array<base_objects::atomic_holder<chunk_data>> to_tick_chunks;
            for (auto& [x, x_axis] : chunks)
                for (auto& [z, chunk] : x_axis)
                    if (chunk->marked_for_tick)
                        to_tick_chunks.push_back(chunk);
            lock.unlock();
            if (!profiling.enable_world_profiling) {
                return futureMoveForEach(to_tick_chunks, [max_random_tick_for_chunk, re = std::mt19937(random_engine()), current_time](base_objects::atomic_holder<chunk_data>& chunk) mutable {
                    chunk->tick(max_random_tick_for_chunk, re, current_time);
                });
            } else {
                auto chunk_speed = current_time;
                for (auto& chunk : to_tick_chunks) {
                    chunk->tick(max_random_tick_for_chunk, random_engine, current_time);

                    auto actual_time = std::chrono::high_resolution_clock::now();
                    std::chrono::milliseconds current_tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - current_time);
                    auto slow_world_threshold = tick_speed * profiling.slow_chunk_tick_callback_threshold;
                    if (slow_world_threshold < current_tick_speed)
                        if (profiling.slow_chunk_tick_callback)
                            profiling.slow_chunk_tick_callback(*this, chunk->chunk_x, chunk->chunk_z, current_tick_speed);
                    chunk_speed = actual_time;
                }
                ++profiling.got_ticks;
                auto actual_time = std::chrono::high_resolution_clock::now();
                std::chrono::milliseconds current_tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - current_time);
                if (update_tps) {
                    profiling.tps_for_world = double(profiling.got_ticks) / current_tick_speed.count() * 1000;
                    if (profiling.got_tps_update)
                        profiling.got_tps_update(*this);
                }

                auto slow_world_threshold = tick_speed * profiling.slow_world_tick_callback_threshold;
                if (slow_world_threshold < current_tick_speed)
                    if (profiling.slow_world_tick_callback)
                        profiling.slow_world_tick_callback(*this, current_tick_speed);
                return Future<void>::make_ready();
            }
        }

        bool world_data::collect_unused_data(std::chrono::high_resolution_clock::time_point current_time, size_t& unload_limit) {
            std::unique_lock lock(mutex);
            if (last_usage + world_lifetime < current_time)
                if (on_load_process.empty() && on_save_process.empty())
                    return true;

            for (auto& [x, x_axis] : chunks) {
                auto begin = x_axis.begin();
                auto end = x_axis.end();
                while (begin != end && unload_limit) {
                    if (begin->second->last_usage + chunk_lifetime < current_time) {
                        make_save(x, begin->first, begin, true);
                        --unload_limit;
                    }
                    ++begin;
                }
                if (!unload_limit)
                    break;
            }
            return false;
        }

#pragma region worlds_data

        base_objects::atomic_holder<world_data> worlds_data::load(uint64_t world_id) {
            if (cached_worlds.find(world_id) == cached_worlds.end()) {
                auto path = base_path / std::to_string(world_id);
                if (!std::filesystem::exists(path))
                    throw std::runtime_error("World not found");
                auto world = base_objects::atomic_holder<world_data>(new world_data(path.string()));
                world->load();
                return cached_worlds[world_id] = world;
            }
            return cached_worlds[world_id];
        }

        worlds_data::worlds_data(base_objects::ServerConfiguration& configuration, const std::filesystem::path& base_path)
            : base_path(base_path), configuration(configuration) {
            if (!std::filesystem::exists(base_path))
                std::filesystem::create_directories(base_path);
        }

        bool worlds_data::exists(uint64_t world_id) {
            std::unique_lock lock(mutex);
            if (cached_worlds.find(world_id) == cached_worlds.end())
                return std::filesystem::exists(base_path / std::to_string(world_id));
            else
                return true;
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
            else
                item->second->save();
        }

        void worlds_data::save_all() {
            std::unique_lock lock(mutex);
            for (auto& [id, world] : cached_worlds)
                world->save();
        }

        void worlds_data::save_and_unload(uint64_t world_id) {
            std::unique_lock lock(mutex);
            if (auto item = cached_worlds.find(world_id); item == cached_worlds.end())
                throw std::runtime_error("World not found");
            else {
                item->second->save();
                cached_worlds.erase(item);
            }
        }

        void worlds_data::save_and_unload_all() {
            std::unique_lock lock(mutex);
            for (auto& [id, world] : cached_worlds)
                world->save();
            cached_worlds.clear();
        }

        //be sure that world is not used by anything, otherwise will throw exception
        void worlds_data::unload(uint64_t world_id) {
            std::unique_lock lock(mutex);
            cached_worlds.erase(world_id);
        }

        void worlds_data::unload_all() {
            std::unique_lock lock(mutex);
            cached_worlds.clear();
        }

        void worlds_data::erase(uint64_t world_id) {
            std::unique_lock lock(mutex);
            std::filesystem::remove_all(std::filesystem::path(base_path) / std::to_string(world_id));
            cached_worlds.erase(world_id);
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

        std::chrono::nanoseconds worlds_data::apply_tick(std::mt19937& random_engine) {
            std::unique_lock lock(mutex);
            auto current_time = std::chrono::high_resolution_clock::now();
            bool calculate_tps = current_time - last_tps_calculated >= std::chrono::seconds(1);
            std::chrono::nanoseconds time_to_sleep = std::chrono::milliseconds(1000) / 20;

            size_t max_random_tick_for_chunk = 1000;

            list_array<std::pair<int64_t, base_objects::atomic_holder<world_data>>> worlds_to_tick;
            for (auto [id, world] : cached_worlds) {
                auto elapsed = current_time - world->last_usage - world->tick_speed;
                if (elapsed >= std::chrono::milliseconds(0)) {
                    worlds_to_tick.push_back({id, world});
                    if (time_to_sleep < elapsed)
                        time_to_sleep = elapsed;
                }
            }

            lock.unlock();

            list_array<uint64_t> to_unload_worlds;

            for (auto [id, world] : worlds_to_tick) {
                world->tick(max_random_tick_for_chunk, random_engine, current_time, calculate_tps)->wait();
                size_t unload_speed = configuration.world.unload_speed;
                if (world->collect_unused_data(current_time, unload_speed))
                    to_unload_worlds.push_back(id);
            }
            futureForEach(to_unload_worlds, [this](auto id) { save(id); })->wait();
            lock.lock();
            got_ticks++;
            auto new_current_time = std::chrono::high_resolution_clock::now();
            if (calculate_tps) {
                tps = double(got_ticks) / std::chrono::duration_cast<std::chrono::milliseconds>(new_current_time - last_tps_calculated).count() * 1000;
                on_tps_changed.async_notify(tps);
                last_tps_calculated = new_current_time;
            }
            if (new_current_time - current_time > time_to_sleep)
                return std::chrono::nanoseconds(0);
            else
                return time_to_sleep;
        }

#pragma endregion
    } // namespace storage

} // namespace crafted_craft
