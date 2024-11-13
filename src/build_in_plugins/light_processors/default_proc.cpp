#include <src/build_in_plugins/light_processors/default_proc.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server {
    namespace build_in_plugins {
        namespace light_processors {

            class DefaultLightProcessor : public storage::chunk_light_processor {
            public:
                void process_chunk(storage::world_data& world, int64_t chunk_x, int64_t chunk_z) override {
                    world.get_chunk(chunk_x, chunk_z, [&](storage::chunk_data& chunk) {
                        chunk.for_each_sub_chunk([&](storage::sub_chunk_data& sub_chunk) {
                            sub_chunk.for_each_block([&](uint8_t x, uint8_t y, uint8_t z, base_objects::block& block) {

                            });
                        });
                    });
                }

                void process_sub_chunk(storage::world_data& world, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z) override {
                }

                void block_changed(storage::world_data& world, int64_t global_x, uint64_t global_y, int64_t global_z) override {
                }
            };

            void DefaultProc::OnRegister(const PluginRegistrationPtr& self) {
                storage::chunk_light_processor::register_it("default", new DefaultLightProcessor());
            }
        }
    }
}