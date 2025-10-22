/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_PALETTE_CONTAINER
#define SRC_BASE_OBJECTS_PALETTE_CONTAINER
#include <boost/container/flat_map.hpp>
#include <cassert>
#include <library/list_array.hpp>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace copper_server::base_objects {
    struct palette_data {
        bit_list_array<uint64_t> data;
        size_t bits_per_entry : 6;

        palette_data() : bits_per_entry(0) {}

        palette_data(uint8_t bits_per_entry)
            : bits_per_entry(bits_per_entry) {
            assert(bits_per_entry < 32);
            data.reserve_back((16 * 16 * 16) * (bits_per_entry / 64 + bool(bits_per_entry)));
        }

        static uint8_t bits_for_max(size_t items) {
            return (uint8_t)std::ceil(std::log2(items + 1));
        }

        static palette_data create_from_max(size_t items) {
            return {bits_for_max(items)};
        }

        constexpr void add(size_t value) {
            if (value >= (size_t(1) << bits_per_entry))
                throw std::out_of_range("value is too large for the given bits_per_entry");
            data.push_back_bits(value, bits_per_entry);
        }

        //allows plugins to modify palette
        constexpr void modify(size_t index, size_t value) {
            size_t real_index = bits_per_entry * index;
            data.set_bits(real_index, value, bits_per_entry);
        }

        constexpr size_t get(size_t index) {
            size_t real_index = bits_per_entry * index;
            return data.get_bits<size_t>(real_index, bits_per_entry);
        }

        template <class FN>
        constexpr void for_each(FN&& fn) {
            data.commit();
            size_t max_i = data.size() / bits_per_entry;
            for (size_t i = 0; i < max_i; i++)
                fn(get(i));
        }

        template <class FN>
        constexpr void for_each(FN&& fn) const {
            size_t max_i = data.size() / bits_per_entry;
            for (size_t i = 0; i < max_i; i++)
                fn(get(i));
        }

        constexpr list_array<uint64_t>& get() {
            data.commit();
            return data.data();
        }

        constexpr const list_array<uint64_t>& get() const {
            return data.data();
        }

        constexpr void clear() {
            return data.clear();
        }
    };

    struct palette_container_single {
        const uint8_t bits_per_entry = 0;
        int32_t id_of_palette = 0;
    };

    struct palette_container_indirect {
        uint8_t bits_per_entry; // 4-8 for blocks and 1-3 for biomes
        list_array<int32_t> palette;
        palette_data data;

        palette_container_indirect(uint8_t bits_per_entry) : bits_per_entry(bits_per_entry), data(bits_per_entry) {}
    };

    class palette_container {
        uint8_t bits_per_entry;
        list_array<int32_t> data;
        bool is_biomes_mode;

    public:
        using compile_result = std::variant<palette_container_single, palette_container_indirect, palette_data>;
        static inline constexpr uint8_t max_indirect_biomes = 0x4;
        static inline constexpr uint8_t max_indirect_blocks = 0x8;
        static inline constexpr uint8_t min_indirect_biomes = 0x1;
        static inline constexpr uint8_t min_indirect_blocks = 0x4;

        static inline constexpr size_t size_biomes = 64;
        static inline constexpr size_t size_blocks = 4096;

        palette_container() : bits_per_entry(0), is_biomes_mode(false) {}

        palette_container(size_t max_items, bool is_biomes) : bits_per_entry(palette_data::bits_for_max(max_items)), is_biomes_mode(is_biomes) {}

        void add_range(int32_t* values, size_t size) {
            data.resize(data.size() + size);
            memcpy(data.data() + data.size() - size, values, size * sizeof(int32_t));
        }

        void add(int32_t value) {
            data.push_back(value);
        }

        int32_t at(size_t index) const {
            return data.at(index);
        }

        int32_t operator[](size_t index) const {
            return data[index];
        }

        int32_t set(size_t index, int32_t value) {
            std::swap(data[index], value);
            return value;
        }

        compile_result compile() const& {
            if (data.empty()) {
                palette_container_single res;
                res.id_of_palette = 0;
                return res;
            }
            boost::container::flat_map<int32_t, uint_fast32_t> palette_map;
            std::vector<int32_t> unique_palette;
            unique_palette.reserve(is_biomes_mode ? max_indirect_biomes + 1 : max_indirect_blocks + 1);

            for (const auto& id : data)
                if (palette_map.find(id) == palette_map.end()) {
                    palette_map.emplace(id, unique_palette.size());
                    unique_palette.push_back(id);
                }

            if (unique_palette.size() == 1) {
                palette_container_single res;
                res.id_of_palette = data.back();
                return res;
            } else if ((is_biomes_mode && unique_palette.size() <= max_indirect_biomes) || (!is_biomes_mode && unique_palette.size() <= max_indirect_blocks)) {
                palette_container_indirect res(std::max<uint8_t>(palette_data::bits_for_max(unique_palette.size()), is_biomes_mode ? min_indirect_biomes : min_indirect_blocks));
                res.palette = std::move(unique_palette);
                for (auto it : data)
                    res.data.add(palette_map[it]);
                return res;
            } else {
                palette_data res(bits_per_entry);
                for (auto it : data)
                    res.add(it);
                return res;
            }
        }

        void decompile(compile_result&& vars) {
            data.clear();
            std::visit(
                [this](auto& it) {
                    using T = std::decay_t<decltype(it)>;
                    if constexpr (std::is_same_v<T, palette_container_single>) {
                        if (is_biomes_mode)
                            for (size_t i = 0; i < size_biomes; i++)
                                add(it.id_of_palette);
                        else
                            for (size_t i = 0; i < size_blocks; i++)
                                add(it.id_of_palette);
                    } else if constexpr (std::is_same_v<T, palette_container_indirect>) {
                        it.data.for_each([this, &it](size_t val) {
                            add(it.palette[val]);
                        });
                    } else
                        it.for_each([this](size_t val) {
                            add((int32_t)val);
                        });
                },
                vars
            );
        }

        size_t size() const {
            return is_biomes_mode ? size_biomes : size_blocks;
        }

        void reserve(size_t size) {
            data.reserve(size);
        }
    };

    struct palette_container_biome : public palette_container {
        palette_container_biome() : palette_container(0, true) {}

        palette_container_biome(size_t max_items) : palette_container(max_items, true) {}
    };

    struct palette_container_block : public palette_container {
        palette_container_block() : palette_container(0, false) {}

        palette_container_block(size_t max_items) : palette_container(max_items, false) {}
    };

    struct palette_data_height_map : public palette_data {
        using palette_data::palette_data;
    };
}

#endif /* SRC_BASE_OBJECTS_PALLETE_CONTAINER */
