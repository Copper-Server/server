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

namespace copper_server::build_in_plugins::light_processors {
    struct default_light_processor : public storage::chunk_light_processor {
        void process_chunk(storage::world_data& world, int64_t chunk_x, int64_t chunk_z) override {
            world.get_chunk(chunk_x, chunk_z, [&](storage::chunk_data& chunk) {
                chunk.for_each_sub_chunk([&](base_objects::world::sub_chunk_data& sub_chunk) {
                    for (int_fast8_t i = 0; i < 16; ++i)
                        for (int_fast8_t j = 0; j < 16; ++j)
                            for (int_fast8_t k = 0; k < 16; ++k) {
                                sub_chunk.sky_light.light_map[i][j][k].light_point = 15;
                                sub_chunk.block_light.light_map[i][j][k].light_point = 15;
                            }
                });
            });
            world.notify_chunk_light(chunk_x, chunk_z);
        }

        void process_sub_chunk(storage::world_data&, int64_t, int64_t, int64_t) override {}

        void block_changed(storage::world_data&, int64_t, int64_t, int64_t) override {}
    };

    struct default_processor : public PluginAutoRegister<"light_processors/default_processor", default_processor> {
        void OnRegister(const PluginRegistrationPtr& _) {
            storage::chunk_light_processor::register_it("default", new default_light_processor());
        }
    };
}