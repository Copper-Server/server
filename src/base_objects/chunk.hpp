#ifndef SRC_BASE_OBJECTS_CHUNK
#define SRC_BASE_OBJECTS_CHUNK
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include <cstdint>
#include <variant>
#include <vector>

namespace crafted_craft {
    namespace base_objects {
        namespace chunk {
            struct pallete_data {
                bit_list_array data;
                uint64_t bits_per_entry : 8;

                pallete_data(uint8_t bits_per_entry)
                    : bits_per_entry(bits_per_entry) {
                    data.reserve_push_back((16 * 16 * 16) * bits_per_entry / 64);
                }

                constexpr void add(size_t value) {
                    if (value >= (1 << bits_per_entry))
                        throw std::out_of_range("value is too large for the given bits_per_entry");
                    for (size_t i = 0; i < bits_per_entry; i++)
                        data.push_back((value >> i) & 1);
                }

                constexpr const list_array<uint8_t>& get() {
                    data.commit();
                    return data.data();
                }
            };

            struct pelleted_container_single {
                // uint8_t bits_per_entry; always 0
                int32_t id_of_palette;
            };

            struct pelleted_container_indirect {
                uint8_t bits_per_entry; // 4-8 for blocks and 1-3 for biomes
                list_array<int32_t> palette;
                pallete_data data;
            };

            struct pelleted_container_direct_blocks {
                //uint8_t bits_per_entry;//only 15 for blocks
                pallete_data data;

                pelleted_container_direct_blocks()
                    : data(15) {}
            };

            struct pelleted_container_direct_biomes {
                //uint8_t bits_per_entry;//only 6 for biomes
                pallete_data data;

                pelleted_container_direct_biomes()
                    : data(6) {}
            };

            struct pelleted_container {
                std::variant<pelleted_container_single, pelleted_container_indirect, pelleted_container_direct_blocks, pelleted_container_direct_biomes> data;

                pelleted_container(pelleted_container_single data)
                    : data(data) {}

                pelleted_container(pelleted_container_indirect data)
                    : data(data) {}

                pelleted_container(pelleted_container_direct_blocks data)
                    : data(data) {}

                pelleted_container(pelleted_container_direct_biomes data)
                    : data(data) {}

                const list_array<uint8_t> serialize() {
                    list_array<uint8_t> result;
                    std::visit([&result](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, pelleted_container_single>) {
                            result.push_back(0);
                            write_var_int(result, arg.id_of_palette);
                            result.push_back(0);
                        } else if constexpr (std::is_same_v<T, pelleted_container_indirect>) {
                            result.push_back(arg.bits_per_entry);
                            write_var_int(result, arg.palette.size());
                            for (auto& i : arg.palette)
                                write_var_int(result, i);
                            auto data = arg.data.get();
                            write_var_int(result, data.size());
                            result.push_back(std::move(data));
                        } else if constexpr (std::is_same_v<T, pelleted_container_direct_blocks>) {
                            result.push_back(15);
                            auto data = arg.data.get();
                            write_var_int(result, data.size());
                            result.push_back(std::move(data));
                        } else if constexpr (std::is_same_v<T, pelleted_container_direct_biomes>) {
                            result.push_back(6);
                            auto data = arg.data.get();
                            write_var_int(result, data.size());
                            result.push_back(std::move(data));
                        }
                    },
                               data);
                    return result;
                }

            private:
                static void write_var_int(list_array<uint8_t>& result, int32_t value) {
                    uint8_t buffer[5];
                    size_t res = ENBT::toVar(buffer, 5, value);
                    for (size_t i = 0; i < res; i++)
                        result.push_back(buffer[i]);
                }
            };

            struct chunk_biomes {
                int32_t x;
                int32_t z;
                list_array<pelleted_container> biomes;
            };

            struct chunk_section {
                uint16_t block_count;
                uint16_t palette_index;
                pelleted_container biomes;
            };

            struct chunk {
                list_array<chunk_section> sections;
            };
        }
    }
}

#endif /* SRC_BASE_OBJECTS_CHUNK */
