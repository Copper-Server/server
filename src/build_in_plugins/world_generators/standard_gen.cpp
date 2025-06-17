#include <src/plugin/main.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::build_in_plugins::world_generators {
    class StandardGeneratorImpl : public storage::chunk_generator {
        public:
        StandardGeneratorImpl() {
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

        void process_chunk(storage::world_data& world, storage::chunk_data& chunk, uint8_t preset_stage) {
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

        enbt::compound generate_chunk(storage::world_data& world, int64_t chunk_x, int64_t chunk_z) override {
            return {
                {"generator_stage", 0}
            };
        }

        enbt::compound generate_sub_chunk(storage::world_data& world, int64_t chunk_x, int64_t sub_chunk_y, int64_t chunk_z) override {
            return {};
        }
    };

    class StandardGen : public PluginAutoRegister<"standard_world_generator", StandardGen> {
    public:
        StandardGen() {}

        void OnRegister(const PluginRegistrationPtr& self) override {
            storage::chunk_generator::register_it("standard", new StandardGeneratorImpl());
        }
    };
}