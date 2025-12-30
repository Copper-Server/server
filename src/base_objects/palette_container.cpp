#include <algorithm>
#include <cmath>
#include <src/base_objects/block.hpp>
#include <src/base_objects/palette_container.hpp>
#include <bit>

namespace copper_server::base_objects {
    uint8_t palette_container::GLOBAL_BITS_PER_ENTRY_BLOCKS = 15;
    uint8_t palette_container::GLOBAL_BITS_PER_ENTRY_BIOMES = 7;

    static inline constexpr size_t BLOCKS_STORAGE_SIZE = 16 * 16 * 16;
    static inline constexpr size_t BIOMES_STORAGE_SIZE = 4 * 4 * 4;

    palette_data::palette_data() : bits_per_entry(0) {}

    palette_data::palette_data(uint8_t bits_per_entry, size_t reserve_size) : bits_per_entry(bits_per_entry) {
        if (bits_per_entry > 0)
            data.reserve_back(reserve_size * bits_per_entry);
    }

    palette_data::palette_data(palette_data&& mov) : data(std::move(mov.data)), bits_per_entry(mov.bits_per_entry) {}

    palette_data::palette_data(const palette_data& copy) : data(copy.data), bits_per_entry(copy.bits_per_entry) {}

    palette_data& palette_data::operator=(palette_data&& mov) {
        data = std::move(mov.data);
        bits_per_entry = mov.bits_per_entry;
        return *this;
    }

    palette_data& palette_data::operator=(const palette_data& copy) {
        data = copy.data;
        bits_per_entry = copy.bits_per_entry;
        return *this;
    }

    uint8_t palette_data::bits_for_max(size_t items) {
        if (items <= 1)
            return 0;
        return (uint8_t)std::max(1.0, std::ceil(std::log2(items)));
    }

    void palette_data::add(int32_t value) {
        if (value >= (size_t(1) << bits_per_entry))
            throw std::out_of_range("value is too large for the given bits_per_entry");
        data.push_back_bits(value, bits_per_entry);
    }

    void palette_data::modify(size_t index, int32_t value) {
        size_t real_index = bits_per_entry * index;
        data.set_bits(real_index, value, bits_per_entry);
    }

    int32_t palette_data::get(size_t index) const {
        size_t real_index = bits_per_entry * index;
        auto res = (uint32_t)data.get_bits<size_t>(real_index, bits_per_entry);
        return std::bit_cast<int32_t>(res);
    }

    list_array<uint64_t>& palette_data::get() {
        data.commit();
        return data.data();
    }

    const list_array<uint64_t>& palette_data::get() const {
        return data.data();
    }

    void palette_data::clear() {
        return data.clear();
    }

    void palette_data::resize_bits_per_entry(uint8_t n_bits_per_entry) {
        if (bits_per_entry != n_bits_per_entry) {
            palette_data new_data(n_bits_per_entry, data.size());
            size_t actual_size = data.size() / bits_per_entry;
            for (size_t i = 0; i < actual_size; ++i)
                new_data.add(get(i));


            data = std::move(new_data.data);
            bits_per_entry = n_bits_per_entry;
        }
    }

    palette_container_indirect::palette_container_indirect(uint8_t bits_per_entry, size_t reserve_size) : bits_per_entry(bits_per_entry), data(bits_per_entry, reserve_size) {}

    palette_container::palette_container(bool is_biomes_mode) : is_biomes_mode(is_biomes_mode) {
        data.emplace<palette_container_single>(0);
        ref_counts[0] = is_biomes_mode ? BIOMES_STORAGE_SIZE : BLOCKS_STORAGE_SIZE;
    }

    static inline constexpr size_t pos_to_index_block(uint8_t x, uint8_t y, uint8_t z) {
        return (static_cast<size_t>(y) << 8) | (static_cast<size_t>(z) << 4) | static_cast<size_t>(x);
    }

    static inline constexpr size_t pos_to_index_biome(uint8_t x, uint8_t y, uint8_t z) {
        return (static_cast<size_t>(y >> 2) << 4) | (static_cast<size_t>(z >> 2) << 2) | static_cast<size_t>(x >> 2);
    }

    int32_t palette_container::get(uint8_t x, uint8_t y, uint8_t z) const {
        const size_t index = is_biomes_mode ? pos_to_index_biome(x, y, z) : pos_to_index_block(x, y, z);

        return std::visit(
            [index](auto&& arg) -> int32_t {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, palette_container_single>) {
                    return arg.id_of_palette;
                } else if constexpr (std::is_same_v<T, palette_container_indirect>) {
                    int32_t palette_index = arg.data.get(index);
                    return arg.palette[palette_index];
                } else if constexpr (std::is_same_v<T, palette_data>)
                    return arg.get(index);
                else
                    return 0;
            },
            data
        );
    }

    void palette_container::set(uint8_t x, uint8_t y, uint8_t z, int32_t new_id) {
        const size_t index = is_biomes_mode ? pos_to_index_biome(x, y, z) : pos_to_index_block(x, y, z);

        const int32_t old_id = get(x, y, z);
        bool should_shrink = false;

        if (new_id == old_id) 
            return;
        else {
            auto old_it = ref_counts.find(old_id);
            old_it->second--;
            if (old_it->second == 0) {
                ref_counts.erase(old_it);
                should_shrink = true;
            }
        }

        std::visit(
            [&, this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, palette_container_single>) {
                    resize_single_to_indirect(arg, new_id, index);
                } else if constexpr (std::is_same_v<T, palette_container_indirect>) {
                    auto it = palette_map.find(new_id);
                    int32_t new_palette_index;

                    if (it != palette_map.end()) {
                        new_palette_index = it->second;
                        ref_counts[new_id]++;
                    } else {
                        new_palette_index = (int32_t)arg.palette.size();
                        arg.palette.push_back(new_id);
                        palette_map[new_id] = new_palette_index;
                        ref_counts[new_id] = 1;

                        uint8_t new_bits = palette_data::bits_for_max(arg.palette.size());
                        new_bits = std::max(is_biomes_mode ? min_indirect_biomes : min_indirect_blocks, new_bits);

                        if (new_bits > arg.bits_per_entry) {
                            if (new_bits > (is_biomes_mode ? max_indirect_biomes : max_indirect_blocks)) {
                                resize_indirect_to_direct(arg);
                                std::get<palette_data>(data).modify(index, new_id);
                                return;
                            } else
                                resize_indirect_to_indirect(arg, new_bits);
                        }
                    }
                    arg.data.modify(index, new_palette_index);
                } else if constexpr (std::is_same_v<T, palette_data>) {
                    ref_counts[old_id]--;
                    ref_counts[new_id]++;
                    arg.modify(index, new_id);
                }
            },
            data
        );
        if (should_shrink) 
            try_shrink();
    }

    void palette_container::resize_single_to_indirect(palette_container_single& single, int32_t new_id, size_t index) {
        int32_t old_id = single.id_of_palette;
        auto min_bits = is_biomes_mode ? min_indirect_biomes : min_indirect_blocks;
        auto size = is_biomes_mode ? BIOMES_STORAGE_SIZE : BLOCKS_STORAGE_SIZE;
        palette_container_indirect new_indirect(min_bits, size);
        new_indirect.palette.push_back(old_id);
        new_indirect.palette.push_back(new_id);

        palette_map[old_id] = 0;
        palette_map[new_id] = 1;

        ref_counts[old_id]--;
        ref_counts[new_id] = 1;

        new_indirect.data.data.resize(min_bits * size);
        new_indirect.data.modify(index, 1);
        data = std::move(new_indirect);
    }

    void palette_container::resize_indirect_to_indirect(palette_container_indirect& indirect, uint8_t new_bits) {
        auto size = is_biomes_mode ? BIOMES_STORAGE_SIZE : BLOCKS_STORAGE_SIZE;
        palette_data new_data(new_bits, size);
        new_data.data.reserve_back(size * new_bits);

        for (size_t i = 0; i < size; ++i) {
            int32_t old_palette_index = indirect.data.get(i);
            new_data.add(old_palette_index);
        }

        indirect.data = std::move(new_data);
        indirect.bits_per_entry = new_bits;
    }

    void palette_container::resize_indirect_to_direct(palette_container_indirect& indirect) {
        auto bits = is_biomes_mode ? GLOBAL_BITS_PER_ENTRY_BIOMES : GLOBAL_BITS_PER_ENTRY_BLOCKS;
        auto size = is_biomes_mode ? BIOMES_STORAGE_SIZE : BLOCKS_STORAGE_SIZE;
        palette_data new_direct(bits, size);

        for (size_t i = 0; i < size; ++i) {
            int32_t old_palette_index = indirect.data.get(i);
            int32_t global_id = indirect.palette[old_palette_index];
            new_direct.add(global_id);
        }

        palette_map.clear();
        data = std::move(new_direct);
    }

    void palette_container::try_shrink() {
        const size_t unique_blocks = ref_counts.size();

        std::visit(
            [&, this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, palette_data>) {
                    if (unique_blocks <= (1u << max_indirect_blocks)) {
                        resize_direct_to_indirect();
                    }
                } else if constexpr (std::is_same_v<T, palette_container_indirect>) {
                    if (unique_blocks == 1) {
                        resize_indirect_to_single();
                    } else {
                        uint8_t new_bits = std::max(min_indirect_blocks, palette_data::bits_for_max(unique_blocks));
                        if (new_bits < arg.bits_per_entry) {
                            // Rebuild palette and map
                            arg.palette.clear();
                            palette_map.clear();
                            arg.palette.reserve(unique_blocks);
                            palette_map.reserve(unique_blocks);

                            for (auto const& [block_id, count] : ref_counts) {
                                palette_map[block_id] = (int32_t)arg.palette.size();
                                arg.palette.push_back(block_id);
                            }
                            resize_indirect_to_indirect(arg, new_bits);
                        }
                    }
                }
            },
            data
        );
    }

    void palette_container::resize_direct_to_indirect() {
        palette_data& direct_data = std::get<palette_data>(data);
        auto min_bits = is_biomes_mode ? min_indirect_biomes : min_indirect_blocks;
        auto size = is_biomes_mode ? BIOMES_STORAGE_SIZE : BLOCKS_STORAGE_SIZE;
        uint8_t new_bits = std::max(min_bits, palette_data::bits_for_max(ref_counts.size()));
        palette_container_indirect new_indirect(new_bits, size);

        new_indirect.palette.reserve(ref_counts.size());
        palette_map.clear();
        palette_map.reserve(ref_counts.size());

        for (auto const& [block_id, count] : ref_counts) {
            palette_map[block_id] = (uint32_t)new_indirect.palette.size();
            new_indirect.palette.push_back(block_id);
        }

        new_indirect.data.data.reserve_back(size * new_bits);
        for (size_t i = 0; i < size; ++i) {
            uint32_t global_id = direct_data.get(i);
            uint32_t new_palette_index = palette_map.at(global_id);
            new_indirect.data.add(new_palette_index);
        }

        data = std::move(new_indirect);
    }

    void palette_container::resize_indirect_to_single() {
        palette_map.clear();
        data = palette_container_single{ref_counts.begin()->first};
    }

    void palette_container::decompile(compiled_variant&& vars) {
        data = std::move(vars);
        ref_counts.clear();

        std::visit(
            [this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, palette_container_single>) {
                    ref_counts[arg.id_of_palette] = is_biomes_mode ? BIOMES_STORAGE_SIZE : BLOCKS_STORAGE_SIZE;
                } else if constexpr (std::is_same_v<T, palette_container_indirect>) {
                    palette_map.clear();
                    for (size_t i = 0; i < arg.palette.size(); ++i) 
                        palette_map[arg.palette[i]] = (int32_t)i;
                    
                    arg.data.for_each([this, &arg](size_t palette_index) {
                        ref_counts[arg.palette[palette_index]]++;
                    });
                } else if constexpr (std::is_same_v<T, palette_data>) {
                    arg.for_each([this](int32_t global_id) {
                        ref_counts[global_id]++;
                    });
                }
            },
            data
        );
        try_shrink();
    }
}