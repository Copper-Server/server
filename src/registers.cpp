
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

        using eveeth = enchantment::value_effects::effect_type_hold;
        using eeeeth = enchantment::entity_effects::effect_type_hold;

        std::string eveeth::type() {
            if (std::holds_alternative<set*>(effect_type))
                return "set";
            else if (std::holds_alternative<add*>(effect_type))
                return "add";
            else if (std::holds_alternative<multiply*>(effect_type))
                return "multiply";
            else if (std::holds_alternative<remove_binomial*>(effect_type))
                return "remove_binomial";
            else if (std::holds_alternative<_custom*>(effect_type))
                return "_custom";
            else if (std::holds_alternative<all_off*>(effect_type))
                return "all_off";
            else
                return "undefined";
        }

        enchantment::value_effects::set& eveeth::as_set() {
            return *std::get<set*>(effect_type);
        }

        enchantment::value_effects::add& eveeth::as_add() {
            return *std::get<add*>(effect_type);
        }

        enchantment::value_effects::multiply& eveeth::as_multiply() {
            return *std::get<multiply*>(effect_type);
        }

        enchantment::value_effects::remove_binomial& eveeth::as_remove_binomial() {
            return *std::get<remove_binomial*>(effect_type);
        }

        enchantment::value_effects::_custom& eveeth::as_custom() {
            return *std::get<_custom*>(effect_type);
        }

        enchantment::value_effects::all_off& eveeth::as_all_off() {
            return *std::get<all_off*>(effect_type);
        }

        eveeth::effect_type_hold(set* move) {
            effect_type = move;
        }

        eveeth::effect_type_hold(add* move) {
            effect_type = move;
        }

        eveeth::effect_type_hold(multiply* move) {
            effect_type = move;
        }

        eveeth::effect_type_hold(remove_binomial* move) {
            effect_type = move;
        }

        eveeth::effect_type_hold(_custom* move) {
            effect_type = move;
        }

        eveeth::effect_type_hold(all_off* move) {
            effect_type = move;
        }

        eveeth::effect_type_hold(effect_type_hold&& move)
            : effect_type(std::move(move.effect_type)) {
        }

        eveeth::~effect_type_hold() {
            std::visit(
                [](auto& value) {
                    delete value;
                },
                effect_type
            );
        }

        std::string eeeeth::type() {
            if (std::holds_alternative<attribute*>(effect_type))
                return "attribute";
            else if (std::holds_alternative<apply_mob_effect*>(effect_type))
                return "apply_mob_effect";
            else if (std::holds_alternative<damage_entity*>(effect_type))
                return "damage_entity";
            else if (std::holds_alternative<explode*>(effect_type))
                return "explode";
            else if (std::holds_alternative<play_sound*>(effect_type))
                return "play_sound";
            else if (std::holds_alternative<replace_block*>(effect_type))
                return "replace_block";
            else if (std::holds_alternative<replace_disk*>(effect_type))
                return "replace_disk";
            else if (std::holds_alternative<run_function*>(effect_type))
                return "run_function";
            else if (std::holds_alternative<set_block_properties*>(effect_type))
                return "set_block_properties";
            else if (std::holds_alternative<spawn_particles*>(effect_type))
                return "spawn_particles";
            else if (std::holds_alternative<_custom*>(effect_type))
                return "custom";
            else if (std::holds_alternative<all_off*>(effect_type))
                return "all_off";
            else
                return "undefined";
        }

        enchantment::entity_effects::attribute& eeeeth::as_attribute() {
            return *std::get<attribute*>(effect_type);
        }

        enchantment::entity_effects::apply_mob_effect& eeeeth::as_apply_mob_effect() {
            return *std::get<apply_mob_effect*>(effect_type);
        }

        enchantment::entity_effects::damage_entity& eeeeth::as_damage_entity() {
            return *std::get<damage_entity*>(effect_type);
        }

        enchantment::entity_effects::explode& eeeeth::as_explode() {
            return *std::get<explode*>(effect_type);
        }

        enchantment::entity_effects::ignite& eeeeth::as_ignite() {
            return *std::get<ignite*>(effect_type);
        }

        enchantment::entity_effects::play_sound& eeeeth::as_play_sound() {
            return *std::get<play_sound*>(effect_type);
        }

        enchantment::entity_effects::replace_block& eeeeth::as_replace_block() {
            return *std::get<replace_block*>(effect_type);
        }

        enchantment::entity_effects::replace_disk& eeeeth::as_replace_disk() {
            return *std::get<replace_disk*>(effect_type);
        }

        enchantment::entity_effects::run_function& eeeeth::as_run_function() {
            return *std::get<run_function*>(effect_type);
        }

        enchantment::entity_effects::set_block_properties& eeeeth::as_set_block_properties() {
            return *std::get<set_block_properties*>(effect_type);
        }

        enchantment::entity_effects::spawn_particles& eeeeth::as_spawn_particles() {
            return *std::get<spawn_particles*>(effect_type);
        }

        enchantment::entity_effects::_custom& eeeeth::as_custom() {
            return *std::get<_custom*>(effect_type);
        }

        enchantment::entity_effects::all_off& eeeeth::as_all_off() {
            return *std::get<all_off*>(effect_type);
        }

        eeeeth::effect_type_hold(attribute* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(apply_mob_effect* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(damage_entity* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(explode* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(ignite* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(play_sound* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(replace_block* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(replace_disk* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(run_function* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(set_block_properties* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(spawn_particles* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(_custom* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(all_off* move)
            : effect_type(move) {}

        eeeeth::effect_type_hold(effect_type_hold&& move)
            : effect_type(std::move(move.effect_type)) {}

        eeeeth::~effect_type_hold() {
            std::visit(
                [](auto& value) {
                    delete value;
                },
                effect_type
            );
        }

        //SERVER
        std::unordered_map<std::string, Advancement> advancements;
        std::unordered_map<std::string, JukeboxSong> jukebox_songs;


        std::unordered_map<uint32_t, enbt::compound> individual_registers;


        std::unordered_map<std::string, potion> potions;
        list_array<decltype(potions)::iterator> potions_cache;

        std::unordered_map<std::string, enchantment> enchantments;
        list_array<decltype(enchantments)::iterator> enchantments_cache;

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