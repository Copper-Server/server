#ifndef SRC_API_ECS_ENTITY_DEFINITION
#define SRC_API_ECS_ENTITY_DEFINITION
#include <src/api/ecs.hpp>

namespace copper_server::util {
    class nbt_read_stream;
    class nbt_write_stream;
    class nbt_write_compound_stream;

    namespace nbt_collection {
        class compound_flex;
    }
}

namespace copper_server::api::ecs {
    namespace detail {
        template <class T, class = void>
        struct has_nbt_path : std::false_type {};

        template <class T>
        struct has_nbt_path<T, std::void_t<decltype(T::nbt_path::value)>> : std::true_type {};

        template <class T>
        concept has_nbt_fields_exclusive = requires(T& it, util::nbt_write_stream& write, util::nbt_read_stream& read) {
            it.to_nbt(write);
            it.from_nbt(read);
        };

        template <class T>
        concept has_nbt_fields_shared = requires(T& it, util::nbt_write_compound_stream& write, util::nbt_collection::compound_flex& read) {
            it.to_nbt(write);
            it.from_nbt(read);
        };

        template <class T>
        constexpr std::string_view get_nbt_path_v() {
            if constexpr (has_nbt_path<T>::value)
                return T::nbt_path::value;
            else
                return "";
        }

        template <class T>
        void serialize_via_fields(void* data, util::nbt_write_stream& stream) {
            static_cast<T*>(data)->to_nbt(stream);
        }

        template <class T>
        void serialize_via_fields_shared(void* data, util::nbt_write_compound_stream& stream) {
            static_cast<T*>(data)->to_nbt(stream);
        }

        template <class T>
        void serialize_via_auto(void* data, util::nbt_write_stream& stream);

        template <class T>
        void deserialize_via_fields(void* data, util::nbt_write_stream& stream) {
            static_cast<T*>(data)->from_nbt(stream);
        }

        template <class T>
        void deserialize_via_fields_shared(void* data, util::nbt_collection::compound_flex& stream) {
            static_cast<T*>(data)->from_nbt(stream);
        }

        template <class T>
        void deserialize_via_auto(void* data, util::nbt_write_stream& stream);
    }

    struct entity_construction;

    struct entity_definition {
        enum class component_remove_act {
            optional,
            locked,
            reset_on_remove,
        };

        struct component_rule {
            component_id id;
            component_remove_act remove_action = component_remove_act::optional;
        };

    private:
        using nbt_serialize_t = void (*)(void* data, util::nbt_write_stream&);
        using nbt_deserialize_t = void (*)(void* data, util::nbt_read_stream&);

        using nbt_serialize_shared_t = void (*)(void* data, util::nbt_write_compound_stream&);
        using nbt_deserialize_shared_t = void (*)(void* data, util::nbt_collection::compound_flex&);

        struct component_node_info {
            component_id id;
            bool is_marker = false;
            bool can_inject = false;


            nbt_serialize_t serializer = nullptr;
            nbt_serialize_shared_t serializer_shared = nullptr;
            nbt_deserialize_t deserializer = nullptr;
            nbt_deserialize_shared_t deserializer_shared = nullptr;
        };

        struct nbt_schema_node {
            std::string name;
            std::vector<std::unique_ptr<nbt_schema_node>> children;
            std::vector<component_node_info> components;

            bool injectable = true;//once the node uses exclusive operation the node becomes non injectable

            nbt_schema_node* get_or_create_child(const std::string& name);
        };

        enum class opcode_e {
            start_compound,
            end_compound,

            write,
            write_inject,

            write_mark_true,
            write_mark_false,
        };

        struct nbt_schema_instruction {
            opcode_e opcode;
            uint32_t name_index;
            size_t chunk_offset;
            size_t component_size;

            union fn_t {
                nbt_serialize_t serializer = nullptr;
                nbt_serialize_shared_t serializer_shared;
                fn_t() = default;
                fn_t(nbt_serialize_t fn) : serializer(fn) {}
                fn_t(nbt_serialize_shared_t fn) : serializer_shared(fn) {}

            } fn;
        };

        struct serialization_plan {
            std::vector<nbt_schema_instruction> instructions;
            std::vector<std::string> string_table;
        };

        std::string identifier;
        entity_recipe base_recipe;
        entity_recipe stripped_recipe;
        std::vector<component_id> stripped_ids;
        std::vector<component_rule> rules;

        std::unordered_map<component_id, size_t> rule_lookup;

        std::unique_ptr<nbt_schema_node> schema_root;

        mutable fast_task::task_mutex plan_mutex;
        mutable std::unordered_map<size_t, std::shared_ptr<serialization_plan>> cached_plans;


        std::shared_ptr<serialization_plan> compile_plan(size_t id, const detail::archetype_layout& layout) const;
        void deserialize_recursive(entity_construction& build, const nbt_schema_node* node, util::nbt_read_stream& stream) const;

        void flatten_tree(const nbt_schema_node* node, const detail::archetype_layout& layout, serialization_plan& plan, bool prev_is_complex) const;

        static std::vector<std::string> parse_path(std::string_view path);

        template <class T>
        entity_definition& map_component(const std::string& nbt_path) {
            auto parts = parse_path(nbt_path);
            nbt_schema_node* current = schema_root.get();
            for (const auto& part : parts)
                current = current->get_or_create_child(part);

            component_node_info info;
            info.id = detail::get_component_id<T>();

            if constexpr (std::is_empty_v<T>)
                info.is_marker = true;
            else {
                if constexpr (std::is_empty_v<T>)
                    info.is_marker = true;
                else if constexpr (detail::has_nbt_fields_exclusive<T>::value) {
                    info.serializer = &detail::serialize_via_fields<T>;
                    info.deserializer = &detail::deserialize_via_fields<T>;
                } else if constexpr (detail::has_nbt_fields_shared<T>::value) {
                    info.serializer_shared = &detail::serialize_via_fields_shared<T>;
                    info.deserializer_shared = &detail::deserialize_via_fields_shared<T>;
                } else {
                    info.serializer = &detail::serialize_via_auto<T>;
                    info.deserializer = &detail::deserialize_via_auto<T>;
                }
            }
            if (current->injectable){
                if (info.serializer || info.deserializer || info.is_marker) {
                    if (current->components.size() > 0 || current->children.size() > 0)
                        throw std::runtime_error("Failed to take ownership for shared node entry " + std::to_string(info.id) + " for path: " + nbt_path);
                    else 
                        current->injectable = false;
                }
            }
            current->components.push_back(info);
            return *this;
        }

        template <class T>
        void try_auto_map() {
            if constexpr (detail::has_nbt_path<T>::value)
                map_component<T>(std::string(detail::get_nbt_path_v<T>()));
        }

    public:
        entity_definition(const std::string& id);
        entity_definition(std::string&& id);
        entity_definition(const entity_definition&) = delete;
        entity_definition(entity_definition&&) = delete;
        entity_definition& operator=(const entity_definition&) = delete;
        entity_definition& operator=(entity_definition&&) = delete;

        const std::string& get_identifier() const;

        template <class T>
        entity_definition& add_locked() {
            base_recipe.with<T>();
            stripped_recipe.with<T>();
            auto id = detail::get_component_id<T>();
            auto res = rules.emplace_back(component_rule{id, component_remove_act::locked});
            rule_lookup[id] = &res;
            try_auto_map<T>();
            return *this;
        }

        template <class T>
        entity_definition& add_locked(const T& value) {
            base_recipe.with_value<T>(value);
            stripped_recipe.with_value<T>(value);
            auto id = detail::get_component_id<T>();
            auto res = rules.emplace_back(component_rule{id, component_remove_act::locked});
            rule_lookup[id] = &res;
            try_auto_map<T>();
            return *this;
        }

        template <class T>
        entity_definition& add_optional() {
            auto id = detail::get_component_id<T>();
            auto res = rules.emplace_back(component_rule{id, component_remove_act::optional});
            rule_lookup[id] = &res;
            try_auto_map<T>();
            return *this;
        }

        template <class T>
        entity_definition& add_reset_on_remove() {
            base_recipe.with<T>();
            stripped_recipe.with<T>();
            auto id = detail::get_component_id<T>();
            auto res = rules.emplace_back(component_rule{id, component_remove_act::reset_on_remove});
            rule_lookup[id] = &res;
            try_auto_map<T>();
            return *this;
        }

        template <class T>
        entity_definition& add_reset_on_remove(const T& value) {
            base_recipe.with_value<T>();
            stripped_recipe.with_value<T>();
            auto id = detail::get_component_id<T>();
            auto res = rules.emplace_back(component_rule{id, component_remove_act::reset_on_remove});
            rule_lookup[id] = &res;
            try_auto_map<T>();
            return *this;
        }

        template <class T>
        entity_definition& add_tag(int32_t value) {
            base_recipe.with_tag<T>(value);
            stripped_recipe.with_tag<T>(value);
            return *this;
        }

        template <class T>
        entity_definition& add_tag(uint32_t value) {
            base_recipe.with_tag<T>(value);
            stripped_recipe.with_tag<T>(value);
            return *this;
        }

        template <class T>
        entity_definition& add_stripable() {
            base_recipe.with<T>();
            auto id = detail::get_component_id<T>();
            stripped_ids.push_back(id);
            auto res = rules.emplace_back(component_rule{id, component_remove_act::optional});
            rule_lookup[id] = &res;
            try_auto_map<T>();
            return *this;
        }

        template <class T>
        entity_definition& add_stripable(T value) {
            base_recipe.with_value<T>(value);
            auto id = detail::get_component_id<T>();
            stripped_ids.push_back(id);
            auto res = rules.emplace_back(component_rule{id, component_remove_act::optional});
            rule_lookup[id] = &res;
            try_auto_map<T>();
            return *this;
        }

        component_remove_act get_remove_action(component_id id) const;
        const entity_recipe& get_recipe() const;
        const entity_recipe& get_stripped_recipe() const;

        void to_nbt(util::nbt_write_stream& stream, entity entity) const;
        entity from_nbt(util::nbt_read_stream& stream, std::optional<world*> world_opt = std::nullopt) const;

        void strip(entity entity);
        void unstrip(entity entity);

        void finish();
    };

    //the ids should be always define the type of entity and then minecraft style identitier
    //ex:
    //  @block_entity:minecraft:chest
    //  @entity:minecraft:player
    const entity_definition& get_ecs_entity_definition(const std::string& id);

    const entity_definition& get_entity_definition(const std::string& id);
    const entity_definition& get_block_entity_definition(const std::string& id);


    namespace initialization {
        //this should be used only during initialization, this allows to extend the recipes during initialization
        // ids should be same as for get_ecs_entity_definition
        entity_definition& make_ecs_entity_definition(const std::string& id);

        entity_definition& make_entity_definition(const std::string& id);
        entity_definition& make_block_entity_definition(const std::string& id);
    }
}

// Bridge to serialization implementation
#include <src/util/encoding/nbt/deserialization.hpp>
#include <src/util/encoding/nbt/serialization.hpp>

namespace copper_server::api::ecs::detail {
    template <class T>
    void serialize_via_auto(void* data, util::nbt_write_stream& stream) {
        util::encoding::nbt::serialize_entry(stream, *static_cast<T*>(data));
    }

    template <class T>
    void deserialize_via_auto(void* data, util::nbt_write_stream& stream) {
        util::encoding::nbt::deserialize_entry(*static_cast<T*>(data), stream);
    }
}

#endif /* SRC_API_ECS_ENTITY_DEFINITION */
