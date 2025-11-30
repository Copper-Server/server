#include <src/api/ecs/block_entity_components.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server::api::ecs::com::block_entity {
    void base_data::to_nbt(util::nbt_write_compound_stream& stream) const {
        stream
            .write("id", id.id)
            .write("x", x)
            .write("y", y)
            .write("z", z);
        if (keep_packed)
            stream.write("keepPacked", keep_packed);
        stream
            .write("components", [this](util::nbt_write_stream& components_stream) {
                auto compound = components_stream.write_compound();
                for (auto& [_, component] : components)
                    component.encode_component(component, compound);
            });
    }

    void base_data::from_nbt(util::nbt_collection::compound_flex& stream) {
        stream
            .collect_as_required("id", id.id)
            .collect_as_required("x", x)
            .collect_as_required("y", y)
            .collect_as_required("z", z)
            .collect_as("keepPacked", keep_packed)
            .collect_iterate_required("components", [this](const std::string& name, util::nbt_read_stream& component_stream) {
                base_objects::component tmp;
                base_objects::component::parse_component(tmp, name, component_stream);
                components[tmp.get_id()] = std::move(tmp);
            });
    }
}