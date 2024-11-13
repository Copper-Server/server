#include <resources/include.hpp>
#include <src/api/recipe.hpp>
#include <src/base_objects/entity.hpp>
#include <src/registers.hpp>
#include <src/util/conversions.hpp>
#include <src/util/json_helpers.hpp>

namespace copper_server {
    namespace resources {
        template <class T, class Iterator>
        void id_assigner(std::unordered_map<std::string, T>& map, list_array<Iterator>& cache) {
            size_t i = 0;
            auto it = map.begin();
            auto end = map.end();
            cache.reserve(map.size());
            for (; it != end; ++it) {
                it->second.id = i++;
                cache.push_back(it);
            }
        }

        template <class T, size_t length>
        void check_override(std::unordered_map<std::string, T>& map, const std::string& id, const char (&type_name)[length]) {
            auto it = map.find(id);
            if (it != map.end())
                if (!it->second.allow_override)
                    throw std::runtime_error("This " + std::string(type_name) + " is already defined and does not allow override. [" + id + "]");
        }

        template <class T, size_t length>
        void check_conflicts(std::unordered_map<std::string, T>& map, const std::string& id, const char (&type_name)[length]) {
            auto it = map.find(id);
            if (it != map.end())
                throw std::runtime_error("This " + std::string(type_name) + " is already defined and cannot be overriden. [" + id + "]");
        }

        using namespace util;
        using namespace registers;

        void registers_reset() {
            biomes.clear();
            biomes.clear();
            chatTypes.clear();
            armorTrimPatterns.clear();
            armorTrimMaterials.clear();
            wolfVariants.clear();
            dimensionTypes.clear();
            damageTypes.clear();
            bannerPatterns.clear();
            paintingVariants.clear();
            biomes_cache.clear();
            biomes_cache.clear();
            chatTypes_cache.clear();
            armorTrimPatterns_cache.clear();
            armorTrimMaterials_cache.clear();
            wolfVariants_cache.clear();
            dimensionTypes_cache.clear();
            damageTypes_cache.clear();
            bannerPatterns_cache.clear();
            paintingVariants_cache.clear();
            tags.clear();
        }

        void load_registers_complete() {
            id_assigner(biomes, biomes_cache);
            id_assigner(chatTypes, chatTypes_cache);
            id_assigner(armorTrimPatterns, armorTrimPatterns_cache);
            id_assigner(armorTrimMaterials, armorTrimMaterials_cache);
            id_assigner(wolfVariants, wolfVariants_cache);
            id_assigner(dimensionTypes, dimensionTypes_cache);
            id_assigner(damageTypes, damageTypes_cache);
            id_assigner(bannerPatterns, bannerPatterns_cache);
            id_assigner(paintingVariants, paintingVariants_cache);

            for (auto& it : tags)
                for (auto& it2 : it.second)
                    for (auto& it3 : it2.second)
                        it3.second.commit();
        }

        void initialize_entities() {
            using namespace base_objects;
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:allay",
                    .name = "Allay",
                    .translation_resource_key = "entity.minecraft.allay",
                    .kind_name = "entity",
                    .base_bounds = {0.35, 0.6},
                    .base_health = 20,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:area_effect_cloud",
                    .name = "Area Effect Cloud",
                    .translation_resource_key = "entity.minecraft.area_effect_cloud",
                    .kind_name = "entity",
                    .base_bounds = {2.0, 0.5}, // Assuming radius is handled elsewhere
                    .base_health = NAN,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:armadillo",
                    .name = "Armadillo",
                    .translation_resource_key = "entity.minecraft.armadillo",
                    .kind_name = "animal",
                    .base_bounds = {2.0, 0.5}, // Assuming radius is handled elsewhere
                    .base_health = 6,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:armor_stand",
                    .name = "Armor Stand",
                    .translation_resource_key = "entity.minecraft.armor_stand",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 1.975}, // Assuming normal size
                    .base_health = NAN,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:arrow",
                    .name = "Arrow",
                    .translation_resource_key = "entity.minecraft.arrow",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.5},
                    .base_health = NAN,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:axolotl",
                    .name = "Axolotl",
                    .translation_resource_key = "entity.minecraft.axolotl",
                    .kind_name = "entity",
                    .base_bounds = {0.75, 0.42}, //TODO unknown
                    .base_health = 14,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:bat",
                    .name = "Bat",
                    .translation_resource_key = "entity.minecraft.bat",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.9},
                    .base_health = 6,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:bee",
                    .name = "Bee",
                    .translation_resource_key = "entity.minecraft.bee",
                    .kind_name = "entity",
                    .base_bounds = {0.7, 0.6},
                    .base_health = 6,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:blaze",
                    .name = "Blaze",
                    .translation_resource_key = "entity.minecraft.blaze",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.8},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:block_display",
                    .name = "Block Display",
                    .translation_resource_key = "entity.minecraft.block_display",
                    .kind_name = "entity",
                    .base_bounds = {0.0, 0.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:boat",
                    .name = "Boat",
                    .translation_resource_key = "entity.minecraft.boat",
                    .kind_name = "entity",
                    .base_bounds = {1.375, 0.5625},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:bogged",
                    .name = "Bogged",
                    .translation_resource_key = "entity.minecraft.bogged",
                    .kind_name = "entity",
                    .base_bounds = {1.375, 0.5625}, //TODO unknown
                    .base_health = 16,
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:breeze",
                    .name = "Breeze",
                    .translation_resource_key = "entity.minecraft.breeze",
                    .kind_name = "entity",
                    .base_bounds = {1.375, 0.5625}, //TODO unknown
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:breeze_wind_charge",
                    .name = "Breeze Wind Charge",
                    .translation_resource_key = "entity.minecraft.breeze_wind_charge",
                    .kind_name = "entity",
                    .base_bounds = {1.375, 0.5625}, //TODO unknown
                }
            );

            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:camel",
                    .name = "Camel",
                    .translation_resource_key = "entity.minecraft.camel",
                    .kind_name = "entity",
                    .base_bounds = {1.7, 2.375},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:cat",
                    .name = "Cat",
                    .translation_resource_key = "entity.minecraft.cat",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:cave_spider",
                    .name = "Cave Spider",
                    .translation_resource_key = "entity.minecraft.cave_spider",
                    .kind_name = "entity",
                    .base_bounds = {0.7, 0.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:chest_boat",
                    .name = "Chest Boat",
                    .translation_resource_key = "entity.minecraft.chest_boat",
                    .kind_name = "entity",
                    .base_bounds = {1.375, 0.5625},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:chest_minecart",
                    .name = "Chest Minecart",
                    .translation_resource_key = "entity.minecraft.chest_minecart",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:chicken",
                    .name = "Chicken",
                    .translation_resource_key = "entity.minecraft.chicken",
                    .kind_name = "entity",
                    .base_bounds = {0.4, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:cod",
                    .name = "Cod",
                    .translation_resource_key = "entity.minecraft.cod",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.3},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:command_block_minecart",
                    .name = "Command Block Minecart",
                    .translation_resource_key = "entity.minecraft.command_block_minecart",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:cow",
                    .name = "Cow",
                    .translation_resource_key = "entity.minecraft.cow",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 1.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:creeper",
                    .name = "Creeper",
                    .translation_resource_key = "entity.minecraft.creeper",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:dolphin",
                    .name = "Dolphin",
                    .translation_resource_key = "entity.minecraft.dolphin",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 0.6},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:donkey",
                    .name = "Donkey",
                    .translation_resource_key = "entity.minecraft.donkey",
                    .kind_name = "entity",
                    .base_bounds = {1.3964844, 1.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:dragon_fireball",
                    .name = "Dragon Fireball",
                    .translation_resource_key = "entity.minecraft.dragon_fireball",
                    .kind_name = "entity",
                    .base_bounds = {1.0, 1.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:drowned",
                    .name = "Drowned",
                    .translation_resource_key = "entity.minecraft.drowned",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:egg",
                    .name = "Egg",
                    .translation_resource_key = "entity.minecraft.egg",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:elder_guardian",
                    .name = "Elder Guardian",
                    .translation_resource_key = "entity.minecraft.elder_guardian",
                    .kind_name = "entity",
                    .base_bounds = {1.9975, 1.9975}, // Assuming guardian size is handled elsewhere
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:end_crystal",
                    .name = "End Crystal",
                    .translation_resource_key = "entity.minecraft.end_crystal",
                    .kind_name = "entity",
                    .base_bounds = {2.0, 2.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:ender_dragon",
                    .name = "Ender Dragon",
                    .translation_resource_key = "entity.minecraft.ender_dragon",
                    .kind_name = "entity",
                    .base_bounds = {16.0, 8.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:ender_pearl",
                    .name = "Ender Pearl",
                    .translation_resource_key = "entity.minecraft.ender_pearl",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:enderman",
                    .name = "Enderman",
                    .translation_resource_key = "entity.minecraft.enderman",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 2.9},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:endermite",
                    .name = "Endermite",
                    .translation_resource_key = "entity.minecraft.endermite",
                    .kind_name = "entity",
                    .base_bounds = {0.4, 0.3},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:evoker",
                    .name = "Evoker",
                    .translation_resource_key = "entity.minecraft.evoker",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:evoker_fangs",
                    .name = "Evoker Fangs",
                    .translation_resource_key = "entity.minecraft.evoker_fangs",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.8},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:experience_bottle",
                    .name = "Experience Bottle",
                    .translation_resource_key = "entity.minecraft.experience_bottle",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:experience_orb",
                    .name = "Experience Orb",
                    .translation_resource_key = "entity.minecraft.experience_orb",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:eye_of_ender",
                    .name = "Eye Of Ender",
                    .translation_resource_key = "entity.minecraft.eye_of_ender",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:falling_block",
                    .name = "Falling Block",
                    .translation_resource_key = "entity.minecraft.falling_block",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.98},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:firework_rocket",
                    .name = "Firework Rocket",
                    .translation_resource_key = "entity.minecraft.firework_rocket",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:fox",
                    .name = "Fox",
                    .translation_resource_key = "entity.minecraft.fox",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:frog",
                    .name = "Frog",
                    .translation_resource_key = "entity.minecraft.frog",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:furnace_minecart",
                    .name = "Furnace Minecart",
                    .translation_resource_key = "entity.minecraft.furnace_minecart",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:ghast",
                    .name = "Ghast",
                    .translation_resource_key = "entity.minecraft.ghast",
                    .kind_name = "entity",
                    .base_bounds = {4.0, 4.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:giant",
                    .name = "Giant",
                    .translation_resource_key = "entity.minecraft.giant",
                    .kind_name = "entity",
                    .base_bounds = {3.6, 12.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:glow_item_frame",
                    .name = "Glow Item Frame",
                    .translation_resource_key = "entity.minecraft.glow_item_frame",
                    .kind_name = "entity",
                    .base_bounds = {0.75, 0.75}, // Assuming normal size
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:glow_squid",
                    .name = "Glow Squid",
                    .translation_resource_key = "entity.minecraft.glow_squid",
                    .kind_name = "entity",
                    .base_bounds = {0.8, 0.8},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:goat",
                    .name = "Goat",
                    .translation_resource_key = "entity.minecraft.goat",
                    .kind_name = "entity",
                    .base_bounds = {1.3, 0.9},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:guardian",
                    .name = "Guardian",
                    .translation_resource_key = "entity.minecraft.guardian",
                    .kind_name = "entity",
                    .base_bounds = {0.85, 0.85},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:hoglin",
                    .name = "Hoglin",
                    .translation_resource_key = "entity.minecraft.hoglin",
                    .kind_name = "entity",
                    .base_bounds = {1.3964844, 1.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:hopper_minecart",
                    .name = "Hopper Minecart",
                    .translation_resource_key = "entity.minecraft.hopper_minecart",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:horse",
                    .name = "Horse",
                    .translation_resource_key = "entity.minecraft.horse",
                    .kind_name = "entity",
                    .base_bounds = {1.3964844, 1.6},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:husk",
                    .name = "Husk",
                    .translation_resource_key = "entity.minecraft.husk",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:illusioner",
                    .name = "Illusioner",
                    .translation_resource_key = "entity.minecraft.illusioner",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:interaction",
                    .name = "Interaction",
                    .translation_resource_key = "entity.minecraft.interaction",
                    .kind_name = "entity",
                    .base_bounds = {0.0, 0.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:iron_golem",
                    .name = "Iron Golem",
                    .translation_resource_key = "entity.minecraft.iron_golem",
                    .kind_name = "entity",
                    .base_bounds = {1.4, 2.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:item",
                    .name = "Item",
                    .translation_resource_key = "entity.minecraft.item",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:item_display",
                    .name = "Item Display",
                    .translation_resource_key = "entity.minecraft.item_display",
                    .kind_name = "entity",
                    .base_bounds = {0.0, 0.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:item_frame",
                    .name = "Item Frame",
                    .translation_resource_key = "entity.minecraft.item_frame",
                    .kind_name = "entity",
                    .base_bounds = {0.75, 0.75}, // Assuming normal size
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:fireball",
                    .name = "Fireball",
                    .translation_resource_key = "entity.minecraft.fireball",
                    .kind_name = "entity",
                    .base_bounds = {1.0, 1.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:leash_knot",
                    .name = "Leash Knot",
                    .translation_resource_key = "entity.minecraft.leash_knot",
                    .kind_name = "entity",
                    .base_bounds = {0.375, 0.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:lightning_bolt",
                    .name = "Lightning Bolt",
                    .translation_resource_key = "entity.minecraft.lightning_bolt",
                    .kind_name = "entity",
                    .base_bounds = {0.0, 0.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:llama",
                    .name = "Llama",
                    .translation_resource_key = "entity.minecraft.llama",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 1.87},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:llama_spit",
                    .name = "Llama Spit",
                    .translation_resource_key = "entity.minecraft.llama_spit",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:magma_cube",
                    .name = "Magma Cube",
                    .translation_resource_key = "entity.minecraft.magma_cube",
                    .kind_name = "entity",
                    .base_bounds = {0.5202, 0.5202},
                    .check_bounds = {nullptr} //TODO
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:marker",
                    .name = "Marker",
                    .translation_resource_key = "entity.minecraft.marker",
                    .kind_name = "entity",
                    .base_bounds = {0.0, 0.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:minecart",
                    .name = "Minecart",
                    .translation_resource_key = "entity.minecraft.minecart",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:mooshroom",
                    .name = "Mooshroom",
                    .translation_resource_key = "entity.minecraft.mooshroom",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 1.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:mule",
                    .name = "Mule",
                    .translation_resource_key = "entity.minecraft.mule",
                    .kind_name = "entity",
                    .base_bounds = {1.3964844, 1.6},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:ocelot",
                    .name = "Ocelot",
                    .translation_resource_key = "entity.minecraft.ocelot",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:ominous_item_spawner",
                    .name = "Ominous Item Spawner",
                    .translation_resource_key = "entity.minecraft.ominous_item_spawner",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 0.7}, //TODO
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:painting",
                    .name = "Painting",
                    .translation_resource_key = "entity.minecraft.painting",
                    .kind_name = "entity",
                    .base_bounds = {1, 1},
                    .check_bounds = {nullptr} //TODO
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:panda",
                    .name = "Panda",
                    .translation_resource_key = "entity.minecraft.panda",
                    .kind_name = "entity",
                    .base_bounds = {1.3, 1.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:parrot",
                    .name = "Parrot",
                    .translation_resource_key = "entity.minecraft.parrot",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.9},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:phantom",
                    .name = "Phantom",
                    .translation_resource_key = "entity.minecraft.phantom",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 0.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:pig",
                    .name = "Pig",
                    .translation_resource_key = "entity.minecraft.pig",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 0.9},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:piglin",
                    .name = "Piglin",
                    .translation_resource_key = "entity.minecraft.piglin",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:piglin_brute",
                    .name = "Piglin Brute",
                    .translation_resource_key = "entity.minecraft.piglin_brute",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:pillager",
                    .name = "Pillager",
                    .translation_resource_key = "entity.minecraft.pillager",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:polar_bear",
                    .name = "Polar Bear",
                    .translation_resource_key = "entity.minecraft.polar_bear",
                    .kind_name = "entity",
                    .base_bounds = {1.4, 1.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:potion",
                    .name = "Potion",
                    .translation_resource_key = "entity.minecraft.potion",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:pufferfish",
                    .name = "Pufferfish",
                    .translation_resource_key = "entity.minecraft.pufferfish",
                    .kind_name = "entity",
                    .base_bounds = {0.7, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:rabbit",
                    .name = "Rabbit",
                    .translation_resource_key = "entity.minecraft.rabbit",
                    .kind_name = "entity",
                    .base_bounds = {0.4, 0.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:ravager",
                    .name = "Ravager",
                    .translation_resource_key = "entity.minecraft.ravager",
                    .kind_name = "entity",
                    .base_bounds = {1.95, 2.2},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:salmon",
                    .name = "Salmon",
                    .translation_resource_key = "entity.minecraft.salmon",
                    .kind_name = "entity",
                    .base_bounds = {0.7, 0.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:sheep",
                    .name = "Sheep",
                    .translation_resource_key = "entity.minecraft.sheep",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 1.3},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:shulker",
                    .name = "Shulker",
                    .translation_resource_key = "entity.minecraft.shulker",
                    .kind_name = "entity",
                    .base_bounds = {1.0, 1.0 - 2.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:shulker_bullet",
                    .name = "Shulker Bullet",
                    .translation_resource_key = "entity.minecraft.shulker_bullet",
                    .kind_name = "entity",
                    .base_bounds = {0.3125, 0.3125},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:silverfish",
                    .name = "Silverfish",
                    .translation_resource_key = "entity.minecraft.silverfish",
                    .kind_name = "entity",
                    .base_bounds = {0.4, 0.3},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:skeleton",
                    .name = "Skeleton",
                    .translation_resource_key = "entity.minecraft.skeleton",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.99},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:skeleton_horse",
                    .name = "Skeleton Horse",
                    .translation_resource_key = "entity.minecraft.skeleton_horse",
                    .kind_name = "entity",
                    .base_bounds = {1.3964844, 1.6},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:slime",
                    .name = "Slime",
                    .translation_resource_key = "entity.minecraft.slime",
                    .kind_name = "entity",
                    .base_bounds = {0.5202, 0.5202},
                    .check_bounds = {nullptr} //TODO
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:small_fireball",
                    .name = "Small Fireball",
                    .translation_resource_key = "entity.minecraft.small_fireball",
                    .kind_name = "entity",
                    .base_bounds = {0.3125, 0.3125},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:sniffer",
                    .name = "Sniffer",
                    .translation_resource_key = "entity.minecraft.sniffer",
                    .kind_name = "entity",
                    .base_bounds = {1.9, 1.75},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:snow_golem",
                    .name = "Snow Golem",
                    .translation_resource_key = "entity.minecraft.snow_golem",
                    .kind_name = "entity",
                    .base_bounds = {0.7, 1.9},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:snowball",
                    .name = "Snowball",
                    .translation_resource_key = "entity.minecraft.snowball",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:spawner_minecart",
                    .name = "Spawner Minecart",
                    .translation_resource_key = "entity.minecraft.spawner_minecart",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:spectral_arrow",
                    .name = "Spectral Arrow",
                    .translation_resource_key = "entity.minecraft.spectral_arrow",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:spider",
                    .name = "Spider",
                    .translation_resource_key = "entity.minecraft.spider",
                    .kind_name = "entity",
                    .base_bounds = {1.4, 0.9},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:squid",
                    .name = "Squid",
                    .translation_resource_key = "entity.minecraft.squid",
                    .kind_name = "entity",
                    .base_bounds = {0.8, 0.8},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:stray",
                    .name = "Stray",
                    .translation_resource_key = "entity.minecraft.stray",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.99},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:strider",
                    .name = "Strider",
                    .translation_resource_key = "entity.minecraft.strider",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 1.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:tadpole",
                    .name = "Tadpole",
                    .translation_resource_key = "entity.minecraft.tadpole",
                    .kind_name = "entity",
                    .base_bounds = {0.4, 0.3},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:text_display",
                    .name = "Text Display",
                    .translation_resource_key = "entity.minecraft.text_display",
                    .kind_name = "entity",
                    .base_bounds = {0.0, 0.0},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:tnt",
                    .name = "Tnt",
                    .translation_resource_key = "entity.minecraft.tnt",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.98},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:tnt_minecart",
                    .name = "Tnt Minecart",
                    .translation_resource_key = "entity.minecraft.tnt_minecart",
                    .kind_name = "entity",
                    .base_bounds = {0.98, 0.7},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:trader_llama",
                    .name = "Trader Llama",
                    .translation_resource_key = "entity.minecraft.trader_llama",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 1.87},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:trident",
                    .name = "Trident",
                    .translation_resource_key = "entity.minecraft.trident",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:tropical_fish",
                    .name = "Tropical Fish",
                    .translation_resource_key = "entity.minecraft.tropical_fish",
                    .kind_name = "entity",
                    .base_bounds = {0.5, 0.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:turtle",
                    .name = "Turtle",
                    .translation_resource_key = "entity.minecraft.turtle",
                    .kind_name = "entity",
                    .base_bounds = {1.2, 0.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:vex",
                    .name = "Vex",
                    .translation_resource_key = "entity.minecraft.vex",
                    .kind_name = "entity",
                    .base_bounds = {0.4, 0.8},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:villager",
                    .name = "Villager",
                    .translation_resource_key = "entity.minecraft.villager",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:vindicator",
                    .name = "Vindicator",
                    .translation_resource_key = "entity.minecraft.vindicator",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:wandering_trader",
                    .name = "Wandering Trader",
                    .translation_resource_key = "entity.minecraft.wandering_trader",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:warden",
                    .name = "Warden",
                    .translation_resource_key = "entity.minecraft.warden",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 2.9},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:wind_charge",
                    .name = "Wind Charge",
                    .translation_resource_key = "entity.minecraft.wind_charge",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 2.9}, //TODO
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:witch",
                    .name = "Witch",
                    .translation_resource_key = "entity.minecraft.witch",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:wither",
                    .name = "Wither",
                    .translation_resource_key = "entity.minecraft.wither",
                    .kind_name = "entity",
                    .base_bounds = {0.9, 3.5},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:wither_skeleton",
                    .name = "Wither Skeleton",
                    .translation_resource_key = "entity.minecraft.wither_skeleton",
                    .kind_name = "entity",
                    .base_bounds = {0.7, 2.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:wither_skull",
                    .name = "Wither Skull",
                    .translation_resource_key = "entity.minecraft.wither_skull",
                    .kind_name = "entity",
                    .base_bounds = {0.3125, 0.3125},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:wolf",
                    .name = "Wolf",
                    .translation_resource_key = "entity.minecraft.wolf",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 0.85},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:zoglin",
                    .name = "Zoglin",
                    .translation_resource_key = "entity.minecraft.zoglin",
                    .kind_name = "entity",
                    .base_bounds = {1.3964844, 1.4},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:zombie",
                    .name = "Zombie",
                    .translation_resource_key = "entity.minecraft.zombie",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:zombie_horse",
                    .name = "Zombie Horse",
                    .translation_resource_key = "entity.minecraft.zombie_horse",
                    .kind_name = "entity",
                    .base_bounds = {1.3964844, 1.6},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:zombie_villager",
                    .name = "Zombie Villager",
                    .translation_resource_key = "entity.minecraft.zombie_villager",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:zombified_piglin",
                    .name = "Zombified Piglin",
                    .translation_resource_key = "entity.minecraft.zombified_piglin",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.95},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:player",
                    .name = "Player",
                    .translation_resource_key = "entity.minecraft.player",
                    .kind_name = "entity",
                    .base_bounds = {0.6, 1.8},
                }
            );
            entity_data::register_entity(
                entity_data{
                    .id = "minecraft:fishing_bobber",
                    .name = "Fishing Bobber",
                    .translation_resource_key = "entity.minecraft.fishing_bobber",
                    .kind_name = "entity",
                    .base_bounds = {0.25, 0.25},
                }
            );


            entity_data::internal_entity_aliases_protocol[765] = {
                {"minecraft:allay", 0},
                {"minecraft:area_effect_cloud", 1},
                {"minecraft:armor_stand", 2},
                {"minecraft:arrow", 3},
                {"minecraft:axolotl", 4},
                {"minecraft:bat", 5},
                {"minecraft:bee", 6},
                {"minecraft:blaze", 7},
                {"minecraft:block_display", 8},
                {"minecraft:boat", 9},
                {"minecraft:breeze", 10},
                {"minecraft:camel", 11},
                {"minecraft:cat", 12},
                {"minecraft:cave_spider", 13},
                {"minecraft:chest_boat", 14},
                {"minecraft:chest_minecart", 15},
                {"minecraft:chicken", 16},
                {"minecraft:cod", 17},
                {"minecraft:command_block_minecart", 18},
                {"minecraft:cow", 19},
                {"minecraft:creeper", 20},
                {"minecraft:dolphin", 21},
                {"minecraft:donkey", 22},
                {"minecraft:dragon_fireball", 23},
                {"minecraft:drowned", 24},
                {"minecraft:egg", 25},
                {"minecraft:elder_guardian", 26},
                {"minecraft:end_crystal", 27},
                {"minecraft:ender_dragon", 28},
                {"minecraft:ender_pearl", 29},
                {"minecraft:enderman", 30},
                {"minecraft:endermite", 31},
                {"minecraft:evoker", 32},
                {"minecraft:evoker_fangs", 33},
                {"minecraft:experience_bottle", 34},
                {"minecraft:experience_orb", 35},
                {"minecraft:eye_of_ender", 36},
                {"minecraft:falling_block", 37},
                {"minecraft:fireball", 58},
                {"minecraft:firework_rocket", 38},
                {"minecraft:fox", 39},
                {"minecraft:frog", 40},
                {"minecraft:furnace_minecart", 41},
                {"minecraft:ghast", 42},
                {"minecraft:giant", 43},
                {"minecraft:glow_item_frame", 44},
                {"minecraft:glow_squid", 45},
                {"minecraft:goat", 46},
                {"minecraft:guardian", 47},
                {"minecraft:hoglin", 48},
                {"minecraft:hopper_minecart", 49},
                {"minecraft:horse", 50},
                {"minecraft:husk", 51},
                {"minecraft:illusioner", 52},
                {"minecraft:interaction", 53},
                {"minecraft:iron_golem", 54},
                {"minecraft:item", 55},
                {"minecraft:item_display", 56},
                {"minecraft:item_frame", 57},
                {"minecraft:leash_knot", 59},
                {"minecraft:lightning_bolt", 60},
                {"minecraft:llama", 61},
                {"minecraft:llama_spit", 62},
                {"minecraft:magma_cube", 63},
                {"minecraft:marker", 64},
                {"minecraft:minecart", 65},
                {"minecraft:mooshroom", 66},
                {"minecraft:mule", 67},
                {"minecraft:ocelot", 68},
                {"minecraft:painting", 69},
                {"minecraft:panda", 70},
                {"minecraft:parrot", 71},
                {"minecraft:phantom", 72},
                {"minecraft:pig", 73},
                {"minecraft:piglin", 74},
                {"minecraft:piglin_brute", 75},
                {"minecraft:pillager", 76},
                {"minecraft:polar_bear", 77},
                {"minecraft:potion", 78},
                {"minecraft:pufferfish", 79},
                {"minecraft:rabbit", 80},
                {"minecraft:ravager", 81},
                {"minecraft:salmon", 82},
                {"minecraft:sheep", 83},
                {"minecraft:shulker", 84},
                {"minecraft:shulker_bullet", 85},
                {"minecraft:silverfish", 86},
                {"minecraft:skeleton", 87},
                {"minecraft:skeleton_horse", 88},
                {"minecraft:slime", 89},
                {"minecraft:small_fireball", 90},
                {"minecraft:sniffer", 91},
                {"minecraft:snow_golem", 92},
                {"minecraft:snowball", 93},
                {"minecraft:spawner_minecart", 94},
                {"minecraft:spectral_arrow", 95},
                {"minecraft:spider", 96},
                {"minecraft:squid", 97},
                {"minecraft:stray", 98},
                {"minecraft:strider", 99},
                {"minecraft:tadpole", 100},
                {"minecraft:text_display", 101},
                {"minecraft:tnt", 102},
                {"minecraft:tnt_minecart", 103},
                {"minecraft:trader_llama", 104},
                {"minecraft:trident", 105},
                {"minecraft:tropical_fish", 106},
                {"minecraft:turtle", 107},
                {"minecraft:vex", 108},
                {"minecraft:villager", 109},
                {"minecraft:vindicator", 110},
                {"minecraft:wandering_trader", 111},
                {"minecraft:warden", 112},
                {"minecraft:wind_charge", 113},
                {"minecraft:witch", 114},
                {"minecraft:wither", 115},
                {"minecraft:wither_skeleton", 116},
                {"minecraft:wither_skull", 117},
                {"minecraft:wolf", 118},
                {"minecraft:zoglin", 119},
                {"minecraft:zombie", 120},
                {"minecraft:zombie_horse", 121},
                {"minecraft:zombie_villager", 122},
                {"minecraft:zombified_piglin", 123},
                {"minecraft:player", 124},
                {"minecraft:fishing_bobber", 125},
                {"minecraft:armadillo", 39},            //ALIAS FOX
                {"minecraft:bogged", 98},               //ALIAS STRAY
                {"minecraft:breeze_wind_charge", 113},  //ALIAS wind_charge
                {"minecraft:ominous_item_spawner", 55}, //ALIAS ITEM
            };
            entity_data::internal_entity_aliases_protocol[766] = {
                {"minecraft:allay", 0},
                {"minecraft:area_effect_cloud", 1},
                {"minecraft:armadillo", 2},
                {"minecraft:armor_stand", 3},
                {"minecraft:arrow", 4},
                {"minecraft:axolotl", 5},
                {"minecraft:bat", 6},
                {"minecraft:bee", 7},
                {"minecraft:blaze", 8},
                {"minecraft:block_display", 9},
                {"minecraft:boat", 10},
                {"minecraft:bogged", 11},
                {"minecraft:breeze", 12},
                {"minecraft:breeze_wind_charge", 13},
                {"minecraft:camel", 14},
                {"minecraft:cat", 15},
                {"minecraft:cave_spider", 16},
                {"minecraft:chest_boat", 17},
                {"minecraft:chest_minecart", 18},
                {"minecraft:chicken", 19},
                {"minecraft:cod", 20},
                {"minecraft:command_block_minecart", 21},
                {"minecraft:cow", 22},
                {"minecraft:creeper", 23},
                {"minecraft:dolphin", 24},
                {"minecraft:donkey", 25},
                {"minecraft:dragon_fireball", 26},
                {"minecraft:drowned", 27},
                {"minecraft:egg", 28},
                {"minecraft:elder_guardian", 29},
                {"minecraft:end_crystal", 30},
                {"minecraft:ender_dragon", 31},
                {"minecraft:ender_pearl", 32},
                {"minecraft:enderman", 33},
                {"minecraft:endermite", 34},
                {"minecraft:evoker", 35},
                {"minecraft:evoker_fangs", 36},
                {"minecraft:experience_bottle", 37},
                {"minecraft:experience_orb", 38},
                {"minecraft:eye_of_ender", 39},
                {"minecraft:falling_block", 40},
                {"minecraft:fireball", 62},
                {"minecraft:firework_rocket", 41},
                {"minecraft:fishing_bobber", 129},
                {"minecraft:fox", 42},
                {"minecraft:frog", 43},
                {"minecraft:furnace_minecart", 44},
                {"minecraft:ghast", 45},
                {"minecraft:giant", 46},
                {"minecraft:glow_item_frame", 47},
                {"minecraft:glow_squid", 48},
                {"minecraft:goat", 49},
                {"minecraft:guardian", 50},
                {"minecraft:hoglin", 51},
                {"minecraft:hopper_minecart", 52},
                {"minecraft:horse", 53},
                {"minecraft:husk", 54},
                {"minecraft:illusioner", 55},
                {"minecraft:interaction", 56},
                {"minecraft:iron_golem", 57},
                {"minecraft:item", 58},
                {"minecraft:item_display", 59},
                {"minecraft:item_frame", 60},
                {"minecraft:leash_knot", 63},
                {"minecraft:lightning_bolt", 64},
                {"minecraft:llama", 65},
                {"minecraft:llama_spit", 66},
                {"minecraft:magma_cube", 67},
                {"minecraft:marker", 68},
                {"minecraft:minecart", 69},
                {"minecraft:mooshroom", 70},
                {"minecraft:mule", 71},
                {"minecraft:ocelot", 72},
                {"minecraft:ominous_item_spawner", 61},
                {"minecraft:painting", 73},
                {"minecraft:panda", 74},
                {"minecraft:parrot", 75},
                {"minecraft:phantom", 76},
                {"minecraft:pig", 77},
                {"minecraft:piglin", 78},
                {"minecraft:piglin_brute", 79},
                {"minecraft:pillager", 80},
                {"minecraft:player", 128},
                {"minecraft:polar_bear", 81},
                {"minecraft:potion", 82},
                {"minecraft:pufferfish", 83},
                {"minecraft:rabbit", 84},
                {"minecraft:ravager", 85},
                {"minecraft:salmon", 86},
                {"minecraft:sheep", 87},
                {"minecraft:shulker", 88},
                {"minecraft:shulker_bullet", 89},
                {"minecraft:silverfish", 90},
                {"minecraft:skeleton", 91},
                {"minecraft:skeleton_horse", 92},
                {"minecraft:slime", 93},
                {"minecraft:small_fireball", 94},
                {"minecraft:sniffer", 95},
                {"minecraft:snow_golem", 96},
                {"minecraft:snowball", 97},
                {"minecraft:spawner_minecart", 98},
                {"minecraft:spectral_arrow", 99},
                {"minecraft:spider", 100},
                {"minecraft:squid", 101},
                {"minecraft:stray", 102},
                {"minecraft:strider", 103},
                {"minecraft:tadpole", 104},
                {"minecraft:text_display", 105},
                {"minecraft:tnt", 106},
                {"minecraft:tnt_minecart", 107},
                {"minecraft:trader_llama", 108},
                {"minecraft:trident", 109},
                {"minecraft:tropical_fish", 110},
                {"minecraft:turtle", 111},
                {"minecraft:vex", 112},
                {"minecraft:villager", 113},
                {"minecraft:vindicator", 114},
                {"minecraft:wandering_trader", 115},
                {"minecraft:warden", 116},
                {"minecraft:wind_charge", 117},
                {"minecraft:witch", 118},
                {"minecraft:wither", 119},
                {"minecraft:wither_skeleton", 120},
                {"minecraft:wither_skull", 121},
                {"minecraft:wolf", 122},
                {"minecraft:zoglin", 123},
                {"minecraft:zombie", 124},
                {"minecraft:zombie_horse", 125},
                {"minecraft:zombie_villager", 126},
                {"minecraft:zombified_piglin", 127},
            };
            entity_data::internal_entity_aliases_protocol[767] = {
                {"minecraft:allay", 0},
                {"minecraft:area_effect_cloud", 1},
                {"minecraft:armadillo", 2},
                {"minecraft:armor_stand", 3},
                {"minecraft:arrow", 4},
                {"minecraft:axolotl", 5},
                {"minecraft:bat", 6},
                {"minecraft:bee", 7},
                {"minecraft:blaze", 8},
                {"minecraft:block_display", 9},
                {"minecraft:boat", 10},
                {"minecraft:bogged", 11},
                {"minecraft:breeze", 12},
                {"minecraft:breeze_wind_charge", 13},
                {"minecraft:camel", 14},
                {"minecraft:cat", 15},
                {"minecraft:cave_spider", 16},
                {"minecraft:chest_boat", 17},
                {"minecraft:chest_minecart", 18},
                {"minecraft:chicken", 19},
                {"minecraft:cod", 20},
                {"minecraft:command_block_minecart", 21},
                {"minecraft:cow", 22},
                {"minecraft:creeper", 23},
                {"minecraft:dolphin", 24},
                {"minecraft:donkey", 25},
                {"minecraft:dragon_fireball", 26},
                {"minecraft:drowned", 27},
                {"minecraft:egg", 28},
                {"minecraft:elder_guardian", 29},
                {"minecraft:end_crystal", 30},
                {"minecraft:ender_dragon", 31},
                {"minecraft:ender_pearl", 32},
                {"minecraft:enderman", 33},
                {"minecraft:endermite", 34},
                {"minecraft:evoker", 35},
                {"minecraft:evoker_fangs", 36},
                {"minecraft:experience_bottle", 37},
                {"minecraft:experience_orb", 38},
                {"minecraft:eye_of_ender", 39},
                {"minecraft:falling_block", 40},
                {"minecraft:fireball", 62},
                {"minecraft:firework_rocket", 41},
                {"minecraft:fishing_bobber", 129},
                {"minecraft:fox", 42},
                {"minecraft:frog", 43},
                {"minecraft:furnace_minecart", 44},
                {"minecraft:ghast", 45},
                {"minecraft:giant", 46},
                {"minecraft:glow_item_frame", 47},
                {"minecraft:glow_squid", 48},
                {"minecraft:goat", 49},
                {"minecraft:guardian", 50},
                {"minecraft:hoglin", 51},
                {"minecraft:hopper_minecart", 52},
                {"minecraft:horse", 53},
                {"minecraft:husk", 54},
                {"minecraft:illusioner", 55},
                {"minecraft:interaction", 56},
                {"minecraft:iron_golem", 57},
                {"minecraft:item", 58},
                {"minecraft:item_display", 59},
                {"minecraft:item_frame", 60},
                {"minecraft:leash_knot", 63},
                {"minecraft:lightning_bolt", 64},
                {"minecraft:llama", 65},
                {"minecraft:llama_spit", 66},
                {"minecraft:magma_cube", 67},
                {"minecraft:marker", 68},
                {"minecraft:minecart", 69},
                {"minecraft:mooshroom", 70},
                {"minecraft:mule", 71},
                {"minecraft:ocelot", 72},
                {"minecraft:ominous_item_spawner", 61},
                {"minecraft:painting", 73},
                {"minecraft:panda", 74},
                {"minecraft:parrot", 75},
                {"minecraft:phantom", 76},
                {"minecraft:pig", 77},
                {"minecraft:piglin", 78},
                {"minecraft:piglin_brute", 79},
                {"minecraft:pillager", 80},
                {"minecraft:player", 128},
                {"minecraft:polar_bear", 81},
                {"minecraft:potion", 82},
                {"minecraft:pufferfish", 83},
                {"minecraft:rabbit", 84},
                {"minecraft:ravager", 85},
                {"minecraft:salmon", 86},
                {"minecraft:sheep", 87},
                {"minecraft:shulker", 88},
                {"minecraft:shulker_bullet", 89},
                {"minecraft:silverfish", 90},
                {"minecraft:skeleton", 91},
                {"minecraft:skeleton_horse", 92},
                {"minecraft:slime", 93},
                {"minecraft:small_fireball", 94},
                {"minecraft:sniffer", 95},
                {"minecraft:snow_golem", 96},
                {"minecraft:snowball", 97},
                {"minecraft:spawner_minecart", 98},
                {"minecraft:spectral_arrow", 99},
                {"minecraft:spider", 100},
                {"minecraft:squid", 101},
                {"minecraft:stray", 102},
                {"minecraft:strider", 103},
                {"minecraft:tadpole", 104},
                {"minecraft:text_display", 105},
                {"minecraft:tnt", 106},
                {"minecraft:tnt_minecart", 107},
                {"minecraft:trader_llama", 108},
                {"minecraft:trident", 109},
                {"minecraft:tropical_fish", 110},
                {"minecraft:turtle", 111},
                {"minecraft:vex", 112},
                {"minecraft:villager", 113},
                {"minecraft:vindicator", 114},
                {"minecraft:wandering_trader", 115},
                {"minecraft:warden", 116},
                {"minecraft:wind_charge", 117},
                {"minecraft:witch", 118},
                {"minecraft:wither", 119},
                {"minecraft:wither_skeleton", 120},
                {"minecraft:wither_skull", 121},
                {"minecraft:wolf", 122},
                {"minecraft:zoglin", 123},
                {"minecraft:zombie", 124},
                {"minecraft:zombie_horse", 125},
                {"minecraft:zombie_villager", 126},
                {"minecraft:zombified_piglin", 127}
            };

            entity_data::initialize_entities();
        }

        void load_file_biomes(js_object&& bio_js, const std::string& id) {
            check_override(biomes, id, "biome");
            Biome bio;
            bio.downfall = bio_js["downfall"];
            bio.temperature = bio_js["temperature"];
            bio.has_precipitation = bio_js["has_precipitation"];
            bio.creature_spawn_probability = bio_js["creature_spawn_probability"].or_apply(0.0);
            if (bio_js.contains("temperature_modifier"))
                bio.temperature_modifier = (std::string)bio_js["temperature_modifier"];

            {
                Biome::Effects effects;
                js_object effects_js = js_object::get_object(bio_js["effects"]);
                effects.sky_color = effects_js["sky_color"];
                effects.water_fog_color = effects_js["water_fog_color"];
                effects.fog_color = effects_js["fog_color"];
                effects.water_color = effects_js["water_color"];
                if (effects_js.contains("foliage_color"))
                    effects.foliage_color = effects_js["foliage_color"];
                if (effects_js.contains("grass_color"))
                    effects.grass_color = effects_js["grass_color"];
                if (effects_js.contains("grass_color_modifier"))
                    effects.grass_color_modifier = (std::string)effects_js["grass_color_modifier"];
                if (effects_js.contains("particle")) {
                    js_object particle_js = js_object::get_object(effects_js["particle"]);
                    Biome::Particle particle;
                    particle.probability = particle_js["probability"];
                    auto options = js_object::get_object(particle_js["options"]);
                    particle.options.type = (std::string)options["type"];
                    particle.options.options = conversions::json::from_json(options.get());
                    effects.particle = std::move(particle);
                }
                if (effects_js.contains("ambient_sound")) {
                    auto eff = effects_js["ambient_sound"];
                    if (eff.is_string())
                        effects.ambient_sound = (std::string)eff;
                    else {
                        js_object ambient_sound_js = js_object::get_object(effects_js["ambient_sound"]);
                        Biome::AmbientSound ambient_sound;
                        ambient_sound.sound = (std::string)ambient_sound_js["sound"];
                        ambient_sound.range = ambient_sound_js["range"];
                        effects.ambient_sound = std::move(ambient_sound);
                    }
                }
                if (effects_js.contains("mood_sound")) {
                    js_object mood_sound_js = js_object::get_object(effects_js["mood_sound"]);
                    Biome::MoodSound mood_sound;
                    mood_sound.sound = (std::string)mood_sound_js["sound"];
                    mood_sound.offset = mood_sound_js["offset"].or_apply(2.0);
                    mood_sound.block_search_extend = mood_sound_js["block_search_extend"].or_apply(8);
                    mood_sound.tick_delay = mood_sound_js["tick_delay"].or_apply(6000);
                    effects.mood_sound = std::move(mood_sound);
                }
                if (effects_js.contains("additions_sound")) {
                    js_object additions_sound_js = js_object::get_object(effects_js["additions_sound"]);
                    Biome::AdditionsSound additions_sound;
                    additions_sound.sound = (std::string)additions_sound_js["sound"];
                    additions_sound.tick_chance = additions_sound_js["tick_chance"];
                    effects.additions_sound = std::move(additions_sound);
                }
                if (effects_js.contains("music")) {
                    js_object music_js = js_object::get_object(effects_js["music"]);
                    Biome::Music music;
                    music.sound = (std::string)music_js["sound"];
                    music.min_delay = music_js["min_delay"].or_apply(12000);
                    music.max_delay = music_js["max_delay"].or_apply(24000);
                    music.replace_current_music = music_js["replace_current_music"].or_apply(true);
                    effects.music = std::move(music);
                }
                bio.effects = std::move(effects);
            }
            {
                js_object cavers_js = js_object::get_object(bio_js["cavers"]);
                for (auto&& [name, values] : cavers_js) {
                    if (values.is_string()) {
                        bio.carvers[name] = {values};
                    } else {
                        auto& res = bio.carvers[name];
                        js_array arr = js_array::get_array(values);
                        res.reserve(arr.size());
                        for (auto&& it : arr)
                            res.push_back(it);
                    }
                }
            }
            {
                auto features_js = js_array::get_array(bio_js["features"]);
                bio.features.reserve(features_js.size());
                for (auto&& items : features_js) {
                    auto feature_js = js_array::get_array(items);
                    std::vector<std::string> feature;
                    feature.reserve(feature_js.size());
                    for (auto&& it : feature_js)
                        feature.push_back(it);
                    bio.features.push_back(feature);
                }
            }
            {
                js_object spawners_js = js_object::get_object(bio_js["spawners"]);
                for (auto&& [name, values] : spawners_js) {
                    auto& category = bio.spawners[name];
                    auto category_values = js_array::get_array(values);
                    category.reserve(category_values.size());
                    for (auto&& it : category_values) {
                        auto category_value = js_object::get_object(it);
                        Biome::SpawnersValue value;
                        value.type = category_value["type"];
                        value.weight = category_value["weight"];
                        value.weight = category_value["minCount"];
                        value.weight = category_value["maxCount"];
                        category.push_back(value);
                    }
                }
            }
            {
                js_object spawn_costs_js = js_object::get_object(bio_js["spawn_costs"]);
                for (auto&& [eid, value] : spawn_costs_js) {
                    auto& category = bio.spawn_costs[eid];
                    auto category_value = js_object::get_object(value);
                    category.energy_budget = category_value["energy_budget"];
                    category.charge = category_value["charge"];
                }
            }

            biomes[id] = std::move(bio);
        }

        void load_file_biomes(const std::filesystem::path& file_path, const std::string& id) {
            check_override(biomes, id, "biome");

            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_biomes(js_object::get_object(res.value()), id);
        }

        ChatType::Decoration to_decoration(js_value&& json) {
            ChatType::Decoration decoration;
            js_object chat_js = js_object::get_object(json);
            if (chat_js.contains("style"))
                decoration.style = Chat::fromEnbt(conversions::json::from_json(chat_js["style"].get()));
            decoration.translation_key = (std::string)chat_js["translation_key"];
            {
                auto params = chat_js["parameters"];
                if (params.is_array()) {
                    auto params_ = js_array::get_array(params);
                    std::vector<std::string> parameters;
                    parameters.reserve(params_.size());
                    for (auto&& chat_ : params_)
                        parameters.push_back((std::string)chat_);
                    decoration.parameters = std::move(parameters);
                } else
                    decoration.parameters = (std::string)params;
            }
            return decoration;
        }

        base_objects::number_provider read_number_provider(js_value&& value) {
            if (value.is_number())
                return value.is_integral() ? base_objects::number_provider_constant((int32_t)value) : base_objects::number_provider_constant((float)value);
            else {
                auto obj = js_object::get_object(value);
                if (obj.contains("type")) {
                    std::string type = obj["type"];

                    if (type == "constant") {
                        auto value = obj.at("value");
                        return value.is_integral() ? base_objects::number_provider_constant((int32_t)value) : base_objects::number_provider_constant((float)value);
                    } else if (type == "uniform") {
                        std::variant<int32_t, float> min;
                        std::variant<int32_t, float> max;

                        if (obj.contains("min")) {
                            auto min_ = obj["min"];
                            min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                        } else if (obj.contains("min_inclusive")) {
                            auto min_ = obj["min_inclusive"];
                            min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                        } else
                            min = std::numeric_limits<int32_t>::min();

                        if (obj.contains("max")) {
                            auto max_ = obj["max"];
                            max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                        } else if (obj.contains("max_inclusive")) {
                            auto max_ = obj["max_inclusive"];
                            max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                        } else
                            max = std::numeric_limits<int32_t>::max();

                        return base_objects::number_provider_uniform(min, max);
                    } else if (type == "binomial")
                        return base_objects::number_provider_binomial(read_number_provider(obj.at("n")), read_number_provider(obj.at("p")));
                    else if (type == "clamped_normal") {
                        float mean = obj.at("mean");
                        float deviation = obj.at("deviation");
                        int32_t min_inclusive = obj.at("min_inclusive");
                        int32_t max_inclusive = obj.at("max_inclusive");
                        return base_objects::number_provider_clamped_normal(mean, deviation, min_inclusive, max_inclusive);
                    } else if (type == "uniform") {
                        std::variant<int32_t, float> min;
                        std::variant<int32_t, float> max;

                        if (obj.contains("min")) {
                            auto min_ = obj["min"];
                            min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                        } else if (obj.contains("min_inclusive")) {
                            auto min_ = obj["min_inclusive"];
                            min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                        } else
                            min = std::numeric_limits<int32_t>::min();

                        if (obj.contains("max")) {
                            auto max_ = obj["max"];
                            max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                        } else if (obj.contains("max_inclusive")) {
                            auto max_ = obj["max_inclusive"];
                            max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                        } else
                            max = std::numeric_limits<int32_t>::max();

                        return base_objects::number_provider_uniform(min, max);
                    } else if (type == "clamped") {
                        std::variant<int32_t, float> min;
                        std::variant<int32_t, float> max;

                        if (obj.contains("min")) {
                            auto min_ = obj["min"];
                            min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                        } else if (obj.contains("min_inclusive")) {
                            auto min_ = obj["min_inclusive"];
                            min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                        } else
                            min = std::numeric_limits<int32_t>::min();

                        if (obj.contains("max")) {
                            auto max_ = obj["max"];
                            max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                        } else if (obj.contains("max_inclusive")) {
                            auto max_ = obj["max_inclusive"];
                            max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                        } else
                            max = std::numeric_limits<int32_t>::max();
                        return base_objects::number_provider_clamped(min, max, read_number_provider(obj.at("source")));
                    } else if (type == "trapezoid") {
                        int32_t min = obj.at("min");
                        int32_t max = obj.at("max");
                        int32_t plateau = obj.at("plateau");
                        return base_objects::number_provider_trapezoid(min, max, plateau);
                    } else if (type == "weighted_list") {
                        std::vector<std::pair<base_objects::number_provider, double>> values;
                        auto values_js = js_array::get_array(obj.at("values"));
                        values.reserve(values_js.size());
                        for (auto&& value : values_js) {
                            auto value_js = js_object::get_object(value);
                            auto weight = value_js.contains("weight") ? value_js["weight"] : 1.0;
                            values.push_back({read_number_provider(value_js.at("data")), weight});
                        }
                        return base_objects::number_provider_weighted_list(values);
                    } else if (type == "biased_to_bottom") {
                        std::variant<int32_t, float> min;
                        std::variant<int32_t, float> max;

                        if (obj.contains("min")) {
                            auto min_ = obj["min"];
                            min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                        } else if (obj.contains("min_inclusive")) {
                            auto min_ = obj["min_inclusive"];
                            min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                        } else
                            min = std::numeric_limits<int32_t>::min();

                        if (obj.contains("max")) {
                            auto max_ = obj["max"];
                            max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                        } else if (obj.contains("max_inclusive")) {
                            auto max_ = obj["max_inclusive"];
                            max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                        } else
                            max = std::numeric_limits<int32_t>::max();

                        return base_objects::number_provider_biased_to_bottom(min, max);
                    } else if (type == "score") {
                        base_objects::number_provider_score res;
                        res.score = obj.at("score");
                        res.scale = obj.contains("scale") ? std::optional<float>((float)obj["scale"]) : std::nullopt;
                        auto target = js_object::get_object(obj.at("target"));
                        std::string type = target.at("type");
                        if (type == "fixed")
                            res.target.value = target.at("name");
                        else if (type == "context")
                            res.target.value = target.at("target");
                        else
                            target.parsing_error("Invalid target type: " + type);
                        return res;
                    } else if (type == "storage") {
                        base_objects::number_provider_storage res;
                        res.storage = obj.at("storage");
                        res.path = obj.at("path");
                        return res;
                    } else if (type == "enchantment_level")
                        return base_objects::number_provider_enchantment_level((std::string)obj.at("amount"));
                    else
                        obj.parsing_error("Invalid number provider type: " + type);
                } else {
                    int32_t min = obj.contains("min") ? obj["min"] : std::numeric_limits<int32_t>::min();
                    int32_t max = obj.contains("max") ? obj["max"] : std::numeric_limits<int32_t>::max();
                    return base_objects::number_provider_uniform(min, max);
                }
            }
        }

        void load_file_chatType(js_object&& type_js, const std::string& id) {
            check_override(chatTypes, id, "chat type");
            ChatType type;
            if (type_js.contains("chat"))
                type.chat = to_decoration(type_js["chat"]);
            if (type_js.contains("narration"))
                type.narration = to_decoration(type_js["narration"]);

            chatTypes[id] = std::move(type);
        }

        void load_file_chatType(const std::filesystem::path& file_path, const std::string& id) {
            check_override(chatTypes, id, "chat type");

            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            js_object type_js = js_object::get_object(res.value());
            ChatType type;
            if (type_js.contains("chat"))
                type.chat = to_decoration(type_js["chat"]);
            if (type_js.contains("narration"))
                type.narration = to_decoration(type_js["narration"]);

            chatTypes[id] = std::move(type);
        }

        void load_file_advancements(js_object&& advancement_js, const std::string& id) {
            check_conflicts(advancements, id, "advancements");
            Advancement advancement;
            if (advancement_js.contains("display")) {
                auto display_js = js_object::get_object(advancement_js["display"]);
                Advancement::Display display;
                {
                    auto icon_js = js_object::get_object(display_js["icon"]);
                    display.icon.item = icon_js["item"];
                    if (icon_js.contains("nbt"))
                        display.icon.nbt = icon_js["nbt"];
                }
                if (display_js.contains("frame"))
                    display.frame = display_js["frame"];
                else
                    display.frame = "task";

                display.description = display_js["description"].to_text();
                display.title = display_js["title"].to_text();

                if (display_js.contains("show_toast"))
                    display.show_toast = display_js["show_toast"];
                if (display_js.contains("announce_to_chat"))
                    display.announce_to_chat = display_js["announce_to_chat"];
                if (display_js.contains("hidden"))
                    display.hidden = display_js["hidden"];
            }
            if (advancement_js.contains("parent"))
                advancement.parent = advancement_js["parent"];
            advancement.criteria = util::conversions::json::from_json(advancement_js["criteria"].get());
            if (advancement_js.contains("requirements")) {
                auto list_of_requirements_js = js_array::get_array(advancement_js["requirements"]);
                advancement.requirements.reserve(list_of_requirements_js.size());
                for (auto&& value : list_of_requirements_js) {
                    auto requirements_js = js_array::get_array(value);
                    std::vector<std::string> requirements;
                    requirements.reserve(requirements_js.size());
                    for (auto&& req : requirements_js)
                        requirements.push_back(req);
                    advancement.requirements.push_back(requirements);
                }
            }
            if (advancement_js.contains("rewards")) {
                auto rewards_js = js_object::get_object(advancement_js["rewards"]);
                if (rewards_js.contains("recipes")) {
                    auto recipes_js = js_array::get_array(rewards_js["recipes"]);
                    advancement.rewards.recipes.reserve(rewards_js.size());
                    for (auto&& req : recipes_js)
                        advancement.rewards.recipes.push_back(req);
                }
                if (rewards_js.contains("loot")) {
                    auto loot_js = js_array::get_array(rewards_js["loot"]);
                    advancement.rewards.loot.reserve(rewards_js.size());
                    for (auto&& req : loot_js)
                        advancement.rewards.loot.push_back(req);
                }
                if (rewards_js.contains("experience"))
                    advancement.rewards.experience = rewards_js["experience"];
                if (rewards_js.contains("function"))
                    advancement.rewards.function = rewards_js["function"];
            }

            if (advancement_js.contains("sends_telemetry_event"))
                advancement.sends_telemetry_event = advancement_js["sends_telemetry_event"];
            advancements[id] = std::move(advancement);
        }

        void load_file_advancements(const std::filesystem::path& file_path, const std::string& id) {
            check_conflicts(advancements, id, "advancement");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_advancements(js_object::get_object(res.value()), id);
        }

        void load_file_jukebox_song(js_object&& song_js, const std::string& id) {
            check_conflicts(jukebox_songs, id, "jukebox song");
            JukeboxSong song;
            song.comparator_output = song_js["comparator_output"];
            song.length_in_seconds = song_js["length_in_seconds"];
            song.description = Chat::fromEnbt(util::conversions::json::from_json(song_js["description"].get()));
            auto sound_event_js = song_js["sound_event"];
            if (sound_event_js.is_string())
                song.sound_event = (std::string)sound_event_js;
            else {
                auto sound_event_obj = js_object::get_object(sound_event_js);
                base_objects::slot_component::inner::sound_extended ex;
                ex.sound_name = sound_event_obj["sound_id"];
                if (sound_event_obj.contains("range"))
                    ex.fixed_range = sound_event_obj["range"];
                song.sound_event = std::move(ex);
            }
            jukebox_songs[id] = std::move(song);
        }

        void load_file_jukebox_song(const std::filesystem::path& file_path, const std::string& id) {
            check_conflicts(jukebox_songs, id, "jukebox songs");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_jukebox_song(js_object::get_object(res.value()), id);
        }

        void load_file_loot_table(js_object&& loot_table_js, const std::string& id) {
            check_conflicts(loot_table, id, "loot table");

            loot_table_item item;
            if (loot_table_js.contains("type"))
                item.type = (std::string)loot_table_js["type"];
            else
                item.type = "generic";

            if (loot_table_js.contains("functions")) {
                auto functions = js_array::get_array(loot_table_js["functions"]);
                item.functions.reserve(functions.size());
                for (auto&& function : functions) {
                    enbt::compound comp;
                    comp = util::conversions::json::from_json(function.get());
                    item.functions.push_back(comp);
                }
            }

            if (loot_table_js.contains("pools")) {
                auto pools = js_array::get_array(loot_table_js["pools"]);
                item.pools.reserve(pools.size());
                for (auto&& pool_item : pools) {
                    auto pool = js_object::get_object(pool_item);
                    loot_table_item::pool pool_;
                    if (pool.contains("conditions")) {
                        auto res = util::conversions::json::from_json(pool["conditions"].get());
                        auto ref = res.as_array();
                        pool_.conditions.reserve(ref.size());
                        for (auto& it : ref)
                            pool_.conditions.push_back(it.as_compound());
                    }
                    if (pool.contains("bonus_rolls"))
                        pool_.bonus_rolls = read_number_provider(pool["bonus_rolls"]);

                    if (pool.contains("functions")) {
                        auto functions = js_array::get_array(pool["functions"]);
                        pool_.functions.reserve(functions.size());
                        for (auto&& function : functions) {
                            enbt::compound comp;
                            comp = util::conversions::json::from_json(function.get());
                            pool_.functions.push_back(comp);
                        }
                    }
                    pool_.rolls = read_number_provider(pool.at("rolls"));
                    auto entries = js_array::get_array(pool.at("entries"));
                    pool_.entries.reserve(entries.size());
                    for (auto&& entry : entries) {
                        enbt::compound comp;
                        comp = util::conversions::json::from_json(entry.get());
                        pool_.entries.push_back(comp);
                    }
                    item.pools.push_back(std::move(pool_));
                }
            }


            if (loot_table_js.contains("random_sequence"))
                item.random_sequence = loot_table_js["random_sequence"];
            loot_table[id] = std::move(item);
        }

        void load_file_loot_table(const std::filesystem::path& file_path, const std::string& id) {
            check_override(armorTrimPatterns, id, "armor trim pattern");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_loot_table(js_object::get_object(res.value()), id);
        }

        void load_file_armorTrimPattern(js_object&& pattern_js, const std::string& id) {
            check_override(armorTrimPatterns, id, "armor trim pattern");
            ArmorTrimPattern pattern;
            pattern.asset_id = (std::string)pattern_js["asset_id"];
            pattern.decal = pattern_js["decal"];
            pattern.template_item = (std::string)pattern_js["template_item"];
            {
                auto desc = pattern_js["description"];
                if (desc.is_string())
                    pattern.description = (std::string)desc;
                else
                    pattern.description = Chat::fromEnbt(conversions::json::from_json(desc.get()));
            }
            armorTrimPatterns[id] = std::move(pattern);
        }

        void load_file_armorTrimPattern(const std::filesystem::path& file_path, const std::string& id) {
            check_override(armorTrimPatterns, id, "armor trim pattern");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_armorTrimPattern(js_object::get_object(res.value()), id);
        }

        void load_file_armorTrimMaterial(js_object&& material_js, const std::string& id) {
            check_override(armorTrimMaterials, id, "armor trim material");
            ArmorTrimMaterial material;
            material.asset_name = (std::string)material_js["asset_name"];
            material.ingredient = (std::string)material_js["ingredient"];
            material.item_model_index = material_js["item_model_index"];
            {
                auto desc = material_js["description"];
                if (desc.is_string())
                    material.description = (std::string)desc;
                else
                    material.description = Chat::fromEnbt(conversions::json::from_json(desc.get()));
            }
            if (material_js.contains("override_armor_materials")) {
                auto override_armor_materials = material_js["override_armor_materials"];
                for (auto&& [name, material_] : js_object::get_object(override_armor_materials))
                    material.override_armor_materials[(std::string)name] = (std::string)material_;
            }
            armorTrimMaterials[id] = std::move(material);
        }

        void load_file_armorTrimMaterial(const std::filesystem::path& file_path, const std::string& id) {
            check_override(armorTrimMaterials, id, "armor trim material");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_armorTrimMaterial(js_object::get_object(res.value()), id);
        }

        void load_file_wolfVariant(js_object&& variant_js, const std::string& id) {
            check_override(wolfVariants, id, "wolf variant");
            WolfVariant variant;
            variant.wild_texture = (std::string)variant_js.at("wild_texture");
            variant.tame_texture = (std::string)variant_js.at("tame_texture");
            variant.angry_texture = (std::string)variant_js.at("angry_texture");
            auto _biomes = variant_js.at("biomes");
            if (_biomes.is_array()) {
                auto biomes = js_array::get_array(variant_js.at("biomes"));
                variant.biomes.reserve(biomes.size());
                for (auto&& biome : biomes)
                    variant.biomes.push_back(biome);
            } else
                variant.biomes.push_back((std::string)_biomes);
            wolfVariants[id] = std::move(variant);
        }

        void load_file_wolfVariant(const std::filesystem::path& file_path, const std::string& id) {
            check_override(wolfVariants, id, "wolf variant");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_wolfVariant(js_object::get_object(res.value()), id);
        }

        void load_file_dimensionType(js_object&& type_js, const std::string& id) {
            check_override(dimensionTypes, id, "dimension type");
            DimensionType type;
            if (type_js.contains("monster_spawn_light_level")) {
                auto monster_spawn_light_level = type_js["monster_spawn_light_level"];
                if (monster_spawn_light_level.is_number())
                    type.monster_spawn_light_level = monster_spawn_light_level;
                else {

                    js_object monster_spawn_light_level_js = js_object::get_object(monster_spawn_light_level);
                    IntegerDistribution monster_spawn_light_level_;
                    monster_spawn_light_level_.value = conversions::json::from_json(monster_spawn_light_level_js.get());
                    monster_spawn_light_level_.type = (std::string)monster_spawn_light_level_js.at("type");
                    type.monster_spawn_light_level = std::move(monster_spawn_light_level_);
                }
            }
            if (type_js.contains("fixed_time"))
                type.fixed_time = type_js["fixed_time"];

            type.infiniburn = (std::string)type_js["infiniburn"];
            type.effects = (std::string)type_js["effects"];
            type.coordinate_scale = type_js["coordinate_scale"];
            type.ambient_light = type_js["ambient_light"];
            type.min_y = type_js["min_y"];
            type.height = type_js["height"];
            type.logical_height = type_js["logical_height"];
            type.monster_spawn_block_light_limit = type_js["monster_spawn_block_light_limit"];
            type.has_skylight = type_js["has_skylight"];
            type.has_ceiling = type_js["has_ceiling"];
            type.ultrawarm = type_js["ultrawarm"];
            type.natural = type_js["natural"];
            type.piglin_safe = type_js["piglin_safe"];
            type.has_raids = type_js["has_raids"];
            type.respawn_anchor_works = type_js["respawn_anchor_works"];
            type.bed_works = type_js["bed_works"];
            dimensionTypes[id] = std::move(type);
        }

        void load_file_dimensionType(const std::filesystem::path& file_path, const std::string& id) {
            check_override(dimensionTypes, id, "dimension type");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_dimensionType(js_object::get_object(res.value()), id);
        }

        void load_file_enchantment(js_object&& type_js, const std::string& id) {
            check_conflicts(enchantments, id, "enchantments");
            enchantment type;
            type.description = Chat::fromEnbt(util::conversions::json::from_json(type_js.at("description").get()));
            type.max_level = type_js.at("max_level");
            type.weight = type_js.at("weight");
            type.anvil_cost = type_js.at("anvil_cost");
            auto slots = js_array::get_array(type_js.at("slots"));
            type.slots.reserve(slots.size());
            for (auto&& slot : slots)
                type.slots.push_back(slot);
            if (type_js.contains("exclusive_set")) {
                if (type_js.at("exclusive_set").is_string())
                    type.exclusive_set = type_js.at("exclusive_set");
                else {
                    auto exclusive_set_js = js_array::get_array(type_js.at("exclusive_set"));
                    std::vector<std::string> exclusive_set;
                    exclusive_set.reserve(exclusive_set_js.size());
                    for (auto&& set : exclusive_set_js)
                        exclusive_set.push_back(set);
                    type.exclusive_set = std::move(exclusive_set);
                }
            }
            if (type_js.at("supported_items").is_string())
                type.supported_items = type_js.at("supported_items");
            else {
                auto supported_items_js = js_array::get_array(type_js.at("supported_items"));
                std::vector<std::string> supported_items;
                supported_items.reserve(supported_items_js.size());
                for (auto&& set : supported_items_js)
                    supported_items.push_back(set);
                type.supported_items = std::move(supported_items);
            }
            if (type_js.contains("primary_items")) {
                if (type_js.at("primary_items").is_string())
                    type.primary_items = type_js.at("primary_items");
                else {
                    auto primary_items_js = js_array::get_array(type_js.at("primary_items"));
                    std::vector<std::string> primary_items;
                    primary_items.reserve(primary_items_js.size());
                    for (auto&& set : primary_items_js)
                        primary_items.push_back(set);
                    type.primary_items = std::move(primary_items);
                }
            }
            auto min_cost = js_object::get_object(type_js.at("min_cost"));
            type.min_cost.base = min_cost.at("base");
            type.min_cost.per_level_above_first = min_cost.at("per_level_above_first");

            auto max_cost = js_object::get_object(type_js.at("max_cost"));
            type.max_cost.base = max_cost.at("base");
            type.max_cost.per_level_above_first = max_cost.at("per_level_above_first");

            if (type_js.contains("effects")) {
                auto effects = js_object::get_object(type_js.at("effects"));
                type.effects.reserve(effects.size());
                for (auto&& [component_id, effect] : effects)
                    type.effects[component_id] = util::conversions::json::from_json(effect.get());
            }
            enchantments[id] = std::move(type);
        }

        void load_file_enchantment(const std::filesystem::path& file_path, const std::string& id) {
            check_conflicts(enchantments, id, "enchantments");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_enchantment(js_object::get_object(res.value()), id);
        }

        void load_file_enchantment_provider(boost::json::object& type_js, const std::string& id) {
            check_conflicts(enchantment_providers, id, "enchantment providers");
            enchantment_providers[id] = util::conversions::json::from_json(type_js);
        }

        void load_file_enchantment_provider(const std::filesystem::path& file_path, const std::string& id) {
            check_conflicts(enchantment_providers, id, "enchantment providers");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_enchantment_provider(res.value(), id);
        }

        void load_file_damageType(js_object&& type_js, const std::string& id) {
            check_override(damageTypes, id, "damage type");
            DamageType type;
            type.message_id = (std::string)type_js["message_id"];
            std::string scaling = type_js["scaling"];
            if (scaling == "never")
                type.scaling = DamageType::ScalingType::never;
            else if (scaling == "when_caused_by_living_non_player")
                type.scaling = DamageType::ScalingType::when_caused_by_living_non_player;
            else if (scaling == "always")
                type.scaling = DamageType::ScalingType::always;
            else
                type_js["scaling"].parsing_error("Unknown scaling type: " + scaling);

            if (type_js.contains("effects")) {
                std::string effects = type_js["effects"];
                if (effects == "hurt")
                    type.effects = DamageType::EffectsType::hurt;
                else if (effects == "thorns")
                    type.effects = DamageType::EffectsType::thorns;
                else if (effects == "drowning")
                    type.effects = DamageType::EffectsType::drowning;
                else if (effects == "burning")
                    type.effects = DamageType::EffectsType::burning;
                else if (effects == "poking")
                    type.effects = DamageType::EffectsType::poking;
                else if (effects == "freezing")
                    type.effects = DamageType::EffectsType::freezing;
                else
                    type_js["effects"].parsing_error("Unknown effects type: " + effects);
            }

            if (type_js.contains("death_message_type")) {
                std::string death_message_type = type_js["death_message_type"];
                if (death_message_type == "default")
                    type.death_message_type = DamageType::DeathMessageType::_default;
                else if (death_message_type == "fall_variants")
                    type.death_message_type = DamageType::DeathMessageType::fall_variants;
                else if (death_message_type == "intentional_game_design")
                    type.death_message_type = DamageType::DeathMessageType::intentional_game_design;
                else
                    type_js["death_message_type"].parsing_error("Unknown death message type: " + death_message_type);
            }

            type.exhaustion = type_js["exhaustion"];
            damageTypes[id] = std::move(type);
        }

        void load_file_damageType(const std::filesystem::path& file_path, const std::string& id) {
            check_override(damageTypes, id, "damage type");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_damageType(js_object::get_object(res.value()), id);
        }

        void load_file_bannerPattern(js_object&& pattern_js, const std::string& id) {
            check_override(bannerPatterns, id, "banner pattern");
            BannerPattern pattern;
            pattern.asset_id = (std::string)pattern_js["asset_id"];
            pattern.translation_key = (std::string)pattern_js["translation_key"];
            bannerPatterns[id] = std::move(pattern);
        }

        void load_file_bannerPattern(const std::filesystem::path& file_path, const std::string& id) {
            check_override(bannerPatterns, id, "banner pattern");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_bannerPattern(js_object::get_object(res.value()), id);
        }

        void load_file_paintingVariant(js_object&& variant_js, const std::string& id) {
            check_override(paintingVariants, id, "painting variant");
            PaintingVariant variant;
            variant.asset_id = (std::string)variant_js["asset_id"];
            variant.height = variant_js["height"];
            variant.width = variant_js["width"];
            paintingVariants[id] = std::move(variant);
        }

        void load_file_paintingVariant(const std::filesystem::path& file_path, const std::string& id) {
            check_override(paintingVariants, id, "painting variant");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_paintingVariant(js_object::get_object(res.value()), id);
        }

        void load_file_recipe(js_object&& variant_js, const std::string& id) {
            if (!api::recipe::registered())
                throw std::runtime_error("Recipe api not registered!");

            enbt::compound res;
            res = util::conversions::json::from_json(variant_js.get());
            api::recipe::set_recipe(id, std::move(res));
        }

        void load_file_recipe(const std::filesystem::path& file_path, const std::string& id) {
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_recipe(js_object::get_object(res.value()), id);
        }

        void apply_tags(js_value val, const std::string& type, const std::string& namespace_, const std::string& path_, bool replace) {
            list_array<std::string> result;
            for (auto&& tag : js_array::get_array(val)) {
                std::string the_tag;
                if (tag.is_string())
                    the_tag = (std::string)tag;
                else {
                    auto tag_ = js_object::get_object(tag);
                    the_tag = (std::string)tag_.at("id");
                }

                if (the_tag.starts_with("#")) {
                    result.push_back(unfold_tag(type, namespace_, the_tag.substr(1)));
                } else
                    result.push_back(the_tag);
            }
            tags[type][namespace_][path_].push_back(std::move(result));
        }

        void load_file_tags(js_object&& tags_, const std::string& type, const std::string& namespace_, const std::string& path_) {
            if (tags_.contains("replace")) {
                bool replace = tags_["replace"];
                if (replace)
                    tags[type][namespace_][path_].clear();
                apply_tags(tags_["values"], type, namespace_, path_, replace);
            } else if (tags_.contains("values"))
                apply_tags(tags_["values"], type, namespace_, path_, false);
            else if (tags_.contains("root"))
                apply_tags(tags_["root"], type, namespace_, path_, false);
            else
                tags_.parsing_error("Invalid tag file format");
        }

        void load_file_tags(const std::filesystem::path& file_path, const std::string& type, const std::string& namespace_, const std::string& path_) {
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_tags(js_object::get_object(res.value()), type, namespace_, path_);
        }

        void load_register_file(std::string_view memory, const std::string& namespace_, const std::string& path_, const std::string& type) {
        }

        void load_register_file(const std::filesystem::path& file_path, const std::string& namespace_, const std::string& path_, const std::string& type) {
            if (path_.empty())
                throw std::runtime_error("Path is empty");
            std::string id = (namespace_.empty() ? default_namespace : namespace_) + ":" + path_;
            if (type == "advancement") {
                load_file_advancements(file_path, id);
            } else if (type == "banner_pattern")
                load_file_bannerPattern(file_path, id);
            else if (type == "chat_type")
                load_file_chatType(file_path, id);
            else if (type == "damage_type")
                load_file_damageType(file_path, id);
            else if (type == "dimension_type")
                load_file_dimensionType(file_path, id);
            else if (type == "enchantment")
                load_file_enchantment(file_path, id);
            else if (type == "enchantment_provider")
                load_file_enchantment_provider(file_path, id);
            else if (type == "jukebox_song")
                load_file_jukebox_song(file_path, id);
            else if (type == "loot_table")
                load_file_loot_table(file_path, id);
            else if (type == "painting_variant")
                load_file_paintingVariant(file_path, id);
            else if (type == "recipe")
                load_file_recipe(file_path, id);
            else if (type == "trim_pattern")
                load_file_armorTrimPattern(file_path, id);
            else if (type == "trim_material")
                load_file_armorTrimMaterial(file_path, id);
            else if (type == "wolf_variant")
                load_file_wolfVariant(file_path, id);
            else if (type == "worldgen/biome")
                load_file_biomes(file_path, id);
            else if (type == "worldgen/configured_carver")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/configured_feature")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/density_function")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/flat_level_generator_preset")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/multi_noise_biome_source_parameter_list")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/noise")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/noise_settings")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/placed_feature")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/processor_list")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/structure")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/structure_set")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/template_pool")
                ; //load_file_biomes(file_path, id);
            else if (type == "worldgen/world_preset")
                ; //load_file_biomes(file_path, id);
            else if (type.starts_with("tag")) {
                std::string tag_type;
                if (type.starts_with("tags/"))
                    tag_type = type.substr(5);
                else if (type.starts_with("tag/"))
                    tag_type = type.substr(4);
                else
                    throw std::runtime_error("Unknown type: " + type);
                load_file_tags(file_path, tag_type, namespace_, path_);
            } else
                throw std::runtime_error("Unknown type: " + type);
        }

        static inline uint64_t as_uint(const boost::json::value& val) {
            if (val.is_int64())
                return val.to_number<int32_t>();
            else if (val.is_uint64())
                return val.as_uint64();
            else
                throw std::runtime_error("Invalid value type");
        }

        void load_blocks() {
            auto parsed = boost::json::parse(resources::registry::blocks);

            base_objects::block::access_full_block_data(std::function(
                [&](
                    std::vector<std::shared_ptr<base_objects::static_block_data>>& full_block_data_,
                    std::unordered_map<std::string, std::shared_ptr<base_objects::static_block_data>>& named_full_block_data
                ) {
                    size_t usable = 30000;
                    full_block_data_.resize(usable);
                    for (auto&& [name, decl] : parsed.as_object()) {
                        std::unordered_map<std::string, std::unordered_set<std::string>> properties_def;
                        if (decl.as_object().contains("properties")) {
                            for (auto&& [prop_name, prop] : decl.at("properties").as_object()) {
                                std::unordered_set<std::string> prop_list;
                                for (auto&& prop_val : prop.as_array())
                                    prop_list.insert((std::string)prop_val.as_string());
                                properties_def[(std::string)prop_name] = std::move(prop_list);
                            }
                        }

                        decltype(base_objects::static_block_data::assigned_states) associated_states;
                        std::optional<base_objects::block_id_t> default_associated_state;
                        for (auto&& state : decl.at("states").as_array()) {
                            auto&& state_ = state.as_object();
                            base_objects::block_id_t id = as_uint(state_.at("id"));

                            std::unordered_map<std::string, std::string> state_properties;
                            if (state_.contains("properties")) {
                                for (auto&& [prop_name, prop] : state_.at("properties").as_object())
                                    state_properties[prop_name] = (std::string)prop.as_string();
                            }

                            if (state_.contains("default")) {
                                if (state_.at("default").as_bool())
                                    default_associated_state = id;
                            }
                            associated_states.insert({id, std::move(state_properties)});
                        }

                        auto data = std::make_shared<base_objects::static_block_data>();


                        for (auto& [id, _unused] : associated_states) {
                            if (full_block_data_[id] != nullptr) {
                                throw std::runtime_error("Duplicate block id: " + std::to_string(id));
                            }
                            full_block_data_[id] = data;
                        }
                        if (named_full_block_data.contains((std::string)name))
                            throw std::runtime_error("Duplicate block name: " + (std::string)name);


                        data->name = name;
                        data->default_state = default_associated_state.value_or(associated_states.left.begin()->first);
                        data->states = std::move(properties_def);
                        data->assigned_states = std::move(associated_states);
                        data->defintion = util::conversions::json::from_json(decl.at("definition"));

                        named_full_block_data[(std::string)name] = std::move(data);
                    }

                    for (auto it = full_block_data_.rbegin(); it != full_block_data_.rend(); ++it) {
                        if (*it != nullptr)
                            break;
                        --usable;
                    }
                    full_block_data_.resize(usable);
                    for (auto it = full_block_data_.begin(); it != full_block_data_.end(); ++it) {
                        if (*it == nullptr) {
                            throw std::runtime_error("Gap between block definitions");
                        }
                    }
                    full_block_data_.shrink_to_fit();
                }
            ));
        }

        void load_items() {
            auto parsed = boost::json::parse(resources::registry::items);
            for (auto&& [name, decl] : parsed.as_object()) {
                base_objects::static_slot_data slot_data;
                slot_data.id = name;
                std::unordered_map<std::string, base_objects::slot_component::unified> components;
                for (auto& [name, value] : decl.as_object().at("components").as_object())
                    components[name] = base_objects::slot_component::parse_component(name, conversions::json::from_json(value));
                slot_data.default_components = std::move(components);
                base_objects::slot_data::add_slot_data(std::move(slot_data));
            }
        }

        void prepare_versions() {
            registers::individual_registers[765] = util::conversions::json::from_json(boost::json::parse(resources::registry::protocol::_765));
            registers::individual_registers[766] = util::conversions::json::from_json(boost::json::parse(resources::registry::protocol::_766));
            registers::individual_registers[767] = util::conversions::json::from_json(boost::json::parse(resources::registry::protocol::_767));
            registers::individual_registers[768] = util::conversions::json::from_json(boost::json::parse(resources::registry::protocol::_768));
        }

        using tags_obj = std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>>>;

        void __prepare_tags(tags_obj& tmp_obj, boost::json::object& parsed, const std::string& type, const std::string& namespace_, const std::string& tag) {
            for (auto&& [name, decl] : parsed) {
                auto& obj = decl.as_object();
                if (name.ends_with(".json")) {
                    if (obj.contains("values") || obj.contains("root")) {
                        auto& values = obj.contains("values") ? obj.at("values") : obj.at("root");
                        if (values.is_array()) {
                            bool replace = obj.contains("replace") ? obj.at("replace").as_bool() : false;
                            auto& items = tmp_obj[type][namespace_][tag + ":" + (std::string)name.substr(0, name.size() - 5)];
                            if (replace)
                                items.clear();
                            for (auto&& value : values.get_array())
                                items.push_back((std::string)value.as_string());
                            continue;
                        }
                    }
                } else
                    __prepare_tags(tmp_obj, obj, type, namespace_, tag + ":" + (std::string)name);
            }
        }

        void prepare_tags(boost::json::object& parsed, const std::string& namespace_) {
            tags_obj tmp_obj = registers::tags;
            for (auto&& [type, decl] : parsed)
                __prepare_tags(tmp_obj, decl.get_object(), type, namespace_, "");

            registers::tags = tmp_obj;
            for (auto&& [type, decl] : tmp_obj) {
                for (auto&& [namespace_, decl] : decl) {
                    for (auto&& [tag, decl] : decl) {
                        list_array<std::string> resolved_items;
                        for (auto& item : decl) {
                            if (item.starts_with("#")) {
                                resolved_items.push_back(unfold_tag(type, item).where([](const std::string& tag) {
                                    return !tag.starts_with("#");
                                }));
                            }
                        }
                        decl = std::move(resolved_items.commit());
                    }
                }
            }

            registers::tags = std::move(tmp_obj);
        }

        void process_item_(boost::json::object& decl, const std::string& namespace_, void (*fn)(js_object&&, const std::string&)) {
            for (auto& [id, value] : decl) {
                if (id.ends_with(".json")) {
                    std::string _id = namespace_ + std::string(id.substr(0, id.size() - 5));
                    fn(js_object::get_object(value), _id);
                } else
                    process_item_(value.as_object(), namespace_ + std::string(id), fn);
            }
        }

        void process_item_(boost::json::object& decl, const std::string& namespace_, void (*fn)(boost::json::object&, const std::string&)) {
            for (auto& [id, value] : decl) {
                if (id.ends_with(".json")) {
                    std::string _id = namespace_ + std::string(id.substr(0, id.size() - 5));
                    fn(value.as_object(), _id);
                } else
                    process_item_(value.as_object(), namespace_ + std::string(id), fn);
            }
        }

        void process_item(js_value& decl, const std::string& namespace_, void (*fn)(js_object&&, const std::string&)) {
            process_item_(decl.get().as_object(), namespace_ + ":", fn);
        }

        void process_item(js_value& decl, const std::string& namespace_, void (*fn)(boost::json::object&, const std::string&)) {
            process_item_(decl.get().as_object(), namespace_ + ":", fn);
        }

        void process_pack(boost::json::object& parsed, const std::string& namespace_, bool allowed_pack_nest) {
            auto data = js_object::get_object(parsed);
            if (data.contains("tags"))
                prepare_tags(data["tags"].get().as_object(), namespace_);
            for (auto&& [name, decl] : data) {
                if (name == "banner_pattern")
                    process_item(decl, namespace_, load_file_bannerPattern);
                else if (name == "painting_variant")
                    process_item(decl, namespace_, load_file_paintingVariant);
                else if (name == "damage_type")
                    process_item(decl, namespace_, load_file_damageType);
                else if (name == "dimension_type")
                    process_item(decl, namespace_, load_file_dimensionType);
                else if (name == "wolf_variant")
                    process_item(decl, namespace_, load_file_wolfVariant);
                else if (name == "trim_material")
                    process_item(decl, namespace_, load_file_armorTrimMaterial);
                else if (name == "trim_pattern")
                    process_item(decl, namespace_, load_file_armorTrimPattern);
                else if (name == "enchantment")
                    process_item(decl, namespace_, load_file_enchantment);
                else if (name == "enchantment_provider")
                    process_item(decl, namespace_, load_file_enchantment_provider);
                else if (name == "chat_type")
                    process_item(decl, namespace_, load_file_chatType);
                else if (name == "worldgen") {
                    for (auto&& [name, decl] : js_object::get_object(decl)) {
                        if (name == "biome")
                            process_item(decl, namespace_, load_file_biomes);
                    }
                }
            }
        }

        void process_pack(boost::json::object& parsed, const std::string& namespace_) {
            process_pack(parsed, namespace_, false);
        }

        void ___recursive_merge_json(boost::json::object& out, const std::filesystem::directory_entry& file);

        void ___recursive_merge_json__file(boost::json::object& out, const std::filesystem::directory_entry& file) {
            auto it = try_read_json_file(file.path());
            if (!it)
                throw std::runtime_error("Failed to read file: " + file.path().string());
            out[file.path().filename().string()] = std::move(*it);
        }

        void ___recursive_merge_json__directory(boost::json::object& out, const std::filesystem::directory_entry& file) {
            for (const auto& f : std::filesystem::directory_iterator(file.path())) {
                auto& inner = (out[f.path().filename().string()] = boost::json::object()).get_object();
                ___recursive_merge_json(inner, f);
            }
        }

        void ___recursive_merge_json(boost::json::object& out, const std::filesystem::directory_entry& file) {
            if (file.is_directory())
                ___recursive_merge_json__directory(out, file);
            else
                ___recursive_merge_json__file(out, file);
        }

        void process_pack(const std::filesystem::path& folder_path_to_data_without_namespace, const std::string& namespace_) {
            boost::json::value value = boost::json::object();
            auto& obj = value.get_object();
            for (auto& entry : std::filesystem::directory_iterator(folder_path_to_data_without_namespace)) {
                auto& inner = (obj[entry.path().filename().string()] = boost::json::object()).get_object();
                ___recursive_merge_json(inner, entry);
            }
            process_pack(obj, namespace_);
        }

        void process_pack(const std::filesystem::path& folder_path_to_data) {
            for (auto& entry : std::filesystem::directory_iterator(folder_path_to_data)) {
                if (entry.is_directory()) {
                    auto& path = entry.path();
                    std::string namespace_ = path.stem().string();
                    process_pack(path, namespace_);
                }
            }
        }

        void prepare_built_in_pack() {
            auto parsed = boost::json::parse(resources::data);
            process_pack(parsed.as_object(), "minecraft");
        }

        void __initialization__versions_inital() { //skips items assignation
        }

        void __initialization__versions_post() {
        }

        void initialize() {
            initialize_entities();
            load_blocks();
            prepare_built_in_pack();
            prepare_versions();
            __initialization__versions_inital();
            load_items();
            __initialization__versions_post();
            load_registers_complete();
        }
    }
}
