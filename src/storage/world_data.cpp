/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <library/enbt/io.hpp>
#include <library/enbt/io_tools.hpp>
#include <library/enbt/senbt.hpp>
#include <library/fast_task/include/files.hpp>
#include <src/api/configuration.hpp>
#include <src/api/tags.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/entity.hpp>
#include <src/log.hpp>
#include <src/mojang/api/hash256.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/task_management.hpp>

namespace enbt::io_helper {
    using namespace copper_server;

    using light_data = base_objects::world::light_data;
    using height_maps = base_objects::world::height_maps;

    template <>
    struct serialization_simple_cast<base_objects::block> {
        static std::uint32_t write_cast(const base_objects::block& value) {
            return value.get_raw();
        }

        static base_objects::block read_cast(std::uint32_t value) {
            base_objects::block bl;
            bl.set_raw(value);
            return bl;
        }
    };

    template <>
    struct serialization_simple_cast<light_data::light_item> {
        static std::uint8_t write_cast(const light_data::light_item& value) {
            return value.light_point;
        }

        static light_data::light_item read_cast(std::uint8_t value) {
            light_data::light_item lit;
            lit.light_point = value;
            return lit;
        }
    };

    template <>
    struct serialization<height_maps> {
        static height_maps read(enbt::io_helper::value_read_stream& self) {
            height_maps height_maps;
            read(height_maps, self);
            return height_maps;
        }

        static void read(height_maps& height_maps, enbt::io_helper::value_read_stream& self) {
            self.iterate(
                [&](std::string_view name, enbt::io_helper::value_read_stream& self) {
                    if (name == "ocean_floor")
                        serialization_read(height_maps.ocean_floor, self);
                    else if (name == "motion_blocking")
                        serialization_read(height_maps.motion_blocking, self);
                    else if (name == "motion_blocking_no_leaves")
                        serialization_read(height_maps.motion_blocking_no_leaves, self);
                    else if (name == "surface")
                        serialization_read(height_maps.surface, self);
                }
            );
        }

        static void write(const height_maps& height_maps, enbt::io_helper::value_write_stream& write_stream) {
            write_stream.write_compound()
                .write("ocean_floor", [&](enbt::io_helper::value_write_stream& write_stream) {
                    serialization_write(height_maps.ocean_floor, write_stream);
                })
                .write("motion_blocking", [&](enbt::io_helper::value_write_stream& write_stream) {
                    serialization_write(height_maps.motion_blocking, write_stream);
                })
                .write("motion_blocking_no_leaves", [&](enbt::io_helper::value_write_stream& write_stream) {
                    serialization_write(height_maps.motion_blocking_no_leaves, write_stream);
                })
                .write("surface", [&](enbt::io_helper::value_write_stream& write_stream) {
                    serialization_write(height_maps.surface, write_stream);
                });
        }
    };

    template <>
    struct serialization<light_data> {
        static light_data read(enbt::io_helper::value_read_stream& self) {
            light_data light_data;
            read(light_data, self);
            return light_data;
        }

        static void read(light_data& light_data, enbt::io_helper::value_read_stream& self) {
            serialization_read(light_data.light_map, self);
        }

        static void write(const light_data& light_data, enbt::io_helper::value_write_stream& write_stream) {
            serialization_write(light_data.light_map, write_stream);
        }
    };

    template <>
    struct serialization<light_data::light_item> {
        static light_data::light_item read(enbt::io_helper::value_read_stream& self) {
            light_data::light_item light_item;
            read(light_item, self);
            return light_item;
        }

        static void read(light_data::light_item& light_data, enbt::io_helper::value_read_stream& self) {
            light_data.light_point = self.read();
        }

        static void write(const light_data::light_item& light_data, enbt::io_helper::value_write_stream& write_stream) {
            write_stream.write(light_data.light_point);
        }
    };

    template <>
    struct serialization<base_objects::block> {
        static base_objects::block read(enbt::io_helper::value_read_stream& self) {
            base_objects::block block;
            read(block, self);
            return block;
        }

        static void read(base_objects::block& block, enbt::io_helper::value_read_stream& self) {
            block.set_raw(self.read());
        }

        static void write(const base_objects::block& block, enbt::io_helper::value_write_stream& write_stream) {
            write_stream.write(block.get_raw());
        }
    };

    template <class T, class Allocator>
    struct serialization<list_array<T, Allocator>> {
        static list_array<T, Allocator> read(enbt::io_helper::value_read_stream& self) {
            list_array<T, Allocator> value;
            read(value, self);
            return value;
        }

        static void read(list_array<T, Allocator>& value, enbt::io_helper::value_read_stream& self) {
            self.iterate(
                [&](std::uint64_t len) { value.reserve(len); },
                [&](value_read_stream& self) { value.push_back(serialization<T>::read(self)); }
            );
        }

        static void write(const list_array<T, Allocator>& value, enbt::io_helper::value_write_stream& write_stream) {
            if constexpr (std::is_integral_v<T>)
                write_stream.write_sarray(value.size()).iterable(value);
            else if constexpr (serialization_simple_cast<T>::value)
                write_stream.write_sarray(value.size()).iterable(value, [](const T& value) { return (typename serialization_simple_cast_data<T>::type)value; });
            else
                write_stream.write_array(value.size()).iterable(value, [](const T& value, value_write_stream& write_stream) { serialization<T>::write(value, write_stream); });
        }
    };
}

namespace copper_server::storage {
    using sub_chunk_data = base_objects::world::sub_chunk_data;

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

#define TO_WORLD_POS_GLOBAL(new_value, raw_value)            \
    int64_t new_value = int64_t(raw_value) + world_y_offset; \
    assert(new_value >= 0 && "Invalid block position, y axis located outside world bound");

#define TO_WORLD_POS_CHUNK(new_value, raw_value)                   \
    int64_t new_value = int64_t(raw_value) + world_y_chunk_offset; \
    assert(new_value >= 0 && "Invalid block position, y axis located outside world bound");


    class world_data;
    class worlds_data;

    bool chunk_data::load(const std::filesystem::path& path, uint64_t tick_counter, world_data& world) {
        if (api::configuration::get().server.world_debug_mode)
            return false;
        if (!std::filesystem::exists(path))
            return false;
        if (std::filesystem::file_size(path) == 0)
            return false;
        fast_task::files::async_iofstream file(
            path,
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            return false;
        std::string mode = enbt::io_helper::read_token(file);

        boost::iostreams::filtering_istream filter;
        if (mode == "zstd")
            filter.push(boost::iostreams::zstd_decompressor());
        else if (mode == "raw")
            ;
        else
            return false;

        filter.push(file);
        uint8_t format_version = enbt::io_helper::read_token(filter);


        enbt::io_helper::value_read_stream stream(filter);
        switch (format_version) {
        case 0:
            stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& self) {
                if (name == "sub_chunks") {
                    self.iterate(
                        [&](enbt::io_helper::value_read_stream& self) {
                            std::shared_ptr<storage::sub_chunk_data> sub_chunk_data = std::make_shared<storage::sub_chunk_data>();
                            bool need_recalculate_light_block_light = true;
                            bool need_recalculate_light_sky_light = true;
                            self.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& self) {
                                if (name == "blocks") {
                                    enbt::io_helper::serialization_read(sub_chunk_data->blocks, self);
                                    for (auto& x : sub_chunk_data->blocks) {
                                        for (auto& y : x) {
                                            for (auto& z : y)
                                                if (bool is_tickable = sub_chunk_data->has_tickable_blocks |= z.is_tickable(); is_tickable)
                                                    break;
                                            if (sub_chunk_data->has_tickable_blocks)
                                                break;
                                        }
                                        if (sub_chunk_data->has_tickable_blocks)
                                            break;
                                    }
                                } else if (name == "block_light") {
                                    try {
                                        enbt::io_helper::serialization_read(sub_chunk_data->block_light, self);
                                        need_recalculate_light_block_light = false;
                                    } catch (...) {
                                    }
                                } else if (name == "sky_light") {
                                    try {
                                        enbt::io_helper::serialization_read(sub_chunk_data->sky_light, self);
                                        need_recalculate_light_block_light = false;
                                    } catch (...) {
                                    }
                                } else if (name == "block_entities") {
                                    self.iterate(
                                        [&](std::uint64_t len) {
                                            sub_chunk_data->block_entities.reserve(len);
                                        },
                                        [&](enbt::io_helper::value_read_stream& self) {
                                            base_objects::local_block_pos local_pos;
                                            struct {
                                                bool x_set = false;
                                                bool y_set = false;
                                                bool z_set = false;
                                                bool id_set = false;
                                            } is_set;
                                            self.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& self) {
                                                if (name == "x") {
                                                    local_pos.x = self.read();
                                                    is_set.x_set = true;
                                                } else if (name == "y") {
                                                    local_pos.y = self.read();
                                                    is_set.y_set = true;
                                                } else if (name == "z") {
                                                    local_pos.z = self.read();
                                                    is_set.z_set = true;
                                                } else if (name == "id") {
                                                    sub_chunk_data->blocks[local_pos.x][local_pos.y][local_pos.z] = base_objects::block{
                                                        (base_objects::block_id_t)self.read()
                                                    };
                                                    is_set.id_set = true;
                                                }
                                            });
                                            if (!is_set.x_set || !is_set.y_set || !is_set.z_set || !is_set.id_set)
                                                throw std::runtime_error("Invalid block entity data");
                                            sub_chunk_data->block_entities[local_pos.z | (local_pos.y << 4) | (local_pos.x << 8)] = self.read();
                                        }
                                    );
                                } else if (name == "biomes")
                                    enbt::io_helper::serialization_read(sub_chunk_data->biomes, self);
                            });
                            sub_chunk_data->need_to_recalculate_light = need_recalculate_light_block_light || need_recalculate_light_sky_light;
                            sub_chunks.push_back(std::move(*sub_chunk_data));
                        }
                    );
                } else if (name == "entities") {
                    self.iterate(
                        [&](std::uint64_t len) {
                            stored_entities.reserve(len);
                        },
                        [&](enbt::io_helper::value_read_stream& self) {
                            auto res = base_objects::entity::load_from_enbt(self.read().as_compound());
                            world.register_entity(res);
                        }
                    );
                } else if (name == "queried_for_tick") {
                    self.iterate(
                        [&](std::uint64_t len) {
                            queried_for_tick.reserve(len);
                        },
                        [&](enbt::io_helper::value_read_stream& self) {
                            list_array<std::pair<uint64_t, base_objects::chunk_block_pos>> queried_for_tick_tmp;
                            self.iterate(
                                [&](std::uint64_t len) {
                                    queried_for_tick_tmp.reserve(len);
                                },
                                [&](enbt::io_helper::value_read_stream& self) {
                                    struct {
                                        bool x_set = false;
                                        bool y_set = false;
                                        bool z_set = false;
                                        bool dur_set = false;
                                    } is_set;
                                    base_objects::chunk_block_pos block_pos;
                                    uint32_t duration;
                                    self.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& self) {
                                        if (name == "x") {
                                            block_pos.x = self.read();
                                            is_set.x_set = true;
                                        } else if (name == "y") {
                                            block_pos.y = self.read();
                                            is_set.y_set = true;
                                        } else if (name == "z") {
                                            block_pos.z = self.read();
                                            is_set.z_set = true;
                                        } else if (name == "duration") {
                                            duration = self.read();
                                            is_set.dur_set = true;
                                        }
                                    });
                                    if (!is_set.x_set || !is_set.y_set || !is_set.z_set || !is_set.dur_set)
                                        throw std::runtime_error("Invalid queried_for_tick data");
                                    queried_for_tick_tmp.push_back({tick_counter + duration, block_pos});
                                }
                            );
                            queried_for_tick.push_back(std::move(queried_for_tick_tmp));
                        }
                    );
                } else if (name == "height_maps") {
                    enbt::io_helper::serialization_read(height_maps, self);
                } else if (name == "generator_stage") {
                    generator_stage = self.read();
                } else if (name == "resume_gen_level")
                    resume_gen_level = self.read();
            });
        }
        return true;
    }

    bool valid_sub_chunk_size(const enbt::value& chunk) {
        try {
            auto dim_0 = chunk.as_array();
            if (dim_0.size() != 16)
                return false;
            for (auto& x : dim_0) {
                auto dim_1 = x.as_array();
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

    void load_light_data(const enbt::value& chunk, base_objects::world::light_data& data, bool& need_to_recalculate_light) {
        if (!valid_sub_chunk_size(chunk)) {
            need_to_recalculate_light = true;
            return;
        }

        size_t x_ = 0;
        for (auto& x : chunk.as_array()) {
            size_t y_ = 0;
            for (auto& y : x.as_array()) {
                size_t z_ = 0;
                for (auto& z : y.as_ui8_array())
                    data.light_map[x_][y_][z_++].light_point = z;
                ++y_;
            }
            ++x_;
        }
    }

    void load_block_data(const enbt::value& chunk, base_objects::block (&data)[16][16][16], bool& has_tickable_blocks) {
        size_t x_ = 0;
        for (auto& x : chunk.as_array()) {
            size_t y_ = 0;
            for (auto& y : x.as_array()) {
                size_t z_ = 0;
                for (auto z : y.as_ui32_array()) {
                    data[x_][y_][z_].set_raw(z);
                    has_tickable_blocks = data[x_][y_][z_].is_tickable();
                    ++z_;
                }
                ++y_;
            }
            ++x_;
        }
    }

    bool chunk_data::load(const enbt::compound_const_ref& chunk_data, uint64_t tick_counter, world_data& world) {
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
                    auto entity_ref = base_objects::entity::load_from_enbt(entity.as_compound());
                    world.register_entity(entity_ref);
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
            sub_chunks.push_back(std::move(*sub_chunk_data));
        }

        sub_chunks.resize(world.get_chunk_y_count());
        if (chunk_data.contains("queried_for_tick")) {
            auto queried_for_tick_ref = chunk_data["queried_for_tick"].as_fixed_array();
            queried_for_tick.reserve(queried_for_tick_ref.size());
            for (auto& inner : queried_for_tick_ref) {
                list_array<std::pair<uint64_t, base_objects::chunk_block_pos>> queried_for_tick_tmp;
                queried_for_tick_tmp.reserve(inner.size());
                for (auto& item : inner.as_array()) {
                    base_objects::chunk_block_pos block_pos{item.at("x"), item.at("y"), item.at("z")};
                    uint32_t duration = item.at("duration");
                    queried_for_tick_tmp.push_back({duration + tick_counter, block_pos});
                }
                queried_for_tick.push_back(queried_for_tick_tmp.take());
            }
        }


        if (chunk_data.contains("generator_stage"))
            generator_stage = chunk_data["generator_stage"];

        if (chunk_data.contains("resume_gen_level"))
            resume_gen_level = chunk_data["resume_gen_level"];
        return true;
    }

    bool chunk_data::save(const std::filesystem::path& path, uint64_t tick_counter, world_data& _) {
        if (api::configuration::get().server.world_debug_mode)
            return false;
        std::filesystem::create_directories(path.parent_path());
        fast_task::files::async_iofstream file(
            path,
            fast_task::files::open_mode::write,
            fast_task::files::on_open_action::always_new,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            return false;
        auto mode = api::configuration::get().world.saving_mode;
        enbt::io_helper::write_token(file, mode);
        boost::iostreams::filtering_ostream filter;
        if (mode == "zstd")
            filter.push(boost::iostreams::zstd_compressor());
        filter.push(file);
        enbt::io_helper::write_token(filter, 0ui8);
        std::stringstream ss;
        enbt::io_helper::value_write_stream stream(ss);
        {
            auto comp
                = stream.write_compound()
                      .write("sub_chunks", [&](enbt::io_helper::value_write_stream& stream) {
                          stream.write_array(sub_chunks.size()).iterable(sub_chunks, [&](const storage::sub_chunk_data& sub_chunk, enbt::io_helper::value_write_stream& stream) {
                              auto compound = stream.write_compound();
                              compound.write("blocks", [&](enbt::io_helper::value_write_stream& stream) {
                                  enbt::io_helper::serialization_write(sub_chunk.blocks, stream);
                              });
                              compound.write("block_entities", [&](enbt::io_helper::value_write_stream& stream) {
                                  stream.write_array(sub_chunk.block_entities.size()).iterable(sub_chunk.block_entities, [&](auto& item, enbt::io_helper::value_write_stream& stream) {
                                      auto& [_pos, data] = item;
                                      base_objects::local_block_pos pos;
                                      pos.x = _pos >> 8;
                                      pos.y = (_pos >> 4) & 0xF;
                                      pos.z = _pos & 0xF;

                                      auto compound = stream.write_compound();
                                      compound.write("id", sub_chunk.blocks[pos.x][pos.y][pos.z].id);
                                      compound.write("state", sub_chunk.blocks[pos.x][pos.y][pos.z].block_state_data);
                                      compound.write("nbt", data);
                                      compound.write("x", pos.x);
                                      compound.write("y", pos.y);
                                      compound.write("z", pos.z);
                                  });
                              });
                              compound.write("biomes", [&](enbt::io_helper::value_write_stream& stream) {
                                  enbt::io_helper::serialization_write(sub_chunk.biomes, stream);
                              });
                              if (!sub_chunk.need_to_recalculate_light) {
                                  compound.write("block_light", [&](enbt::io_helper::value_write_stream& stream) {
                                      enbt::io_helper::serialization_write(sub_chunk.block_light.light_map, stream);
                                  });
                                  compound.write("sky_light", [&](enbt::io_helper::value_write_stream& stream) {
                                      enbt::io_helper::serialization_write(sub_chunk.block_light.light_map, stream);
                                  });
                              }
                          });
                      })
                      .write("entities", [&](enbt::io_helper::value_write_stream& stream) {
                          auto entities = stream.write_array(stored_entities.size());
                          for (auto& [id, entity] : stored_entities) {
                              if (entity->const_data().is_saveable)
                                  entities.write(entity->copy_to_enbt());
                          }
                      })
                      .write("queried_for_tick", [&](enbt::io_helper::value_write_stream& stream) {
                          stream.write_array(queried_for_tick.size()).iterable(queried_for_tick, [&](auto& item, enbt::io_helper::value_write_stream& stream) {
                              stream.write_array(item.size()).iterable(item, [&](auto& item, enbt::io_helper::value_write_stream& stream) {
                                  auto& [till_tick, block_pos] = item;
                                  auto compound = stream.write_compound();
                                  compound.write("x", block_pos.x);
                                  compound.write("y", block_pos.y);
                                  compound.write("z", block_pos.z);
                                  compound.write("duration", till_tick - tick_counter);
                              });
                          });
                      })
                      .write("height_maps", [&](enbt::io_helper::value_write_stream& stream) {
                          serialization_write(height_maps, stream);
                      });
            if (generator_stage != 0xFF)
                comp.write("generator_stage", generator_stage);
            if (resume_gen_level != 255)
                comp.write("resume_gen_level", resume_gen_level);
        }


        auto tmp = ss.str();
        filter.write(tmp.c_str(), tmp.size());
        return true;
    }

    chunk_data::chunk_data(int64_t chunk_x, int64_t chunk_z)
        : chunk_x(chunk_x), chunk_z(chunk_z) {}

    void chunk_data::update_height_map_on(uint8_t local_x, uint64_t local_y, uint8_t local_z) {
        uint64_t to_skip = local_y;
        uint64_t local_y_block = local_y * 16;
        auto& leaves = api::tags::unfold_tag(api::tags::builtin_entry::block, "minecraft:block/leaves");
        auto end = sub_chunks.rend();

        for (auto beg = sub_chunks.rbegin(); beg != end; beg++) {
            if (to_skip) {
                --to_skip;
                continue;
            }
            auto& schunk = *beg;
            for (int8_t y = 15; y >= 0; y--) {
                auto block = schunk.blocks[local_x][y][local_z];
                if (!block.is_air()) {
                    auto y_pos = y + local_y_block;

                    if (!height_maps.ocean_floor[local_x][local_z])
                        height_maps.ocean_floor[local_x][local_z] = y_pos;

                    if (block.is_liquid())
                        if (!height_maps.surface[local_x][local_z])
                            height_maps.surface[local_x][local_z] = y_pos;

                    if (block.is_solid()) {
                        if (!height_maps.motion_blocking[local_x][local_z])
                            height_maps.motion_blocking[local_x][local_z] = y_pos;

                        if (!leaves.contains(block.general_block_id()))
                            if (!height_maps.motion_blocking_no_leaves[local_x][local_z])
                                height_maps.motion_blocking_no_leaves[local_x][local_z] = y_pos;
                    }
                }
            }
        }
    }

    void chunk_data::update_height_map() {
        height_maps.make_zero();
        uint64_t local_y_block = (sub_chunks.size() - 1) * 16;
        auto& leaves = api::tags::unfold_tag(api::tags::builtin_entry::block, "minecraft:block/leaves");
        auto end = sub_chunks.rend();
        for (auto beg = sub_chunks.rbegin(); beg != end; beg++) {
            auto& schunk = *beg;
            for (uint8_t x = 0; x < 16; x++) {
                for (int8_t y = 15; y >= 0; y--) {
                    for (uint8_t z = 0; z < 16; z++) {
                        auto block = schunk.blocks[x][y][z];
                        if (!block.is_air()) {
                            auto y_pos = y + local_y_block;

                            if (!height_maps.ocean_floor[x][z])
                                height_maps.ocean_floor[x][z] = y_pos;

                            if (block.is_liquid())
                                if (!height_maps.surface[x][z])
                                    height_maps.surface[x][z] = y_pos;

                            if (block.is_solid()) {
                                if (!height_maps.motion_blocking[x][z])
                                    height_maps.motion_blocking[x][z] = y_pos;

                                if (!leaves.contains(block.general_block_id()))
                                    if (!height_maps.motion_blocking_no_leaves[x][z])
                                        height_maps.motion_blocking_no_leaves[x][z] = y_pos;
                            }
                        }
                    }
                }
            }
        }
    }

    void chunk_data::for_each_entity(std::function<void(base_objects::entity_ref& entity)> func) {
        for (auto& [id, entity] : stored_entities)
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

    void chunk_data::for_each_block_entity(uint64_t local_y, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        if (local_y < sub_chunks.size())
            for (auto& [_pos, data] : sub_chunks[local_y].block_entities) {
                base_objects::local_block_pos pos;
                pos.x = _pos >> 8;
                pos.y = (_pos >> 4) & 0xF;
                pos.z = _pos & 0xF;
                func(sub_chunks[local_y].blocks[pos.x][pos.y][pos.z], data);
            }
    }

    void chunk_data::for_each_sub_chunk(std::function<void(sub_chunk_data& sub_chunk)> func) {
        for (auto& sub_chunk : sub_chunks)
            func(sub_chunk);
    }

    void chunk_data::get_sub_chunk(uint64_t sub_chunk_y, std::function<void(sub_chunk_data& sub_chunk)> func) {
        if (sub_chunk_y < sub_chunks.size())
            func(sub_chunks[sub_chunk_y]);
    }

    void chunk_data::query_for_tick(uint8_t local_x, uint64_t global_y, uint8_t local_z, uint64_t on_tick, int8_t priority) {
        if (priority > 0)
            throw std::runtime_error("Priority must be negative");
        uint8_t real_priority = +priority;
        if (real_priority >= queried_for_tick.size())
            queried_for_tick.resize(real_priority + 1);
        queried_for_tick[real_priority].push_back({on_tick, base_objects::chunk_block_pos{local_x, uint8_t(global_y & 15), local_z}});
    }

    void chunk_data::query_for_liquid_tick(uint8_t local_x, uint64_t global_y, uint8_t local_z, uint64_t on_tick) {
        queried_for_liquid_tick.push_back({on_tick, base_objects::chunk_block_pos{local_x, uint8_t(global_y & 15), local_z}});
    }

    void chunk_data::tick(world_data& world, size_t random_tick_speed, std::mt19937& random_engine, [[maybe_unused]] std::chrono::high_resolution_clock::time_point current_time) {
        if (load_level > 32)
            return;

        for (auto& priority : queried_for_tick) {
            for (
                auto& [till, block_pos] :
                priority.take([&world](auto& it) {
                    return it.first >= world.tick_counter;
                })
            ) {
                auto sub_chunk_y = convert_chunk_global_pos(block_pos.y);
                auto local = convert_chunk_local_pos(block_pos.y);
                auto& sub_chunk = sub_chunks.at(sub_chunk_y);

                sub_chunk.blocks[block_pos.x][local][block_pos.z].tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, block_pos.x, (uint8_t)local, block_pos.z, false);
            }
        }

        for (
            auto& [till, block_pos] :
            queried_for_liquid_tick.take([&world](auto& it) {
                return it.first >= world.tick_counter;
            })
        ) {
            auto sub_chunk_y = convert_chunk_global_pos(block_pos.y);
            auto local = convert_chunk_local_pos(block_pos.y);
            auto& sub_chunk = sub_chunks.at(sub_chunk_y);

            sub_chunk.blocks[block_pos.x][local][block_pos.z].tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, block_pos.x, (uint8_t)local, block_pos.z, false);
        }

        if (load_level <= 31)
            for (auto& [id, entity] : stored_entities)
                entity->tick();
        uint64_t sub_chunk_y = 0;
        for (auto& sub_chunk : sub_chunks) {
            auto max_random_tick_per_sub_chunk = random_tick_speed;
            while (sub_chunk.has_tickable_blocks && max_random_tick_per_sub_chunk) {
                union {
                    struct {
                        uint8_t x;
                        uint8_t y;
                        uint8_t z;
                    } dec;

                    uint32_t value;
                } pos;

                pos.value = random_engine();
                if (sub_chunk.blocks[pos.dec.x][pos.dec.y][pos.dec.z].tickable != base_objects::block::tick_opt::no_tick)
                    sub_chunk.blocks[pos.dec.x][pos.dec.y][pos.dec.z].tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, pos.dec.x, pos.dec.y, pos.dec.z, true);
                --max_random_tick_per_sub_chunk;
            }
            sub_chunk_y++;
        }
    }

    //generator functions
    void chunk_data::gen_set_block(const base_objects::full_block_data& block, uint8_t local_x, uint64_t local_y, uint8_t local_z) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, block);
    }

    void chunk_data::gen_set_block(base_objects::full_block_data&& block, uint8_t local_x, uint64_t local_y, uint8_t local_z) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, std::move(block));
    }

    void chunk_data::gen_remove_block(uint8_t local_x, uint64_t local_y, uint8_t local_z) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, base_objects::block());
    }

    base_objects::full_block_data_ref chunk_data::gen_get_block(uint8_t local_x, uint64_t local_y, uint8_t local_z) {
        std::optional<base_objects::full_block_data_ref> res;
        sub_chunks.at(local_y >> 4).get_block(local_x, local_y & 15, local_z, [&res](auto& block) { res = block; }, [&res](auto& block, auto& enbt) { res.emplace(base_objects::block_entity_ref(block, enbt)); });
        return *res;
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

    fast_task::protected_value<std::unordered_map<std::string, base_objects::atomic_holder<chunk_light_processor>>> light_processors;

    void chunk_light_processor::register_it(const std::string& id, base_objects::atomic_holder<chunk_light_processor> processor) {
        light_processors.set([&](auto& map) {
            map[id] = std::move(processor);
        });
    }

    void chunk_light_processor::unregister_it(const std::string& id) {
        light_processors.set([&](auto& map) {
            map.erase(id);
        });
    }

    base_objects::atomic_holder<chunk_light_processor> chunk_light_processor::get_it(const std::string& id) {
        return light_processors.set([&](auto& map) {
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
                        if (chunk)
                            chunk->save(path / "chunks" / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"), tick_counter, *this);
                    } catch (...) {
                        return false;
                    }
                    std::unique_lock lock(mutex);
                    on_save_process.erase({chunk_x, chunk_z});
                    if (also_unload) {
                        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end()) {
                            x_axis->second.erase(chunk_z);
                            if (profiling.enable_world_profiling) {
                                if (profiling.chunk_total_loaded)
                                    --profiling.chunk_total_loaded;
                                if (profiling.chunk_unloaded)
                                    profiling.chunk_unloaded(*this, chunk_x, chunk_z);
                            }
                        }
                    }

                    return true;
                }
            );
        }
    }

    FuturePtr<base_objects::atomic_holder<chunk_data>> world_data::create_chunk_generate_future(base_objects::atomic_holder<chunk_data>& chunk) {
        if (profiling.enable_world_profiling)
            ++profiling.chunk_generator_counter;

        return Future<base_objects::atomic_holder<chunk_data>>::start(
            [this, chunk = chunk]() {
                auto gen = get_generator();
                fast_task::mutex_unify unify(generator.mutex);
                std::unique_lock lock(unify);
                ++generator.count;
                generator.chunks_next_blocking_stage = generator.lowest_sync_stage;
                while (chunk->generator_stage != 0xFF) {
                    while (chunk->generator_stage >= generator.chunks_next_blocking_stage && !generator.sync_mode) { //sync all tasks to switch mode
                        ++generator.lock_count;
                        if (generator.lock_count < generator.count)
                            generator.notifier.wait(lock);
                        else {
                            generator.sync_mode = true;
                            generator.notifier.notify_all();
                        }
                        --generator.lock_count;
                    }

                    while (chunk->generator_stage > generator.chunks_next_blocking_stage) { //block all tasks that has too big mode
                        ++generator.stage_complete_count;
                        if (generator.stage_complete_count < generator.count)
                            generator.notifier.wait(lock);
                        else {
                            generator.next_stage_sync();
                            generator.notifier.notify_all();
                        }
                        --generator.stage_complete_count;
                    }

                    bool make_lock = generator.sync_mode && chunk->generator_stage == generator.chunks_next_blocking_stage;
                    lock.unlock();
                    std::unique_lock sync_guard(generator.limiter, std::defer_lock);
                    if (make_lock)
                        sync_guard.lock();
                    gen->process_chunk(*this, *chunk, chunk->generator_stage);
                    if (chunk->load_level > chunk->resume_gen_level)
                        break;
                }
                --generator.count;
                if (profiling.enable_world_profiling)
                    --profiling.chunk_generator_counter;
                return chunk;
            }
        );
    }

    base_objects::atomic_holder<chunk_data> world_data::load_chunk_sync(int64_t chunk_x, int64_t chunk_z) {
        try {
            auto chunk = base_objects::atomic_holder<chunk_data>(new chunk_data(chunk_x, chunk_z));
            if (!chunk->load(path / "chunks" / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"), tick_counter, *this)) {
                if (!chunk->load(get_generator()->generate_chunk(*this, chunk_x, chunk_z), tick_counter, *this))
                    return nullptr;
            }
            if (chunk->generator_stage != 0xFF) {
                chunk->load_level = 31;
                std::unique_lock lock(mutex);
                if (auto process = on_generate_process.find({chunk_x, chunk_z}); process == on_generate_process.end()) {
                    chunks[chunk_x][chunk_z] = chunk;
                    auto it = on_generate_process[{chunk_x, chunk_z}] = create_chunk_generate_future(chunk);
                    it->wait_with(lock);
                    on_generate_process.erase({chunk_x, chunk_z});
                    return it->get();
                } else {
                    auto fut = process->second;
                    fut->wait_with(lock);
                    return fut->get();
                }
            } else {
                chunk->load_level = 34;
                get_light_processor();
                uint64_t y = chunk->sub_chunks.size();
                auto end = chunk->sub_chunks.rend();
                bool done_process = false;
                for (auto beg = chunk->sub_chunks.rbegin(); beg != end; beg++) {
                    --y;
                    if (beg->need_to_recalculate_light || done_process) {
                        light_processor->process_sub_chunk(*this, chunk_x, y, chunk_z);
                        done_process = true;
                    }
                }
                chunk->update_height_map();
            }
            return chunk;
        } catch (...) {
            return nullptr;
        }
    }

    base_objects::atomic_holder<chunk_generator>& world_data::get_generator() {
        if (!generator.process) {
            generator.process = chunk_generator::get_it(light_processor_id);
            generator.calculate();
        }
        return generator.process;
    }

    base_objects::atomic_holder<chunk_light_processor>& world_data::get_light_processor() {
        if (!light_processor) {
            light_processor = chunk_light_processor::get_it(light_processor_id);
            enable_entity_light_source_updates = light_processor->enable_entity_light_source_updates;
            enable_entity_light_source_updates_include_rot = light_processor->enable_entity_light_source_updates_include_rot;
        }
        return light_processor;
    }

    template <auto fun, class... Args>
    inline void entity_notify_block(auto& entities, auto& self, auto x, auto y, auto z, Args&&... args) {
        auto chunk_x = convert_chunk_global_pos(x);
        auto chunk_z = convert_chunk_global_pos(z);
        for (auto& [id, entity] : entities) {
            if (&*entity != &self) {
                auto processor = entity->const_data().processor;
                if (entity->world_syncing_data && processor)
                    if (entity->world_syncing_data->processing_region.in_bounds(chunk_x, chunk_z))
                        ((*processor).*fun)(*entity, self, x, y, z, std::forward<Args>(args)...);
            }
        }
    }

    template <auto fun, class... Args>
    inline void entity_notify_change(auto& entities, auto& self, Args&&... args) {
        auto chunk_x = convert_chunk_global_pos(self.position.x);
        auto chunk_z = convert_chunk_global_pos(self.position.z);
        for (auto& [id, entity] : entities) {
            if (&*entity != &self) {
                auto processor = entity->const_data().processor;
                if (entity->world_syncing_data && processor)
                    if (entity->world_syncing_data->processing_region.in_bounds((int64_t)chunk_x, (int64_t)chunk_z))
                        ((*processor).*fun)(*entity, self, std::forward<Args>(args)...);
            }
        }
    }

    template <auto fun, class... Args>
    inline void entity_notify_change_all(auto& entities, auto& self, Args&&... args) {
        auto chunk_x = convert_chunk_global_pos(self.position.x);
        auto chunk_z = convert_chunk_global_pos(self.position.z);
        for (auto& [id, entity] : entities) {
            auto processor = entity->const_data().processor;
            if (entity->world_syncing_data && processor)
                if (entity->world_syncing_data->processing_region.in_bounds((int64_t)chunk_x, (int64_t)chunk_z))
                    ((*processor).*fun)(*entity, self, std::forward<Args>(args)...);
        }
    }

    template <auto fun, class... Args>
    void entity_notify_change_w_e(auto& entities, auto& self, auto other_entity_id, Args&&... args) {
        auto other_entity_it = entities.find(other_entity_id);
        if (other_entity_it == entities.end())
            throw std::runtime_error("Entity not registered on world");
        auto chunk_x = convert_chunk_global_pos(self.position.x);
        auto chunk_z = convert_chunk_global_pos(self.position.z);
        auto& other_entity = other_entity_it->second;
        for (auto& [id, entity] : entities) {
            auto processor = entity->const_data().processor;
            if (entity->world_syncing_data && processor)
                if (entity->world_syncing_data->processing_region.in_bounds((int64_t)chunk_x, (int64_t)chunk_z))
                    ((*processor).*fun)(*entity, self, other_entity, std::forward<Args>(args)...);
        }
    }

    template <auto fun, class... Args>
    void world_notify(auto& entities, auto x, auto z, Args&&... args) {
        auto chunk_x = convert_chunk_global_pos(x);
        auto chunk_z = convert_chunk_global_pos(z);
        for (auto& [id, entity] : entities) {
            auto processor = entity->const_data().processor;
            if (entity->world_syncing_data && processor) {
                if (entity->world_syncing_data->processing_region.in_bounds(chunk_x, chunk_z))
                    ((*processor).*fun)(*entity, std::forward<Args>(args)...);
            }
        }
    }

#define WORLD_ASYNC_RUN(function, ...) \
    fast_task::task::run([=, this] { api::world::get(world_id, [&](auto& world) { world.function(__VA_ARGS__); }); })

    void world_data::entity_init(base_objects::entity& self) {
        std::unique_lock lock(mutex);
        auto chunk_x = convert_chunk_global_pos(self.position.x);
        auto chunk_z = convert_chunk_global_pos(self.position.z);
        for (auto& [id, entity] : entities) {
            auto processor = entity->const_data().processor;
            if (entity->world_syncing_data && processor) {
                if (entity->world_syncing_data->processing_region.in_bounds((int64_t)chunk_x, (int64_t)chunk_z))
                    processor->entity_init(*entity, self);
            }
        }
    }

    using ew_processor = base_objects::entity_data::world_processor;

    void world_data::entity_teleport(base_objects::entity& self, util::VECTOR new_pos) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_teleport>(entities, self, new_pos);
        if (enable_entity_light_source_updates)
            get_light_processor()->process_entity_light_source(*this, self, new_pos);
    }

    void world_data::entity_move(base_objects::entity& self, util::VECTOR move) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_move>(entities, self, move);
        if (enable_entity_light_source_updates)
            get_light_processor()->process_entity_light_source(*this, self, move);
    }

    void world_data::entity_look_changes(base_objects::entity& self, util::ANGLE_DEG new_rotation) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_look_changes>(entities, self, new_rotation);
        if (enable_entity_light_source_updates_include_rot)
            get_light_processor()->process_entity_light_source_rot(*this, self, new_rotation);
    }

    void world_data::entity_rotation_changes(base_objects::entity& self, util::ANGLE_DEG new_rotation) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_rotation_changes>(entities, self, new_rotation);
    }

    void world_data::entity_motion_changes(base_objects::entity& self, util::VECTOR new_motion) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_motion_changes>(entities, self, new_motion);
    }

    void world_data::entity_rides(base_objects::entity& self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entities.at(other_entity_id)->ride_by_entity.push_back(entities.at(self.world_syncing_data->assigned_world_id));
        entity_notify_change_w_e<&ew_processor::entity_rides>(entities, self, other_entity_id);
    }

    void world_data::entity_leaves_ride(base_objects::entity& self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entities.at(other_entity_id)->ride_by_entity.remove_if([&self](auto& it) {
            return &*it == &self;
        });
        entity_notify_change_w_e<&ew_processor::entity_leaves_ride>(entities, self, other_entity_id);
    }

    void world_data::entity_attach(base_objects::entity& self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entity_notify_change_w_e<&ew_processor::entity_attach>(entities, self, other_entity_id);
    }

    void world_data::entity_detach(base_objects::entity& self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entity_notify_change_w_e<&ew_processor::entity_detach>(entities, self, other_entity_id);
    }

    void world_data::entity_damage(base_objects::entity& self, float health, int32_t type_id, std::optional<util::VECTOR> pos) {
        entity_notify_change_all<&ew_processor::entity_damage>(entities, self, health, type_id, pos);
    }

    void world_data::entity_damage(base_objects::entity& self, float health, int32_t type_id, base_objects::entity_ref& source, std::optional<util::VECTOR> pos) {
        entity_notify_change_all<&ew_processor::entity_damage_with_source>(entities, self, health, type_id, source, pos);
    }

    void world_data::entity_damage(base_objects::entity& self, float health, int32_t type_id, base_objects::entity_ref& source, base_objects::entity_ref& source_direct, std::optional<util::VECTOR> pos) {
        entity_notify_change_all<&ew_processor::entity_damage_with_sources>(entities, self, health, type_id, source, source_direct, pos);
    }

    void world_data::entity_attack(base_objects::entity& self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entity_notify_change_w_e<&ew_processor::entity_attack>(entities, self, other_entity_id);
    }

    void world_data::entity_iteract(base_objects::entity& self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entity_notify_change_w_e<&ew_processor::entity_iteract>(entities, self, other_entity_id);
    }

    void world_data::entity_iteract(base_objects::entity& self, int64_t x, int64_t y, int64_t z) {
        std::unique_lock lock(mutex);
        entity_notify_block<&ew_processor::entity_iteract_block>(entities, self, x, y, z);
    }

    void world_data::entity_break(base_objects::entity& self, int64_t x, int64_t y, int64_t z, uint8_t state) {
        if (state > 9)
            return;
        std::unique_lock lock(mutex);
        entity_notify_block<&ew_processor::entity_break>(entities, self, x, y, z, state);
    }

    void world_data::entity_cancel_break(base_objects::entity& self, int64_t x, int64_t y, int64_t z) {
        std::unique_lock lock(mutex);
        entity_notify_block<&ew_processor::entity_cancel_break>(entities, self, x, y, z);
    }

    void world_data::entity_finish_break(base_objects::entity& self, int64_t x, int64_t y, int64_t z) {
        std::unique_lock lock(mutex);
        entity_notify_block<&ew_processor::entity_finish_break>(entities, self, x, y, z);
    }

    void world_data::entity_place(base_objects::entity& self, bool is_main_hand, int64_t x, int64_t y, int64_t z, base_objects::block block) {
        std::unique_lock lock(mutex);
        for (auto& [id, entity] : entities)
            if (&*entity != &self) {
                auto processor = entity->const_data().processor;
                if (entity->world_syncing_data && processor) {
                    if (entity->world_syncing_data->processing_region.in_bounds(convert_chunk_global_pos(x), convert_chunk_global_pos(z)))
                        processor->entity_place_block(*entity, self, is_main_hand, x, y, z, block);
                }
            }
    }

    void world_data::entity_place(base_objects::entity& self, bool is_main_hand, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block) {
        std::unique_lock lock(mutex);
        for (auto& [id, entity] : entities)
            if (&*entity != &self) {
                auto processor = entity->const_data().processor;
                if (entity->world_syncing_data && processor) {
                    if (entity->world_syncing_data->processing_region.in_bounds(convert_chunk_global_pos(x), convert_chunk_global_pos(z)))
                        processor->entity_place_block_entity(self, *entity, is_main_hand, x, y, z, block);
                }
            }
    }

    void world_data::entity_animation(base_objects::entity& self, base_objects::entity_animation animation) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_animation>(entities, self, animation);
    }

    void world_data::entity_event(base_objects::entity& self, base_objects::entity_event status) {
        entity_notify_change<&ew_processor::entity_event>(entities, self, status);
    }

    void world_data::entity_add_effect(base_objects::entity& self, uint32_t effect_id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
        entity_notify_change_all<&ew_processor::entity_add_effect>(entities, self, effect_id, duration, amplifier, ambient, show_particles, show_icon, use_blend);
    }

    void world_data::entity_remove_effect(base_objects::entity& self, uint32_t effect_id) {
        entity_notify_change_all<&ew_processor::entity_remove_effect>(entities, self, effect_id);
    }

    void world_data::entity_death(base_objects::entity& self) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_death>(entities, self);
    }

    void world_data::entity_deinit(base_objects::entity& self) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_deinit>(entities, self);
    }

    void world_data::notify_block_event(const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_event>(entities, x, z, action, x, y, z);
    }

    void world_data::notify_block_change(int64_t x, int64_t y, int64_t z, base_objects::block block) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_change>(entities, x, z, x, y, z, block);
    }

    void world_data::notify_block_change(int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_entity_change>(entities, x, z, x, y, z, block);
    }

    void world_data::notify_block_destroy_change(int64_t x, int64_t y, int64_t z, base_objects::block block) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_destroy_change>(entities, x, z, x, y, z, block);
    }

    void world_data::notify_block_destroy_change(int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_entity_destroy_change>(entities, x, z, x, y, z, block);
    }

    void world_data::notify_biome_change(int64_t x, int64_t y, int64_t z, uint32_t biome_id) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_biome_change>(entities, x, z, x, y, z, biome_id);
    }

    void world_data::notify_sub_chunk(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z) {
        get_sub_chunk(
            chunk_x,
            chunk_y,
            chunk_z,
            [this, chunk_x, chunk_y, chunk_z](auto& sub_chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity->const_data().processor;
                    if (entity->world_syncing_data && processor) {
                        if (entity->world_syncing_data->processing_region.in_bounds(chunk_x, chunk_z))
                            processor->notify_sub_chunk(*entity, chunk_x, chunk_y, chunk_z, sub_chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_chunk(int64_t chunk_x, int64_t chunk_z) {
        get_chunk(
            chunk_x,
            chunk_z,
            [this, chunk_x, chunk_z](auto& chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity->const_data().processor;
                    if (entity->world_syncing_data && processor) {
                        if (entity->world_syncing_data->processing_region.in_bounds(chunk_x, chunk_z))
                            processor->notify_chunk(*entity, chunk_x, chunk_z, chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_sub_chunk_light(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z) {
        get_sub_chunk(
            chunk_x,
            chunk_y,
            chunk_z,
            [this, chunk_x, chunk_y, chunk_z](auto& sub_chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity->const_data().processor;
                    if (entity->world_syncing_data && processor) {
                        if (entity->world_syncing_data->processing_region.in_bounds(chunk_x, chunk_z))
                            processor->notify_sub_chunk_light(*entity, chunk_x, chunk_y, chunk_z, sub_chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_chunk_light(int64_t chunk_x, int64_t chunk_z) {
        get_chunk(
            chunk_x,
            chunk_z,
            [this, chunk_x, chunk_z](auto& chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity->const_data().processor;
                    if (entity->world_syncing_data && processor) {
                        if (entity->world_syncing_data->processing_region.in_bounds(chunk_x, chunk_z))
                            processor->notify_chunk_light(*entity, chunk_x, chunk_z, chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_sub_chunk_blocks(int64_t chunk_x, int64_t chunk_y, int64_t chunk_z) {
        get_sub_chunk(
            chunk_x,
            chunk_y,
            chunk_z,
            [this, chunk_x, chunk_y, chunk_z](auto& sub_chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity->const_data().processor;
                    if (entity->world_syncing_data && processor) {
                        if (entity->world_syncing_data->processing_region.in_bounds(chunk_x, chunk_z))
                            processor->notify_sub_chunk_blocks(*entity, chunk_x, chunk_y, chunk_z, sub_chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_chunk_blocks(int64_t chunk_x, int64_t chunk_z) {
        get_chunk(
            chunk_x,
            chunk_z,
            [this, chunk_x, chunk_z](auto& chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity->const_data().processor;
                    if (entity->world_syncing_data && processor) {
                        if (entity->world_syncing_data->processing_region.in_bounds(chunk_x, chunk_z))
                            processor->notify_chunk_blocks(*entity, chunk_x, chunk_z, chunk);
                    }
                }
            }
        );
    }

    void world_data::__set_block_silent(const base_objects::full_block_data& block, int64_t global_x, int64_t global_y_raw, int64_t global_z, block_set_mode mode) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, block);
        });

        std::visit(
            [&](auto& b) {
                if (mode == block_set_mode::destroy)
                    WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y_raw, global_z, b);
                __update_block(global_x, global_y_raw, global_z, mode, b.general_block_id());
            },
            block
        );
    }

    void world_data::__set_block_silent(base_objects::full_block_data&& block, int64_t global_x, int64_t global_y_raw, int64_t global_z, block_set_mode mode) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        auto b = std::visit(
            [&](auto& b) {
                if (mode == block_set_mode::destroy)
                    WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y_raw, global_z, b);
                return b.general_block_id();
            },
            block
        );
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, block);
        });
        __update_block(global_x, global_y_raw, global_z, mode, b);
    }

    void world_data::__update_block(int64_t global_x, int64_t global_y_raw, int64_t global_z, block_set_mode mode, base_objects::block_id_t b) {
        if (mode != block_set_mode::keep) {
            if (base_objects::block::get_general_block(b).is_liquid && general_world_data.liquid.contains(b))
                query_for_liquid_tick(global_x, global_y_raw, global_z, tick_counter + general_world_data.liquid.at(b).spread_ticks);
            else
                query_for_tick(global_x, global_y_raw, global_z, tick_counter);
        }
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

    void world_data::set_seed(int32_t seed) {
        world_seed = seed;
        mojang::api::hash256 hash;
        hash.update(&seed, sizeof(int32_t));
        hashed_seed_value = hash.to_part_hash();
    }

    void world_data::load(const enbt::compound_const_ref& load_from_nbt) {
        general_world_data.other = load_from_nbt.at("general_world_data");
        if (general_world_data.other.contains("liquid"))
            for (auto& [block, settings] : general_world_data.other.at("liquid").as_compound())
                general_world_data.liquid[base_objects::block::get_block(block).general_block_id] = {.spread_size = settings.at("spread_size"), .spread_ticks = settings.at("spread_ticks")};
        else
            general_world_data.liquid.clear();
        world_game_rules = load_from_nbt.at("world_game_rules");
        world_generator_data = load_from_nbt.at("world_generator_data");
        world_records = load_from_nbt.at("world_records");

        set_seed(load_from_nbt.at("world_seed"));
        wandering_trader_id = load_from_nbt.at("wandering_trader_id");


        wandering_trader_spawn_chance = load_from_nbt.at("wandering_trader_spawn_chance");
        wandering_trader_spawn_delay = load_from_nbt.at("wandering_trader_spawn_delay");
        world_name = (std::string)load_from_nbt.at("world_name");
        world_type = (std::string)load_from_nbt.at("world_type");
        light_processor_id = (std::string)load_from_nbt.at("light_processor_id");
        generator_id = (std::string)load_from_nbt.at("generator_id");

        {
            auto _spawn_data = load_from_nbt.at("spawn_data").as_compound();
            spawn_data.angle = _spawn_data.at("angle");
            spawn_data.radius = _spawn_data.at("radius");
            spawn_data.x = _spawn_data.at("x");
            spawn_data.y = _spawn_data.at("y");
            spawn_data.z = _spawn_data.at("z");
        }

        border_center_x = load_from_nbt.at("border_center_x");
        border_center_z = load_from_nbt.at("border_center_z");
        border_size = load_from_nbt.at("border_size");
        border_safe_zone = load_from_nbt.at("border_safe_zone");
        border_damage_per_block = load_from_nbt.at("border_damage_per_block");
        border_lerp_target = load_from_nbt.at("border_lerp_target");
        border_lerp_time = load_from_nbt.at("border_lerp_time");
        border_warning_blocks = load_from_nbt.at("border_warning_blocks");
        border_warning_time = load_from_nbt.at("border_warning_time");
        day_time = load_from_nbt.at("day_time");
        time = load_from_nbt.at("time");
        random_tick_speed = load_from_nbt.at("random_tick_speed");
        ticks_per_second = load_from_nbt.at("ticks_per_second");
        portal_teleport_boundary = load_from_nbt.at("portal_teleport_boundary");
        ticking_frozen = load_from_nbt.at("ticking_frozen");

        chunk_lifetime = std::chrono::milliseconds((long long)load_from_nbt.at("chunk_lifetime"));
        world_lifetime = std::chrono::milliseconds((long long)load_from_nbt.at("world_lifetime"));
        clear_weather_time = load_from_nbt.at("clear_weather_time");
        weather_time = load_from_nbt.at("weather_time");
        current_weather = base_objects::weather::from_string(load_from_nbt.at("current_weather"));

        internal_version = load_from_nbt.at("internal_version");
        chunk_y_count = load_from_nbt.at("chunk_y_count");
        world_y_offset = load_from_nbt.at("world_y_offset");
        difficulty = load_from_nbt.at("difficulty");
        default_gamemode = load_from_nbt.at("default_gamemode");
        difficulty_locked = load_from_nbt.at("difficulty_locked");
        is_hardcore = load_from_nbt.at("is_hardcore");
        initialized = load_from_nbt.at("initialized");
        has_skylight = load_from_nbt.at("has_skylight");
        increase_time = load_from_nbt.at("increase_time");
    }

    void world_data::load() {
        std::unique_lock lock(mutex);
        fast_task::files::async_iofstream file(
            path / "world.senbt",
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            throw std::runtime_error("Can't open world file");
        std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        load(senbt::parse(res).as_compound());
    }

    void world_data::save() {
        std::unique_lock lock(mutex);
        std::filesystem::create_directories(path);
        enbt::compound world_data_file;
        {
            enbt::compound res;
            res.reserve(general_world_data.liquid.size());
            for (auto& [it, data] : general_world_data.liquid)
                res[base_objects::block::get_block(it).name] = enbt::compound{{"spread_size", data.spread_size}, {"spread_ticks", data.spread_ticks}};
            general_world_data.other["liquid"] = std::move(res);
        }

        world_data_file["general_world_data"] = general_world_data.other;
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

        world_data_file["spawn_data"] = enbt::compound{
            {"angle", spawn_data.angle},
            {"radius", spawn_data.radius},
            {"x", spawn_data.x},
            {"y", spawn_data.y},
            {"z", spawn_data.z}
        };

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
        world_data_file["random_tick_speed"] = random_tick_speed;
        world_data_file["ticks_per_second"] = ticks_per_second;
        world_data_file["portal_teleport_boundary"] = portal_teleport_boundary;
        world_data_file["ticking_frozen"] = ticking_frozen;

        world_data_file["chunk_lifetime"] = chunk_lifetime.count();
        world_data_file["world_lifetime"] = world_lifetime.count();
        world_data_file["clear_weather_time"] = clear_weather_time;
        world_data_file["weather_time"] = weather_time;
        world_data_file["current_weather"] = current_weather.to_string();

        world_data_file["internal_version"] = internal_version;
        world_data_file["chunk_y_count"] = chunk_y_count;
        world_data_file["world_y_offset"] = world_y_offset;
        world_data_file["difficulty"] = difficulty;
        world_data_file["default_gamemode"] = default_gamemode;
        world_data_file["difficulty_locked"] = difficulty_locked;
        world_data_file["is_hardcore"] = is_hardcore;
        world_data_file["initialized"] = initialized;
        world_data_file["has_skylight"] = has_skylight;
        world_data_file["increase_time"] = increase_time;

        auto stringized = senbt::serialize(world_data_file, false, true);
        std::filesystem::create_directories(path);
        fast_task::files::async_iofstream file(
            path / "world.senbt",
            fast_task::files::open_mode::write,
            fast_task::files::on_open_action::always_new,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            throw std::runtime_error("Can't open world file");
        file.write(stringized.data(), stringized.size());
    }

    std::string world_data::preview_world_name() {
        std::unique_lock lock(mutex);
        fast_task::files::async_iofstream file(
            path / "world.senbt",
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            throw std::runtime_error("Can't open world file");
        std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return (std::string)(std::string)senbt::parse(res)["world_name"];
    }

    world_data::world_data(int32_t world_id, const std::filesystem::path& path)
        : path(path), world_id(world_id) {
        world_game_rules["reducedDebugInfo"] = api::configuration::get().game_play.reduced_debug_screen;
        if (!std::filesystem::exists(path))
            std::filesystem::create_directories(path);
        world_spawn_ticket_id = add_loading_ticket(
            base_objects::world::loading_point_ticket{
                [](auto&, auto, auto&) { return true; },
                {convert_chunk_global_pos(spawn_data.x),
                 convert_chunk_global_pos(spawn_data.z),
                 convert_chunk_global_pos(spawn_data.radius)},
                "Start ticket",
                22
            }
        );
    }

    void world_data::update_spawn_data(int64_t x, int64_t z, int64_t radius, float angle) {
        std::unique_lock lock(mutex);
        spawn_data = {x, z, radius, 0, angle};
        loading_tickets.at(world_spawn_ticket_id).point = {
            convert_chunk_global_pos(x),
            convert_chunk_global_pos(z),
            convert_chunk_global_pos(radius)
        };
        get_height_maps_at(x, z, [&](base_objects::world::height_maps& height_maps) {
            auto mt = height_maps.motion_blocking[x % 16][z % 16];
            auto oc_flor = height_maps.ocean_floor[x % 16][z % 16];
            auto oc = height_maps.surface[x % 16][z % 16];
            spawn_data.y = std::max(mt, std::max(oc_flor, oc));
        });
    }

    size_t world_data::add_loading_ticket(base_objects::world::loading_point_ticket&& ticket) {
        std::unique_lock lock(mutex);
        size_t id = loading_tickets.size();
        while (loading_tickets.contains(id))
            ++id;
        loading_tickets.emplace(id, std::move(ticket));
        return id;
    }

    void world_data::remove_loading_ticket(size_t id) {
        std::unique_lock lock(mutex);
        loading_tickets.erase(id);
    }

    size_t world_data::loaded_chunks_count() {
        std::unique_lock lock(mutex);
        size_t count = 0;
        for (auto& x_axis : chunks)
            for (auto& z_axis : x_axis.second)
                count += (bool)z_axis.second;
        return count;
    }

    bool world_data::exists(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                return true;
        lock.unlock();
        bool res = std::filesystem::exists(path / "chunks" / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"));
        lock.lock();
        if (res)
            chunks[chunk_x][chunk_z] = nullptr;
        return res;
    }

    base_objects::atomic_holder<chunk_data> world_data::processed_load_chunk_sync(int64_t chunk_x, int64_t chunk_z, bool is_async_context) {
        auto chunk = load_chunk_sync(chunk_x, chunk_z);
        std::unique_lock lock(mutex);
        chunks[chunk_x][chunk_z] = chunk;
        if (is_async_context) {
            on_load_process.erase({chunk_x, chunk_z});
            if (profiling.enable_world_profiling)
                --profiling.chunk_load_counter;
        }
        if (profiling.enable_world_profiling) {
            if (chunk) {
                ++profiling.chunk_total_loaded;
                if (profiling.chunk_loaded)
                    profiling.chunk_loaded(*this, *chunk);
                WORLD_ASYNC_RUN(notify_chunk, chunk_x, chunk_z);
            } else {
                if (profiling.chunk_load_failed)
                    profiling.chunk_load_failed(*this, chunk_x, chunk_z);
            }
        }
        return chunk;
    }

    FuturePtr<base_objects::atomic_holder<chunk_data>> world_data::create_chunk_load_future(int64_t chunk_x, int64_t chunk_z) {
        if (profiling.enable_world_profiling)
            ++profiling.chunk_load_counter;
        return Future<base_objects::atomic_holder<chunk_data>>::start(
            [this, chunk_x, chunk_z]() -> base_objects::atomic_holder<chunk_data> {
                return processed_load_chunk_sync(chunk_x, chunk_z, true);
            }
        );
    }

    FuturePtr<base_objects::atomic_holder<chunk_data>> world_data::create_chunk_load_future(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback, std::function<void()> fault) {
        if (profiling.enable_world_profiling)
            ++profiling.chunk_load_counter;
        return Future<base_objects::atomic_holder<chunk_data>>::start(
            [this, chunk_x, chunk_z, callback, fault]() -> base_objects::atomic_holder<chunk_data> {
                auto chunk = processed_load_chunk_sync(chunk_x, chunk_z, true);
                if (chunk)
                    callback(*chunk);
                else
                    fault();
                return chunk;
            }
        );
    }

    base_objects::atomic_holder<chunk_data> world_data::request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                if (y_axis->second)
                    if (y_axis->second->generator_stage == 0xFF)
                        return y_axis->second;

        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            auto it = on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
            it->wait_with(lock);
            return it->get();
        } else {
            auto fut = process->second;
            fut->wait_with(lock);
            return fut->get();
        }
    }

    FuturePtr<base_objects::atomic_holder<chunk_data>> world_data::request_chunk_data(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                if (y_axis->second)
                    if (y_axis->second->generator_stage == 0xFF)
                        return make_ready_future(y_axis->second);

        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
            return on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
        else
            return process->second;
    }

    std::optional<base_objects::atomic_holder<chunk_data>> world_data::request_chunk_data_weak_gen(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                if (y_axis->second)
                    return std::make_optional(y_axis->second);


        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            if (!exists(chunk_x, chunk_z))
                on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
        }
        return std::nullopt;
    }

    std::optional<base_objects::atomic_holder<chunk_data>> world_data::request_chunk_data_weak(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                if (y_axis->second)
                    return std::make_optional(y_axis->second);

        return std::nullopt;
    }

    std::optional<base_objects::atomic_holder<chunk_data>> world_data::request_chunk_data_weak_sync(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                if (y_axis->second)
                    return std::make_optional(y_axis->second);
        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            if (exists(chunk_x, chunk_z)) {
                auto it = on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
                it->wait_with(lock);
                return it->get();
            } else
                return std::nullopt;
        } else {
            auto fut = process->second;
            fut->wait_with(lock);
            return fut->get();
        }
    }

    void world_data::request_chunk_gen(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                return;
        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            bool make_gen = false;
            if (!exists(chunk_x, chunk_z))
                make_gen = true;
            else if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
                if (auto z_axis = x_axis->second.find(chunk_z); z_axis != x_axis->second.end()) {
                    auto chunk = z_axis->second;
                    if (chunk)
                        if (chunk->generator_stage != 0xFF)
                            if (chunk->load_level <= chunk->resume_gen_level)
                                if (!on_generate_process.contains({chunk_x, chunk_z})) {
                                    make_gen = true;
                                    chunk->resume_gen_level = 0xFF;
                                }
                }

            if (make_gen)
                on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
        }
    }

    bool world_data::request_chunk_data_sync(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto z_axis = x_axis->second.find(chunk_z); z_axis != x_axis->second.end())
                if (z_axis->second) {
                    callback(*z_axis->second);
                    return true;
                }

        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            auto res = processed_load_chunk_sync(chunk_x, chunk_z, false);
            if (res)
                callback(*(chunks[chunk_x][chunk_z] = res));
            else
                return false;
            return true;
        } else {
            process->second->wait_with(lock);
            return request_chunk_data_sync(chunk_x, chunk_z, callback);
        }
    }

    void world_data::request_chunk_data(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> callback, std::function<void()> fault) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto y_axis = x_axis->second.find(chunk_z); y_axis != x_axis->second.end())
                if (y_axis->second) {
                    callback(*y_axis->second);
                    return;
                }

        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
            on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z, callback, fault);
        else
            process->second->when_ready([callback, fault](base_objects::atomic_holder<chunk_data> chunk) {
                if (chunk)
                    callback(*chunk);
                else
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

    void world_data::save_chunks(bool unload) {
        std::unique_lock lock(mutex);
        for (auto& [x, x_axis] : chunks)
            for (auto& [z, chunk] : x_axis)
                make_save(x, z, unload);
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
            on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
    }

    void world_data::reset_light_data(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        get_light_processor()->process_chunk(*this, chunk_x, chunk_z);
    }

    void world_data::save_and_unload_chunk_at(int64_t global_x, int64_t global_z) {
        save_and_unload_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::unload_chunk_at(int64_t global_x, int64_t global_z) {
        unload_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::save_chunk_at(int64_t global_x, int64_t global_z) {
        save_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::erase_chunk_at(int64_t global_x, int64_t global_z) {
        erase_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::regenerate_chunk_at(int64_t global_x, int64_t global_z) {
        regenerate_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::reset_light_data_at(int64_t global_x, int64_t global_z) {
        reset_light_data(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::for_each_chunk(std::function<void(chunk_data& chunk)> func) {
        std::unique_lock lock(mutex);
        for (auto& [x, x_axis] : chunks)
            for (auto& [z, chunk] : x_axis)
                if (chunk)
                    if (chunk->generator_stage == 0xFF)
                        func(*chunk);
    }

    void world_data::for_each_chunk(base_objects::cubic_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func) {
        std::unique_lock lock(mutex);
        for (int64_t x = bounds.x1; x <= bounds.x2; x++)
            for (int64_t z = bounds.z1; z <= bounds.z2; z++)
                if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                    if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                        if (chunk->second)
                            if (chunk->second->generator_stage == 0xFF)
                                func(*chunk->second);
    }

    void world_data::for_each_chunk(base_objects::spherical_bounds_chunk bounds, std::function<void(chunk_data& chunk)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        if (chunk->second->generator_stage == 0xFF)
                            func(*chunk->second);
        });
    }

    void world_data::for_each_sub_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(sub_chunk_data& chunk)> func) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                if (chunk->second)
                    if (chunk->second->generator_stage == 0xFF)
                        chunk->second->for_each_sub_chunk(func);
    }

    void world_data::get_sub_chunk(int64_t chunk_x, int64_t chunk_y_raw, int64_t chunk_z, std::function<void(sub_chunk_data& chunk)> func) {
        std::unique_lock lock(mutex);
        TO_WORLD_POS_CHUNK(chunk_y, chunk_y_raw);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                if (chunk->second)
                    if (chunk->second->generator_stage == 0xFF)
                        chunk->second->get_sub_chunk(chunk_y, func);
    }

    void world_data::get_chunk(int64_t chunk_x, int64_t chunk_z, std::function<void(chunk_data& chunk)> func) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                if (chunk->second)
                    if (chunk->second->generator_stage == 0xFF)
                        func(*chunk->second);
    }

    void world_data::for_each_chunk(base_objects::cubic_bounds_block bounds, std::function<void(chunk_data& chunk)> func) {
        for_each_chunk((base_objects::cubic_bounds_chunk)bounds, func);
    }

    void world_data::for_each_chunk(base_objects::spherical_bounds_block bounds, std::function<void(chunk_data& chunk)> func) {
        for_each_chunk((base_objects::spherical_bounds_chunk)bounds, func);
    }

    void world_data::for_each_sub_chunk_at(int64_t global_x, int64_t global_z, std::function<void(sub_chunk_data& chunk)> func) {
        for_each_sub_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), func);
    }

    void world_data::get_sub_chunk_at(int64_t global_x, int64_t global_y_raw, int64_t global_z, std::function<void(sub_chunk_data& chunk)> func) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_y), convert_chunk_global_pos(global_z), func);
    }

    void world_data::get_chunk_at(int64_t global_x, int64_t global_z, std::function<void(chunk_data& chunk)> func) {
        get_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), func);
    }

    void world_data::for_each_entity(std::function<void(const base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        for (auto& [x, entity] : entities)
            func(entity);
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_entity(func);
        });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_chunk_radius bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_entity(func);
        });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_chunk_radius_out bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_entity(func);
        });
    }

    void world_data::for_each_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_entity(func);
        });
    }

    void world_data::for_each_entity(base_objects::spherical_bounds_chunk_out bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_entity(func);
        });
    }

    void world_data::for_each_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                if (chunk->second)
                    chunk->second->for_each_entity(func);
    }

    void world_data::for_each_block_entity(base_objects::cubic_bounds_chunk bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(base_objects::cubic_bounds_chunk_radius bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(base_objects::cubic_bounds_chunk_radius_out bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(base_objects::spherical_bounds_chunk bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(base_objects::spherical_bounds_chunk_out bounds, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int64_t x, int64_t z) {
            if (auto x_axis = chunks.find(x); x_axis != chunks.end())
                if (auto chunk = x_axis->second.find(z); chunk != x_axis->second.end())
                    if (chunk->second)
                        chunk->second->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(int64_t chunk_x, int64_t chunk_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        std::unique_lock lock(mutex);
        get_chunk(chunk_x, chunk_z, [&](auto& chunk) { chunk.for_each_block_entity(func); });
    }

    void world_data::for_each_block_entity(int64_t chunk_x, int64_t chunk_y_raw, int64_t chunk_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        TO_WORLD_POS_CHUNK(chunk_y, chunk_y_raw);
        std::unique_lock lock(mutex);
        get_chunk(chunk_x, chunk_z, [&](auto& chunk) { chunk.for_each_block_entity(chunk_y, func); });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_block bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        for_each_entity((base_objects::cubic_bounds_chunk)bounds, [&](auto& entity) {
            if (bounds.in_bounds((int64_t)entity->position.x, (int64_t)entity->position.y, (int64_t)entity->position.z))
                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_block_radius bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        for_each_entity((base_objects::cubic_bounds_chunk_radius)bounds, [&](auto& entity) {
            if (bounds.in_bounds((int64_t)entity->position.x, (int64_t)entity->position.y, (int64_t)entity->position.z))
                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_block_radius_out bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        for_each_entity((base_objects::cubic_bounds_chunk_radius_out)bounds, [&](auto& entity) {
            if (bounds.in_bounds((int64_t)entity->position.x, (int64_t)entity->position.y, (int64_t)entity->position.z))
                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::spherical_bounds_block bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        for_each_entity((base_objects::spherical_bounds_chunk)bounds, [&](auto& entity) {
            if (bounds.in_bounds((int64_t)entity->position.x, (int64_t)entity->position.y, (int64_t)entity->position.z))
                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::spherical_bounds_block_out bounds, std::function<void(base_objects::entity_ref& entity)> func) {
        for_each_entity((base_objects::spherical_bounds_chunk_out)bounds, [&](auto& entity) {
            if (bounds.in_bounds((int64_t)entity->position.x, (int64_t)entity->position.y, (int64_t)entity->position.z))
                func(entity);
        });
    }

    void world_data::for_each_entity_at(int64_t global_x, int64_t global_z, std::function<void(const base_objects::entity_ref& entity)> func) {
        for_each_entity(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), func);
    }

    void world_data::for_each_block_entity_at(int64_t global_x, int64_t global_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        for_each_block_entity(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), func);
    }

    void world_data::for_each_block_entity_at(int64_t global_x, int64_t global_y_raw, int64_t global_z, std::function<void(base_objects::block& block, enbt::value& extended_data)> func) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        for_each_block_entity(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), convert_chunk_global_pos(global_y), func);
    }

    void world_data::query_for_tick(int64_t global_x, int64_t global_y_raw, int64_t global_z, uint64_t duration, int8_t priority) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        std::unique_lock lock(mutex);
        auto chunk_x = global_x >> 4;
        auto chunk_z = global_z >> 4;
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                if (chunk->second)
                    chunk->second->query_for_tick(global_x & 15, global_y, global_z & 15, duration + tick_counter, priority);
    }

    void world_data::query_for_liquid_tick(int64_t global_x, int64_t global_y_raw, int64_t global_z, uint64_t duration) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        std::unique_lock lock(mutex);
        auto chunk_x = global_x >> 4;
        auto chunk_z = global_z >> 4;
        if (auto x_axis = chunks.find(chunk_x); x_axis != chunks.end())
            if (auto chunk = x_axis->second.find(chunk_z); chunk != x_axis->second.end())
                if (chunk->second)
                    chunk->second->query_for_liquid_tick(global_x & 15, global_y, global_z & 15, duration + tick_counter);
    }

    void world_data::set_block(const base_objects::full_block_data& block, int64_t global_x, int64_t global_y_raw, int64_t global_z, block_set_mode mode) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        bool updates_height_map = false;
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            auto gen_block_id = std::visit(
                [&](auto& it) {
                    if (mode == block_set_mode::destroy)
                        WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y, global_z, it);
                    else
                        WORLD_ASYNC_RUN(notify_block_change, global_x, global_y, global_z, it);
                    updates_height_map = it.is_solid();
                    return it.general_block_id();
                },
                block
            );
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, block);
            get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            __update_block(global_x, global_y_raw, global_z, mode, gen_block_id);
        });


        if (updates_height_map)
            get_chunk_at(global_x, global_z, [&](auto& chunk) {
                chunk.update_height_map_on((uint8_t)convert_chunk_local_pos(global_x), global_y, (uint8_t)convert_chunk_local_pos(global_z));
            });
    }

    void world_data::set_block(base_objects::full_block_data&& block, int64_t global_x, int64_t global_y_raw, int64_t global_z, block_set_mode mode) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            auto gen_block_id = std::visit(
                [&](auto& it) {
                    if (mode == block_set_mode::destroy)
                        WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y, global_z, it);
                    else
                        WORLD_ASYNC_RUN(notify_block_change, global_x, global_y, global_z, it);
                    return it.general_block_id();
                },
                block
            );
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, std::move(block));
            get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            __update_block(global_x, global_y_raw, global_z, mode, gen_block_id);
        });
    }

    void world_data::remove_block(int64_t global_x, int64_t global_y_raw, int64_t global_z) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            base_objects::block air;
            WORLD_ASYNC_RUN(notify_block_change, global_x, global_y, global_z, air);
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, air);
            get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            __update_block(global_x, global_y_raw, global_z, block_set_mode::replace, air.general_block_id());
        });
    }

    void world_data::get_block(int64_t global_x, int64_t global_y_raw, int64_t global_z, std::function<void(base_objects::block& block)> func, std::function<void(base_objects::block& block, enbt::value& extended_data)> block_entity) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15, func, block_entity);
        });
    }

    void world_data::query_block(int64_t global_x, int64_t global_y_raw, int64_t global_z, std::function<void(base_objects::block& block)> func, std::function<void(base_objects::block& block, enbt::value& extended_data)> block_entity, std::function<void()> fault) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
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

    void world_data::block_updated(int64_t global_x, int64_t global_y, int64_t global_z) {
        std::unique_lock lock(mutex);
        get_block(
            global_x,
            global_y,
            global_z,
            [&](auto& normal) { WORLD_ASYNC_RUN(notify_block_change, global_x, global_y, global_z, normal); },
            [&](auto& block, auto& nbt) {
                base_objects::const_block_entity_ref ref(block, nbt);
                WORLD_ASYNC_RUN(notify_block_change, global_x, global_y, global_z, ref);
            }
        );
        get_light_processor()->block_changed(*this, global_x, global_y, global_z);
    }

    void world_data::chunk_updated(int64_t chunk_x, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        get_light_processor()->process_chunk(*this, chunk_x, chunk_z);
    }

    void world_data::sub_chunk_updated(int64_t chunk_x, int64_t chunk_y_raw, int64_t chunk_z) {
        std::unique_lock lock(mutex);
        TO_WORLD_POS_CHUNK(chunk_y, chunk_y_raw)
        get_light_processor()->process_sub_chunk(*this, chunk_x, chunk_y_raw, chunk_z);
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
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
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
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
                    if (i == max)
                        i = 0;
                });
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
            });
        }

        if (mode != block_set_mode::destroy)
            ((base_objects::cubic_bounds_chunk)bounds).enum_points([&](int64_t x, int64_t z) {
                WORLD_ASYNC_RUN(notify_chunk_blocks, x, z);
            });
    }

    void world_data::set_block_range(base_objects::cubic_bounds_block bounds, list_array<base_objects::full_block_data>&& blocks, block_set_mode mode) {
        if (blocks.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.__set_block_silent(std::move(blocks[i++]), x, y, z, mode);
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
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
                    if (i == max)
                        i = 0;
                });
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
            });
        }

        if (mode != block_set_mode::destroy)
            ((base_objects::cubic_bounds_chunk)bounds).enum_points([&](int64_t x, int64_t z) {
                WORLD_ASYNC_RUN(notify_chunk_blocks, x, z);
            });
    }

    void world_data::set_block_range(base_objects::spherical_bounds_block bounds, const list_array<base_objects::full_block_data>& blocks, block_set_mode mode) {
        if (blocks.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
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
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
                });
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
                if (i == max)
                    i = 0;
            });
        }

        if (mode != block_set_mode::destroy)
            ((base_objects::spherical_bounds_chunk)bounds).enum_points([&](int64_t x, int64_t z) {
                WORLD_ASYNC_RUN(notify_chunk_blocks, x, z);
            });
    }

    void world_data::set_block_range(base_objects::spherical_bounds_block bounds, list_array<base_objects::full_block_data>&& blocks, block_set_mode mode) {
        if (blocks.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.__set_block_silent(std::move(blocks[i++]), x, y, z, mode);
                });
                ((base_objects::spherical_bounds_chunk)bounds).enum_points([&](int64_t x, int64_t z) {
                    get_light_processor()->process_chunk(world, x, z);
                });
            });
        } else {
            size_t i = 0;
            size_t max = blocks.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    if (i == max)
                        return;
                    world.__set_block_silent(std::move(blocks[i++]), x, y, z, mode);
                    ++i;
                });
                ((base_objects::spherical_bounds_chunk)bounds).enum_points([&](int64_t x, int64_t z) {
                    get_light_processor()->process_chunk(world, x, z);
                });
            });
        }
        if (mode != block_set_mode::destroy)
            ((base_objects::spherical_bounds_chunk)bounds).enum_points([&](int64_t x, int64_t z) {
                WORLD_ASYNC_RUN(notify_chunk_blocks, x, z);
            });
    }

    int32_t world_data::get_biome(int64_t global_x, int64_t global_y_raw, int64_t global_z) {
        uint32_t res = 0;
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            res = sub_chunk.get_biome(global_x & 15, global_y & 15, global_z & 15);
        });
        return res;
    }

    void world_data::set_biome(int64_t global_x, int64_t global_y_raw, int64_t global_z, int32_t biome_id) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.set_biome(global_x & 15, global_y & 15, global_z & 15, biome_id);
            WORLD_ASYNC_RUN(notify_biome_change, global_x, global_y, global_z, biome_id);
        });
    }

    void world_data::set_biome_range(base_objects::cubic_bounds_block bounds, const list_array<int32_t>& biomes) {
        if (biomes.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                });
            });
        } else {
            size_t i = 0;
            size_t max = biomes.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                    if (i == max)
                        i = 0;
                });
            });
        }
    }

    void world_data::set_biome_range(base_objects::cubic_bounds_block bounds, list_array<int32_t>&& biomes) {
        if (biomes.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                });
            });
        } else {
            size_t i = 0;
            size_t max = biomes.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                    if (i == max)
                        i = 0;
                });
            });
        }
    }

    void world_data::set_biome_range(base_objects::spherical_bounds_block bounds, const list_array<int32_t>& biomes) {
        if (biomes.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                });
            });
        } else {
            size_t i = 0;
            size_t max = biomes.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                    if (i == max)
                        i = 0;
                });
            });
        }
    }

    void world_data::set_biome_range(base_objects::spherical_bounds_block bounds, list_array<int32_t>&& biomes) {
        if (biomes.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                });
            });
        } else {
            size_t i = 0;
            size_t max = biomes.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int64_t x, int64_t y, int64_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                    if (i == max)
                        i = 0;
                });
            });
        }
    }

    void world_data::get_height_maps(int64_t chunk_x, int64_t chunk_z, std::function<void(base_objects::world::height_maps& height_maps)> func) {
        get_chunk(chunk_x, chunk_z, [&](chunk_data& chunk) {
            func(chunk.height_maps);
        });
    }

    void world_data::get_height_maps_at(int64_t global_x, int64_t global_z, std::function<void(base_objects::world::height_maps& height_maps)> func) {
        get_chunk_at(global_x, global_z, [&](chunk_data& chunk) {
            func(chunk.height_maps);
        });
    }

    void world_data::register_entity(base_objects::entity_ref& entity) {
        if (entity->world_syncing_data)
            throw std::runtime_error("Entity already registered in another world");
        std::unique_lock lock(mutex);
        uint64_t id = local_entity_id_generator++;
        while (entities.contains(id))
            id = local_entity_id_generator++;

        base_objects::cubic_bounds_chunk_radius processing_region((int64_t)entity->position.x, (int64_t)entity->position.z, entity->const_data().max_track_distance);

        entity->world_syncing_data = std::make_optional<base_objects::entity::world_syncing>(
            bit_list_array(),
            processing_region,
            id,
            this
        );
        entity->world_syncing_data->flush_processing();
        entities[id] = entity;
        to_load_entities[id] = entity;
        entity_init(*entity);
        if (auto loading_level = entity->const_data().loading_ticket_level; loading_level <= 44)
            add_loading_ticket({base_objects::world::loading_point_ticket::entity_bound_ticket{id}, processing_region, "entity ticket", loading_level});
    }

    void world_data::unregister_entity(base_objects::entity_ref& entity) {
        std::unique_lock lock(mutex);
        if (entity->world_syncing_data) {
            entity_deinit(*entity);
            entities.erase(entity->world_syncing_data->assigned_world_id);
            to_load_entities.erase(entity->world_syncing_data->assigned_world_id);
            entity->world_syncing_data = std::nullopt;
        }
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
        list_array<size_t> expired_tickets;

        for (auto& [x, x_axis] : chunks) {
            for (auto& [z, z_axis] : x_axis) {
                if (z_axis)
                    if (z_axis->load_level <= 44)
                        z_axis->load_level++;
            }
        }

        std::unordered_map<int64_t, std::unordered_set<int64_t>> loading_tickets_cc;
        size_t target_load_count = 0;
        for (auto& [id, ticket] : loading_tickets) {
            bool expired = false;
            std::visit(
                [&](auto& expr) {
                    using T = std::decay_t<decltype(expr)>;
                    if constexpr (std::is_same_v<T, uint16_t>) {
                        if (expr)
                            --expr;
                        else
                            expired = true;
                    } else if constexpr (std::is_same_v<T, base_objects::world::loading_point_ticket::callback>) {
                        if (!expr(*this, id, ticket))
                            expired = true;
                    } else if constexpr (std::is_same_v<T, base_objects::world::loading_point_ticket::entity_bound_ticket>) {
                        if (auto it = entities.find(expr.id); it != entities.end()) {
                            if (it->second->world_syncing_data)
                                ticket.point = it->second->world_syncing_data->processing_region;
                            else
                                expired = true;
                        } else
                            expired = true;
                    }
                },
                ticket.expiration
            );
            if (expired)
                expired_tickets.push_back(id);
            else if (ticket.level < 44) {
                to_tick_chunks.reserve(ticket.point.count());
                ticket.point.enum_points_from_center([&](int64_t x, int64_t z) {
                    auto& local_x = loading_tickets_cc[x];
                    if (local_x.contains(z))
                        return;
                    local_x.insert(z);
                    ++target_load_count;
                    auto res = request_chunk_data(x, z);
                    if (res->is_ready()) {
                        res->get()->load_level = std::min<uint8_t>(res->get()->load_level, ticket.level);
                        if (res->get()->load_level < 33)
                            to_tick_chunks.push_back(res->get());
                    }
                });
                uint8_t propagation = 44 - ticket.level;
                if (propagation) {
                    base_objects::cubic_bounds_chunk_radius_out bounds(ticket.point.center_x, ticket.point.center_z, ticket.point.radius, ticket.point.radius + propagation);
                    to_tick_chunks.reserve(bounds.count());
                    bounds.enum_points_from_center_w_layer_no_center([&](int64_t x, int64_t z, int64_t layer) {
                        auto set_load_level = propagation + layer;
                        if (set_load_level <= 33) {
                            auto& local_x = loading_tickets_cc[x];
                            if (local_x.contains(z))
                                return;
                            local_x.insert(z);
                            ++target_load_count;
                            auto res = request_chunk_data(x, z);
                            if (res->is_ready()) {
                                res->get()->load_level = (uint8_t)std::min<int64_t>(res->get()->load_level, set_load_level);
                                if (res->get()->load_level < 32)
                                    to_tick_chunks.push_back(res->get());
                            }
                        } else if (set_load_level <= 44)
                            request_chunk_gen(x, z);
                    });
                }
            }
        }
        expired_tickets.for_each([&](size_t id) {
            loading_tickets.erase(id);
        });

        {
            list_array<uint64_t> loaded_entities;
            for (auto& [id, entity] : to_load_entities) {
                auto chunk = request_chunk_data_weak_gen((int64_t)convert_chunk_global_pos(entity->position.x), (int64_t)convert_chunk_global_pos(entity->position.z));
                if (chunk) {
                    if ((*chunk)->generator_stage == 0xFF) {
                        if ((*chunk)->stored_entities.insert({id, entity}).second)
                            loaded_entities.push_back(id);
                    }
                }
            }
            for (auto id : loaded_entities)
                to_load_entities.erase(id);
        }
        lock.unlock();
        tick_counter++;
        if (!profiling.enable_world_profiling) {
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick(*this, random_tick_speed, random_engine, current_time);
            });
        } else {
            profiling.chunk_target_to_load = target_load_count;
            const auto tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(1) / ticks_per_second);
            auto tick_local_time = std::chrono::high_resolution_clock::now();
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick(*this, random_tick_speed, random_engine, current_time);

                auto actual_time = std::chrono::high_resolution_clock::now();
                auto current_tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                auto slow_chunk_threshold = tick_speed * profiling.slow_chunk_tick_callback_threshold;
                if (slow_chunk_threshold < current_tick_speed)
                    if (profiling.slow_chunk_tick_callback)
                        profiling.slow_chunk_tick_callback(*this, chunk->chunk_x, chunk->chunk_z, current_tick_speed);
                tick_local_time = actual_time;
                if (profiling.chunk_speedometer_callback)
                    profiling.chunk_speedometer_callback(*this, chunk->chunk_x, chunk->chunk_z, current_tick_speed);
            });
            if (profiling.chunk_speedometer_callback)
                profiling.chunk_speedometer_callback(*this, INT64_MAX, INT64_MAX, std::chrono::milliseconds(0));
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
        if (tick_counter % api::configuration::get().world.auto_save == 0) {
            save_chunks();
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
                    if (begin->second)
                        if (begin->second->load_level > 44) {
                            make_save(x, begin->first, begin, true);
                            --unload_limit;
                            chunk_unloaded = true;
                            break;
                        }
                    ++begin;
                }
                if (!unload_limit)
                    return false;
            } while (chunk_unloaded);
        }
        return false;
    }

#pragma region worlds_data

    base_objects::atomic_holder<world_data> worlds_data::load(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (cached_worlds.find(world_id) == cached_worlds.end()) {
            auto path = base_path / std::to_string(world_id);
            if (!std::filesystem::exists(path))
                throw std::runtime_error("World not found");

            auto world = base_objects::atomic_holder<world_data>(new world_data(world_id, path.string()));
            world->load();
            auto& res = cached_worlds[world_id] = world;
            on_world_loaded(world_id);
            return res;
        }
        return cached_worlds[world_id];
    }

    worlds_data::worlds_data(const std::filesystem::path& base_path)
        : base_path(base_path) {
        if (!std::filesystem::exists(base_path))
            std::filesystem::create_directories(base_path);
    }

    const list_array<int32_t>& worlds_data::get_ids() {
        std::unique_lock lock(mutex);
        if (!cached_ids.empty())
            return cached_ids;
        list_array<int32_t> result;
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

    size_t worlds_data::loaded_chunks_count() {
        std::unique_lock lock(mutex);
        size_t res = 0;
        for (auto& world : cached_worlds)
            res += world.second->loaded_chunks_count();
        return res;
    }

    size_t worlds_data::loaded_chunks_count(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto on_load = cached_worlds.find(world_id); on_load != cached_worlds.end())
            return on_load->second->loaded_chunks_count();
        return 0;
    }

    bool worlds_data::exists(int32_t world_id) {
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

    const list_array<int32_t>& worlds_data::get_list() {
        std::unique_lock lock(mutex);
        return get_ids();
    }

    std::string worlds_data::get_name(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto on_load = cached_worlds.find(world_id); on_load != cached_worlds.end())
            return on_load->second->world_name;

        auto world_path = base_path / std::to_string(world_id);
        if (std::filesystem::exists(world_path))
            return world_data(world_id, world_path).preview_world_name();
        else
            throw std::runtime_error("World with id " + std::to_string(world_id) + " not found.");
    }

    int32_t worlds_data::get_id(const std::string& name) {
        std::unique_lock lock(mutex);
        for (auto& world : cached_worlds)
            if (world.second->world_name == name)
                return world.first;

        if (cached_ids.empty())
            get_ids();
        size_t found = cached_ids.find_if([this, &name](int32_t id) {
            if (std::filesystem::exists(base_path / std::to_string(id) / "world.senbt"))
                return world_data(id, base_path / std::to_string(id)).preview_world_name() == name;
            else
                return false;
        });
        return found == list_array<int32_t>::npos ? -1 : cached_ids[found];
    }

    list_array<int32_t> worlds_data::get_all_ids() {
        return get_ids();
    }

    base_objects::atomic_holder<world_data> worlds_data::get(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
            return load(world_id);
        else
            return world->second;
    }

    void worlds_data::save(int32_t world_id) {
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

    void worlds_data::save_and_unload(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto item = cached_worlds.find(world_id); item == cached_worlds.end())
            throw std::runtime_error("World not found");
        else {
            auto& world = item->second;
            on_world_unloaded(world_id);
            world->save();
            world->save_chunks(true);
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
    void worlds_data::unload(int32_t world_id) {
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

    void worlds_data::erase(int32_t world_id) {
        std::unique_lock lock(mutex);
        std::filesystem::remove_all(std::filesystem::path(base_path) / std::to_string(world_id));
        cached_worlds.erase(world_id);
        cached_ids.remove(world_id);
        on_world_unloaded(world_id);
    }

    int32_t worlds_data::create(const std::string& name) {
        std::unique_lock lock(mutex);
        if (get_id(name) != -1)
            throw std::runtime_error("World with name " + name + " already exists.");
        int32_t id = 0;
        while (exists(id))
            id++;
        cached_ids.push_back(id);
        cached_worlds[id] = new world_data(id, base_path / std::to_string(id));
        cached_worlds[id]->world_name = name;
        cached_worlds[id]->save();
        on_world_loaded(id);
        return id;
    }

    int32_t worlds_data::create(const std::string& name, std::function<void(world_data& world)> init) {
        std::unique_lock lock(mutex);
        if (get_id(name) != -1)
            throw std::runtime_error("World with name " + name + " already exists.");
        int32_t id = 0;
        while (exists(id))
            id++;
        base_objects::atomic_holder<world_data> world = new world_data(id, base_path / std::to_string(id));
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

    void worlds_data::for_each_entity(int32_t world_id, std::function<void(const base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
            load(world_id)->for_each_entity(func);
        else
            world->second->for_each_entity(func);
    }

    void worlds_data::for_each_entity(int32_t world_id, int64_t chunk_x, int64_t chunk_z, std::function<void(const base_objects::entity_ref& entity)> func) {
        std::unique_lock lock(mutex);
        if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
            load(world_id)->for_each_entity(chunk_x, chunk_z, func);
        else
            world->second->for_each_entity(chunk_x, chunk_z, func);
    }

    void worlds_data::for_each_world(std::function<void(int32_t id, world_data& world)> func) {
        std::unique_lock lock(mutex);
        for (auto& [id, world] : cached_worlds)
            func(id, *world);
    }

    void worlds_data::apply_tick(std::chrono::high_resolution_clock::time_point current_time) {
        std::unique_lock lock(mutex);
        list_array<std::pair<int32_t, base_objects::atomic_holder<world_data>>> worlds_to_tick;
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
        size_t unload_speed = api::configuration::get().world.unload_speed;
        future::forEachMove(
            future::process<std::optional<int32_t>>(
                worlds_to_tick,
                [current_time, unload_speed](const auto& it) mutable -> std::optional<int32_t> {
                    auto id = it.first;
                    auto& world = it.second;
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    world->tick(gen, current_time);
                    if (world->collect_unused_data(current_time, unload_speed))
                        return id;
                    return std::nullopt;
                }
            ),
            [this](auto id) { if(id) save_and_unload(*id); }
        )->wait();
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
}
