#ifndef SRC_BASE_OBJECTS_RECIPE
#define SRC_BASE_OBJECTS_RECIPE
#include <library/list_array.hpp>
#include <src/base_objects/slot.hpp>
#include <src/base_objects/slot_display.hpp>
#include <string>
#include <variant>

namespace copper_server::base_objects {
    namespace recipes {
        enum class compatibility_category_t {
            building = 0,
            redstone = 1,
            equipment = 2,
            misc = 3,
        };

        namespace minecraft {
            template <int32_t id>
            struct special_recipe {
                compatibility_category_t category;
            };

            struct crafting_shaped {
                int32_t width;
                int32_t height;
                std::vector<slot_display> ingredients;
                slot_display result;
                slot_display crafting_station;
                compatibility_category_t category : 2;
                bool show_notification : 1;
            };

            struct crafting_shapeless {
                std::vector<slot_display> ingredients;
                slot_display result;
                slot_display crafting_station;
                compatibility_category_t category : 2;
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
                slot_display ingredient;
                slot_display fuel;
                slot_display result;
                slot_display crafting_station;
                int32_t cooking_time;
                float experience;
                compatibility_category_t category : 2;
            };

            using smelting = smelting_types<0>;
            using blasting = smelting_types<1>;
            using smoking = smelting_types<2>;
            using campfire_cooking = smelting_types<3>;

            struct stonecutting {
                slot_display ingredient;
                slot_display result;
                slot_display crafting_station;
            };

            struct smithing_transform {
                slot_display _template;
                slot_display base;
                slot_display addition;
                slot_display result;
                slot_display crafting_station;
            };

            struct smithing_trim {
                slot_display _template;
                slot_display base;
                slot_display addition;
                slot_display crafting_station;
            };

            struct crafting_transmute {
                slot_display input;
                slot_display material;
                slot_display result;
                slot_display crafting_station;
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
            constexpr static inline bool must_display = true;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_shapeless> {
            constexpr static inline const char* name = "minecraft:crafting_shapeless";
            constexpr static inline bool must_display = true;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_armordye> {
            constexpr static inline const char* name = "minecraft:crafting_special_armordye";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_bookcloning> {
            constexpr static inline const char* name = "minecraft:crafting_special_bookcloning";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_mapcloning> {
            constexpr static inline const char* name = "minecraft:crafting_special_mapcloning";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_mapextending> {
            constexpr static inline const char* name = "minecraft:crafting_special_mapextending";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_firework_rocket> {
            constexpr static inline const char* name = "minecraft:crafting_special_firework_rocket";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_firework_star> {
            constexpr static inline const char* name = "minecraft:crafting_special_firework_star";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_firework_star_fade> {
            constexpr static inline const char* name = "minecraft:crafting_special_firework_star_fade";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_tippedarrow> {
            constexpr static inline const char* name = "minecraft:crafting_special_tippedarrow";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_bannerduplicate> {
            constexpr static inline const char* name = "minecraft:crafting_special_bannerduplicate";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_shielddecoration> {
            constexpr static inline const char* name = "minecraft:crafting_special_shielddecoration";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_special_shulkerboxcoloring> { //deprecated
            constexpr static inline const char* name = "minecraft:crafting_special_shulkerboxcoloring";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = true;
        };

        template <>
        struct variant_data<minecraft::crafting_special_suspiciousstew> { //deprecated
            constexpr static inline const char* name = "minecraft:crafting_special_suspiciousstew";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = true;
        };

        template <>
        struct variant_data<minecraft::crafting_special_repairitem> {
            constexpr static inline const char* name = "minecraft:crafting_special_repairitem";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_decorated_pot> {
            constexpr static inline const char* name = "minecraft:crafting_decorated_pot";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::smelting> {
            constexpr static inline const char* name = "minecraft:smelting";
            constexpr static inline bool must_display = true;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::blasting> {
            constexpr static inline const char* name = "minecraft:blasting";
            constexpr static inline bool must_display = true;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::smoking> {
            constexpr static inline const char* name = "minecraft:smoking";
            constexpr static inline bool must_display = true;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::campfire_cooking> {
            constexpr static inline const char* name = "minecraft:campfire_cooking";
            constexpr static inline bool must_display = true;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::stonecutting> {
            constexpr static inline const char* name = "minecraft:stonecutting";
            constexpr static inline bool must_display = true;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::smithing_transform> {
            constexpr static inline const char* name = "minecraft:smithing_transform";
            constexpr static inline bool must_display = true;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::smithing_trim> {
            constexpr static inline const char* name = "minecraft:smithing_trim";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<minecraft::crafting_transmute> {
            constexpr static inline const char* name = "minecraft:crafting_transmute";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
        };

        template <>
        struct variant_data<custom> {
            constexpr static inline const char* name = "copper_server:custom";
            constexpr static inline bool must_display = false;
            constexpr static inline bool is_deprecated = false;
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
            minecraft::crafting_transmute,
            custom>;
    }

    struct recipe {
        recipes::varies data;
        std::string full_id;
        std::string name;
        std::string group;
        std::string category;
        std::vector<
            std::variant<
                std::string,
                std::vector<std::string>>>
            ingredients;
        uint32_t id;
        bool show_notification;
        bool highlight_as_new;
    };
}
#endif /* SRC_BASE_OBJECTS_RECIPE */
