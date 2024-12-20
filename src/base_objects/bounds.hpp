#ifndef SRC_BASE_OBJECTS_BOUNDS
#define SRC_BASE_OBJECTS_BOUNDS
#include <cstdint>
#include <random>


namespace copper_server {
    namespace base_objects {
        namespace __impl {
            template <class T>
            T convert_chunk_global_pos(T pos) {
                if (pos == 0)
                    return 0;
                if (pos < 0)
                    return (pos + 1) / 16 - 1;
                return pos / 16;
            }
        }

        struct cubic_bounds_chunk {
            int64_t x1;
            int64_t z1;
            int64_t x2;
            int64_t z2;

            cubic_bounds_chunk(int64_t set_x1, int64_t set_z1, int64_t set_x2, int64_t set_z2)
                : x1(set_x1), z1(set_z1), x2(set_x2), z2(set_z2) {
                if (x1 > x2)
                    std::swap(x1, x2);
                if (z1 > z2)
                    std::swap(z1, z2);
            }

            template <class _FN>
            void enum_points(_FN fn) const {
                for (int64_t i = x1; i <= x2; i++)
                    for (int64_t j = z1; j <= z2; j++)
                        fn(i, j);
            }

            template <class _FN>
            void enum_points_from_center(_FN fn) const {
                int64_t centerX = (x1 + x2) / 2;
                int64_t centerZ = (z1 + z2) / 2;

                int64_t maxLayerX = std::max(x2 - centerX, centerX - x1);
                int64_t maxLayerZ = std::max(z2 - centerZ, centerZ - z1);
                int64_t maxLayer = std::max(maxLayerX, maxLayerZ);
                fn(centerX, centerZ);

                for (int64_t layer = 1; layer <= maxLayer; ++layer) {
                    if (layer <= maxLayerZ) {
                        int64_t minX = std::max(-layer, -maxLayerX);
                        int64_t maxX = std::min(layer, maxLayerX);
                        for (int64_t i = minX; i <= maxX; ++i) {
                            fn(centerX + i, centerZ - layer);
                            fn(centerX + i, centerZ + layer);
                        }
                    }
                    if (layer <= maxLayerX) {
                        int64_t minZ = std::max(-layer, -maxLayerZ);
                        int64_t maxZ = std::min(layer, maxLayerZ);
                        for (int64_t i = minZ + 1; i < maxZ; ++i) {
                            fn(centerX - layer, centerZ + i);
                            fn(centerX + layer, centerZ + i);
                        }
                    }
                }
            }

            bool in_bounds(int64_t x, int64_t z) const {
                return x >= x1 && x <= x2 && z >= z1 && z <= z2;
            }

            bool out_of_bounds(int64_t x, int64_t z) const {
                return !in_bounds(x, z);
            }

            size_t count() const {
                return (x2 - x1 + 1) * (z2 - z1 + 1);
            }

            std::pair<int64_t, int64_t> random_point() const {
                std::mt19937_64 gen(std::random_device{}());
                std::uniform_int_distribution<int64_t> disX(x1, x2);
                std::uniform_int_distribution<int64_t> disZ(z1, z2);
                return std::make_pair(disX(gen), disZ(gen));
            }

            auto operator<=>(const cubic_bounds_chunk& other) const = default;
        };

        struct cubic_bounds_chunk_radius {
            int64_t center_x;
            int64_t center_z;
            int64_t radius;

            template <class _FN>
            void enum_points(_FN fn) const {
                int64_t max_x = center_x + radius;
                int64_t max_z = center_z + radius;
                for (int64_t i = center_x - radius; i <= max_x; i++)
                    for (int64_t j = center_z - radius; j <= max_z; j++)
                        fn(i, j);
            }

            template <class _FN>
            void enum_points_from_center(_FN fn) const {
                fn(center_x, center_z);
                for (int64_t layer = 1; layer <= radius; ++layer) {
                    for (int64_t i = -layer + 1; i < layer; ++i) {
                        fn(center_x + i, center_z - layer);
                        fn(center_x - layer, center_z + i);
                        fn(center_x + i, center_z + layer);
                        fn(center_x + layer, center_z + i);
                    }
                    fn(center_x - layer, center_z - layer);
                    fn(center_x + layer, center_z - layer);
                    fn(center_x - layer, center_z + layer);
                    fn(center_x + layer, center_z + layer);
                }
            }

            bool in_bounds(int64_t x, int64_t z) const {
                return x >= (center_x - radius) && x <= (center_x + radius) && z >= (center_z - radius) && z <= (center_z + radius);
            }

            bool out_of_bounds(int64_t x, int64_t z) const {
                return !in_bounds(x, z);
            }

            size_t count() {
                return (radius * 2 + 1) * (radius * 2 + 1);
            }

            std::pair<int64_t, int64_t> random_point() const {
                std::mt19937_64 gen(std::random_device{}());
                std::uniform_int_distribution<int64_t> dis_x(center_x - radius, center_x + radius);
                std::uniform_int_distribution<int64_t> dis_z(center_z - radius, center_z + radius);
                return std::make_pair(dis_x(gen), dis_z(gen));
            }

            auto operator<=>(const cubic_bounds_chunk_radius& other) const = default;
        };

        struct cubic_bounds_chunk_radius_out {
            int64_t center_x;
            int64_t center_z;
            int64_t radius_begin;
            int64_t radius_end;

            template <class _FN>
            void enum_points(_FN fn) const {
                int64_t max_x = center_x + radius_end;
                int64_t max_z = center_z + radius_end;
                for (int64_t i = center_x - radius_begin; i <= max_x; i++)
                    for (int64_t j = center_z - radius_begin; j <= max_z; j++)
                        fn(i, j);
            }

            template <class _FN>
            void enum_points_from_center(_FN fn) const {
                if (radius_begin == 0)
                    fn(center_x, center_z);
                for (int64_t layer = radius_begin ? radius_begin : 1; layer <= radius_end; ++layer) {
                    for (int64_t i = -layer + 1; i < layer; ++i) {
                        fn(center_x + i, center_z - layer);
                        fn(center_x - layer, center_z + i);
                        fn(center_x + i, center_z + layer);
                        fn(center_x + layer, center_z + i);
                    }
                    fn(center_x - layer, center_z - layer);
                    fn(center_x + layer, center_z - layer);
                    fn(center_x - layer, center_z + layer);
                    fn(center_x + layer, center_z + layer);
                }
            }

            template <class _FN>
            void enum_points_from_center_w_layer(_FN fn) const {
                if (radius_begin == 0)
                    fn(center_x, center_z, 0);
                for (int64_t layer = radius_begin ? radius_begin : 1; layer <= radius_end; ++layer) {
                    for (int64_t i = -layer + 1; i < layer; ++i) {
                        fn(center_x + i, center_z - layer, layer);
                        fn(center_x - layer, center_z + i, layer);
                        fn(center_x + i, center_z + layer, layer);
                        fn(center_x + layer, center_z + i, layer);
                    }
                    fn(center_x - layer, center_z - layer, layer);
                    fn(center_x + layer, center_z - layer, layer);
                    fn(center_x - layer, center_z + layer, layer);
                    fn(center_x + layer, center_z + layer, layer);
                }
            }

            bool in_bounds(int64_t x, int64_t z) const {
                return x >= (center_x - radius_begin) && x <= (center_x + radius_end) && z >= (center_z - radius_begin) && z <= (center_z + radius_end);
            }

            bool out_of_bounds(int64_t x, int64_t z) const {
                return !in_bounds(x, z);
            }

            size_t count() {
                return (radius_end - radius_begin + 1) * (radius_end - radius_begin + 1);
            }

            std::pair<int64_t, int64_t> random_point() const {
                std::mt19937_64 gen(std::random_device{}());
                std::uniform_int_distribution<int64_t> dis_x(center_x - radius_begin, center_x + radius_end);
                std::uniform_int_distribution<int64_t> dis_z(center_z - radius_begin, center_z + radius_end);
                return std::make_pair(dis_x(gen), dis_z(gen));
            }

            auto operator<=>(const cubic_bounds_chunk_radius_out& other) const = default;
        };

        struct cubic_bounds_block {
            int64_t x1;
            int64_t y1;
            int64_t z1;
            int64_t x2;
            int64_t y2;
            int64_t z2;

            cubic_bounds_block(int64_t set_x1, int64_t set_y1, int64_t set_z1, int64_t set_x2, int64_t set_y2, int64_t set_z2)
                : x1(set_x1), y1(set_y1), z1(set_z1), x2(set_x2), y2(set_y2), z2(set_z2) {
                if (x1 > x2)
                    std::swap(x1, x2);
                if (y1 > y2)
                    std::swap(y1, y2);
                if (z1 > z2)
                    std::swap(z1, z2);
            }

            template <class _FN>
            void enum_points(_FN fn) const {
                for (int64_t i = x1; i <= x2; i++)
                    for (int64_t j = y1; j <= y2; j++)
                        for (int64_t k = z1; k <= z2; k++)
                            fn(i, j, k);
            }

            bool in_bounds(int64_t x, int64_t y, int64_t z) const {
                return x >= x1 && x <= x2 && y >= y1 && y <= y2 && z >= z1 && z <= z2;
            }

            bool out_of_bounds(int64_t x, int64_t y, int64_t z) const {
                return !in_bounds(x, y, z);
            }

            size_t count() const {
                return (x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1);
            }

            std::tuple<int64_t, int64_t, int64_t> random_point() const {
                std::mt19937_64 gen(std::random_device{}());
                std::uniform_int_distribution<int64_t> dis_x(x1, x2);
                std::uniform_int_distribution<int64_t> dis_y(y1, y2);
                std::uniform_int_distribution<int64_t> dis_z(z1, z2);
                return std::make_tuple(dis_x(gen), dis_y(gen), dis_z(gen));
            }

            auto operator<=>(const cubic_bounds_block& other) const
                = default;

            explicit operator cubic_bounds_chunk() {
                return {
                    __impl::convert_chunk_global_pos(x1),
                    __impl::convert_chunk_global_pos(z1),
                    __impl::convert_chunk_global_pos(x2),
                    __impl::convert_chunk_global_pos(z2)
                };
            }
        };

        struct cubic_bounds_block_radius {
            int64_t x;
            int64_t y;
            int64_t z;
            int64_t radius;

            template <class _FN>
            void enum_points(_FN fn) const {
                for (int64_t i = x - radius; i <= x + radius; i++)
                    for (int64_t j = y - radius; j <= y + radius; j++)
                        for (int64_t k = z - radius; k <= z + radius; k++)
                            fn(i, j, k);
            }

            bool in_bounds(int64_t x, int64_t y, int64_t z) const {
                return ((x - this->x) * (x - this->x) + (y - this->y) * (y - this->y) + (z - this->z) * (z - this->z)) <= radius * radius;
            }

            bool out_of_bounds(int64_t x, int64_t y, int64_t z) const {
                return !in_bounds(x, y, z);
            }

            size_t count() const {
                return (radius * 2 + 1) * (radius * 2 + 1) * (radius * 2 + 1);
            }

            std::tuple<int64_t, int64_t, int64_t> random_point() const {
                std::mt19937_64 gen(std::random_device{}());
                std::uniform_int_distribution<int64_t> dis_x(x - radius, x + radius);
                std::uniform_int_distribution<int64_t> dis_y(y - radius, y + radius);
                std::uniform_int_distribution<int64_t> dis_z(z - radius, z + radius);

                return {dis_x(gen), dis_y(gen), dis_z(gen)};
            }

            auto operator<=>(const cubic_bounds_block_radius& other) const = default;

            explicit operator cubic_bounds_chunk_radius() {
                return {
                    __impl::convert_chunk_global_pos(x),
                    __impl::convert_chunk_global_pos(z),
                    __impl::convert_chunk_global_pos(radius),
                };
            }
        };

        struct spherical_bounds_chunk {
            int64_t x;
            int64_t z;
            double radius;

            template <class _FN>
            void enum_points(_FN fn) const {
                double radius2 = radius * radius;
                int64_t start_x = x - radius;
                int64_t end_x = x + radius;


                int64_t start_z = x - radius;
                int64_t end_z = x + radius;

                for (int64_t i = start_x; i <= end_x; i++)
                    for (int64_t j = start_z; j <= end_z; j++)
                        if (((i - x) * (i - x) + (j - z) * (j - z)) <= radius2)
                            fn(i, j);
            }

            bool in_bounds(int64_t x, int64_t z) const {
                return ((x - this->x) * (x - this->x) + (z - this->z) * (z - this->z)) <= radius * radius;
            }

            bool out_of_bounds(int64_t x, int64_t z) const {
                return !in_bounds(x, z);
            }

            size_t count() const {
                return (radius * 2 + 1) * (radius * 2 + 1);
            }

            std::tuple<double, double> random_point() const {
                std::mt19937 gen(std::random_device{}());
                std::normal_distribution<> dis(0, radius);
                return {x + dis(gen), z + dis(gen)};
            }

            auto operator<=>(const spherical_bounds_chunk& other) const = default;
        };

        struct spherical_bounds_block {
            int64_t x;
            int64_t y;
            int64_t z;
            double radius;

            template <class _FN>
            void enum_points(_FN fn) const {
                double radius2 = radius * radius;
                int64_t start_x = x - radius;
                int64_t end_x = x + radius;

                int64_t start_y = y - radius;
                int64_t end_y = y + radius;

                int64_t start_z = z - radius;
                int64_t end_z = z + radius;

                for (int64_t i = start_x; i <= end_x; i++)
                    for (int64_t j = start_y; j <= end_y; j++)
                        for (int64_t k = start_z; k <= end_z; k++)
                            if (((i - x) * (i - x) + (j - y) * (j - y) + (k - z) * (k - z)) <= radius2)
                                fn(i, j, k);
            }

            bool in_bounds(int64_t x, int64_t y, int64_t z) const {
                return ((x - this->x) * (x - this->x) + (y - this->y) * (y - this->y) + (z - this->z) * (z - this->z)) <= radius * radius;
            }

            bool out_of_bounds(int64_t x, int64_t y, int64_t z) const {
                return !in_bounds(x, y, z);
            }

            size_t count() const {
                return (radius * 2 + 1) * (radius * 2 + 1) * (radius * 2 + 1);
            }

            std::tuple<double, double, double> random_point() const {
                std::mt19937 gen(std::random_device{}());
                std::normal_distribution<> dis(0, radius);
                return std::make_tuple(x + dis(gen), y + dis(gen), z + dis(gen));
            }

            auto operator<=>(const spherical_bounds_block& other) const = default;

            explicit operator spherical_bounds_chunk() {
                return {
                    __impl::convert_chunk_global_pos(x),
                    __impl::convert_chunk_global_pos(z),
                    __impl::convert_chunk_global_pos(radius),
                };
            }
        };

        struct bounding {
            double xz;
            double y;

            auto operator<=>(const bounding& other) const = default;
        };
    }
}

#endif /* SRC_BASE_OBJECTS_BOUNDS */
