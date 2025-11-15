#include <src/base_objects/block_entity.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server::base_objects {
    block_entity::block_entity() = default;
    block_entity::~block_entity() = default;

    void block_entity::from_nbt_base_data(util::nbt_collection::compound_flex<std::unordered_map>& collector) {
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
}