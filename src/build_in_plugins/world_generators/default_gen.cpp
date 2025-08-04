#include <src/plugin/main.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::build_in_plugins::world_generators {
    struct default_generator : public storage::chunk_generator {
        enbt::compound generate_chunk(storage::world_data& world, int64_t chunk_x, int64_t chunk_z) override {
            enbt::fixed_array subchunks;
            for (size_t i = 0; i < world.get_chunk_y_count(); i++)
                subchunks.push_back(generate_sub_chunk(world, chunk_x, i, chunk_z));
            return {
                {"sub_chunks", std::move(subchunks)}
            };
        }

        enbt::compound generate_sub_chunk(storage::world_data& world, int64_t chunk_x, int64_t sub_chunk_y, int64_t chunk_z) override {
            if (sub_chunk_y != 0) {
                return {};
            } else {
                enbt::fixed_array x_dim(16);
                for (uint8_t x = 0; x < 16; x++) {
                    enbt::fixed_array y_dim(16);
                    for (uint8_t y = 0; y < 16; y++) {
                        enbt::simple_array_ui32 z_dim(16);
                        for (uint8_t z = 0; z < 16; z++) {
                            if (y == 0)
                                z_dim[z] = 1;
                        }
                        y_dim.set(y, std::move(z_dim));
                    }
                    x_dim.set(x, std::move(y_dim));
                }
                return {
                    {"blocks", std::move(x_dim)}
                };
            }
        }
    };

    struct default_gen : public PluginAutoRegister<"world_generators/default", default_gen> {
        void OnRegister(const PluginRegistrationPtr& self) override {
            storage::chunk_generator::register_it("default", new default_generator());
        }
    };
}