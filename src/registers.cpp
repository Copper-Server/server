
#include <src/registers.hpp>

namespace copper_server {
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
        std::unordered_map<std::string, Advancement> advancements;
        std::unordered_map<std::string, JukeboxSong> jukebox_songs;


        std::unordered_map<uint32_t, enbt::compound> individual_registers;


        std::unordered_map<std::string, potion> potions;
        list_array<decltype(potions)::iterator> potions_cache;

        std::unordered_map<std::string, enchantment> enchantments;
        list_array<decltype(enchantments)::iterator> enchantments_cache;
        std::unordered_map<std::string, enbt::compound> enchantment_providers;

        std::unordered_map<std::string, loot_table_item> loot_table;
        list_array<decltype(loot_table)::iterator> loot_table_cache;

        std::unordered_map<int32_t, ItemType*> itemList;
        std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> blockPalette;
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
            if (tag.starts_with('#')) {
                if (auto nam = tag.find(':'); nam != tag.npos) {
                    std::string _namespace = tag.substr(1, nam - 1);
                    std::string _tag = tag.substr(nam);
                    return unfold_tag(type, _namespace, _tag);
                } else
                    return unfold_tag(type, default_namespace, tag.substr(1));
            }
            return unfold_tag(type, default_namespace, tag);
        }
    }
}