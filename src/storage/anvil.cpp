/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

#include <src/api/id.hpp>
#include <src/storage/anvil.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server::storage {
    anvil::anvil(const std::filesystem::path& region_path) : manager(region_path, "mca") {}

    constexpr auto world_version = 4556;

    fast_task::future_ptr<std::shared_ptr<base_objects::world::chunk_data>> anvil::get_chunk(int32_t chunk_x, int32_t chunk_z) {
        return fast_task::future_tool::chain<
            std::shared_ptr<base_objects::world::chunk_data>,
            std::vector<uint8_t>>(
            manager.get_chunk(chunk_x, chunk_z),
            [](std::vector<uint8_t>&& data) {
                boost::iostreams::stream<boost::iostreams::basic_array_source<char>> stream((const char*)data.data(), data.size());
                util::nbt_read_stream read(stream);

                int8_t section_pos = 0;
                base_objects::world::sub_chunk_data current_section;
                base_objects::palette_container_indirect palette_build(1, 1);
                util::nbt_collection::compound_flex section_collect;
                bool block_light_loaded = false;
                bool sky_light_loaded = false;
                section_collect.collect_as_required("Y", section_pos);
                section_collect.collect_required("block_states", [&](util::nbt_read_stream& it) {
                    it.read_compound()
                        .collect("palette", [&palette_build](auto& it) {
                            std::vector<int32_t> block_pallete;
                            it.iterate(
                                [&block_pallete](auto size) { block_pallete.reserve(size); },
                                [&block_pallete](util::nbt_read_stream& iter) {
                                    std::string name;
                                    std::unordered_map<std::string, std::string> props;
                                    util::nbt_collection::compound_flex props_collect;
                                    props_collect.collect_into_required("Name", name);
                                    props_collect.collect_iterate("Properties", [&props](auto& name, auto& value_it) {
                                        std::string value;
                                        value_it.read_into(value);
                                        props.emplace(name, value);
                                    });
                                    props_collect.make_collect(iter);

                                    block_pallete.push_back(
                                        base_objects::block::get_block(name)
                                            .assigned_states_to_properties
                                            ->right
                                            .at(props)
                                    );
                                }
                            );
                            palette_build.bits_per_entry = base_objects::palette_data::bits_for_max(block_pallete.size());
                            palette_build.palette = std::move(block_pallete);
                        })
                        .collect("data", [&palette_build](auto& it) {
                            palette_build.data.get() = it.iterate_into<uint64_t>();
                        })
                        .force_all_collect();

                    palette_build.data.bits_per_entry = palette_build.bits_per_entry;
                    current_section.blocks.decompile(std::move(palette_build));
                });
                section_collect.collect_required("biomes", [&](util::nbt_read_stream& it) {
                    it.read_compound()
                        .collect("palette", [&palette_build](auto& it) {
                            std::vector<int32_t> block_pallete;
                            it.iterate(
                                [&block_pallete](auto size) { block_pallete.reserve(size); },
                                [&block_pallete](util::nbt_read_stream& iter) {
                                    std::string name;
                                    util::nbt_collection::compound_flex props_collect;
                                    props_collect.collect_into_required("Name", name);
                                    block_pallete.push_back(api::id::worldgen__biome(name).value);
                                }
                            );
                            palette_build.bits_per_entry = base_objects::palette_data::bits_for_max(block_pallete.size());
                            palette_build.palette = std::move(block_pallete);
                        })
                        .collect("data", [&palette_build](auto& it) {
                            palette_build.data.get() = it.iterate_into<uint64_t>();
                        })
                        .force_all_collect();

                    palette_build.data.bits_per_entry = palette_build.bits_per_entry;
                    current_section.biomes.decompile(std::move(palette_build));
                });
                section_collect.collect("BlockLight", [&](util::nbt_read_stream& it) {
                    it.iterate_into((int8_t*)current_section.block_light.light_map, sizeof(current_section.block_light.light_map));
                    current_section.block_lighted = current_section.block_light.is_lighted();
                    block_light_loaded = true;
                    if (sky_light_loaded)
                        current_section.need_to_recalculate_light = false;
                });
                section_collect.collect("SkyLight", [&](util::nbt_read_stream& it) {
                    it.iterate_into((int8_t*)current_section.sky_light.light_map, sizeof(current_section.sky_light.light_map));
                    current_section.sky_lighted = current_section.sky_light.is_lighted();
                    sky_light_loaded = true;
                    if (block_light_loaded)
                        current_section.need_to_recalculate_light = false;
                });

                auto collect_section = [&](util::nbt_read_stream& it) {
                    block_light_loaded = false;
                    sky_light_loaded = false;
                    current_section = base_objects::world::sub_chunk_data{};
                    section_collect.make_collect(it);
                    return std::move(current_section);
                };


                int32_t data_version;
                int32_t x_pos;
                int32_t y_pos;
                int32_t z_pos;
                std::string status;
                int64_t last_update;
                std::vector<base_objects::world::sub_chunk_data> sections;

                util::nbt_collection::compound_flex collect;
                collect
                    .collect_as_required("DataVersion", data_version)
                    .collect_as_required("xPos", x_pos)
                    .collect_as_required("zPos", z_pos)
                    .collect_as_required("yPos", y_pos)
                    .collect_as_required("Status", status)
                    .collect_as_required("LastUpdate", last_update)
                    .collect_iterate_required("sections", [&collect_section, &section_pos, &sections](auto& it) {
                        auto sect = collect_section(it);
                        if (sections.size() <= (uint8_t)section_pos)
                            sections.resize(uint16_t(section_pos) + 1);
                        sections[(uint8_t)section_pos] = std::move(sect);
                    })
                    .collect_iterate_required("block_entities", [](auto& name, auto& it) {
                        api::id::block_entity_type id;


                        //TODO
                    })


                    .make_collect(read)


                    ;


                if (data_version != world_version) //TODO add automatic upgrades
                    throw std::runtime_error("Incompatible world version");

                auto res = std::make_shared<base_objects::world::chunk_data>(x_pos, z_pos);

                if (status == "minecraft:empty" || status == "empty")
                    res->generator_stage = 0;
                else if (status == "minecraft:structures_starts" || status == "structures_starts")
                    res->generator_stage = 1;
                else if (status == "minecraft:biomes" || status == "biomes")
                    res->generator_stage = 2;
                else if (status == "minecraft:noise" || status == "noise")
                    res->generator_stage = 3;
                else if (status == "minecraft:surface" || status == "surface")
                    res->generator_stage = 5;
                else if (status == "minecraft:carvers" || status == "carvers" || status == "minecraft:liquid_carvers" || status == "liquid_carvers")
                    res->generator_stage = 6;
                else if (status == "minecraft:features" || status == "features")
                    res->generator_stage = 7;
                else if (status == "minecraft:initialize_light" || status == "initialize_light")
                    res->generator_stage = 8;
                else if (status == "minecraft:light" || status == "light")
                    res->generator_stage = 9;
                else if (status == "minecraft:spawn" || status == "spawn")
                    res->generator_stage = 10;
                else if (status == "minecraft:full" || status == "full")
                    res->generator_stage = 0xFF;
                else
                    throw std::runtime_error("Invalid chunk status.");


                return res;
            }
        );
    }

    fast_task::future_ptr<void> anvil::write_chunk(int32_t chunk_x, int32_t chunk_z, std::shared_ptr<base_objects::world::chunk_data> chunk, region_storage::compression_type type, bool use_external_file){
        return fast_task::future<void>::make_ready();
    }

    fast_task::future_ptr<void> anvil::write_chunk(int32_t chunk_x, int32_t chunk_z, std::shared_ptr<base_objects::world::chunk_data> chunk, const std::string& type, bool use_external_file) {
        return fast_task::future<void>::make_ready();
    }
}
