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
        };

        struct cubic_bounds_block {
            int64_t x1;
            int64_t y1;
            int64_t z1;
            int64_t x2;
            int64_t y2;
            int64_t z2;
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
