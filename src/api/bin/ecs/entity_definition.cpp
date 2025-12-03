#include <src/api/bin/ecs/manager.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/ecs/entity_construction.hpp>
#include <src/api/ecs/entity_definition.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server::api::ecs {

    entity_definition::entity_definition(const std::string& id) : identifier(id) {
        add_locked(ecs::com::type_definition(this));
    }

    entity_definition::entity_definition(std::string&& id) : identifier(std::move(id)) {
        add_locked(ecs::com::type_definition(this));
    }

    const std::string& entity_definition::get_identifier() const {
        return identifier;
    }

    auto entity_definition::get_remove_action(component_id id) const -> component_remove_act {
        if (rule_lookup.contains(id))
            return rules[rule_lookup.at(id)].remove_action;
        return component_remove_act::optional;
    }

    const entity_recipe& entity_definition::get_recipe() const {
        return base_recipe;
    }

    const entity_recipe& entity_definition::get_stripped_recipe() const {
        return stripped_recipe;
    }

    static std::vector<std::string> parse_path(std::string_view path) {
        std::vector<std::string> parts;
        if (path.empty())
            return parts;
        std::string current;
        bool in_quote = false;
        for (char c : path) {
            if (c == '\'')
                in_quote = !in_quote;
            else if (c == '.' && !in_quote) {
                parts.push_back(current);
                current.clear();
            } else
                current += c;
        }
        if (!current.empty())
            parts.push_back(current);
        return parts;
    }

    entity_definition::nbt_schema_node* entity_definition::nbt_schema_node::get_or_create_child(const std::string& name) {
        for (auto& child : children) {
            if (child->name == name)
                return child.get();
        }
        auto new_node = std::make_unique<nbt_schema_node>();
        new_node->name = name;
        children.push_back(std::move(new_node));
        return children.back().get();
    }

    entity_definition::entity_definition(const std::string& id)
        : identifier(id), schema_root(std::make_unique<nbt_schema_node>()) {}

    entity_definition::entity_definition(std::string&& id)
        : identifier(std::move(id)), schema_root(std::make_unique<nbt_schema_node>()) {}

    void entity_definition::finish() {
        base_recipe.freeze();
    }

    void entity_definition::flatten_tree(
        const nbt_schema_node* node,
        const detail::archetype_layout& layout,
        serialization_plan& plan,
        bool prev_is_complex
    ) const {
        std::vector<const component_node_info*> active_components;
        for (const auto& comp_info : node->components) {
            if (layout.component_index_map.contains(comp_info.id))
                active_components.push_back(&comp_info);
        }
        bool is_complex = active_components.size() > 1;
        bool is_root = node->name.empty();

        if (is_complex && !is_root) {
            plan.string_table.push_back(node->name);
            uint32_t name_idx = (uint32_t)plan.string_table.size() - 1;

            if (!prev_is_complex)
                plan.instructions.emplace_back(opcode_e::start_compound, name_idx, 0, 0);

            for (const auto* info : active_components) {
                uint32_t comp_idx = layout.component_index_map.at(info->id);
                plan.instructions.emplace_back(
                    opcode_e::write_inject,
                    0,
                    layout.component_offsets[comp_idx],
                    detail::component_info_registry[info->id].size,
                    info->serializer_shared
                );
            }
        } else {
            if (active_components.size() == 1) {
                const auto* info = active_components[0];

                plan.string_table.push_back(node->name);
                uint32_t name_idx = (uint32_t)plan.string_table.size() - 1;

                if (info->is_marker)
                    plan.instructions.push_back({opcode_e::write_mark_false, name_idx, 0, 0});
                else {
                    uint32_t comp_idx = layout.component_index_map.at(info->id);
                    plan.instructions.emplace_back(opcode_e::write, name_idx, layout.component_offsets[comp_idx], detail::component_info_registry[info->id].size, info->serializer);
                }
            }
        }

        for (const auto& child : node->children)
            flatten_tree(child.get(), layout, plan, is_complex);

        if (!is_root && is_complex && !prev_is_complex)
            plan.instructions.push_back({opcode_e::end_compound, 0, 0, 0});
    }

    std::shared_ptr<entity_definition::serialization_plan> entity_definition::compile_plan(size_t archetype_id, const detail::archetype_layout& layout) const {
        for (const auto& rule : rules) {
            if ((rule.remove_action != component_remove_act::optional)
                && layout.component_index_map.find(rule.id) == layout.component_index_map.end()) {
                throw std::runtime_error("Archetype missing required serialization component ID: " + std::to_string(rule.id));
            }
        }

        auto plan = std::make_shared<serialization_plan>();

        for (const auto& child : schema_root->children)
            flatten_tree(child.get(), layout, *plan, false);

        return plan;
    }

    void entity_definition::to_nbt(util::nbt_write_stream& stream, entity entity) const {
        assert(base_recipe.is_frozen() && "entity_definition was used before finish() was called.");
        size_t arch_id = detail::get_entity_archetype_id(entity.id, entity.generation);
        if (arch_id == 0)
            return;

        std::shared_ptr<serialization_plan> plan;
        {
            fast_task::lock_guard lock(plan_mutex);
            auto it = cached_plans.find(arch_id);
            if (it != cached_plans.end()) {
                plan = it->second;
            } else {
                auto layout = detail::get_archetype_layout(entity.id, entity.generation);
                plan = compile_plan(arch_id, layout);
                cached_plans[arch_id] = plan;
            }
        }

        if (plan->instructions.empty())
            return;

        auto& record = manager::instance().records.at(entity.id);
        char* chunk_base = record.chunk->memory_block.get();
        size_t entity_idx = record.chunk_index;

        auto it = plan->instructions.begin();
        auto end = plan->instructions.end();

        auto root_comp = stream.write_compound();

        auto exec = [&](this auto& self, util::nbt_write_compound_stream& c_stream) {
            while (it != end) {
                const auto& op = *it;

                if (op.opcode == opcode_e::end_compound) {
                    it++;
                    return;
                }

                const std::string& name = plan->string_table[op.name_index];

                if (op.opcode == opcode_e::start_compound) {
                    it++;
                    c_stream.write(name, [&](util::nbt_write_stream& inner) {
                        auto sub = inner.write_compound();
                        self(sub);
                    });
                } else if (op.opcode == opcode_e::write) {
                    void* ptr = chunk_base + op.chunk_offset + (entity_idx * op.component_size);
                    c_stream.write(name, [&](util::nbt_write_stream& inner) {
                        op.fn.serializer(ptr, inner);
                    });
                    it++;
                } else if (op.opcode == opcode_e::write_inject) {
                    void* ptr = chunk_base + op.chunk_offset + (entity_idx * op.component_size);
                    op.fn.serializer_shared(ptr, c_stream);
                } else if (op.opcode == opcode_e::write_mark_true) {
                    c_stream.write(name, (int8_t)1);
                    it++;
                } else if (op.opcode == opcode_e::write_mark_false) {
                    c_stream.write(name, (int8_t)0);
                    it++;
                }
            }
        };

        exec(root_comp);
    }

    void entity_definition::deserialize_recursive(
        entity_construction& build,
        const nbt_schema_node* node,
        util::nbt_read_stream& stream
    ) const {
        if (node->components.size() == 1 && node->children.empty()) {
            const auto& info = node->components[0];
            if (info.is_marker) {
                bool tmp;
                stream.read_as(tmp);
                if (tmp)
                    build.get_raw_or_create(info.id);
            } else if (info.can_inject) {
                util::nbt_collection::compound_flex collector;
                info.deserializer_shared(build.get_raw_or_create(info.id), collector);
                collector.make_collect(stream);
            } else if (info.deserializer)
                info.deserializer(build.get_raw_or_create(info.id), stream);
        } else {
            util::nbt_collection::compound_flex collector;
            for (const auto& info : node->components)
                if (info.deserializer_shared)
                    info.deserializer_shared(build.get_raw_or_create(info.id), collector);

            for (const auto& child : node->children)
                collector.collect(child->name, [&](util::nbt_read_stream& sub_stream) {
                    deserialize_recursive(build, child.get(), sub_stream);
                });
            collector.make_collect(stream);
        }
    }

    entity entity_definition::from_nbt(util::nbt_read_stream& stream, std::optional<world*> world) const {
        assert(base_recipe.is_frozen() && "entity_definition was used before finish() was called.");
        entity_construction construct;
        deserialize_recursive(construct, schema_root.get(), stream);
        construct.emplace<com::type_definition>(this);
        return std::move(construct).create_and_wait(base_recipe, world);
    }

    void entity_definition::strip(entity entity) {
        if (!entity.is_valid())
            return;
        for (auto& it : stripped_ids)
            detail::queue_remove_entity_component(entity.id, entity.generation, it);
    }

    void entity_definition::unstrip(entity entity) {
        if (!entity.is_valid())
            return;

        auto& defaults = base_recipe.get_defaults();
        for (auto& it : stripped_ids) {
            auto& info = detail::component_info_registry.at(it);
            detail::mutation_queue_item queue{entity.id, entity.generation, it};
            queue.data.resize(info.size);
            if (auto default_it = defaults.find(it); defaults.end() != default_it) {
                info.construct(queue.data.data());
                info.copy_assign(queue.data.data(), default_it->second);
            } else
                info.construct(queue.data.data());
            detail::queue_command(std::move(queue));
        }
    }

    void entity_definition::finish() {
        base_recipe.freeze();
        stripped_recipe.freeze();
    }

    static auto& get_entity_definitions() {
        static std::unordered_map<std::string, entity_definition> res;
        return res;
    }

    const entity_definition& get_ecs_entity_definition(const std::string& id) {
        return get_entity_definitions().at(id);
    }

    const entity_definition& get_entity_definition(const std::string& id) {
        return get_ecs_entity_definition("@entity:" + id);
    }

    const entity_definition& get_block_entity_definition(const std::string& id) {
        return get_ecs_entity_definition("@block_entity:" + id);
    }

    namespace initialization {
        entity_definition& make_ecs_entity_definition(const std::string& id) {
            auto res = get_entity_definitions().emplace(id, id);
            return res.first->second;
        }

        entity_definition& make_entity_definition(const std::string& id) {
            return make_ecs_entity_definition("@entity:" + id);
        }

        entity_definition& make_block_entity_definition(const std::string& id) {
            return make_ecs_entity_definition("@block_entity:" + id);
        }

        void finish_definitions() {
            for (auto& [id, def] : get_entity_definitions())
                def.finish();
        }
    }
}