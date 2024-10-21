#include "../registers.hpp"
#include "../base_objects/entity.hpp"
#include "../util/conversions.hpp"
#include "../util/json_helpers.hpp"
#include "../util/slot_reader.hpp"
#include "embed/blocks.json.hpp"
#include "embed/items.json.hpp"

#include "versions/embed/765.json.hpp"
#include "versions/embed/766.json.hpp"
#include "versions/embed/767.json.hpp"

#include "../api/recipe.hpp"

namespace crafted_craft {
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

        void initialize_registers() {
            using namespace registers;
            biomes = {
                {
                    "minecraft:badlands",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.0,
                        .downfall = 0,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .foliage_color = 10387789,
                            .grass_color = 9470285,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.badlands",
                            },
                        },
                    },
                },
                {
                    "minecraft:bamboo_jungle",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.95,
                        .downfall = 0.9,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7842047,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.bamboo_jungle",
                            },
                        },
                    },
                },
                {
                    "minecraft:basalt_deltas",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.00,
                        .downfall = 0,
                        .effects = {
                            .fog_color = 6840176,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .particle = Biome::Particle{
                                .options = {
                                    .type = "minecraft:white_ash",
                                },
                                .probability = 0.118093334,
                            },
                            .ambient_sound = Biome::AmbientSound{
                                .sound = "minecraft:ambient.basalt_deltas.loop",
                            },
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.basalt_deltas.mood",
                            },
                            .additions_sound = Biome::AdditionsSound{
                                .sound = "minecraft:ambient.basalt_deltas.additions",
                                .tick_chance = 0.0111,
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.nether.basalt_deltas",
                            },
                        },
                    },
                },
                {
                    "minecraft:beach",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.8,
                        .downfall = 0.4,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7907327,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:birch_forest",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.6,
                        .downfall = 0.6,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8037887,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.forest",
                            },
                        },
                    },
                },
                {
                    "minecraft:cherry_grove",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 6141935,
                            .water_fog_color = 6141935,
                            .sky_color = 8103167,
                            .foliage_color = 11983713,
                            .grass_color = 11983713,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.cherry_grove",
                            },
                        },
                    },
                },
                {
                    "minecraft:cold_ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4020182,
                            .water_fog_color = 6141935,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:crimson_forest",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.00,
                        .downfall = 0,
                        .effects = {
                            .fog_color = 3343107,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .particle = Biome::Particle{
                                .options = {
                                    .type = "minecraft:crimson_spore",
                                },
                                .probability = 0.025,
                            },
                            .ambient_sound = Biome::AmbientSound{
                                .sound = "minecraft:ambient.crimson_forest.loop",
                            },
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.crimson_forest.mood",
                            },
                            .additions_sound = Biome::AdditionsSound{
                                .sound = "minecraft:ambient.crimson_forest.additions",
                                .tick_chance = 0.0111,
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.nether.crimson_forest",
                            },
                        },
                    },
                },
                {
                    "minecraft:dark_forest",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.7,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7972607,
                            .grass_color_modifier = "dark_forest",
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.forest",
                            },
                        },
                    },
                },
                {
                    "minecraft:deep_cold_ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4020182,
                            .water_fog_color = 329011,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:deep_dark",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.8,
                        .downfall = 0.4,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7907327,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.deep_dark",
                            },
                        },
                    },
                },
                {
                    "minecraft:deep_frozen_ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .temperature_modifier = "frozen",
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 3750089,
                            .water_fog_color = 329011,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:deep_lukewarm_ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4566514,
                            .water_fog_color = 267827,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:deep_ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:desert",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2,
                        .downfall = 0,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.desert",
                            },
                        },
                    },
                },
                {
                    "minecraft:dripstone_caves",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.8,
                        .downfall = 0.4,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7907327,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.dripstone_caves",
                            },
                        },
                    },
                },
                {
                    "minecraft:end_barrens",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 10518688,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 0,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:end_highlands",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 10518688,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 0,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:end_midlands",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 10518688,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 0,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:eroded_badlands",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2,
                        .downfall = 0,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .foliage_color = 10387789,
                            .grass_color = 9470285,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.badlands",
                            },
                        },
                    },
                },
                {
                    "minecraft:flower_forest",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.7,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7972607,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.flower_forest",
                            },
                        },
                    },
                },
                {
                    "minecraft:flower_forest",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.7,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7972607,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.flower_forest",
                            },
                        },
                    },
                },
                {
                    "minecraft:forest",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.7,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7972607,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.forest",
                            },
                        },
                    },
                },
                {
                    "minecraft:frozen_ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.0,
                        .downfall = 0.5,
                        .temperature_modifier = "frozen",
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 3750089,
                            .water_fog_color = 329011,
                            .sky_color = 8364543,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:frozen_peaks",
                    Biome{
                        .has_precipitation = true,
                        .temperature = -0.7,
                        .downfall = 0.9,
                        .temperature_modifier = "frozen",
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8756735,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.frozen_peaks",
                            },
                        },
                    },
                },
                {
                    "minecraft:frozen_river",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.0,
                        .downfall = 0.5,
                        .temperature_modifier = "frozen",
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 3750089,
                            .water_fog_color = 329011,
                            .sky_color = 8364543,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:grove",
                    Biome{
                        .has_precipitation = true,
                        .temperature = -0.2,
                        .downfall = 0.8,
                        .temperature_modifier = "frozen",
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8495359,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.grove",
                            },
                        },
                    },
                },
                {
                    "minecraft:ice_spikes",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.0,
                        .downfall = 0.5,
                        .temperature_modifier = "frozen",
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8364543,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:jagged_peaks",
                    Biome{
                        .has_precipitation = true,
                        .temperature = -0.7,
                        .downfall = 0.9,
                        .temperature_modifier = "frozen",
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8756735,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.jagged_peaks",
                            },
                        },
                    },
                },
                {
                    "minecraft:jungle",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.95,
                        .downfall = 0.9,
                        .temperature_modifier = "frozen",
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7842047,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.jungle",
                            },
                        },
                    },
                },
                {
                    "minecraft:lukewarm_ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4566514,
                            .water_fog_color = 267827,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:mangrove_swamp",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.8,
                        .downfall = 0.9,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 3832426,
                            .water_fog_color = 5077600,
                            .sky_color = 7907327,
                            .foliage_color = 9285927,
                            .grass_color = 9285927,
                            .grass_color_modifier = "swamp",
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.swamp",
                            },
                        },
                    },
                },
                {
                    "minecraft:meadow",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 937679,
                            .water_fog_color = 329011,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.meadow",
                            },
                        },
                    },
                },
                {
                    "minecraft:mushroom_fields",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.9,
                        .downfall = 1.0,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 415920,
                            .water_fog_color = 329011,
                            .sky_color = 7842047,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:nether_wastes",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.0,
                        .downfall = 0.0,
                        .effects = {
                            .fog_color = 3344392,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .ambient_sound = Biome::AmbientSound{
                                .sound = "minecraft:ambient.nether_wastes.loop",
                                .range = 8.0,
                            },
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.nether_wastes.mood",
                            },
                            .additions_sound = Biome::AdditionsSound{
                                .sound = "minecraft:ambient.nether_wastes.additions",
                                .tick_chance = 0.0111,
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.nether.nether_wastes",
                            },
                        },
                    },
                },
                {
                    "minecraft:ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:old_growth_birch_forest",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.6,
                        .downfall = 0.6,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8037887,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.forest",
                            },
                        },
                    },
                },
                {
                    "minecraft:old_growth_pine_taiga",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.3,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8168447,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.old_growth_taiga",
                            },
                        },
                    },
                },
                {
                    "minecraft:old_growth_spruce_taiga",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.25,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8233983,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.old_growth_taiga",
                            },
                        },
                    },
                },
                {
                    "minecraft:plains",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.8,
                        .downfall = 0.4,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7907327,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:river",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:savanna",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.0,
                        .downfall = 0.0,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:savanna_plateau",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.0,
                        .downfall = 0.0,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:small_end_islands",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 10518688,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 0,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:snowy_beach",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.05,
                        .downfall = 0.3,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4020182,
                            .water_fog_color = 329011,
                            .sky_color = 8364543,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:snowy_mountains",
                    Biome{
                        .has_precipitation = true,
                        .temperature = -0.5,
                        .downfall = 0.3,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8364543,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.snowy_mountains",
                            },
                        },
                    },
                },
                {
                    "minecraft:snowy_taiga",
                    Biome{
                        .has_precipitation = true,
                        .temperature = -0.5,
                        .downfall = 0.4,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4020182,
                            .water_fog_color = 329011,
                            .sky_color = 8625919,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:snowy_tundra",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.0,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4020182,
                            .water_fog_color = 329011,
                            .sky_color = 8364543,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:soul_sand_valley",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.0,
                        .downfall = 0.0,
                        .effects = {
                            .fog_color = 1787717,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .particle = Biome::Particle{
                                .options = {
                                    .type = "minecraft:ash",
                                },
                                .probability = 0.00625,
                            },
                            .ambient_sound = Biome::AmbientSound{
                                .sound = "minecraft:ambient.soul_sand_valley.loop",
                            },
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.soul_sand_valley.mood",
                            },
                            .additions_sound = Biome::AdditionsSound{
                                .sound = "minecraft:ambient.soul_sand_valley.additions",
                                .tick_chance = 0.0111,
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.nether.soul_sand_valley",
                            },
                        },
                    },
                },
                {
                    "minecraft:sparse_jungle",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.95,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7842047,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.sparse_jungle",
                            },
                        },
                    },
                },
                {
                    "minecraft:stony_peaks",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 1.0,
                        .downfall = 0.3,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7776511,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.stony_peaks",
                            },
                        },
                    },
                },
                {
                    "minecraft:stony_shore",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.2,
                        .downfall = 0.3,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8233727,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:sunflower_plains",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.8,
                        .downfall = 0.4,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7907327,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:swamp",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.8,
                        .downfall = 0.9,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 6388580,
                            .water_fog_color = 2302743,
                            .sky_color = 7907327,
                            .foliage_color = 6975545,
                            .grass_color = 6975545,
                            .grass_color_modifier = "swamp",
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.swamp",
                            },
                        },
                    },
                },
                {
                    "minecraft:taiga",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.25,
                        .downfall = 0.8,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8233983,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:the_end",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 10518688,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 0,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:the_void",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:warm_ocean",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.5,
                        .downfall = 0.5,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4445678,
                            .water_fog_color = 270131,
                            .sky_color = 8103167,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:warped_forest",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.0,
                        .downfall = 0.0,
                        .effects = {
                            .fog_color = 1705242,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .particle = Biome::Particle{
                                .options = {
                                    .type = "minecraft:warped_spore",
                                },
                                .probability = 0.01428,
                            },
                            .ambient_sound = "minecraft:ambient.warped_forest.loop",
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.warped_forest.mood",
                            },
                            .additions_sound = Biome::AdditionsSound{
                                .sound = "minecraft:ambient.warped_forest.additions",
                                .tick_chance = 0.0111,
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.nether.warped_forest",
                            },
                        },
                    },
                },
                {
                    "minecraft:windswept_forest",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.2,
                        .downfall = 0.3,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8233727,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:windswept_gravelly_hills",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.2,
                        .downfall = 0.3,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8233727,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:windswept_hills",
                    Biome{
                        .has_precipitation = true,
                        .temperature = 0.2,
                        .downfall = 0.3,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 8233727,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:windswept_savanna",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.0,
                        .downfall = 0.0,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                        },
                    },
                },
                {
                    "minecraft:wooded_badlands",
                    Biome{
                        .has_precipitation = false,
                        .temperature = 2.0,
                        .downfall = 0.0,
                        .effects = {
                            .fog_color = 12638463,
                            .water_color = 4159204,
                            .water_fog_color = 329011,
                            .sky_color = 7254527,
                            .foliage_color = 10387789,
                            .grass_color = 9470285,
                            .mood_sound = Biome::MoodSound{
                                .sound = "minecraft:ambient.cave",
                            },
                            .music = Biome::Music{
                                .sound = "minecraft:music.overworld.badlands",
                            },
                        },
                    },
                },
            };
            chatTypes = {
                {
                    "minecraft:chat",
                    ChatType{
                        .chat = ChatType::Decoration{
                            .translation_key = "chat.type.text",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                        .narration = ChatType::Decoration{
                            .translation_key = "chat.type.text.narrate",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                    },
                },
                {
                    "minecraft:emote_command",
                    ChatType{
                        .chat = ChatType::Decoration{
                            .translation_key = "chat.type.emote",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                        .narration = ChatType::Decoration{
                            .translation_key = "chat.type.emote",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                    },
                },
                {
                    "minecraft:msg_command_incoming",
                    ChatType{
                        .chat = ChatType::Decoration{
                            .translation_key = "commands.message.display.incoming",
                            .style = Chat().SetColor("gray").SetItalic(true),
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                        .narration = ChatType::Decoration{
                            .translation_key = "chat.type.text.narrate",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                    },
                },
                {
                    "minecraft:msg_command_outgoing",
                    ChatType{
                        .chat = ChatType::Decoration{
                            .translation_key = "commands.message.display.outgoing",
                            .style = Chat().SetColor("gray").SetItalic(true),
                            .parameters = std::vector<std::string>{"target", "content"},
                        },
                        .narration = ChatType::Decoration{
                            .translation_key = "chat.type.text.narrate",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                    },
                },
                {
                    "minecraft:say_command",
                    ChatType{
                        .chat = ChatType::Decoration{
                            .translation_key = "chat.type.announcement",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                        .narration = ChatType::Decoration{
                            .translation_key = "chat.type.text.narrate",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                    },
                },
                {
                    "minecraft:team_msg_command_incoming",
                    ChatType{
                        .chat = ChatType::Decoration{
                            .translation_key = "chat.type.team.text",
                            .parameters = std::vector<std::string>{"target", "sender", "content"},
                        },
                        .narration = ChatType::Decoration{
                            .translation_key = "chat.type.text.narrate",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                    },
                },
                {
                    "minecraft:team_msg_command_outgoing",
                    ChatType{
                        .chat = ChatType::Decoration{
                            .translation_key = "chat.type.team.sent",
                            .parameters = std::vector<std::string>{"target", "sender", "content"},
                        },
                        .narration = ChatType::Decoration{
                            .translation_key = "chat.type.text.narrate",
                            .parameters = std::vector<std::string>{"sender", "content"},
                        },
                    },
                },
            };
            armorTrimPatterns = {
                {
                    "minecraft:coast",
                    {
                        .assert_id = "minecraft:coast",
                        .template_item = "minecraft:coast_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.coast"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:dune",
                    {
                        .assert_id = "minecraft:dune",
                        .template_item = "minecraft:dune_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.dune"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:eye",
                    {
                        .assert_id = "minecraft:eye",
                        .template_item = "minecraft:eye_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.eye"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:host",
                    {
                        .assert_id = "minecraft:host",
                        .template_item = "minecraft:host_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.host"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:raiser",
                    {
                        .assert_id = "minecraft:raiser",
                        .template_item = "minecraft:raiser_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.raiser"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:rib",
                    {
                        .assert_id = "minecraft:rib",
                        .template_item = "minecraft:rib_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.rib"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:sentry",
                    {
                        .assert_id = "minecraft:sentry",
                        .template_item = "minecraft:sentry_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.sentry"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:shaper",
                    {
                        .assert_id = "minecraft:shaper",
                        .template_item = "minecraft:shaper_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.shaper"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:silence",
                    {
                        .assert_id = "minecraft:silence",
                        .template_item = "minecraft:silence_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.silence"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:snout",
                    {
                        .assert_id = "minecraft:snout",
                        .template_item = "minecraft:snout_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.snout"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:spire",
                    {
                        .assert_id = "minecraft:spire",
                        .template_item = "minecraft:spire_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.spire"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:tide",
                    {
                        .assert_id = "minecraft:tide",
                        .template_item = "minecraft:tide_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.tide"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:vex",
                    {
                        .assert_id = "minecraft:vex",
                        .template_item = "minecraft:vex_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.vex"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:ward",
                    {
                        .assert_id = "minecraft:ward",
                        .template_item = "minecraft:ward_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.ward"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:wayfinder",
                    {
                        .assert_id = "minecraft:wayfinder",
                        .template_item = "minecraft:wayfinder_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.wayfinder"),
                        .decal = 0,
                    },
                },
                {
                    "minecraft:wild",
                    {
                        .assert_id = "minecraft:wild",
                        .template_item = "minecraft:wild_armor_trim_smithing_template",
                        .description = Chat().SetTranslation("trim_pattern.minecraft.wild"),
                        .decal = 0,
                    },
                },
            };
            armorTrimMaterials = {
                {
                    "minecraft:amethyst",
                    {
                        .asset_name = "amethyst",
                        .ingredient = "minecraft:amethyst_shard",
                        .description = Chat().SetColor("#9A5CC6").SetTranslation("trim_material.minecraft.amethyst"),
                        .item_model_index = 1.0,
                    },
                },
                {
                    "minecraft:copper",
                    {

                        .asset_name = "copper",
                        .ingredient = "minecraft:copper_ingot",
                        .description = Chat().SetColor("#B4684D").SetTranslation("trim_material.minecraft.copper"),
                        .item_model_index = 0.5,
                    },
                },
                {
                    "minecraft:diamond",
                    {

                        .asset_name = "diamond",
                        .ingredient = "minecraft:diamond",
                        .override_armor_materials = {{"minecraft:diamond", "diamond_darker"}},
                        .description = Chat().SetColor("#6EECD2").SetTranslation("trim_material.minecraft.diamond"),
                        .item_model_index = 0.8,
                    },
                },
                std::pair<const std::string, ArmorTrimMaterial>{
                    "minecraft:emerald",
                    {

                        .asset_name = "emerald",
                        .ingredient = "minecraft:emerald",
                        .description = Chat().SetColor("#11A036").SetTranslation("trim_material.minecraft.emerald"),
                        .item_model_index = 0.7,
                    }
                },
                {
                    "minecraft:gold",
                    {

                        .asset_name = "gold",
                        .ingredient = "minecraft:gold_ingot",
                        .override_armor_materials = {{"minecraft:gold", "gold_darker"}},
                        .description = Chat().SetColor("#DEB12D").SetTranslation("trim_material.minecraft.gold"),
                        .item_model_index = 0.6,
                    },
                },
                {
                    "minecraft:iron",
                    {

                        .asset_name = "iron",
                        .ingredient = "minecraft:iron_ingot",
                        .override_armor_materials = {{"minecraft:iron", "iron_darker"}},
                        .description = Chat().SetColor("#ECECEC").SetTranslation("trim_material.minecraft.iron"),
                        .item_model_index = 0.2,
                    },
                },
                {
                    "minecraft:lapis",
                    {

                        .asset_name = "lapis",
                        .ingredient = "minecraft:lapis_lazuli",
                        .description = Chat().SetColor("#416E97").SetTranslation("trim_material.minecraft.lapis"),
                        .item_model_index = 0.9,
                    },
                },
                {
                    "minecraft:netherite",
                    {

                        .asset_name = "netherite",
                        .ingredient = "minecraft:netherite_ingot",
                        .override_armor_materials = {{"minecraft:netherite", "netherite_darker"}},
                        .description = Chat().SetColor("#625859").SetTranslation("trim_material.minecraft.netherite"),
                        .item_model_index = 0.3,
                    },
                },
                {
                    "minecraft:quartz",
                    {

                        .asset_name = "quartz",
                        .ingredient = "minecraft:quartz",
                        .description = Chat().SetColor("#E3D4C4").SetTranslation("trim_material.minecraft.quartz"),
                        .item_model_index = 0.3,
                    },
                },
                {
                    "minecraft:redstone",
                    {

                        .asset_name = "redstone",
                        .ingredient = "minecraft:redstone",
                        .description = Chat().SetColor("#971607").SetTranslation("trim_material.minecraft.redstone"),
                        .item_model_index = 0.4,
                    },
                },
            };
            wolfVariants = {
                {
                    "minecraft:ashen",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf_ashen",
                        .tame_texture = "minecraft:entity/wolf/wolf_ashen_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_ashen_angry",
                        .biomes = {"minecraft:snowy_taiga"},
                    },
                },
                {
                    "minecraft:black",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf_black",
                        .tame_texture = "minecraft:entity/wolf/wolf_black_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_black_angry",
                        .biomes = {"minecraft:old_growth_pine_taiga"},
                    },
                },
                {
                    "minecraft:chestnut",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf_chestnut",
                        .tame_texture = "minecraft:entity/wolf/wolf_chestnut_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_chestnut_angry",
                        .biomes = {"minecraft:old_growth_spruce_taiga"},
                    },
                },
                {
                    "minecraft:pale",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf",
                        .tame_texture = "minecraft:entity/wolf/wolf_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_angry",
                        .biomes = {"minecraft:taiga"},
                    },
                },
                {
                    "minecraft:rusty",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf_rusty",
                        .tame_texture = "minecraft:entity/wolf/wolf_rusty_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_rusty_angry",
                        .biomes = {"minecraft:is_jungle"},
                    },
                },
                {
                    "minecraft:snowy",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf_snowy",
                        .tame_texture = "minecraft:entity/wolf/wolf_snowy_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_snowy_angry",
                        .biomes = {"minecraft:grove"},
                    },
                },
                {
                    "minecraft:spotted",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf_spotted",
                        .tame_texture = "minecraft:entity/wolf/wolf_spotted_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_spotted_angry",
                        .biomes = {"minecraft:is_savanna"},
                    },
                },
                {
                    "minecraft:striped",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf_striped",
                        .tame_texture = "minecraft:entity/wolf/wolf_striped_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_striped_angry",
                        .biomes = {"minecraft:is_badlands"},
                    },
                },
                {
                    "minecraft:woods",
                    {
                        .wild_texture = "minecraft:entity/wolf/wolf_woods",
                        .tame_texture = "minecraft:entity/wolf/wolf_woods_tame",
                        .angry_texture = "minecraft:entity/wolf/wolf_woods_angry",
                        .biomes = {"minecraft:forest"},
                    },
                },
            };
            dimensionTypes = {
                {
                    "minecraft:overworld",
                    {
                        .monster_spawn_light_level = IntegerDistribution{
                            .type = "minecraft:uniform",
                            .value = enbt::compound{
                                {"min_inclusive", 0},
                                {"max_inclusive", 7},
                            },
                        },
                        .infiniburn = "#minecraft:infiniburn_overworld",
                        .effects = "minecraft:overworld",
                        .coordinate_scale = 1,
                        .ambient_light = 0,
                        .min_y = -64,
                        .height = 384,
                        .logical_height = 384,
                        .monster_spawn_block_light_limit = 0,
                        .has_skylight = true,
                        .has_ceiling = false,
                        .ultrawarm = false,
                        .natural = true,
                        .piglin_safe = false,
                        .has_raids = true,
                        .respawn_anchor_works = false,
                        .bed_works = true,
                    },
                },
                {
                    "minecraft:overworld_caves",
                    {
                        .monster_spawn_light_level = IntegerDistribution{
                            .type = "minecraft:uniform",
                            .value = enbt::compound{
                                {"min_inclusive", 0},
                                {"max_inclusive", 7},
                            },
                        },
                        .infiniburn = "#minecraft:infiniburn_overworld",
                        .effects = "minecraft:overworld",
                        .coordinate_scale = 1,
                        .ambient_light = 0,
                        .min_y = -64,
                        .height = 384,
                        .logical_height = 384,
                        .monster_spawn_block_light_limit = 0,
                        .has_skylight = true,
                        .has_ceiling = false,
                        .ultrawarm = false,
                        .natural = true,
                        .piglin_safe = false,
                        .has_raids = true,
                        .respawn_anchor_works = false,
                        .bed_works = true,
                    },
                },
                {
                    "minecraft:the_end",
                    {
                        .monster_spawn_light_level = IntegerDistribution{
                            .type = "minecraft:uniform",
                            .value = enbt::compound{
                                {"min_inclusive", 0},
                                {"max_inclusive", 7},
                            },
                        },
                        .fixed_time = 6000,
                        .infiniburn = "#minecraft:infiniburn_end",
                        .effects = "minecraft:the_end",
                        .coordinate_scale = 1,
                        .ambient_light = 0,
                        .min_y = 0,
                        .height = 256,
                        .logical_height = 256,
                        .monster_spawn_block_light_limit = 0,
                        .has_skylight = false,
                        .has_ceiling = false,
                        .ultrawarm = false,
                        .natural = false,
                        .piglin_safe = false,
                        .has_raids = true,
                        .respawn_anchor_works = false,
                        .bed_works = false,
                    },
                },
                {
                    "minecraft:the_nether",
                    {
                        .monster_spawn_light_level = IntegerDistribution{
                            .type = "minecraft:uniform",
                            .value = enbt::compound{
                                {"min_inclusive", 0},
                                {"max_inclusive", 7},
                            },
                        },
                        .fixed_time = 18000,
                        .infiniburn = "#minecraft:infiniburn_nether",
                        .effects = "minecraft:the_nether",
                        .coordinate_scale = 8,
                        .ambient_light = 0.1,
                        .min_y = 0,
                        .height = 256,
                        .logical_height = 128,
                        .monster_spawn_block_light_limit = 7,
                        .has_skylight = false,
                        .has_ceiling = true,
                        .ultrawarm = true,
                        .natural = false,
                        .piglin_safe = true,
                        .has_raids = false,
                        .respawn_anchor_works = true,
                        .bed_works = false,
                    },
                },
            };
            damageTypes = {
                {
                    "minecraft:arrow",
                    DamageType{
                        .message_id = "arrow",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:bad_respawn_point",
                    DamageType{
                        .message_id = "badRespawnPoint",
                        .scaling = DamageType::ScalingType::always,
                        .death_message_type = DamageType::DeathMessageType::intentional_game_design,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:cactus",
                    DamageType{
                        .message_id = "cactus",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:cramming",
                    DamageType{
                        .message_id = "cramming",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:dragon_breath",
                    DamageType{
                        .message_id = "dragonBreath",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:drown",
                    DamageType{
                        .message_id = "drown",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::drowning,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:dry_out",
                    DamageType{
                        .message_id = "dryout",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:explosion",
                    DamageType{
                        .message_id = "explosion",
                        .scaling = DamageType::ScalingType::always,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:fall",
                    DamageType{
                        .message_id = "fall",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .death_message_type = DamageType::DeathMessageType::fall_variants,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:fall",
                    DamageType{
                        .message_id = "fall",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .death_message_type = DamageType::DeathMessageType::fall_variants,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:falling_anvil",
                    DamageType{
                        .message_id = "anvil",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:falling_block",
                    DamageType{
                        .message_id = "fallingBlock",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:falling_stalactite",
                    DamageType{
                        .message_id = "fallingStalactite",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:fireball",
                    DamageType{
                        .message_id = "fireball",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:fireworks",
                    DamageType{
                        .message_id = "fireworks",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:fly_into_wall",
                    DamageType{
                        .message_id = "flyIntoWall",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:freeze",
                    DamageType{
                        .message_id = "freeze",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::freezing,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:generic",
                    DamageType{
                        .message_id = "generic",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:generic_kill",
                    DamageType{
                        .message_id = "genericKill",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:hot_floor",
                    DamageType{
                        .message_id = "hotFloor",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:in_fire",
                    DamageType{
                        .message_id = "inFire",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:in_wall",
                    DamageType{
                        .message_id = "inWall",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:indirect_magic",
                    DamageType{
                        .message_id = "indirectMagic",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:lava",
                    DamageType{
                        .message_id = "lava",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:lightning_bolt",
                    DamageType{
                        .message_id = "lightningBolt",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:magic",
                    DamageType{
                        .message_id = "magic",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:mob_attack",
                    DamageType{
                        .message_id = "mob",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:mob_attack_no_aggro",
                    DamageType{
                        .message_id = "mob",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:mob_projectile",
                    DamageType{
                        .message_id = "mob",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:on_fire",
                    DamageType{
                        .message_id = "onFire",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:out_of_world",
                    DamageType{
                        .message_id = "outOfWorld",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:outside_border",
                    DamageType{
                        .message_id = "outsideBorder",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:player_attack",
                    DamageType{
                        .message_id = "player",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:player_explosion",
                    DamageType{
                        .message_id = "explosion.player",
                        .scaling = DamageType::ScalingType::always,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:sonic_boom",
                    DamageType{
                        .message_id = "sonic_boom",
                        .scaling = DamageType::ScalingType::always,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:spit",
                    DamageType{
                        .message_id = "mob",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:stalagmite",
                    DamageType{
                        .message_id = "stalagmite",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:starve",
                    DamageType{
                        .message_id = "starve",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:sting",
                    DamageType{
                        .message_id = "sting",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:sweet_berry_bush",
                    DamageType{
                        .message_id = "sweetBerryBush",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::poking,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:thorns",
                    DamageType{
                        .message_id = "thorns",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::thorns,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:thrown",
                    DamageType{
                        .message_id = "thrown",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:trident",
                    DamageType{
                        .message_id = "trident",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:unattributed_fireball",
                    DamageType{
                        .message_id = "onFire",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:wither",
                    DamageType{
                        .message_id = "wither",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:wither_skull",
                    DamageType{
                        .message_id = "witherSkull",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
            };
            bannerPatterns = {
                {
                    "minecraft:base",
                    BannerPattern{
                        .asset_id = "minecraft:base",
                        .translation_key = "block.minecraft.banner.base",
                    },
                },
                {
                    "minecraft:border",
                    BannerPattern{
                        .asset_id = "minecraft:border",
                        .translation_key = "block.minecraft.banner.border",
                    },
                },
                {
                    "minecraft:bricks",
                    BannerPattern{
                        .asset_id = "minecraft:bricks",
                        .translation_key = "block.minecraft.banner.bricks",
                    },
                },
                {
                    "minecraft:circle",
                    BannerPattern{
                        .asset_id = "minecraft:circle",
                        .translation_key = "block.minecraft.banner.circle",
                    },
                },
                {
                    "minecraft:creeper",
                    BannerPattern{
                        .asset_id = "minecraft:creeper",
                        .translation_key = "block.minecraft.banner.creeper",
                    },
                },
                {
                    "minecraft:cross",
                    BannerPattern{
                        .asset_id = "minecraft:cross",
                        .translation_key = "block.minecraft.banner.cross",
                    },
                },
                {
                    "minecraft:curly_border",
                    BannerPattern{
                        .asset_id = "minecraft:curly_border",
                        .translation_key = "block.minecraft.banner.curly_border",
                    },
                },
                {
                    "minecraft:diagonal_left",
                    BannerPattern{
                        .asset_id = "minecraft:diagonal_left",
                        .translation_key = "block.minecraft.banner.diagonal_left",
                    },
                },
                {
                    "minecraft:diagonal_right",
                    BannerPattern{
                        .asset_id = "minecraft:diagonal_right",
                        .translation_key = "block.minecraft.banner.diagonal_right",
                    },
                },
                {
                    "minecraft:diagonal_up_left",
                    BannerPattern{
                        .asset_id = "minecraft:diagonal_up_left",
                        .translation_key = "block.minecraft.banner.diagonal_up_left",
                    },
                },
                {
                    "minecraft:diagonal_up_right",
                    BannerPattern{
                        .asset_id = "minecraft:diagonal_up_right",
                        .translation_key = "block.minecraft.banner.diagonal_up_right",
                    },
                },
                {
                    "minecraft:flower",
                    BannerPattern{
                        .asset_id = "minecraft:flower",
                        .translation_key = "block.minecraft.banner.flower",
                    },
                },
                {
                    "minecraft:globe",
                    BannerPattern{
                        .asset_id = "minecraft:globe",
                        .translation_key = "block.minecraft.banner.globe",
                    },
                },
                {
                    "minecraft:gradient",
                    BannerPattern{
                        .asset_id = "minecraft:gradient",
                        .translation_key = "block.minecraft.banner.gradient",
                    },
                },
                {
                    "minecraft:gradient_up",
                    BannerPattern{
                        .asset_id = "minecraft:gradient_up",
                        .translation_key = "block.minecraft.banner.gradient_up",
                    },
                },
                {
                    "minecraft:half_horizontal",
                    BannerPattern{
                        .asset_id = "minecraft:half_horizontal",
                        .translation_key = "block.minecraft.banner.half_horizontal",
                    },
                },
                {
                    "minecraft:half_horizontal_bottom",
                    BannerPattern{
                        .asset_id = "minecraft:half_horizontal_bottom",
                        .translation_key = "block.minecraft.banner.half_horizontal_bottom",
                    },
                },
                {
                    "minecraft:half_vertical",
                    BannerPattern{
                        .asset_id = "minecraft:half_vertical",
                        .translation_key = "block.minecraft.banner.half_vertical",
                    },
                },
                {
                    "minecraft:half_vertical_right",
                    BannerPattern{
                        .asset_id = "minecraft:half_vertical_right",
                        .translation_key = "block.minecraft.banner.half_vertical_right",
                    },
                },
                {
                    "minecraft:mojang",
                    BannerPattern{
                        .asset_id = "minecraft:mojang",
                        .translation_key = "block.minecraft.banner.mojang",
                    },
                },
                {
                    "minecraft:piglin",
                    BannerPattern{
                        .asset_id = "minecraft:piglin",
                        .translation_key = "block.minecraft.banner.piglin",
                    },
                },
                {
                    "minecraft:rhombus",
                    BannerPattern{
                        .asset_id = "minecraft:rhombus",
                        .translation_key = "block.minecraft.banner.rhombus",
                    },
                },
                {
                    "minecraft:skull",
                    BannerPattern{
                        .asset_id = "minecraft:skull",
                        .translation_key = "block.minecraft.banner.skull",
                    },
                },
                {
                    "minecraft:small_stripes",
                    BannerPattern{
                        .asset_id = "minecraft:small_stripes",
                        .translation_key = "block.minecraft.banner.small_stripes",
                    },
                },
                {
                    "minecraft:square_bottom_left",
                    BannerPattern{
                        .asset_id = "minecraft:square_bottom_left",
                        .translation_key = "block.minecraft.banner.square_bottom_left",
                    },
                },
                {
                    "minecraft:square_bottom_right",
                    BannerPattern{
                        .asset_id = "minecraft:square_bottom_right",
                        .translation_key = "block.minecraft.banner.square_bottom_right",
                    },
                },
                {
                    "minecraft:square_top_left",
                    BannerPattern{
                        .asset_id = "minecraft:square_top_left",
                        .translation_key = "block.minecraft.banner.square_top_left",
                    },
                },
                {
                    "minecraft:square_top_right",
                    BannerPattern{
                        .asset_id = "minecraft:square_top_right",
                        .translation_key = "block.minecraft.banner.square_top_right",
                    },
                },
                {
                    "minecraft:straight_cross",
                    BannerPattern{
                        .asset_id = "minecraft:straight_cross",
                        .translation_key = "block.minecraft.banner.straight_cross",
                    },
                },
                {
                    "minecraft:stripe_bottom",
                    BannerPattern{
                        .asset_id = "minecraft:stripe_bottom",
                        .translation_key = "block.minecraft.banner.stripe_bottom",
                    },
                },
                {
                    "minecraft:stripe_center",
                    BannerPattern{
                        .asset_id = "minecraft:stripe_center",
                        .translation_key = "block.minecraft.banner.stripe_center",
                    },
                },
                {
                    "minecraft:stripe_downleft",
                    BannerPattern{
                        .asset_id = "minecraft:stripe_downleft",
                        .translation_key = "block.minecraft.banner.stripe_downleft",
                    },
                },
                {
                    "minecraft:stripe_downright",
                    BannerPattern{
                        .asset_id = "minecraft:stripe_downright",
                        .translation_key = "block.minecraft.banner.stripe_downright",
                    },
                },
                {
                    "minecraft:stripe_left",
                    BannerPattern{
                        .asset_id = "minecraft:stripe_left",
                        .translation_key = "block.minecraft.banner.stripe_left",
                    },
                },
                {
                    "minecraft:stripe_middle",
                    BannerPattern{
                        .asset_id = "minecraft:stripe_middle",
                        .translation_key = "block.minecraft.banner.stripe_middle",
                    },
                },
                {
                    "minecraft:stripe_right",
                    BannerPattern{
                        .asset_id = "minecraft:stripe_right",
                        .translation_key = "block.minecraft.banner.stripe_right",
                    },
                },
                {
                    "minecraft:stripe_top",
                    BannerPattern{
                        .asset_id = "minecraft:stripe_top",
                        .translation_key = "block.minecraft.banner.stripe_top",
                    },
                },
                {
                    "minecraft:triangle_bottom",
                    BannerPattern{
                        .asset_id = "minecraft:triangle_bottom",
                        .translation_key = "block.minecraft.banner.triangle_bottom",
                    },
                },
                {
                    "minecraft:triangle_top",
                    BannerPattern{
                        .asset_id = "minecraft:triangle_top",
                        .translation_key = "block.minecraft.banner.triangle_top",
                    },
                },
                {
                    "minecraft:triangles_bottom",
                    BannerPattern{
                        .asset_id = "minecraft:triangles_bottom",
                        .translation_key = "block.minecraft.banner.triangles_bottom",
                    },
                },
                {
                    "minecraft:triangles_top",
                    BannerPattern{
                        .asset_id = "minecraft:triangles_top",
                        .translation_key = "block.minecraft.banner.triangles_top",
                    },
                },
            };
            paintingVariants = {
                {
                    "minecraft:alban",
                    PaintingVariant{
                        .asset_id = "minecraft:alban",
                        .height = 1,
                        .width = 1,
                    },
                },
                {
                    "minecraft:aztec",
                    PaintingVariant{
                        .asset_id = "minecraft:aztec",
                        .height = 1,
                        .width = 1,
                    },
                },
                {
                    "minecraft:aztec2",
                    PaintingVariant{
                        .asset_id = "minecraft:aztec2",
                        .height = 1,
                        .width = 1,
                    },
                },
                {
                    "minecraft:backyard",
                    PaintingVariant{
                        .asset_id = "minecraft:backyard",
                        .height = 4,
                        .width = 3,
                    },
                },
                {
                    "minecraft:baroque",
                    PaintingVariant{
                        .asset_id = "minecraft:baroque",
                        .height = 2,
                        .width = 2,
                    },
                },
                {
                    "minecraft:bomb",
                    PaintingVariant{
                        .asset_id = "minecraft:bomb",
                        .height = 1,
                        .width = 1,
                    },
                },
                {
                    "minecraft:bouquet",
                    PaintingVariant{
                        .asset_id = "minecraft:bouquet",
                        .height = 3,
                        .width = 3,
                    },
                },
                {
                    "minecraft:burning_skull",
                    PaintingVariant{
                        .asset_id = "minecraft:burning_skull",
                        .height = 4,
                        .width = 4,
                    },
                },
                {
                    "minecraft:bust",
                    PaintingVariant{
                        .asset_id = "minecraft:bust",
                        .height = 2,
                        .width = 2,
                    },
                },
                {
                    "minecraft:cavebird",
                    PaintingVariant{
                        .asset_id = "minecraft:cavebird",
                        .height = 3,
                        .width = 3,
                    },
                },
                {
                    "minecraft:changing",
                    PaintingVariant{
                        .asset_id = "minecraft:changing",
                        .height = 2,
                        .width = 4,
                    },
                },
                {
                    "minecraft:cotan",
                    PaintingVariant{
                        .asset_id = "minecraft:cotan",
                        .height = 3,
                        .width = 3,
                    },
                },
                {
                    "minecraft:courbet",
                    PaintingVariant{
                        .asset_id = "minecraft:courbet",
                        .height = 1,
                        .width = 2,
                    },
                },
                {
                    "minecraft:creebet",
                    PaintingVariant{
                        .asset_id = "minecraft:creebet",
                        .height = 1,
                        .width = 2,
                    },
                },
                {
                    "minecraft:donkey_kong",
                    PaintingVariant{
                        .asset_id = "minecraft:donkey_kong",
                        .height = 3,
                        .width = 4,
                    },
                },
                {
                    "minecraft:earth",
                    PaintingVariant{
                        .asset_id = "minecraft:earth",
                        .height = 2,
                        .width = 2,
                    },
                },
                {
                    "minecraft:endboss",
                    PaintingVariant{
                        .asset_id = "minecraft:endboss",
                        .height = 3,
                        .width = 3,
                    },
                },
                {
                    "minecraft:fern",
                    PaintingVariant{
                        .asset_id = "minecraft:fern",
                        .height = 3,
                        .width = 3,
                    },
                },
                {
                    "minecraft:fighters",
                    PaintingVariant{
                        .asset_id = "minecraft:fighters",
                        .height = 2,
                        .width = 4,
                    },
                },
                {
                    "minecraft:finding",
                    PaintingVariant{
                        .asset_id = "minecraft:finding",
                        .height = 2,
                        .width = 4,
                    },
                },
                {
                    "minecraft:fire",
                    PaintingVariant{
                        .asset_id = "minecraft:fire",
                        .height = 2,
                        .width = 2,
                    },
                },
                {
                    "minecraft:graham",
                    PaintingVariant{
                        .asset_id = "minecraft:graham",
                        .height = 2,
                        .width = 1,
                    },
                },
                {
                    "minecraft:humble",
                    PaintingVariant{
                        .asset_id = "minecraft:humble",
                        .height = 2,
                        .width = 2,
                    },
                },
                {
                    "minecraft:kebab",
                    PaintingVariant{
                        .asset_id = "minecraft:kebab",
                        .height = 1,
                        .width = 1,
                    },
                },
                {
                    "minecraft:lowmist",
                    PaintingVariant{
                        .asset_id = "minecraft:lowmist",
                        .height = 2,
                        .width = 4,
                    },
                },
                {
                    "minecraft:match",
                    PaintingVariant{
                        .asset_id = "minecraft:match",
                        .height = 2,
                        .width = 2,
                    },
                },
                {
                    "minecraft:meditative",
                    PaintingVariant{
                        .asset_id = "minecraft:meditative",
                        .height = 1,
                        .width = 1,
                    },
                },
                {
                    "minecraft:orb",
                    PaintingVariant{
                        .asset_id = "minecraft:orb",
                        .height = 4,
                        .width = 4,
                    },
                },
                {
                    "minecraft:owlemons",
                    PaintingVariant{
                        .asset_id = "minecraft:owlemons",
                        .height = 3,
                        .width = 3,
                    },
                },
                {
                    "minecraft:passage",
                    PaintingVariant{
                        .asset_id = "minecraft:passage",
                        .height = 2,
                        .width = 4,
                    },
                },
                {
                    "minecraft:pigscene",
                    PaintingVariant{
                        .asset_id = "minecraft:pigscene",
                        .height = 4,
                        .width = 4,
                    },
                },
                {
                    "minecraft:plant",
                    PaintingVariant{
                        .asset_id = "minecraft:plant",
                        .height = 1,
                        .width = 1,
                    },
                },
                {
                    "minecraft:pointer",
                    PaintingVariant{
                        .asset_id = "minecraft:pointer",
                        .height = 4,
                        .width = 4,
                    },
                },
                {
                    "minecraft:pond",
                    PaintingVariant{
                        .asset_id = "minecraft:pond",
                        .height = 4,
                        .width = 3,
                    },
                },
                {
                    "minecraft:pool",
                    PaintingVariant{
                        .asset_id = "minecraft:pool",
                        .height = 1,
                        .width = 2,
                    },
                },
                {
                    "minecraft:prairie_ride",
                    PaintingVariant{
                        .asset_id = "minecraft:prairie_ride",
                        .height = 2,
                        .width = 1,
                    },
                },
                {
                    "minecraft:sea",
                    PaintingVariant{
                        .asset_id = "minecraft:sea",
                        .height = 1,
                        .width = 2,
                    },
                },
                {
                    "minecraft:skeleton",
                    PaintingVariant{
                        .asset_id = "minecraft:skeleton",
                        .height = 3,
                        .width = 4,
                    },
                },
                {
                    "minecraft:skull_and_roses",
                    PaintingVariant{
                        .asset_id = "minecraft:skull_and_roses",
                        .height = 2,
                        .width = 2,
                    },
                },
                {
                    "minecraft:stage",
                    PaintingVariant{
                        .asset_id = "minecraft:stage",
                        .height = 2,
                        .width = 2,
                    },
                },
                {
                    "minecraft:sunflowers",
                    PaintingVariant{
                        .asset_id = "minecraft:sunflowers",
                        .height = 3,
                        .width = 3,
                    },
                },
                {
                    "minecraft:sunset",
                    PaintingVariant{
                        .asset_id = "minecraft:sunset",
                        .height = 1,
                        .width = 2,
                    },
                },
                {
                    "minecraft:wither",
                    PaintingVariant{
                        .asset_id = "minecraft:wither",
                        .height = 2,
                        .width = 2,
                    },
                },
            };
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
                    particle.options.type = (std::string)particle_js["type"];
                    particle.options.options = conversions::json::from_json(particle_js["options"].get());
                    effects.particle = std::move(particle);
                }
                if (effects_js.contains("ambient_sound")) {
                    auto eff = effects_js["ambient_sound"];
                    if (eff.get().is_string())
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
            decoration.style = Chat::fromEnbt(conversions::json::from_json(chat_js["style"].get()));
            decoration.translation_key = (std::string)chat_js["translation_key"];
            {
                auto params = chat_js["parameters"];
                if (params.get().is_array()) {
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
            check_override(advancements, id, "advancements");
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
            check_override(advancements, id, "advancement");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_advancements(js_object::get_object(res.value()), id);
        }

        void load_file_jukebox_song(js_object&& song_js, const std::string& id) {
            check_override(jukebox_songs, id, "jukebox song");
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
            check_override(jukebox_songs, id, "jukebox songs");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_jukebox_song(js_object::get_object(res.value()), id);
        }

        void load_file_loot_table(js_object&& loot_table_js, const std::string& id) {
            check_override(loot_table, id, "loot table");

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
                    if (pool.contains("conditions"))
                        pool_.conditions = util::conversions::json::from_json(pool["conditions"].get());
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
            pattern.assert_id = (std::string)pattern_js["assert_id"];
            pattern.decal = pattern_js["decal"];
            pattern.template_item = (std::string)pattern_js["template_item"];
            {
                auto desc = pattern_js["description"];
                if (desc.get().is_string())
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
            material.asset_name = (std::string)material_js["texture"];
            material.ingredient = (std::string)material_js["ingredient"];
            material.item_model_index = material_js["item_model_index"];
            {
                auto desc = material_js["description"];
                if (desc.get().is_string())
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
            js_object variant_js = js_object::get_object(res.value());
            WolfVariant variant;
            variant.wild_texture = (std::string)variant_js["wild_texture"];
            variant.tame_texture = (std::string)variant_js["tame_texture"];
            variant.angry_texture = (std::string)variant_js["angry_texture"];
            auto biomes = js_array::get_array(variant_js["biomes"]);
            variant.biomes.reserve(biomes.size());
            for (auto&& biome : biomes)
                variant.biomes.push_back(biome);
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
                if (monster_spawn_light_level.get().is_number())
                    type.monster_spawn_light_level = monster_spawn_light_level;
                else {
                    js_object monster_spawn_light_level_js = js_object::get_object(monster_spawn_light_level);
                    IntegerDistribution monster_spawn_light_level_;
                    monster_spawn_light_level_.value = conversions::json::from_json(monster_spawn_light_level_js["value"].get());
                    monster_spawn_light_level_.type = (std::string)monster_spawn_light_level_js["type"];
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
            check_override(enchantments, id, "enchantments");
            enchantment type;
            type.description = Chat::fromEnbt(util::conversions::json::from_json(type_js.at("description").get()));
            type.max_level = type_js.at("max_level");
            type.weight = type_js.at("weight");
            type.anvil_cost = type_js.at("anvil_cost");
            auto slots = js_array::get_array(type_js.at("slots"));
            type.slots.reserve(slots.size());
            for (auto&& slot : slots)
                type.slots.push_back(slot);
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

            auto effects = js_object::get_object(type_js.at("effects"));
            type.effects.reserve(effects.size());
            for (auto&& [component_id, effect] : effects) {
                auto list_of = js_array::get_array(effect);
                std::vector<enbt::compound> effects;
                effects.reserve(list_of.size());
                for (auto&& effect : list_of) {
                    enbt::compound comp;
                    comp = util::conversions::json::from_json(effect.get());
                    effects.push_back(comp);
                }
                type.effects[component_id] = std::move(effects);
            }
            enchantments[id] = std::move(type);
        }

        void load_file_enchantment(const std::filesystem::path& file_path, const std::string& id) {
            check_override(enchantments, id, "enchantments");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_enchantment(js_object::get_object(res.value()), id);
        }

        void load_file_enchantment_provider(boost::json::object& type_js, const std::string& id) {
            check_override(enchantment_providers, id, "enchantment providers");
            enchantment_providers[id] = util::conversions::json::from_json(type_js);
        }

        void load_file_enchantment_provider(const std::filesystem::path& file_path, const std::string& id) {
            check_override(enchantment_providers, id, "enchantment providers");
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

        void apply_tags(js_value val, const std::string& type, const std::string& namespace_, const std::string& path_) {
            list_array<std::string> result;
            for (auto&& tag : js_array::get_array(val)) {
                std::string the_tag;
                if (tag.get().is_string())
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
                apply_tags(tags_["values"], type, namespace_, path_);
            } else if (tags_.contains("values"))
                apply_tags(tags_["values"], type, namespace_, path_);
            else if (tags_.contains("root"))
                apply_tags(tags_["root"], type, namespace_, path_);
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
            auto parsed = boost::json::parse(std::string_view((const char*)resources__blocks.data(), resources__blocks.size()));

            base_objects::block::access_full_block_data(std::function(
                [&](
                    std::vector<std::shared_ptr<base_objects::static_block_data>>& full_block_data_,
                    std::unordered_map<base_objects::shared_string, std::shared_ptr<base_objects::static_block_data>>& named_full_block_data
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

                        named_full_block_data[(std::string_view)name] = std::move(data);
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
            auto parsed = boost::json::parse(std::string_view((const char*)resources__items.data(), resources__items.size()));
            for (auto&& [name, decl] : parsed.as_object()) {
                base_objects::static_slot_data slot_data;
                slot_data.id = name;
                std::unordered_map<std::string, base_objects::slot_component::unified> components;
                for (auto& [name, value] : decl.as_object().at("components").as_object())
                    components[name] = parse_component(name, value);
                slot_data.default_components = std::move(components);
                base_objects::slot_data::add_slot_data(std::move(slot_data));
            }
        }

        void prepare_versions() {
            auto parsed = boost::json::parse(std::string_view((const char*)resources__versions_765.data(), resources__versions_765.size()));
            registers::individual_registers[765] = util::conversions::json::from_json(parsed);
            parsed = boost::json::parse(std::string_view((const char*)resources__versions_766.data(), resources__versions_766.size()));
            registers::individual_registers[766] = util::conversions::json::from_json(parsed);
            parsed = boost::json::parse(std::string_view((const char*)resources__versions_767.data(), resources__versions_767.size()));
            registers::individual_registers[767] = util::conversions::json::from_json(parsed);
        }
    }
}
