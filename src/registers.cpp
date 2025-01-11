
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

        std::unordered_map<int32_t, std::unordered_map<int32_t, uint32_t>> item_attribute::protocol_aliases;
        std::unordered_map<std::string, item_attribute> attributes;
        list_array<decltype(attributes)::iterator> attributes_cache;

        std::unordered_map<std::string, JukeboxSong> jukebox_songs;


        std::unordered_map<uint32_t, enbt::compound> individual_registers;
        uint32_t use_registry_lastest = -1;

        enbt::compound& default_registry() {
            if (use_registry_lastest == -1)
                throw std::runtime_error("No registry selected");
            return individual_registers.at(use_registry_lastest);
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
            } else {
                if (auto nam = tag.find(':'); nam != tag.npos) {
                    std::string _namespace = tag.substr(0, nam - 1);
                    std::string _tag = tag.substr(nam);
                    return unfold_tag(type, _namespace, _tag);
                } else
                    return unfold_tag(type, default_namespace, tag);
            }
        }
    }
}