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
    struct default_generator : public storage::chunk_generator {
        default_generator() {
            config = {{0, preset_mode::parallel}};
        }

        void process_chunk([[maybe_unused]] storage::world_data& world, storage::chunk_data& chunk, uint8_t preset_stage) override {
            auto& bottom = chunk.sub_chunks.front();
            auto& blocks = bottom.blocks;
            auto stone = base_objects::block::make_block("minecraft:stone");

            for (uint8_t x = 0; x < 16; x++) 
                for (uint8_t y = 0; y < 16; y++)
                    for (uint8_t z = 0; z < 16; z++)
                        if (y == 0)
                            blocks[x][y][z] = stone;

            process_complete(world, chunk);
        }
    };

    struct default_gen : public PluginAutoRegister<"world_generators/default", default_gen> {
        void OnRegister(const PluginRegistrationPtr&) override {
            storage::chunk_generator::register_it("default", new default_generator());
        }
    };
}
