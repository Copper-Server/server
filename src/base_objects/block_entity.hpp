#ifndef SRC_BASE_OBJECTS_BLOCK_ENTITY
#define SRC_BASE_OBJECTS_BLOCK_ENTITY
#include <src/base_objects/block.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/component.hpp>
#include <src/util/calculations.hpp>
#include <src/util/nbt.hpp>

namespace copper_server::util {
    class nbt_read_stream;
    class nbt_write_stream;

    namespace nbt_collection {
        template <template <class...> class map_base>
        class compound_flex;
    }

    class nbt_write_compound_stream;
}

namespace copper_server::base_objects {
    struct block_entity {
        std::unordered_map<int32_t, component> components;
        block id;
        int32_t x, y, z;
        bool keep_packed = false;

        block_entity();
        virtual ~block_entity();

        virtual std::unique_ptr<block_entity> clone() const = 0;

        void from_nbt_base_data(util::nbt_collection::compound_flex<std::unordered_map>& collector);
        void to_nbt_base_data(util::nbt_write_compound_stream& collector);
        virtual void to_nbt(util::nbt_write_stream& stream) = 0;

        template <class T>
        T& get_component() {
            return std::get<T>(components.at(T::item_id::value).type);
        }

        template <class T>
        T& access_component() {
            if (components.contains(T::item_id::value))
                return std::get<T>(components[T::item_id::value].type);
            else
                return std::get<T>(components[T::item_id::value].type = T{});
        }

        template <class T>
        const T& get_component() const {
            return std::get<T>(components.at(T::item_id::value).type);
        }

        template <class T>
        void remove_component() {
            components.erase(T::item_id::value);
        }

        void add_component(component&& copy) {
            std::visit(
                [this, &copy](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    components[T::item_id::value] = std::move(copy);
                },
                copy.type
            );
        }

        void add_component(const component& copy) {
            std::visit(
                [this, &copy](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    components[T::item_id::value] = copy;
                },
                copy.type
            );
        }

        template <class T>
        void add_component(const T& copy) {
            components[T::item_id::value].type = copy;
        }

        template <class T>
        void add_component(T&& copy) {
            components[T::item_id::value].type = std::move(copy);
        }

        template <class T>
        bool has_component() const {
            return components.contains(T::item_id::value);
        }

        inline bool is_tickable() const {
            return id.is_tickable();
        }

        inline bool is_solid() const {
            return id.is_solid();
        }

        const std::vector<shape_data*>& collision_shapes() const {
            return id.collision_shapes();
        }

        const base_objects::chat& display_name() const {
            return id.display_name();
        }

        const std::string& instrument() const {
            return id.instrument();
        }

        const std::string& piston_behavior() const {
            return id.piston_behavior();
        }

        const std::string& name() const {
            return id.name();
        }

        const std::string& translation_key() const {
            return id.translation_key();
        }

        inline block_id_t general_block_id() const {
            return id.general_block_id();
        }

        inline float slipperiness() const {
            return id.slipperiness();
        }

        inline float velocity_multiplier() const {
            return id.velocity_multiplier();
        }

        inline float jump_velocity_multiplier() const {
            return id.jump_velocity_multiplier();
        }

        inline float hardness() const {
            return id.hardness();
        }

        inline float blast_resistance() const {
            return id.blast_resistance();
        }

        inline int32_t map_color_rgb() const {
            return id.map_color_rgb();
        }

        inline int32_t block_entity_id() const {
            return id.block_entity_id();
        }

        inline int32_t item_id() const {
            return id.item_id();
        }

        inline int32_t experience() const {
            return id.experience();
        }

        inline block_id_t default_state() const {
            return id.default_state();
        }

        inline uint8_t luminance() const {
            return id.luminance();
        }

        inline uint8_t opacity() const {
            return id.opacity();
        }

        inline bool is_air() const {
            return id.is_air();
        }

        inline bool is_liquid() const {
            return id.is_liquid();
        }

        inline bool is_burnable() const {
            return id.is_burnable();
        }

        inline bool is_emits_redstone() const {
            return id.is_emits_redstone();
        }

        inline bool is_full_cube() const {
            return id.is_full_cube();
        }

        inline bool is_tool_required() const {
            return id.is_tool_required();
        }

        inline bool is_sided_transparency() const {
            return id.is_sided_transparency();
        }

        inline bool is_replaceable() const {
            return id.is_replaceable();
        }

        inline bool is_block_entity() const {
            return id.is_block_entity();
        }

        struct container_lock;
        struct test_instance;
        struct sculk_catalyst;
        struct mob_spawner_entry;
    };

    struct viewer_count_manager {
    };

    struct block_entity::container_lock {
    };

    struct block_entity::test_instance {
        struct error {
            util::xyz<int32_t> pos;
            chat text;
        };
    };

    struct block_entity::test_instance {
        struct error {
            util::xyz<int32_t> pos;
            chat text;
        };
    };

    struct block_entity::sculk_catalyst {
        struct listener {
        };
    };

    struct block_entity::mob_spawner_entry {
    };

    struct vibration_listener {
    };

    struct any_block : public std::variant<block, std::unique_ptr<block_entity>> {
        using std::variant<block, std::unique_ptr<block_entity>>::variant;
        using std::variant<block, std::unique_ptr<block_entity>>::operator=;
    };
}

#endif /* SRC_BASE_OBJECTS_BLOCK_ENTITY */
