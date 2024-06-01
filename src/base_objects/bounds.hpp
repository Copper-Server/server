#ifndef SRC_BASE_OBJECTS_BOUNDS
#define SRC_BASE_OBJECTS_BOUNDS
#include <cstdint>

namespace crafted_craft {
    namespace base_objects {
        struct cubic_bounds_chunk {
            int64_t x1;
            int64_t z1;
            int64_t x2;
            int64_t z2;

            template <class _FN>
            void enum_points(_FN fn) {
                for (int64_t i = x1; i <= x2; i++)
                    for (int64_t j = z1; j <= z2; j++)
                        fn(i, j);
            }

            template <class _FN>
            void enum_points_from_center(_FN fn) {
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

            bool in_bounds(int64_t x, int64_t z) {
                return x >= x1 && x <= x2 && z >= z1 && z <= z2;
            }

            bool out_of_bounds(int64_t x, int64_t z) {
                return !in_bounds(x, z);
            }
        };

        struct cubic_bounds_chunk_radius {
            int64_t center_x;
            int64_t center_z;
            int64_t radius;

            template <class _FN>
            void enum_points(_FN fn) {
                int64_t max_x = center_x + radius;
                int64_t max_z = center_z + radius;
                for (int64_t i = center_x - radius; i <= max_x; i++)
                    for (int64_t j = center_z - radius; j <= max_z; j++)
                        fn(i, j);
            }

            template <class _FN>
            void enum_points_from_center(_FN fn) {
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

            bool in_bounds(int64_t x, int64_t z) {
                return x >= (center_x - radius) && x <= (center_x + radius) && z >= (center_z - radius) && z <= (center_z + radius);
            }

            bool out_of_bounds(int64_t x, int64_t z) {
                return !in_bounds(x, z);
            }
        };

        struct cubic_bounds_block {
            int64_t x1;
            int64_t y1;
            int64_t z1;
            int64_t x2;
            int64_t y2;
            int64_t z2;

            template <class _FN>
            void enum_points(_FN fn) {
                for (int64_t i = x1; i <= x2; i++)
                    for (int64_t j = y1; j <= y2; j++)
                        for (int64_t k = z1; k <= z2; k++)
                            fn(i, j, k);
            }

            bool in_bounds(int64_t x, int64_t y, int64_t z) {
                return x >= x1 && x <= x2 && y >= y1 && y <= y2 && z >= z1 && z <= z2;
            }

            bool out_of_bounds(int64_t x, int64_t y, int64_t z) {
                return !in_bounds(x, y, z);
            }
        };

        struct spherical_bounds_chunk {
            int64_t x;
            int64_t z;
            double radius;

            template <class _FN>
            void enum_points(_FN fn) {
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

            bool in_bounds(int64_t x, int64_t z) {
                return ((x - this->x) * (x - this->x) + (z - this->z) * (z - this->z)) <= radius * radius;
            }

            bool out_of_bounds(int64_t x, int64_t z) {
                return !in_bounds(x, z);
            }
        };

        struct spherical_bounds_blocks {
            int64_t x;
            int64_t y;
            int64_t z;
            double radius;

            template <class _FN>
            void enum_points(_FN fn) {
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

            bool in_bounds(int64_t x, int64_t y, int64_t z) {
                return ((x - this->x) * (x - this->x) + (y - this->y) * (y - this->y) + (z - this->z) * (z - this->z)) <= radius * radius;
            }

            bool out_of_bounds(int64_t x, int64_t y, int64_t z) {
                return !in_bounds(x, y, z);
            }
        };

        struct square_bounds_chunk {
            int64_t x1;
            int64_t z1;

            int64_t x2;
            int64_t z2;
        };
    }
}

#endif /* SRC_BASE_OBJECTS_BOUNDS */
