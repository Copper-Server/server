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
#include <variant>
#include <vector>

namespace copper_server::base_objects {
    struct palette_data {
        bit_list_array<uint64_t> data;
        size_t bits_per_entry : 6;

        palette_data();
        palette_data(uint8_t bits_per_entry, size_t reserve_size);
        palette_data(palette_data&&);
        palette_data(const palette_data&);

        palette_data& operator=(palette_data&&);
        palette_data& operator=(const palette_data&);

        static uint8_t bits_for_max(size_t items);
        void add(int32_t value);
        void modify(size_t index, int32_t value);

        int32_t get(size_t index) const;

        template <class FN>
        void for_each(FN&& fn) {
            data.commit();
            size_t max_i = data.size() / bits_per_entry;
            for (size_t i = 0; i < max_i; i++)
                fn(get(i));
        }

        template <class FN>
        void for_each(FN&& fn) const {
            size_t max_i = data.size() / bits_per_entry;
            for (size_t i = 0; i < max_i; i++)
                fn(get(i));
        }

        list_array<uint64_t>& get();

        const list_array<uint64_t>& get() const;

        void clear();

        void resize_bits_per_entry(uint8_t bits_per_entry);
    };

    struct palette_container_single {
        int32_t id_of_palette;
    };

    struct palette_container_indirect {
        uint8_t bits_per_entry;
        std::vector<int32_t> palette;
        palette_data data;

        palette_container_indirect(uint8_t bits_per_entry, size_t reserve_size);
    };

    class palette_container {
    public:
        using compiled_variant = std::variant<palette_container_single, palette_container_indirect, palette_data>;

        static inline constexpr uint8_t min_indirect_blocks = 4;
        static inline constexpr uint8_t max_indirect_blocks = 8;
        static inline constexpr uint8_t min_indirect_biomes = 1;
        static inline constexpr uint8_t max_indirect_biomes = 3;

        static uint8_t GLOBAL_BITS_PER_ENTRY_BLOCKS;
        static uint8_t GLOBAL_BITS_PER_ENTRY_BIOMES;

    private:
        compiled_variant data;
        boost::container::flat_map<int32_t, uint32_t> ref_counts;
        boost::container::flat_map<int32_t, int32_t> palette_map;
        bool is_biomes_mode;

        void resize_indirect_to_indirect(palette_container_indirect& indirect, uint8_t new_bits);
        void resize_indirect_to_direct(palette_container_indirect& indirect);
        void resize_single_to_indirect(palette_container_single& single, int32_t new_id, size_t index);

        void try_shrink();
        void resize_direct_to_indirect();
        void resize_indirect_to_single();

    public:
        palette_container(bool is_biomes_mode = false);

        const compiled_variant& compile() const& {
            return data;
        }

        int32_t get(uint8_t x, uint8_t y, uint8_t z) const;
        void set(uint8_t x, uint8_t y, uint8_t z, int32_t new_id);
        void decompile(compiled_variant&& vars);
    };

    struct palette_container_biome : public palette_container {
        palette_container_biome() : palette_container(true) {}
    };

    struct palette_container_block : public palette_container {
        palette_container_block() : palette_container(false) {}
    };

    struct palette_data_height_map : public palette_data {
        palette_data_height_map(int32_t max_height = 384)
            : palette_data(palette_data::bits_for_max((size_t)max_height), 256) {
            data.resize(256 * bits_per_entry);
        }

        palette_data_height_map(palette_data_height_map&& mov) : palette_data(std::move(mov)) {}

        palette_data_height_map(const palette_data_height_map& copy) : palette_data(copy) {}

        palette_data_height_map& operator=(palette_data_height_map&& mov) {
            palette_data::operator=(std::move(mov));
            return *this;
        }

        palette_data_height_map& operator=(const palette_data_height_map& copy) {
            palette_data::operator=(copy);
            return *this;
        }

        int32_t get(uint8_t x, uint8_t z) const {
            return palette_data::get(to_pos(x, z));
        }

        void set(uint8_t x, uint8_t z, int32_t new_value) {
            return palette_data::modify(to_pos(x, z), new_value);
        }

        void set_height(int32_t max_height) {
            resize_bits_per_entry(palette_data::bits_for_max((size_t)max_height));
        }

    private:
        static constexpr inline size_t to_pos(uint8_t x, uint8_t z) {
            return (static_cast<size_t>(z) << 4) | static_cast<size_t>(x);
        }
    };
}

#endif /* SRC_BASE_OBJECTS_PALLETE_CONTAINER */
