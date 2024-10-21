#ifndef SRC_RESOURCES_REGISTERS
#define SRC_RESOURCES_REGISTERS
#include <filesystem>

namespace crafted_craft {
    namespace resources {
        void initialize_registers();
        void initialize_entities();

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
        // tag,/[the tag type] // like from file data/<namespace>/tags/<function>/<name>.json // the `function` is the tag type
        // tags,/[the tag type]
        // trim_pattern,
        // trim_material,
        // wolf_variant,
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
}
#endif /* SRC_RESOURCES_REGISTERS */
