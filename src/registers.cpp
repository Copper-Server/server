
#include "registers.hpp"

namespace crafted_craft {
    namespace registers {
        std::unordered_map<std::string, ArmorTrimMaterial> armorTrimMaterials;
        std::unordered_map<std::string, ArmorTrimPattern> armorTrimPatterns;
        std::unordered_map<std::string, Biome> biomes;
        std::unordered_map<std::string, ChatType> chatTypes;
        std::unordered_map<std::string, DamageType> damageTypes;
        std::unordered_map<std::string, DimensionType> dimensionTypes;
        std::unordered_map<std::string, WolfVariant> wolfVariants;
        std::unordered_map<std::string, BannerPattern> bannerPatterns;
        std::unordered_map<std::string, PaintingVariant> paintingVariants;

        list_array<std::unordered_map<std::string, ArmorTrimMaterial>::iterator> armorTrimMaterials_cache;
        list_array<std::unordered_map<std::string, ArmorTrimPattern>::iterator> armorTrimPatterns_cache;
        list_array<std::unordered_map<std::string, Biome>::iterator> biomes_cache;
        list_array<std::unordered_map<std::string, ChatType>::iterator> chatTypes_cache;
        list_array<std::unordered_map<std::string, DamageType>::iterator> damageTypes_cache;
        list_array<std::unordered_map<std::string, DimensionType>::iterator> dimensionTypes_cache;
        list_array<std::unordered_map<std::string, WolfVariant>::iterator> wolfVariants_cache;
        list_array<std::unordered_map<std::string, BannerPattern>::iterator> bannerPatterns_cache;
        list_array<std::unordered_map<std::string, PaintingVariant>::iterator> paintingVariants_cache;


        //SERVER
        std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> blockPalette;
        std::unordered_map<uint16_t, EntityType*> entityList;
        std::unordered_map<int32_t, ItemType*> itemList;
        std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>>> tags; //[type][namespace][tag][values]
        std::string default_namespace = "minecraft";

        const list_array<std::string>& unfold_tag(const std::string& type, const std::string& namespace_, const std::string& tag) {
            static list_array<std::string> empty;
            auto ns = tags.find(type);
            if (ns == tags.end())
                return empty;
            auto t = ns->second.find(namespace_);
            if (t == ns->second.end())
                return empty;
            auto y = t->second.find(tag);
            if (y == t->second.end())
                return empty;
            return y->second;
        }

        const list_array<std::string>& unfold_tag(const std::string& type, const std::string& tag) {
            return unfold_tag(type, default_namespace, tag);
        }
    }
}