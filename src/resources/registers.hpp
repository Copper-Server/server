#ifndef SRC_RESOURCES_REGISTERS
#define SRC_RESOURCES_REGISTERS
#include <filesystem>

namespace crafted_craft {
    namespace resources {
        void initialize_registers();
        void initialize_entities();

        //Accepts types:
        // biomes,
        // chatType
        // armorTrimPattern
        // armorTrimMaterial
        // wolfVariant
        // dimensionType
        // damageType
        // bannerPattern
        // paintingVariant
        // tag/[the tag type] // like from file data/<namespace>/tags/<function>/<name>.json // the `function` is the tag type
        // tags/[the tag type]
        //
        // *Note: The tags can be recursive, ignores unregistered tags and does not check object resource location because this is plugin responsibility, tags can be any type(like data/<namespace>/tags/Hello world/<name>.json)
        // plugin can use the `registers::unfold_tag` function to get the values of the tag
        //
        // plugins can set `namespace_`/`path_`/`type` to any value, the `file_path` not checked and used only to read file
        // to use default namespace set `namespace_` to empty string
        void load_register_file(const std::filesystem::path& file_path, std::string namespace_, const std::string& path_, const std::string& type);

        //resets all registers
        void registers_reset();

        //optimizes tags access and sets ids for all registers
        void load_registers_complete();
    }
}
#endif /* SRC_RESOURCES_REGISTERS */
