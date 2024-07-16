#ifndef SRC_BASE_OBJECTS_RECIPE
#define SRC_BASE_OBJECTS_RECIPE
#include "../library/list_array.hpp"
#include "slot.hpp"
#include <string>
#include <variant>

namespace crafted_craft {
    namespace base_objects {
        namespace recipes {
            enum class category_t {
                building = 0,
                redstone = 1,
                equipment = 2,
                misc = 3,
            };

            using ingredient = list_array<slot>;

            namespace minecraft {
                template <int32_t id>
                struct special_recipe {
                    category_t category;
                };

                struct crafting_shaped {
                    std::string group;
                    int32_t width;
                    int32_t height;
                    list_array<ingredient> ingredients;
                    slot result;
                    category_t category : 2;
                    bool show_notification : 1;
                };

                struct crafting_shapeless {
                    std::string group;
                    list_array<ingredient> ingredients;
                    slot result;
                    category_t category : 2;
                    bool show_notification : 1;
                };

                using crafting_special_armordye = special_recipe<0>;
                using crafting_special_bookcloning = special_recipe<1>;
                using crafting_special_mapcloning = special_recipe<2>;
                using crafting_special_mapextending = special_recipe<3>;
                using crafting_special_firework_rocket = special_recipe<4>;
                using crafting_special_firework_star = special_recipe<5>;
                using crafting_special_firework_star_fade = special_recipe<6>;
                using crafting_special_tippedarrow = special_recipe<7>;
                using crafting_special_bannerduplicate = special_recipe<8>;
                using crafting_special_shielddecoration = special_recipe<9>;
                using crafting_special_shulkerboxcoloring = special_recipe<10>;
                using crafting_special_suspiciousstew = special_recipe<11>;
                using crafting_special_repairitem = special_recipe<12>;
                using crafting_decorated_pot = special_recipe<13>;

                template <int32_t id>
                struct smelting_types {
                    std::string group;
                    ingredient ingredient;
                    slot result;
                    float experience;
                    int32_t cooking_time;
                    category_t category : 2;
                };

                using smelting = smelting_types<0>;
                using blasting = smelting_types<1>;
                using smoking = smelting_types<2>;
                using campfire_cooking = smelting_types<3>;

                struct stonecutting {
                    std::string group;
                    ingredient ingredient;
                    slot result;
                };

                struct smithing_transform {
                    ingredient _template;
                    ingredient base;
                    ingredient addition;
                    slot result;
                };

                struct smithing_trim {
                    ingredient _template;
                    ingredient base;
                    ingredient addition;
                };
            }

            struct custom {
                list_array<uint8_t> data;
            };

            template <class T>
            struct variant_data {
            };

            template <>
            struct variant_data<minecraft::crafting_shaped> {
                constexpr static inline const char* name = "minecraft:crafting_shaped";
                constexpr static inline int32_t id = 0;
            };

            template <>
            struct variant_data<minecraft::crafting_shapeless> {
                constexpr static inline const char* name = "minecraft:crafting_shapeless";
                constexpr static inline int32_t id = 1;
            };

            template <>
            struct variant_data<minecraft::crafting_special_armordye> {
                constexpr static inline const char* name = "minecraft:crafting_special_armordye";
                constexpr static inline int32_t id = 2;
            };

            template <>
            struct variant_data<minecraft::crafting_special_bookcloning> {
                constexpr static inline const char* name = "minecraft:crafting_special_bookcloning";
                constexpr static inline int32_t id = 3;
            };

            template <>
            struct variant_data<minecraft::crafting_special_mapcloning> {
                constexpr static inline const char* name = "minecraft:crafting_special_mapcloning";
                constexpr static inline int32_t id = 4;
            };

            template <>
            struct variant_data<minecraft::crafting_special_mapextending> {
                constexpr static inline const char* name = "minecraft:crafting_special_mapextending";
                constexpr static inline int32_t id = 5;
            };

            template <>
            struct variant_data<minecraft::crafting_special_firework_rocket> {
                constexpr static inline const char* name = "minecraft:crafting_special_firework_rocket";
                constexpr static inline int32_t id = 6;
            };

            template <>
            struct variant_data<minecraft::crafting_special_firework_star> {
                constexpr static inline const char* name = "minecraft:crafting_special_firework_star";
                constexpr static inline int32_t id = 7;
            };

            template <>
            struct variant_data<minecraft::crafting_special_firework_star_fade> {
                constexpr static inline const char* name = "minecraft:crafting_special_firework_star_fade";
                constexpr static inline int32_t id = 8;
            };

            template <>
            struct variant_data<minecraft::crafting_special_tippedarrow> {
                constexpr static inline const char* name = "minecraft:crafting_special_tippedarrow";
                constexpr static inline int32_t id = 9;
            };

            template <>
            struct variant_data<minecraft::crafting_special_bannerduplicate> {
                constexpr static inline const char* name = "minecraft:crafting_special_bannerduplicate";
                constexpr static inline int32_t id = 10;
            };

            template <>
            struct variant_data<minecraft::crafting_special_shielddecoration> {
                constexpr static inline const char* name = "minecraft:crafting_special_shielddecoration";
                constexpr static inline int32_t id = 11;
            };

            template <>
            struct variant_data<minecraft::crafting_special_shulkerboxcoloring> {
                constexpr static inline const char* name = "minecraft:crafting_special_shulkerboxcoloring";
                constexpr static inline int32_t id = 12;
            };

            template <>
            struct variant_data<minecraft::crafting_special_suspiciousstew> {
                constexpr static inline const char* name = "minecraft:crafting_special_suspiciousstew";
                constexpr static inline int32_t id = 13;
            };

            template <>
            struct variant_data<minecraft::crafting_special_repairitem> {
                constexpr static inline const char* name = "minecraft:crafting_special_repairitem";
                constexpr static inline int32_t id = 14;
            };

            template <>
            struct variant_data<minecraft::crafting_decorated_pot> {
                constexpr static inline const char* name = "minecraft:crafting_decorated_pot";
                constexpr static inline int32_t id = 22;
            };

            template <>
            struct variant_data<minecraft::smelting> {
                constexpr static inline const char* name = "minecraft:smelting";
                constexpr static inline int32_t id = 15;
            };

            template <>
            struct variant_data<minecraft::blasting> {
                constexpr static inline const char* name = "minecraft:blasting";
                constexpr static inline int32_t id = 16;
            };

            template <>
            struct variant_data<minecraft::smoking> {
                constexpr static inline const char* name = "minecraft:smoking";
                constexpr static inline int32_t id = 17;
            };

            template <>
            struct variant_data<minecraft::campfire_cooking> {
                constexpr static inline const char* name = "minecraft:campfire_cooking";
                constexpr static inline int32_t id = 18;
            };

            template <>
            struct variant_data<minecraft::stonecutting> {
                constexpr static inline const char* name = "minecraft:stonecutting";
                constexpr static inline int32_t id = 19;
            };

            template <>
            struct variant_data<minecraft::smithing_transform> {
                constexpr static inline const char* name = "minecraft:smithing_transform";
                constexpr static inline int32_t id = 20;
            };

            template <>
            struct variant_data<minecraft::smithing_trim> {
                constexpr static inline const char* name = "minecraft:smithing_trim";
                constexpr static inline int32_t id = 21;
            };

            template <>
            struct variant_data<custom> {
                constexpr static inline const char* name = "crafted_craft:custom";
                constexpr static inline int32_t id = -1;
            };

            using varies = std::variant<
                minecraft::crafting_shaped,
                minecraft::crafting_shapeless,
                minecraft::crafting_special_armordye,
                minecraft::crafting_special_bookcloning,
                minecraft::crafting_special_mapcloning,
                minecraft::crafting_special_mapextending,
                minecraft::crafting_special_firework_rocket,
                minecraft::crafting_special_firework_star,
                minecraft::crafting_special_firework_star_fade,
                minecraft::crafting_special_tippedarrow,
                minecraft::crafting_special_bannerduplicate,
                minecraft::crafting_special_shielddecoration,
                minecraft::crafting_special_shulkerboxcoloring,
                minecraft::crafting_special_suspiciousstew,
                minecraft::crafting_special_repairitem,
                minecraft::crafting_decorated_pot,
                minecraft::smelting,
                minecraft::blasting,
                minecraft::smoking,
                minecraft::campfire_cooking,
                minecraft::stonecutting,
                minecraft::smithing_transform,
                minecraft::smithing_trim,
                custom>;
        }

        struct recipe {
            recipes::varies data;
            std::string id;
        };
    }
}

#endif /* SRC_BASE_OBJECTS_RECIPE */
