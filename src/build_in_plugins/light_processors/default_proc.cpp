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

        void process_sub_chunk(storage::world_data& world, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z) override {}

        void block_changed(storage::world_data& world, int64_t global_x, int64_t global_y, int64_t global_z) override {}
    };

    struct default_processor : public PluginAutoRegister<"light_processors/default_processor", default_processor> {
        void OnRegister(const PluginRegistrationPtr& self) {
            storage::chunk_light_processor::register_it("default", new default_light_processor());
        }
    };
}