#include <src/base_objects/item_predicate.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server::base_objects {
    void item_predicate::from_nbt_base_data(util::nbt_collection::compound_flex<std::unordered_map>& collector) {
        collector.collect("items", [this](util::nbt_read_stream& items_stream) {
            if (items_stream.get_type() == util::nbt_type::tag_string) {
                std::string res;
                items_stream.read_into(res);
                items = api::id::set::item(res);
            } else if (items_stream.get_type() == util::nbt_type::tag_list) {
                std::vector<api::id::item> res;
                items_stream.iterate([&res](size_t size) { res.reserve(size); }, [&res](util::nbt_read_stream& item_stream) {
                        std::string tmp;
                        item_stream.read_into(tmp);
                        res.emplace_back(tmp); });
                items = std::move(res);
            } else
                throw std::runtime_error("Invalid encoding");
        });
        collector.collect("count", [this](util::nbt_read_stream& count_stream) {
            if (count_stream.get_type() == util::nbt_type::tag_int) {
                int32_t res;
                count_stream.read_into(res);
                count = res;
            } else if (count_stream.get_type() == util::nbt_type::tag_compound) {
                count_minmax res;
                count_stream.iterate([&res](std::string_view name, util::nbt_read_stream& value_stream) {
                    if (name == "min") {
                        int32_t tmp;
                        value_stream.read_into(tmp);
                        res.min = tmp;
                    } else if (name == "max") {
                        int32_t tmp;
                        value_stream.read_into(tmp);
                        res.max = tmp;
                    } else
                        throw std::runtime_error("Invalid encoding");
                });
                count = std::move(res);
            } else
                throw std::runtime_error("Invalid encoding");
        });
        collector.collect_iterate("components", [this](auto& name, auto& stream) {
            base_objects::component com;
            com.parse_component(com, name, stream);
            components[com.get_id()] = std::move(com);
        });
        collector.collect_into("predicates", predicates);
    }

    void item_predicate::to_nbt_base_data(util::nbt_write_compound_stream& writer) {
        if (items.has_value()) {
            writer.write("items", [this](util::nbt_write_stream& items_stream) {
                std::visit(
                    [this, &items_stream]<class T>(const T& items) {
                        if constexpr (std::is_same_v<T, api::id::set::item>)
                            items_stream.write(items.to_string());
                        else {
                            auto list = items_stream.write_list();
                            for (const auto& item : items)
                                list.write(item.to_string());
                        }
                    },
                    items.value()
                );
            });
        }
        if (count.has_value()) {
            writer.write("count", [this](util::nbt_write_stream& count_stream) {
                std::visit(
                    [this, &count_stream]<class T>(const T& count) {
                        if constexpr (std::is_same_v<T, int32_t>)
                            count_stream.write(count);
                        else {
                            auto comp = count_stream.write_compound();
                            if (count.min.has_value())
                                comp.write("min", count.min.value());
                            if (count.max.has_value())
                                comp.write("max", count.max.value());
                        }
                    },
                    count.value()
                );
            });
        }
        if (components.size()) {
            writer.write("components", [this](util::nbt_write_stream& components_stream) {
                auto compound = components_stream.write_compound();
                for (auto& [_, component] : components)
                    component.encode_component(component, compound);
            });
        }
        if (predicates.get_type() != util::nbt_type::tag_end)
            writer.write("predicates", predicates);
    }

    void item_predicate::from_nbt(util::nbt_read_stream& stream) {
        util::nbt_collection::compound_flex flex;
        from_nbt_base_data(flex);
        flex.make_collect(stream);
    }

    void item_predicate::to_nbt(util::nbt_write_stream& stream) {
        auto writer = stream.write_compound();
        to_nbt_base_data(writer);
    }

    bool item_predicate::is_matches(base_objects::slot_data& slot) const {
        if (items.has_value()) {
            if (!std::visit(
                    [this, &slot]<class T>(const T& items) {
                        if constexpr (std::is_same_v<T, api::id::set::item>) {
                            return items.contains(slot.id);
                        } else {
                            for (const auto& item : items)
                                if (slot.id == item.value)
                                    return true;
                            return false;
                        }
                    },
                    items.value()
                ))
                return false;
        }
        if (count.has_value()) {
            std::visit(
                [this, &slot]<class T>(const T& count) {
                    if constexpr (std::is_same_v<T, int32_t>) {
                        if (slot.count != count)
                            return false;
                    } else {
                        if (count.min.has_value() && slot.count < count.min.value())
                            return false;
                        if (count.max.has_value() && slot.count > count.max.value())
                            return false;
                    }
                },
                count.value()
            );
        }
        if (components.size()) {
            for (auto& [id, component] : components) {
                if (slot.components.contains(id)) {
                    if (component != slot.components[id])
                        return false;
                } else
                    return false;
            }
        }
        //TODO implement data component predicates
        return true;
    }
}