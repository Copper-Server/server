
#include <src/registers.hpp>

namespace copper_server {
    namespace registers {
        std::unordered_map<std::string, ArmorTrimMaterial> armorTrimMaterials;
        std::unordered_map<std::string, ArmorTrimPattern> armorTrimPatterns;
        std::unordered_map<std::string, Biome> biomes;
        std::unordered_map<std::string, ChatType> chatTypes;
        std::unordered_map<std::string, DamageType> damageTypes;
        std::unordered_map<std::string, DimensionType> dimensionTypes;
        std::unordered_map<std::string, WolfSoundVariant> wolfSoundVariants;
        std::unordered_map<std::string, WolfVariant> wolfVariants;
        std::unordered_map<std::string, EntityVariant> catVariants;
        std::unordered_map<std::string, EntityVariant> chickenVariants;
        std::unordered_map<std::string, EntityVariant> cowVariants;
        std::unordered_map<std::string, EntityVariant> pigVariants;
        std::unordered_map<std::string, EntityVariant> frogVariants;
        std::unordered_map<std::string, BannerPattern> bannerPatterns;
        std::unordered_map<std::string, PaintingVariant> paintingVariants;
        std::unordered_map<std::string, Instrument> instruments;

        list_array<std::unordered_map<std::string, ArmorTrimMaterial>::iterator> armorTrimMaterials_cache;
        list_array<std::unordered_map<std::string, ArmorTrimPattern>::iterator> armorTrimPatterns_cache;
        list_array<std::unordered_map<std::string, Biome>::iterator> biomes_cache;
        list_array<std::unordered_map<std::string, ChatType>::iterator> chatTypes_cache;
        list_array<std::unordered_map<std::string, DamageType>::iterator> damageTypes_cache;
        list_array<std::unordered_map<std::string, DimensionType>::iterator> dimensionTypes_cache;
        list_array<std::unordered_map<std::string, WolfSoundVariant>::iterator> wolfSoundVariants_cache;
        list_array<std::unordered_map<std::string, WolfVariant>::iterator> wolfVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> catVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> chickenVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> cowVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> pigVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> frogVariants_cache;
        list_array<std::unordered_map<std::string, BannerPattern>::iterator> bannerPatterns_cache;
        list_array<std::unordered_map<std::string, PaintingVariant>::iterator> paintingVariants_cache;
        list_array<std::unordered_map<std::string, Instrument>::iterator> instruments_cache;

        //SERVER
        std::unordered_map<std::string, Advancement> advancements;

        std::unordered_map<int32_t, std::unordered_map<int32_t, uint32_t>> item_attribute::protocol_aliases;
        std::unordered_map<std::string, item_attribute> attributes;
        list_array<decltype(attributes)::iterator> attributes_cache;

        std::unordered_map<std::string, JukeboxSong> jukebox_songs;


        std::unordered_map<uint32_t, enbt::compound> individual_registers;
        uint32_t use_registry_latest = -1;

        enbt::compound& default_registry() {
            if (use_registry_latest == -1)
                throw std::runtime_error("No registry selected");
            return individual_registers.at(use_registry_latest);
        }

        enbt::value& default_registry_entries(const std::string& registry) {
            return default_registry().at(registry).at("entries");
        }

        enbt::compound& view_registry(int32_t protocol) {
            if (protocol == -1)
                return default_registry();
            return individual_registers.at(protocol);
        }

        enbt::value& view_registry_entries(const std::string& registry, int32_t protocol) {
            return view_registry(protocol).at(registry).at("entries");
        }

        enbt::value& view_registry_proto_invert(const std::string& registry, int32_t protocol) {
            return view_registry(protocol).at(registry).at("proto_invert");
        }

        int32_t view_reg_pro_id(const std::string& registry, const std::string& item, int32_t protocol) {
            if (item.contains(":"))
                return view_registry_entries(registry, protocol).at(item).at("protocol_id");
            else
                return view_registry_entries(registry, protocol).at("minecraft:" + item).at("protocol_id");
        }

        std::string view_reg_pro_name(const std::string& registry, int32_t id, int32_t protocol) {
            return view_registry_proto_invert(registry, protocol).at(id);
        }

        list_array<int32_t> convert_reg_pro_id(const std::string& registry, const list_array<std::string>& item, int32_t protocol) {
            auto& entries = view_registry_entries(registry, protocol);
            return item.convert<int32_t>([&entries](const auto& item) {
                if (item.contains(":"))
                    return entries.at(item).at("protocol_id");
                else
                    return entries.at("minecraft:" + item).at("protocol_id");
            });
        }

        list_array<std::string> convert_reg_pro_name(const std::string& registry, const list_array<int32_t>& item, int32_t protocol) {
            auto& entries = view_registry_proto_invert(registry, protocol);
            return item.convert<std::string>([&entries](const auto& item) { return entries.at(item); });
        }

        list_array<int32_t> convert_reg_pro_id(const std::string& registry, const std::vector<std::string>& item, int32_t protocol) {
            auto& entries = view_registry_entries(registry, protocol);
            list_array<int32_t> result;
            result.reserve(item.size());
            for (const auto& i : item) {
                if (i.contains(":"))
                    result.push_back(entries.at(i).at("protocol_id"));
                else
                    result.push_back(entries.at("minecraft:" + i).at("protocol_id"));
            }
            return result;
        }

        list_array<std::string> convert_reg_pro_name(const std::string& registry, const std::vector<int32_t>& item, int32_t protocol) {
            auto& entries = view_registry_proto_invert(registry, protocol);
            list_array<std::string> result;
            result.reserve(item.size());
            for (const auto& i : item)
                result.push_back(entries.at(i));
            return result;
        }


        std::unordered_map<int32_t, std::unordered_map<int32_t, uint32_t>> potion::protocol_aliases;
        std::unordered_map<std::string, potion> potions;
        list_array<decltype(potions)::iterator> potions_cache;

        std::unordered_map<int32_t, std::unordered_map<int32_t, uint32_t>> effect::protocol_aliases;
        std::unordered_map<std::string, effect> effects;
        list_array<decltype(effects)::iterator> effects_cache;

        std::unordered_map<std::string, enchantment> enchantments;
        list_array<decltype(enchantments)::iterator> enchantments_cache;
        std::unordered_map<std::string, enbt::compound> enchantment_providers;

        std::unordered_map<std::string, loot_table_item> loot_table;
        list_array<decltype(loot_table)::iterator> loot_table_cache;

        std::unordered_map<std::string, base_objects::recipe> recipe_table;
        list_array<decltype(recipe_table)::iterator> recipe_table_cache;

        std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> blockPalette;
    }
}