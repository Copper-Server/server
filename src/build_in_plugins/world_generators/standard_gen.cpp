/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/plugin/main.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::build_in_plugins::world_generators {
    struct standard_generator : public storage::chunk_generator {
        standard_generator() {
            config = {
                {0, preset_mode::parallel},  //empty
                {1, preset_mode::parallel},  //structures_starts
                {2, preset_mode::sync},      //structures_references
                {3, preset_mode::parallel},  //biomes
                {4, preset_mode::parallel},  //noise
                {5, preset_mode::parallel},  //surface
                {6, preset_mode::sync},      //carvers
                {7, preset_mode::parallel},  //features
                {8, preset_mode::parallel},  //initialize_light
                {9, preset_mode::sync},      //light
                {10, preset_mode::parallel}, //spawn + full(optimized)
                //{11, preset_mode::parallel}, //full
            };
        }

        void process_chunk([[maybe_unused]] storage::world_data& world, storage::chunk_data& chunk, uint8_t preset_stage) override {
            switch (preset_stage) {
            case 0:
                chunk.generator_stage = 1;
            case 1:
                chunk.generator_stage = 2;
                break;
            case 2:
                chunk.generator_stage = 3;
                break;
            case 3:
                chunk.generator_stage = 4;
                break;
            case 4:
                chunk.generator_stage = 5;
                break;
            case 5:
                chunk.generator_stage = 6;
                break;
            case 6:
                chunk.generator_stage = 7;
                break;
            case 7:
                chunk.generator_stage = 8;
                break;
            case 8:
                chunk.generator_stage = 9;
                break;
            case 9:
                chunk.generator_stage = 10;
                break;
            case 10:
                chunk.generator_stage = 0xff;
                break;

            default:
                break;
            }
        }

        enbt::compound generate_chunk(storage::world_data&, int64_t, int64_t) override {
            return {
                {"generator_stage", 0}
            };
        }

        enbt::compound generate_sub_chunk(storage::world_data&, int64_t, int64_t, int64_t) override {
            return {};
        }
    };

    struct standard_gen : public PluginAutoRegister<"world_generators/standard", standard_gen> {
        void OnRegister(const PluginRegistrationPtr&) override {
            storage::chunk_generator::register_it("standard", new standard_generator());
        }
    };
}