#include "../registers.hpp"
#include "../base_objects/entity.hpp"

namespace crafted_craft {
    namespace resources {
        template <class T>
        void id_assigner(T& map) {
            size_t i = 0;
            for (auto& [name, it] : map)
                it.id = i++;
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
                    {

                        .message_id = "badRespawnPoint",
                        .scaling = DamageType::ScalingType::always,
                        .death_message_type = DamageType::DeathMessageType::intentional_game_design,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:cactus",
                    {

                        .message_id = "cactus",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:cramming",
                    {

                        .message_id = "cramming",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:dragon_breath",
                    {

                        .message_id = "dragonBreath",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:drown",
                    {

                        .message_id = "drown",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::drowning,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:dry_out",
                    {

                        .message_id = "dryout",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:explosion",
                    {

                        .message_id = "explosion",
                        .scaling = DamageType::ScalingType::always,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:fall",
                    {

                        .message_id = "fall",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .death_message_type = DamageType::DeathMessageType::fall_variants,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:fall",
                    {

                        .message_id = "fall",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .death_message_type = DamageType::DeathMessageType::fall_variants,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:falling_anvil",
                    {

                        .message_id = "anvil",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:falling_block",
                    {

                        .message_id = "fallingBlock",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:falling_stalactite",
                    {

                        .message_id = "fallingStalactite",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:fireball",
                    {

                        .message_id = "fireball",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:fireworks",
                    {

                        .message_id = "fireworks",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:fly_into_wall",
                    {

                        .message_id = "flyIntoWall",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:freeze",
                    {

                        .message_id = "freeze",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::freezing,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:generic",
                    {

                        .message_id = "generic",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:generic_kill",
                    {

                        .message_id = "genericKill",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:hot_floor",
                    {

                        .message_id = "hotFloor",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:in_fire",
                    {

                        .message_id = "inFire",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:in_wall",
                    {

                        .message_id = "inWall",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:indirect_magic",
                    {

                        .message_id = "indirectMagic",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:lava",
                    {

                        .message_id = "lava",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:lightning_bolt",
                    {

                        .message_id = "lightningBolt",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:magic",
                    {

                        .message_id = "magic",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:mob_attack",
                    {

                        .message_id = "mob",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:mob_attack_no_aggro",
                    {

                        .message_id = "mob",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:mob_projectile",
                    {

                        .message_id = "mob",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:on_fire",
                    {

                        .message_id = "onFire",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:out_of_world",
                    {

                        .message_id = "outOfWorld",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:outside_border",
                    {

                        .message_id = "outsideBorder",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:player_attack",
                    {

                        .message_id = "player",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:player_explosion",
                    {

                        .message_id = "explosion.player",
                        .scaling = DamageType::ScalingType::always,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:sonic_boom",
                    {

                        .message_id = "sonic_boom",
                        .scaling = DamageType::ScalingType::always,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:spit",
                    {

                        .message_id = "mob",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:stalagmite",
                    {

                        .message_id = "stalagmite",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:starve",
                    {

                        .message_id = "starve",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0,
                    },
                },
                {
                    "minecraft:sting",
                    {

                        .message_id = "sting",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:sweet_berry_bush",
                    {

                        .message_id = "sweetBerryBush",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::poking,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:thorns",
                    {

                        .message_id = "thorns",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::thorns,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:thrown",
                    {

                        .message_id = "thrown",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:trident",
                    {

                        .message_id = "trident",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:unattributed_fireball",
                    {

                        .message_id = "onFire",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .effects = DamageType::EffectsType::burning,
                        .exhaustion = 0.1,
                    },
                },
                {
                    "minecraft:wither",
                    {

                        .message_id = "wither",
                        .scaling = DamageType::ScalingType::when_caused_by_living_non_player,
                        .exhaustion = 0.0,
                    },
                },
                {
                    "minecraft:wither_skull",
                    {

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
            id_assigner(biomes);
            id_assigner(chatTypes);
            id_assigner(armorTrimPatterns);
            id_assigner(armorTrimMaterials);
            id_assigner(wolfVariants);
            id_assigner(dimensionTypes);
            id_assigner(damageTypes);
            id_assigner(bannerPatterns);
            id_assigner(paintingVariants);
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
        }
    }
}