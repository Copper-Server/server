/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <library/enbt/io_tools.hpp>
#include <library/fast_task/include/files.hpp>

#include <src/api/configuration.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/entity.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/world/chunk.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/readers.hpp>

namespace enbt::io_helper {

    using light_data = copper_server::base_objects::world::light_data;
    using height_maps = copper_server::base_objects::world::height_maps;
    using palette_container_block = copper_server::base_objects::palette_container_block;
    using palette_container_biome = copper_server::base_objects::palette_container_biome;

    template <>
    struct compact_matrix_simple_cast<int32_t[4][4][4]> {
        using direct_type = std::int32_t;
    };

    template <>
    struct compact_matrix_simple_cast<light_data::light_item[16][16][8]> {
        using direct_type = std::uint8_t;
    };

    template <>
    struct serialization_simple_cast<copper_server::base_objects::block> {
        using direct_type = std::int32_t;
    };

    template <>
    struct serialization<palette_container_block> {
        static palette_container_block read(enbt::io_helper::value_read_stream& self) {
            palette_container_block palette;
            read(palette, self);
            return palette;
        }

        static void read(palette_container_block& value, enbt::io_helper::value_read_stream& self) {
            auto arr = self.iterate_into<uint8_t>();
            copper_server::ArrayStream stream(arr.data(), arr.size());
            auto bits_per_entry = stream.read_value<uint8_t>();
            static constexpr auto max_indirect = copper_server::base_objects::palette_container::max_indirect_blocks;
            static constexpr auto entries_count = 4096;
            if (bits_per_entry == 0) {
                copper_server::base_objects::palette_container_single res;
                res.id_of_palette = stream.read_var<int32_t>();
                value.decompile(std::move(res));
            } else if (bits_per_entry <= max_indirect) {
                copper_server::base_objects::palette_container_indirect res(bits_per_entry, entries_count);
                uint32_t palette = stream.read_var<uint32_t>();
                res.palette.reserve(palette);
                for (uint32_t i = 0; i < palette; i++)
                    res.palette.push_back(stream.read_var<uint32_t>());
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                size /= 8;
                auto range = stream.range_read(size);
                res.data.bits_per_entry = bits_per_entry;
                res.data.data.data() = list_array<uint64_t>((uint64_t*)range.data_read(), range.size_read() / 8);
                value.decompile(std::move(res));
            } else {
                copper_server::base_objects::palette_data res(bits_per_entry, entries_count);
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                size /= 8;
                auto range = stream.range_read(size);
                res.data.data() = list_array<uint64_t>((uint64_t*)range.data_read(), range.size_read() / 8);
                value.decompile(std::move(res));
            }
        }

        static void write(const palette_container_block& palette, enbt::io_helper::value_write_stream& write_stream) {
            copper_server::base_objects::network::response_item res;
            std::visit(
                [&]<class IT>(const IT& it) {
                    if constexpr (std::is_same_v<copper_server::base_objects::palette_container_indirect, IT>) {
                        res.write_value(it.bits_per_entry);
                        res.write_var32_check(it.palette.size());
                        for (auto& i : it.palette)
                            res.write_var32(i);
                        res.write_direct(it.data.get());
                    } else if constexpr (std::is_same_v<copper_server::base_objects::palette_container_single, IT>) {
                        res.write_value((uint8_t)0);
                        res.write_var32(it.id_of_palette);
                    } else if constexpr (std::is_same_v<copper_server::base_objects::palette_data, IT>) {
                        res.write_value((uint8_t)it.bits_per_entry);
                        res.write_direct(it.get());
                    }
                },
                palette.compile()
            );
            write_stream.write_sarray_dir(res.data.data(), res.data.size());
        }
    };

    template <>
    struct serialization<palette_container_biome> {
        static palette_container_biome read(enbt::io_helper::value_read_stream& self) {
            palette_container_biome palette;
            read(palette, self);
            return palette;
        }

        static void read(palette_container_biome& value, enbt::io_helper::value_read_stream& self) {
            auto arr = self.iterate_into<uint8_t>();
            copper_server::ArrayStream stream(arr.data(), arr.size());
            auto bits_per_entry = stream.read_value<uint8_t>();
            static constexpr auto max_indirect = copper_server::base_objects::palette_container::max_indirect_biomes;
            static constexpr auto entries_count = 64;
            if (bits_per_entry == 0) {
                copper_server::base_objects::palette_container_single res;
                res.id_of_palette = stream.read_var<int32_t>();
                value.decompile(std::move(res));
            } else if (bits_per_entry <= max_indirect) {
                copper_server::base_objects::palette_container_indirect res(bits_per_entry, entries_count);
                uint32_t palette = stream.read_var<uint32_t>();
                res.palette.reserve(palette);
                for (uint32_t i = 0; i < palette; i++)
                    res.palette.push_back(stream.read_var<uint32_t>());
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                size /= 8;
                auto range = stream.range_read(size);
                res.data.bits_per_entry = bits_per_entry;
                res.data.data.data() = list_array<uint64_t>((uint64_t*)range.data_read(), range.size_read() / 8);
                value.decompile(std::move(res));
            } else {
                copper_server::base_objects::palette_data res(bits_per_entry, entries_count);
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                size /= 8;
                auto range = stream.range_read(size);
                res.data.data() = list_array<uint64_t>((uint64_t*)range.data_read(), range.size_read() / 8);
                value.decompile(std::move(res));
            }
        }

        static void write(const palette_container_biome& palette, enbt::io_helper::value_write_stream& write_stream) {
            copper_server::base_objects::network::response_item res;
            std::visit(
                [&]<class IT>(const IT& it) {
                    if constexpr (std::is_same_v<copper_server::base_objects::palette_container_indirect, IT>) {
                        res.write_value(it.bits_per_entry);
                        res.write_var32_check(it.palette.size());
                        for (auto& i : it.palette)
                            res.write_var32(i);
                        res.write_direct(it.data.get());
                    } else if constexpr (std::is_same_v<copper_server::base_objects::palette_container_single, IT>) {
                        res.write_value((uint8_t)0);
                        res.write_var32(it.id_of_palette);
                    } else if constexpr (std::is_same_v<copper_server::base_objects::palette_data, IT>) {
                        res.write_value((uint8_t)it.bits_per_entry);
                        res.write_direct(it.get());
                    }
                },
                palette.compile()
            );
            write_stream.write_sarray_dir(res.data.data(), res.data.size());
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
            self
                .read_compound()
                .collect("ocean_floor", [&height_maps](auto& stream) { serialization_read(height_maps.ocean_floor, stream); })
                .collect("motion_blocking", [&height_maps](auto& stream) { serialization_read(height_maps.motion_blocking, stream); })
                .collect("motion_blocking_no_leaves", [&height_maps](auto& stream) { serialization_read(height_maps.motion_blocking_no_leaves, stream); })
                .collect("surface", [&height_maps](auto& stream) { serialization_read(height_maps.surface, stream); })
                .make_collect();
        }

        static void write(const height_maps& height_maps, enbt::io_helper::value_write_stream& write_stream) {
            write_stream.write_compound(4)
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

namespace copper_server::base_objects::world {
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

    bool chunk_data::load(const std::filesystem::path& path, uint64_t tick_counter, storage::world_data& world) {
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
        sub_chunks.reserve(world.get_chunk_y_count());
        uint8_t format_version = enbt::io_helper::read_token(filter);
        enbt::io_helper::value_read_stream stream(filter);
        switch (format_version) {
        case 0:
            stream
                .read_compound()
                .collect("height_maps", [&](auto& self) { enbt::io_helper::serialization_read(height_maps, self); })
                .collect_as("generator_stage", generator_stage)
                .collect_as("resume_gen_level", resume_gen_level)
                .collect_iterate("sub_chunks", [&](enbt::io_helper::value_read_stream& self) {
                    sub_chunks.emplace_back();
                    auto& sub_chunk_data = sub_chunks.back();
                    bool need_recalculate_light_block_light = true;
                    bool need_recalculate_light_sky_light = true;
                    self
                        .read_compound()
                        .collect("blocks", [&](auto& stream) { enbt::io_helper::serialization_read(sub_chunk_data.blocks, stream); })
                        .collect("has_tickable_blocks", [&](auto& stream) { sub_chunk_data.has_tickable_blocks = true; })
                        .collect("block_light", [&](auto& stream) { enbt::io_helper::serialization_read(sub_chunk_data.block_light, stream); need_recalculate_light_block_light = false; })
                        .collect("sky_light", [&](auto& stream) { enbt::io_helper::serialization_read(sub_chunk_data.sky_light, stream); need_recalculate_light_sky_light = false; })
                        .collect("biomes", [&](auto& stream) { enbt::io_helper::serialization_read(sub_chunk_data.biomes, stream); need_recalculate_light_sky_light = false; })
                        .collect_iterate(
                            "block_entities",
                            [&sub_chunk_data](std::uint64_t len) { sub_chunk_data.block_entities.reserve(len); },
                            [&sub_chunk_data](enbt::io_helper::value_read_stream& self) {
                                base_objects::local_block_pos local_pos;
                                self
                                    .read_compound(true)
                                    .collect("x", [&](auto& stream) { local_pos.x = stream.read(); })
                                    .collect("y", [&](auto& stream) { local_pos.y = stream.read(); })
                                    .collect("z", [&](auto& stream) { local_pos.z = stream.read(); })
                                    .collect("id", [&](auto& stream) {
                                        base_objects::block_id_t id = 0;
                                        self.read_as(id);
                                        sub_chunk_data.set_block(local_pos.x, local_pos.y, local_pos.z, base_objects::block(id), world.);
                                    })
                                    .collect("nbt", [&](auto& stream) {
                                        sub_chunk_data.block_entities[local_pos.z | (local_pos.y << 4) | (local_pos.x << 8)] = stream.read();
                                    })
                                    .force_all_collect();
                            }
                        )
                        .make_collect([](auto& name, auto& stream) { stream.read(); });
                    sub_chunk_data.need_to_recalculate_light = need_recalculate_light_block_light || need_recalculate_light_sky_light;
                    if (!sub_chunk_data.need_to_recalculate_light) {
                        if (!need_recalculate_light_block_light)
                            sub_chunk_data.block_lighted = sub_chunk_data.block_light.is_lighted();

                        if (!need_recalculate_light_sky_light)
                            sub_chunk_data.sky_lighted = sub_chunk_data.sky_light.is_lighted();
                    }
                })
                .collect_iterate( //format-fix
                    "entities",

                    [&](std::uint64_t len) { stored_entities.reserve(len); },
                    [&](enbt::io_helper::value_read_stream& self) {
                        auto res = api::entity::load_from_file(self);
                        world.register_entity(res);
                    }
                )
                .collect_iterate( //format-fix
                    "queried_for_tick",
                    [&](std::uint64_t len) { queried_for_tick.reserve(len); },
                    [&](enbt::io_helper::value_read_stream& self) {
                        list_array<std::pair<uint64_t, base_objects::chunk_block_pos>> queried_for_tick_tmp;
                        self.iterate(
                            [&queried_for_tick_tmp](std::uint64_t len) { queried_for_tick_tmp.reserve(len); },
                            [&queried_for_tick_tmp, &tick_counter](enbt::io_helper::value_read_stream& self) {
                                base_objects::chunk_block_pos block_pos;
                                uint32_t duration;
                                self
                                    .read_compound(true)
                                    .collect("x", [&block_pos](auto& stream) { block_pos.x = stream.read(); })
                                    .collect("y", [&block_pos](auto& stream) { block_pos.x = stream.read(); })
                                    .collect("z", [&block_pos](auto& stream) { block_pos.x = stream.read(); })
                                    .collect_as("duration", duration)
                                    .force_all_collect();
                                queried_for_tick_tmp.push_back({tick_counter + duration, block_pos});
                            }
                        );
                        queried_for_tick.push_back(std::move(queried_for_tick_tmp));
                    }
                )
                .make_collect([](auto& name, auto& stream) { stream.read(); });
        }
        return true;
    }

    bool chunk_data::load(const enbt::compound_const_ref& chunk_data, uint64_t tick_counter, storage::world_data& world) {
        return false;
    }

    bool chunk_data::save(const std::filesystem::path& path, uint64_t tick_counter, storage::world_data& world) {
        if (api::configuration::get().server.world_debug_mode)
            return false;
        {
            fast_task::files::async_iofstream file( //the std::filesystem::create_directories is slower without check
                path,
                fast_task::files::open_mode::write,
                fast_task::files::on_open_action::open,
                fast_task::files::_sync_flags{}
            );
            if (!file.is_open())
                std::filesystem::create_directories(path.parent_path());
        }
        fast_task::files::atomic_async_ofstream file(path);
        if (!file.is_open())
            return false;
        auto mode = api::configuration::get().world.saving_mode;
        enbt::io_helper::write_token(file, mode);
        boost::iostreams::filtering_ostream filter;
        if (mode == "zstd")
            filter.push(boost::iostreams::zstd_compressor());
        filter.push(file);
        enbt::io_helper::write_token(filter, (uint8_t)0);
        std::ostringstream str;
        enbt::io_helper::value_write_stream stream(str);
        {
            auto comp
                = stream.write_compound(5 + (generator_stage != 0xFF) + (resume_gen_level != 255))
                      .write("sub_chunks", [&](enbt::io_helper::value_write_stream& stream) {
                          stream.write_array(sub_chunks.size()).iterable(sub_chunks, [&](const world::sub_chunk_data& sub_chunk, enbt::io_helper::value_write_stream& stream) {
                              auto compound = stream.write_compound(3 + (sub_chunk.need_to_recalculate_light ? 0 : 2) + sub_chunk.has_tickable_blocks);
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

                                      auto compound = stream.write_compound(5);
                                      compound.write("x", pos.x);
                                      compound.write("y", pos.y);
                                      compound.write("z", pos.z);
                                      compound.write("id", sub_chunk.blocks.get(pos.x, pos.y, pos.z));
                                      compound.write("nbt", data);
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
                              if (sub_chunk.has_tickable_blocks)
                                  compound.write("has_tickable_blocks", true);
                          });
                      })
                      .write("entities", [&](enbt::io_helper::value_write_stream& stream) {
                          auto entities = stream.write_array(stored_entities.size());
                          for (auto& [id, entity] : stored_entities)
                              if (entity.is_assigned_to_world(world.world_id))
                                  if (entity.get<api::ecs::com::world_syncing>().assigned_world_id == id)
                                      if (entity.get<api::ecs::com::entity_type>().const_data().is_saveable)
                                          entities.write([&entity](auto& stream) {
                                              api::entity::store_to_file(entity, stream);
                                          });
                      })
                      .write("queried_for_tick", [&](enbt::io_helper::value_write_stream& stream) {
                          stream.write_array(queried_for_tick.size()).iterable(queried_for_tick, [&](auto& item, enbt::io_helper::value_write_stream& stream) {
                              stream.write_array(item.size()).iterable(item, [&](auto& item, enbt::io_helper::value_write_stream& stream) {
                                  auto& [till_tick, block_pos] = item;
                                  auto compound = stream.write_compound(4);
                                  compound.write("x", block_pos.x);
                                  compound.write("y", block_pos.y);
                                  compound.write("z", block_pos.z);
                                  compound.write("duration", till_tick - tick_counter);
                              });
                          });
                      })
                      .write("queried_for_liquid_tick", [&](enbt::io_helper::value_write_stream& stream) {
                          stream.write_array(queried_for_tick.size()).iterable(queried_for_liquid_tick, [&](auto& item, enbt::io_helper::value_write_stream& stream) {
                              auto& [till_tick, block_pos] = item;
                              auto compound = stream.write_compound(4);
                              compound.write("x", block_pos.x);
                              compound.write("y", block_pos.y);
                              compound.write("z", block_pos.z);
                              compound.write("duration", till_tick - tick_counter);
                          });
                      })
                      .write("height_maps", [&](enbt::io_helper::value_write_stream& stream) {
                          enbt::io_helper::serialization_write(height_maps, stream);
                      });
            if (generator_stage != 0xFF)
                comp.write("generator_stage", generator_stage);
            if (resume_gen_level != 255)
                comp.write("resume_gen_level", resume_gen_level);
        }

        filter << str.view();
        filter.flush();
        file.flush();
        return true;
    }

    chunk_data::chunk_data(int64_t chunk_x, int64_t chunk_z)
        : chunk_x(chunk_x), chunk_z(chunk_z) {}

    void chunk_data::update_height_map_on(uint8_t local_x, uint64_t local_y_block, uint8_t local_z) {
        if (local_y_block == 0)
            return;
        auto& leaves = api::tags::unfold_tag(api::tags::builtin_entry::block, "minecraft:block/leaves");
        auto bloc = get_block(local_x, local_y_block, local_z);
        if (!bloc.is_air()) {
            if (height_maps.ocean_floor[local_x][local_z] < local_y_block)
                height_maps.ocean_floor[local_x][local_z] = local_y_block;
            if (bloc.is_liquid()) {
                if (height_maps.surface[local_x][local_z] < local_y_block)
                    height_maps.surface[local_x][local_z] = local_y_block;
            }
            if (bloc.is_solid()) {
                if (height_maps.motion_blocking[local_x][local_z] < local_y_block)
                    height_maps.motion_blocking[local_x][local_z] = local_y_block;

                if (!leaves.contains(bloc.general_block_id()))
                    if (height_maps.motion_blocking_no_leaves[local_x][local_z] < local_y_block)
                        height_maps.motion_blocking_no_leaves[local_x][local_z] = local_y_block;
            }

        } else {
            uint64_t to_skip = local_y_block / 16 + bool(local_y_block % 16);
            auto end = sub_chunks.rend();

            for (auto beg = sub_chunks.rbegin(); beg != end; ++beg) {
                if (to_skip) {
                    --to_skip;
                    continue;
                }
                auto& schunk = *beg;
                for (int8_t y = 15; y >= 0; y--) {
                    auto block = schunk.get_block(local_x, y, local_z);
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
                local_y_block -= 16;
            }
        }
    }

    void chunk_data::update_height_map() {
        height_maps.make_zero();
        uint64_t local_y_block = (sub_chunks.size() - 1) * 16;
        auto& leaves = api::tags::unfold_tag(api::tags::builtin_entry::block, "minecraft:block/leaves");
        auto end = sub_chunks.rend();
        for (auto beg = sub_chunks.rbegin(); beg != end; ++beg) {
            auto& schunk = *beg;
            for (uint8_t x = 0; x < 16; x++) {
                for (int8_t y = 15; y >= 0; y--) {
                    for (uint8_t z = 0; z < 16; z++) {
                        auto block = schunk.get_block(x, y, z);
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
            local_y_block -= 16;
        }
    }

    void chunk_data::calculate_active() {
        for (auto& schunk : sub_chunks) {
            schunk.active_blocks = 0;
            for (uint8_t x = 0; x < 16; x++)
                for (int8_t y = 0; y < 16; y++)
                    for (uint8_t z = 0; z < 16; z++)
                        schunk.active_blocks += !schunk.get_block(x, y, z).is_air();
        }
    }

    void chunk_data::update_metadata() {
        height_maps.make_zero();
        uint64_t local_y_block = (sub_chunks.size() - 1) * 16;
        auto& leaves = api::tags::unfold_tag(api::tags::builtin_entry::block, "minecraft:block/leaves");
        auto end = sub_chunks.rend();
        for (auto beg = sub_chunks.rbegin(); beg != end; ++beg) {
            auto& schunk = *beg;
            schunk.active_blocks = 0;
            for (uint8_t x = 0; x < 16; x++) {
                for (int8_t y = 15; y >= 0; y--) {
                    for (uint8_t z = 0; z < 16; z++) {
                        auto block = schunk.get_block(x, y, z);
                        if (!block.is_air()) {
                            schunk.active_blocks += 1;
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
            local_y_block -= 16;
        }
    }

    void chunk_data::for_each_block_entity(const std::function<void(api::ecs::entity block_entity)>& func) {
        for (auto& sub_chunk : sub_chunks)
            for (auto& [_pos, data] : sub_chunk.block_entities) {
                base_objects::local_block_pos pos;
                pos.x = _pos >> 8;
                pos.y = (_pos >> 4) & 0xF;
                pos.z = _pos & 0xF;
                func(data);
            }
    }

    void chunk_data::for_each_block_entity(uint64_t local_y, const std::function<void(api::ecs::entity block_entity)>& func) {
        if (local_y < sub_chunks.size())
            for (auto& [_pos, data] : sub_chunks[local_y].block_entities) {
                base_objects::local_block_pos pos;
                pos.x = _pos >> 8;
                pos.y = (_pos >> 4) & 0xF;
                pos.z = _pos & 0xF;
                func(data);
            }
    }

    void chunk_data::for_each_sub_chunk(const std::function<void(sub_chunk_data& sub_chunk)>& func) {
        for (auto& sub_chunk : sub_chunks)
            func(sub_chunk);
    }

    void chunk_data::get_sub_chunk(uint64_t sub_chunk_y, const std::function<void(sub_chunk_data& sub_chunk)>& func) {
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

    void chunk_data::tick_players_sleep(storage::chunk_tick_result& rr, storage::world_data& world) {
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_scheduled_blocks(storage::chunk_tick_result& rr, storage::world_data& world) {
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

                sub_chunk.get_block(block_pos.x, (uint8_t)local, block_pos.z).tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, block_pos.x, (uint8_t)local, block_pos.z, false);
            }
        }
    }

    void chunk_data::tick_scheduled_fluids(storage::chunk_tick_result& rr, storage::world_data& world) {
        if (load_level > 32)
            return;
        for (
            auto& [till, block_pos] :
            queried_for_liquid_tick.take([&world](auto& it) {
                return it.first >= world.tick_counter;
            })
        ) {
            auto sub_chunk_y = convert_chunk_global_pos(block_pos.y);
            auto local = convert_chunk_local_pos(block_pos.y);
            auto& sub_chunk = sub_chunks.at(sub_chunk_y);

            sub_chunk.get_block(block_pos.x, (uint8_t)local, block_pos.z).tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, block_pos.x, (uint8_t)local, block_pos.z, false);
        }
    }

    void chunk_data::tick_raid(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_spawn_mobs(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_ice_snow(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_random_ticks(storage::chunk_tick_result& rr, storage::world_data& world, size_t random_tick_speed, std::mt19937& random_engine) {
        if (load_level > 32)
            return;

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
                auto block = sub_chunk.get_block(pos.dec.x, pos.dec.y, pos.dec.z);
                if (block.is_tickable())
                    block.tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, pos.dec.x, pos.dec.y, pos.dec.z, true);
                --max_random_tick_per_sub_chunk;
            }
            sub_chunk_y++;
        }
    }

    void chunk_data::tick_poi(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_block_event(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_dragon(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_entity(storage::chunk_tick_result& rr, storage::world_data& world, std::mt19937& random_engine) {
        auto max_inactivity = api::configuration::get().game_play.entity.despawn.despawn_after_inactivity;
        auto despawn_chance = api::configuration::get().game_play.entity.despawn.despawn_chance;
        std::normal_distribution<> dis(0.0, 1.0);
        for (auto& [id, entity] : stored_entities) {
            if (entity.is_assigned_to_world(world.world_id)) {
                auto sd = entity.modify<api::ecs::com::world_syncing>();
                if (entity.has<api::ecs::com::assigned_player>())
                    ; //skip check
                else if (sd->despawn_immune)
                    ; //skip check
                else if (sd->state == api::ecs::com::world_syncing::state_e::scheduled_for_despawn) {
                    world.unregister_entity(entity);
                    rr.unrelated_entities.push_back(id);
                } else if (sd->state == api::ecs::com::world_syncing::state_e::no_player) {
                    sd->state = api::ecs::com::world_syncing::state_e::scheduled_for_despawn;
                } else if (sd->inactivity_counter > max_inactivity) {
                    if (dis(random_engine) >= despawn_chance)
                        sd->state = api::ecs::com::world_syncing::state_e::scheduled_for_despawn;
                } else
                    sd->state = api::ecs::com::world_syncing::state_e::no_player;

                if (entity.has<api::ecs::com::assigned_player>()) {
                    auto& pos = entity.get<api::ecs::com::position>();
                    world.for_each_entity(
                        base_objects::spherical_bounds_block{
                            (int64_t)pos.x,
                            (int64_t)pos.y,
                            (int64_t)pos.z,
                            api::configuration::get().game_play.entity.despawn_mobs_outside
                        },
                        [&pos, t_m_r = api::configuration::get().game_play.entity.squared_values.tick_mobs_in_range](auto mark_entity) {
                            auto& mark_pos = mark_entity.get<api::ecs::com::position>();
                            if (mark_entity.has<api::ecs::com::assigned_player>())
                                return;
                            auto sd = mark_entity.modify<api::ecs::com::world_syncing>();
                            if (sd->despawn_immune || sd->inactivity_immune)
                                return;
                            switch (sd->state) {
                            case api::ecs::com::world_syncing::state_e::init:
                            case api::ecs::com::world_syncing::state_e::no_player: {
                                auto dist_sq = util::distance_sq(pos, mark_pos); //how to be with fish? fish has different despawn range
                                sd->state = dist_sq > t_m_r ? api::ecs::com::world_syncing::state_e::player_far : api::ecs::com::world_syncing::state_e::player_near;
                                if (sd->state == api::ecs::com::world_syncing::state_e::player_near)
                                    sd->inactivity_counter = 0;
                                else
                                    ++sd->inactivity_counter;
                                break;
                            }
                            default:
                                break;
                            }
                        }
                    );
                }
                if (load_level > 31)
                    continue;

                if (entity.has<api::ecs::com::ride_entity>()) {
                    if (!entity.get<api::ecs::com::ride_entity>().other) {
                        entity.get<api::ecs::com::entity_type>().tick(entity);
                        if (entity.has<api::ecs::com::ride_by_entity>())
                            for (auto ride_entity : entity.get<api::ecs::com::ride_by_entity>().ride_by)
                                ride_entity.get<api::ecs::com::entity_type>().tick(ride_entity);
                    }
                } else {
                    entity.get<api::ecs::com::entity_type>().tick(entity);
                    if (entity.has<api::ecs::com::ride_by_entity>())
                        for (auto ride_entity : entity.get<api::ecs::com::ride_by_entity>().ride_by)
                            ride_entity.get<api::ecs::com::entity_type>().tick(ride_entity);
                }
                continue;
            } else
                rr.unrelated_entities.push_back(id);
        }
        for (auto id : rr.unrelated_entities)
            stored_entities.erase(id);
    }

    void chunk_data::tick_block_entity(storage::chunk_tick_result& rr, storage::world_data& world) {
        if (load_level > 32)
            return;
        uint64_t y = 0;
        for (auto& sub_chunk : sub_chunks) {
            for (auto& [_pos, data] : sub_chunk.block_entities) {
                base_objects::local_block_pos pos;
                pos.x = _pos >> 8;
                pos.y = (_pos >> 4) & 0xF;
                pos.z = _pos & 0xF;
                auto block = sub_chunk.get_block(pos.x, pos.y, pos.z);
                if (block.is_tickable())
                    block.tick(world, sub_chunk, chunk_x, y, chunk_z, pos.x, pos.y, pos.z, false);
            }
            ++y;
        }
    }

    void chunk_data::tick_game_event(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::set_state(uint8_t local_x, uint64_t local_y, uint8_t local_z, base_objects::block_id_t id, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_state(local_x, local_y & 15, local_z, id, world);
    }

    void chunk_data::set_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, base_objects::block block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::set_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, api::ecs::entity&& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, std::move(block), world);
    }

    void chunk_data::set_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, const api::ecs::entity& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::set_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, const base_objects::any_block& block, api::ecs::world_local_registry& world){
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::set_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, base_objects::any_block&& block, api::ecs::world_local_registry& world){
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, std::move(block), world);
    }

    base_objects::block chunk_data::get_block(uint8_t local_x, uint64_t local_y, uint8_t local_z) {
        return sub_chunks.at(local_y >> 4).get_block(local_x, local_y & 15, local_z);
    }

    api::ecs::entity chunk_data::get_block_entity(uint8_t local_x, uint64_t local_y, uint8_t local_z) {
        return sub_chunks.at(local_y >> 4).get_block_entity(local_x, local_y & 15, local_z);
    }

    //generator functions
    void chunk_data::gen_set_state(uint8_t local_x, uint64_t local_y, uint8_t local_z, base_objects::block_id_t id, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_state_gen(local_x, local_y & 15, local_z, id, world);
    }

    void chunk_data::gen_set_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, base_objects::block block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block_gen(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::gen_set_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, api::ecs::entity&& block, api::ecs::world_local_registry& world){
        sub_chunks.at(local_y >> 4).set_block_gen(local_x, local_y & 15, local_z, std::move(block), world);
    }

    void chunk_data::gen_set_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, const api::ecs::entity& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block_gen(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::gen_remove_block(uint8_t local_x, uint64_t local_y, uint8_t local_z, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block_gen(local_x, local_y & 15, local_z, base_objects::block(), world);
    }
}
