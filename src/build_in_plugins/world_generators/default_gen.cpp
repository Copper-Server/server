/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/collection/block.hpp>
#include <src/plugin/main.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::build_in_plugins::world_generators {
    struct default_generator : public storage::chunk_generator {
        default_generator() {
            config = {{0, preset_mode::parallel}};
        }

        void process_chunk([[maybe_unused]] storage::world_data& world, storage::chunk_data& chunk, uint8_t preset_stage) override {
            auto& bottom = chunk.sub_chunks.front();

            for (uint8_t x = 0; x < 16; x++) 
                for (uint8_t y = 0; y < 16; y++)
                    for (uint8_t z = 0; z < 16; z++)
                        if (y == 0)
                            bottom.set_block_gen(x, y, z, api::collection::block::stone);

            process_complete(world, chunk);
        }
    };

    struct default_gen : public plugin_auto_register<"world_generators/default", default_gen> {
        void on_register(const plugin_registration_ptr&) override {
            storage::chunk_generator::register_it("default", std::make_shared<default_generator>());
        }
    };
}
