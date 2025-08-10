/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_RESOURCES_REGISTERS
#define SRC_RESOURCES_REGISTERS
#include <boost/json/value.hpp>
#include <filesystem>
#include <library/list_array.hpp>
#include <src/base_objects/data_packs/known_pack.hpp>

namespace copper_server::resources {
    void initialize_entities();

    void initialize();
    void prepare_built_in_pack();

    //processes whole pack data,
    //tags always processed first, correctly handles replace flag and processes circular dependencies without problem
    //! Thread unsafe, should be called from wrapped api instead
    void process_pack(const std::filesystem::path& folder_path_to_data_packs);
    void process_pack(const std::filesystem::path& folder_path_to_data_with_namespace, const std::string& namespace_, const std::string& id);
    void process_pack(boost::json::object& memory, const std::string& namespace_, const std::string& id);

    list_array<base_objects::data_packs::known_pack> loaded_packs();
    //Accepts types:
    // advancement,
    // banner_pattern,
    // chat_type,
    // damage_type,
    // dimension_type,
    // enchantment,
    // enchantment_provider,
    // jukebox_song,
    // loot_table,
    // painting_variant,
    // recipe,
    // tag/[the tag entry], // like from file data/<namespace>/tags/<entry>/<name>.json // the `entry` is the tag entry
    // tags/[the tag entry],
    // trim_pattern,
    // trim_material,
    // wolf_variant,
    // wolf_sound_variant,
    // cat_variant,
    // chicken_variant,
    // cow_variant,
    // frog_variant,
    // pig_variant,
    // worldgen/biome,
    // worldgen/configured_carver,
    // worldgen/configured_feature,
    // worldgen/density_function,
    // worldgen/flat_level_generator_preset,
    // worldgen/multi_noise_biome_source_parameter_list,
    // worldgen/noise,
    // worldgen/noise_settings,
    // worldgen/placed_feature,
    // worldgen/processor_list,
    // worldgen/structure,
    // worldgen/structure_set,
    // worldgen/template_pool,
    // worldgen/world_preset,
    //
    // *Note: The tags can be recursive, ignores unregistered tags and does not check object resource location because this is plugin responsibility, tags can be any type(like data/<namespace>/tags/Hello world/<name>.json)
    // plugin can use the `registers::unfold_tag` function to get the values of the tag
    //
    // plugins can set `namespace_`/`path_`/`type` to any value, the `file_path` not checked and used only to read file
    // to use default namespace set `namespace_` to empty string
    void load_register_file(const std::filesystem::path& file_path, const std::string& namespace_, const std::string& path_, const std::string& type);

    //used to load resource from memory
    void load_register_file(std::string_view memory, const std::string& namespace_, const std::string& path_, const std::string& type);

    //resets all registers
    void registers_reset();

    //optimizes tags access and sets ids for all registers
    void load_registers_complete();


    void load_blocks();
    void load_items();
    void prepare_versions();
}
#endif /* SRC_RESOURCES_REGISTERS */
