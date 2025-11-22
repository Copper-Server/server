#include <src/base_objects/block_entity.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server::base_objects {
    block_entity::block_entity() = default;
    block_entity::~block_entity() = default;

    void block_entity::from_nbt_base_data(util::nbt_collection::compound_flex& collector) {
        collector.collect_as_required("id", id.id);
        collector.collect_as_required("x", x);
        collector.collect_as_required("y", y);
        collector.collect_as_required("z", z);
        collector.collect_as("keepPacked", keep_packed);
        collector.collect_iterate_required("components", [this](const std::string& name, util::nbt_read_stream& component_stream) {
            base_objects::component res;
            base_objects::component::parse_component(res, name, component_stream);
            components[res.get_id()] = std::move(res);
        });
    }

    void block_entity::to_nbt_base_data(util::nbt_write_compound_stream& collector) {
        collector
            .write("id", id.id)
            .write("x", x)
            .write("y", y)
            .write("z", z)
            .write("keepPacked", keep_packed)
            .write("components", [this](util::nbt_write_stream& components_stream) {
                auto compound = components_stream.write_compound();
                for (auto& [_, component] : components)
                    component.encode_component(component, compound);
            });
    }

    void block_entity::mob_spawner_entry::equipment_t::from_nbt_base_data(util::nbt_collection::compound_flex& collector) {
        collector.collect_as_required("loot_table", loot_table);
        collector.collect("slot_drop_chances", [this](util::nbt_read_stream& slot_drop_chances_stream) {
            if (slot_drop_chances_stream.get_type() == util::nbt_type::tag_float) {
                float res;
                slot_drop_chances_stream.read_into(res);
                slot_drop_chances = res;
            } else if (slot_drop_chances_stream.get_type() == util::nbt_type::tag_compound) {
                chances_t res;
                slot_drop_chances_stream.iterate([&res](std::string_view name, util::nbt_read_stream& value_stream) {
                    if (name == "feet") {
                        float tmp;
                        value_stream.read_into(tmp);
                        res.feet = tmp;
                    } else if (name == "legs") {
                        float tmp;
                        value_stream.read_into(tmp);
                        res.legs = tmp;
                    } else if (name == "chest") {
                        float tmp;
                        value_stream.read_into(tmp);
                        res.chest = tmp;
                    } else if (name == "head") {
                        float tmp;
                        value_stream.read_into(tmp);
                        res.head = tmp;
                    } else if (name == "body") {
                        float tmp;
                        value_stream.read_into(tmp);
                        res.body = tmp;
                    } else if (name == "mainhand") {
                        float tmp;
                        value_stream.read_into(tmp);
                        res.mainhand = tmp;
                    } else if (name == "offhand") {
                        float tmp;
                        value_stream.read_into(tmp);
                        res.offhand = tmp;
                    } else
                        throw std::runtime_error("Invalid encoding");
                });
                slot_drop_chances = std::move(res);
            } else
                throw std::runtime_error("Invalid encoding");
        });
    }

    void block_entity::mob_spawner_entry::equipment_t::to_nbt_base_data(util::nbt_write_compound_stream& collector) {
        collector.write("loot_table", loot_table);
        if (slot_drop_chances.has_value()) {
            collector.write("slot_drop_chances", [this](util::nbt_write_stream& slot_drop_chances_stream) {
                std::visit(
                    [this, &slot_drop_chances_stream]<class T>(const T& slot_drop_chances) {
                        if constexpr (std::is_same_v<T, float>)
                            slot_drop_chances_stream.write(slot_drop_chances);
                        else {
                            auto comp = slot_drop_chances_stream.write_compound();
                            if (slot_drop_chances.feet.has_value())
                                comp.write("feet", slot_drop_chances.feet.value());
                            if (slot_drop_chances.legs.has_value())
                                comp.write("legs", slot_drop_chances.legs.value());
                            if (slot_drop_chances.chest.has_value())
                                comp.write("chest", slot_drop_chances.chest.value());
                            if (slot_drop_chances.head.has_value())
                                comp.write("head", slot_drop_chances.head.value());
                            if (slot_drop_chances.body.has_value())
                                comp.write("body", slot_drop_chances.body.value());
                            if (slot_drop_chances.mainhand.has_value())
                                comp.write("mainhand", slot_drop_chances.mainhand.value());
                            if (slot_drop_chances.offhand.has_value())
                                comp.write("offhand", slot_drop_chances.offhand.value());
                        }
                    },
                    slot_drop_chances.value()
                );
            });
        }
    }

    void block_entity::mob_spawner_entry::equipment_t::from_nbt(util::nbt_read_stream& stream) {
        util::nbt_collection::compound_flex flex;
        from_nbt_base_data(flex);
        flex.make_collect(stream);
    }

    void block_entity::mob_spawner_entry::equipment_t::to_nbt(util::nbt_write_stream& stream) {
        auto writer = stream.write_compound();
        to_nbt_base_data(writer);
    }
}