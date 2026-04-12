/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace pt = boost::property_tree;

// Data structures to hold parsed information
struct MetadataField {
    std::string component_name;
    std::string field_name;
    std::string type_name;
    int32_t network_id;
};

struct EntityInfo {
    int32_t id;
    std::string entity_name;
    std::string class_name;
    std::vector<MetadataField> metadata_fields;
};

std::string sanitize_name(std::string name) {
    boost::replace_first(name, "minecraft:", "");
    return name;
}

std::string to_lower_case(std::string name) {
    return boost::to_lower_copy(name);
}

std::map<std::string, std::pair<std::string, std::string>> type_map = {
    {"Float", {"float", "base_objects::entity_metadata::float_"}},
    {"Boolean", {"bool", "base_objects::entity_metadata::boolean"}},
    {"Byte", {"int8_t", "base_objects::entity_metadata::byte"}},
    {"Integer", {"int32_t", "base_objects::entity_metadata::var_int"}},
    {"OptionalInt", {"base_objects::optional_var_int32", "base_objects::entity_metadata::optional_var_int"}},
    {"Long", {"int64_t", "base_objects::entity_metadata::var_long"}},
    {"String", {"std::string", "base_objects::entity_metadata::string"}},
    {"Text", {"base_objects::chat", "base_objects::entity_metadata::text_component"}},
    {"Optional<Text>", {"std::optional<base_objects::chat>", "base_objects::entity_metadata::optional_text_component"}},
    {"BlockPos", {"base_objects::position", "base_objects::entity_metadata::position"}},
    {"Optional<BlockPos>", {"std::optional<base_objects::position>", "base_objects::entity_metadata::optional_position"}},
    {"EntityPose", {"base_objects::entity_metadata::entity_pose", "base_objects::entity_metadata::entity_pose"}},
    {"ParticleEffect", {"base_objects::entity_metadata::particle", "base_objects::entity_metadata::particle"}},
    {"List<ParticleEffect>", {"list_array<base_objects::entity_metadata::particle>", "base_objects::entity_metadata::particles"}},
    {"ItemStack", {"base_objects::slot", "base_objects::entity_metadata::slot"}},
    {"Direction", {"base_objects::entity_metadata::direction", "base_objects::entity_metadata::direction"}},
    {"Optional<LazyEntityReference<LivingEntity>>", {"std::optional<base_objects::uuid>", "base_objects::entity_metadata::optional_living_entity_reference"}},
    {"Optional<BlockState>", {"base_objects::optional_var_int32::block_state", "base_objects::entity_metadata::optional_block_state"}},
    {"BlockState", {"base_objects::var_int32::block_state", "base_objects::entity_metadata::block_state"}},
    {"EulerAngle", {"base_objects::entity_metadata::rotations", "base_objects::entity_metadata::rotations"}},
    {"Vector3f", {"base_objects::entity_metadata::vector3", "base_objects::entity_metadata::vector3"}},
    {"Quaternionf", {"base_objects::entity_metadata::vector4", "base_objects::entity_metadata::vector4"}},
    {"VillagerData", {"base_objects::entity_metadata::villager_data", "base_objects::entity_metadata::villager_data"}},
    {"RegistryEntry<CatVariant>", {"base_objects::var_int32::cat_variant", "base_objects::entity_metadata::cat_variant"}},
    {"RegistryEntry<ChickenVariant>", {"base_objects::var_int32::chicken_variant", "base_objects::entity_metadata::chicken_variant"}},
    {"RegistryEntry<CowVariant>", {"base_objects::var_int32::cow_variant", "base_objects::entity_metadata::cow_variant"}},
    {"RegistryEntry<FrogVariant>", {"base_objects::var_int32::frog_variant", "base_objects::entity_metadata::frog_variant"}},
    {"RegistryEntry<PaintingVariant>", {"base_objects::entity_metadata::painting_variant", "base_objects::entity_metadata::painting_variant"}},
    {"RegistryEntry<PigVariant>", {"base_objects::var_int32::pig_variant", "base_objects::entity_metadata::pig_variant"}},
    {"RegistryEntry<WolfVariant>", {"base_objects::var_int32::wolf_variant", "base_objects::entity_metadata::wolf_variant"}},
    {"RegistryEntry<WolfSoundVariant>", {"base_objects::var_int32::wolf_sound_variant", "base_objects::entity_metadata::wolf_sound_variant"}},
    {"ArmadilloEntity$State", {"base_objects::entity_metadata::armadillo_state", "base_objects::entity_metadata::armadillo_state"}},
    {"Oxidizable$OxidationLevel", {"base_objects::entity_metadata::weathering_copper_state", "base_objects::entity_metadata::weathering_copper_state"}},
    {"SnifferEntity$State", {"base_objects::entity_metadata::sniffer_state", "base_objects::entity_metadata::sniffer_state"}},
    {"component.type.ProfileComponent", {"base_objects::entity_metadata::resolvable_profile", "base_objects::entity_metadata::resolvable_profile"}},
    {"CopperGolemState", {"base_objects::entity_metadata::copper_golem_state", "base_objects::entity_metadata::copper_golem_state"}},
    // Add other type mappings here as needed
};

void generate_components_file(const std::string& path, const std::map<std::string, MetadataField>& unique_components) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Error opening components output file: " << path << std::endl;
        exit(1);
    }

    out << "// Generated by entity_proxy_gen tool. Contains all component type definitions.\n";
    out << "#pragma once\n\n";
    out << "#include <src/base_objects/entity/metadata.hpp>\n\n";
    out << "namespace copper_server::generated::com {\n";
    out << "// --- Component Struct Definitions ---\n";
    for (const auto& pair : unique_components) {
        const std::string& component_name = pair.first;
        const MetadataField& field = pair.second;

        auto it = type_map.find(field.type_name);
        if (it != type_map.end()) {
            out << "    struct " << component_name << " {\n";
            out << "        static constexpr uint8_t network_id = " << field.network_id << ";\n";
            out << "        " << it->second.first << " value;\n";
            out << "    };\n";
        } else
            std::cerr << "Warning: No C++ type mapping for component '" << component_name << "'. Skipping.\n";
    }

    out << "    using all_metadata_components = std::tuple<\n";
    size_t count = 0;
    for (const auto& pair : unique_components)
        out << "        " << pair.first << (++count == unique_components.size() ? "" : ",") << "\n";
    out << "    >;\n";
    out << "}\n";
}

void generate_components_to_packet_file(const std::string& components_path, const std::string& path, const std::map<std::string, MetadataField>& unique_components) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Error opening components output file: " << path << std::endl;
        exit(1);
    }

    out << "// Generated by entity_proxy_gen tool. Contains all component type definitions.\n";
    out << "#pragma once\n\n";
    out << "#include \"" << components_path << "\"\n\n";
    out << "namespace copper_server::generated::com {\n\n";

    for (const auto& pair : unique_components) {
        const std::string& component_name = pair.first;
        const MetadataField& field = pair.second;

            auto it = type_map.find(field.type_name);
            if (it != type_map.end()) {
                out << "    inline std::pair<uint8_t, base_objects::entity_metadata> to_metadata(const " << component_name << "& comp) {\n";
                if (it->second.first == it->second.second)
                    out << "        return {" << component_name << "::network_id, base_objects::entity_metadata{ " << it->second.second << "{comp.value} }};\n";
                else
                    out << "        return {" << component_name << "::network_id, base_objects::entity_metadata{ " << it->second.second << "{.value = comp.value} }};\n";

                out << "    }\n";
            }
    }
    out << "\n}\n";
}

void generate_wrappers_file(const std::string& components_path, const std::string& path, const std::vector<EntityInfo>& entities) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Error opening wrappers output file: " << path << std::endl;
        exit(1);
    }

    out << "// Generated by entity_proxy_gen tool. Contains strongly-typed entity wrappers.\n";
    out << "#pragma once\n";
    out << "#include <src/api/ecs/base_components.hpp>\n";
    out << "#include \"" << components_path << "\"\n";
    out <<
        R"CODE(
#include <optional>
#include <type_traits>

namespace copper_server::generated::wrapper {

// --- Abstraction Layer ---

template <class... CoreComponents>
class entity_wrapper {
protected:
    api::ecs::entity m_entity;
    template<class T>
    static consteval bool is_one_of(){
        return (... || std::is_same_v<T, CoreComponents>) 
        || std::is_same_v<api::ecs::com::entities::position, T> 
        || std::is_same_v<api::ecs::com::entities::motion, T> 
        || std::is_same_v<api::ecs::com::entities::gravity, T> 
        || std::is_same_v<api::ecs::com::entities::bounding_box, T> 
        || std::is_same_v<api::ecs::com::entities::on_ground, T> 
        || std::is_same_v<api::ecs::com::entities::rotation, T> 
        || std::is_same_v<api::ecs::com::entities::head_rotation, T> 
        || std::is_same_v<api::ecs::com::entities::protocol_id, T> 
        || std::is_same_v<api::ecs::com::entities::uuid, T> 
        || std::is_same_v<api::ecs::com::entities::entity_type, T> 
        || std::is_same_v<api::ecs::com::entities::nbt, T> 
        || std::is_same_v<api::ecs::com::entities::server_nbt, T>
        || std::is_same_v<api::ecs::com::entities::spectating_players, T>
        || std::is_same_v<api::ecs::com::entities::world_syncing, T>
        || std::is_same_v<api::ecs::com::entities::ride_entity, T>
        || std::is_same_v<api::ecs::com::entities::ride_by_entity, T>
        || std::is_same_v<api::ecs::com::entities::attached_to, T>
        || std::is_same_v<api::ecs::com::entities::attached, T>
        || std::is_same_v<api::ecs::com::entities::effects, T>;
    }

public:
    using core_components_tuple = std::tuple<
        api::ecs::com::entities::position,
        api::ecs::com::entities::motion,
        api::ecs::com::entities::gravity,
        api::ecs::com::entities::bounding_box,
        api::ecs::com::entities::on_ground,
        api::ecs::com::entities::rotation, 
        api::ecs::com::entities::head_rotation,
        api::ecs::com::entities::protocol_id,
        api::ecs::com::entities::uuid,
        api::ecs::com::entities::entity_type,
        api::ecs::com::entities::nbt,
        api::ecs::com::entities::server_nbt,
        api::ecs::com::entities::spectating_players,
        api::ecs::com::entities::world_syncing,
        api::ecs::com::entities::ride_entity,
        api::ecs::com::entities::ride_by_entity,
        api::ecs::com::entities::attached_to,
        api::ecs::com::entities::attached,
        api::ecs::com::entities::effects,
        CoreComponents...>;

    explicit entity_wrapper(api::ecs::entity e) : m_entity(e) {}

    [[nodiscard]] api::ecs::entity raw() const { return m_entity; }

    template <class T>
        requires(is_one_of<T>())
    [[nodiscard]] const T& get() const { return m_entity.get<T>(); }

    template <class T>
        requires(is_one_of<T>())
    [[nodiscard]] api::ecs::mutable_component<T> modify() { return m_entity.modify<T>(); }

    template <class T, class... Args>
    void add(Args&&... args) { m_entity.add<T>(std::forward<Args>(args)...); }
    
    template <class T>
    void set(T&& component) { m_entity.set(std::move(component)); }

    template <class T>
    void remove() {
        static_assert(!is_one_of<T>(), "Cannot remove a core component from this entity wrapper.");
        m_entity.remove<T>();
    }

    template <class T>
    [[nodiscard]] bool has() const { return m_entity.has<T>(); }
    void destroy() { m_entity.destroy(); }
    operator api::ecs::entity() const { return m_entity; }


    api::ecs::com::entities::position& position() { return *modify<api::ecs::com::entities::position>(); }
    const api::ecs::com::entities::position& position() const { return get<api::ecs::com::entities::position>(); }

    api::ecs::com::entities::motion& motion() { return *modify<api::ecs::com::entities::motion>(); }
    const api::ecs::com::entities::motion& motion() const { return get<api::ecs::com::entities::motion>(); }

    double gravity() const { return get<api::ecs::com::entities::gravity>().value; }

    int32_t protocol_id() const { return get<api::ecs::com::entities::protocol_id>().value; }

    base_objects::uuid uuid() const { return get<api::ecs::com::entities::uuid>().value; }

    int32_t entity_type() const { return get<api::ecs::com::entities::entity_type>().value; }

    const api::entity_data& const_data() const { return get<api::ecs::com::entities::entity_type>().const_data(); }

    util::nbt_compound& nbt() { return modify<api::ecs::com::entities::nbt>()->get(); }
    const util::nbt_compound& nbt() const { return get<api::ecs::com::entities::nbt>().get(); }

    util::nbt_compound& server_nbt() { return modify<api::ecs::com::entities::server_nbt>()->get(); }
    const util::nbt_compound& server_nbt() const { return get<api::ecs::com::entities::server_nbt>().get(); }

    api::ecs::com::entities::bounding_box& bounding_box() { return modify<api::ecs::com::entities::bounding_box>(); }
    const api::ecs::com::entities::bounding_box& bounding_box() const { return get<api::ecs::com::entities::bounding_box>(); }

    bool& on_ground() { return modify<api::ecs::com::entities::on_ground>()->value; }
    bool on_ground() const { return get<api::ecs::com::entities::on_ground>().value; }

    api::ecs::com::entities::rotation& rotation() { return modify<api::ecs::com::entities::rotation>(); }
    const api::ecs::com::entities::rotation& rotation() const { return get<api::ecs::com::entities::rotation>(); }

    api::ecs::com::entities::head_rotation& head_rotation() { return modify<api::ecs::com::entities::head_rotation>()->value; }
    const api::ecs::com::entities::head_rotation& head_rotation() const { return get<api::ecs::com::entities::head_rotation>().value; }

    list_array<base_objects::client_data_holder>& spectating_players() { return modify<api::ecs::com::entities::spectating_players>()->value; }
    const list_array<base_objects::client_data_holder>& spectating_players() const { return get<api::ecs::com::entities::spectating_players>().value; }

    api::ecs::com::entities::world_syncing& world_syncing() { return *modify<api::ecs::com::entities::world_syncing>(); }
    const api::ecs::com::entities::world_syncing& world_syncing() const { return get<api::ecs::com::entities::world_syncing>(); }

    std::optional<api::ecs::entity>& ride_entity() { return modify<api::ecs::com::entities::ride_entity>()->value; }
    const std::optional<api::ecs::entity>& ride_entity() const { return get<api::ecs::com::entities::ride_entity>().value; }

    list_array<api::ecs::entity>& ride_by_entity() { return modify<api::ecs::com::entities::ride_by_entity>()->value; }
    const list_array<api::ecs::entity>& ride_by_entity() const { return get<api::ecs::com::entities::ride_by_entity>().value; }

    std::optional<std::variant<api::ecs::entity, base_objects::uuid>>& attached_to() { return modify<api::ecs::com::entities::attached_to>()->value; }
    const std::optional<std::variant<api::ecs::entity, base_objects::uuid>>& attached_to() const { return get<api::ecs::com::entities::attached_to>().value; }

    list_array<std::variant<api::ecs::entity, base_objects::uuid>>& attached() { return modify<api::ecs::com::entities::attached>()->value; }
    const list_array<std::variant<api::ecs::entity, base_objects::uuid>>& attached() const { return get<api::ecs::com::entities::attached>().value; }

    api::ecs::com::entities::effects& effects() { return *modify<api::ecs::com::entities::effects>(); }
    const api::ecs::com::entities::effects& effects() const { return get<api::ecs::com::entities::effects>(); }
};

template <class WrapperType, class... Comps>
bool check_all_components(api::ecs::entity e, std::tuple<Comps...>) {
    return (e.has<Comps>() && ...) 
        && e.has<api::ecs::com::entities::position> ()
        && e.has<api::ecs::com::entities::motion>()
        && e.has<api::ecs::com::entities::gravity>()
        && e.has<api::ecs::com::entities::bounding_box>()
        && e.has<api::ecs::com::entities::on_ground>()
        && e.has<api::ecs::com::entities::rotation>()
        && e.has<api::ecs::com::entities::head_rotation>()
        && e.has<api::ecs::com::entities::protocol_id>()
        && e.has<api::ecs::com::entities::uuid>()
        && e.has<api::ecs::com::entities::entity_type>()
        && e.has<api::ecs::com::entities::nbt>()
        && e.has<api::ecs::com::entities::server_nbt>()
        && e.has<api::ecs::com::entities::spectating_players>()
        && e.has<api::ecs::com::entities::world_syncing>()
        && e.has<api::ecs::com::entities::ride_entity>()
        && e.has<api::ecs::com::entities::ride_by_entity>()
        && e.has<api::ecs::com::entities::attached_to>()
        && e.has<api::ecs::com::entities::attached>()
        && e.has<api::ecs::com::entities::effects>()
        ;
}

template <class WrapperType>
std::optional<WrapperType> try_wrap_entity(api::ecs::entity e) {
    if (check_all_components(e, typename WrapperType::core_components_tuple{})) {
        return WrapperType(e);
    }
    return std::nullopt;
}


using generic_entity = entity_wrapper<>;

// --- Generated Entity Classes ---

)CODE";

    // Generate the specific entity classes
    for (const auto& entity : entities) {
        out << "struct " << entity.class_name << " : public entity_wrapper<\n";
        for (size_t i = 0; i < entity.metadata_fields.size(); ++i) {
            auto& field = entity.metadata_fields[i];
            out << "    " << field.component_name << (i == entity.metadata_fields.size() - 1 ? "" : ",") << "\n";
        }
        out << "> {\n";
        out << "    explicit " << entity.class_name << "(api::ecs::entity e) : entity_wrapper(e) {}\n\n";
        out << "    // --- Convenience Accessors ---\n";

        for (const auto& field : entity.metadata_fields) {
            auto it = type_map.find(field.type_name);
            if (it != type_map.end()) {
                out << "    " << it->second.first << "& " << field.field_name << "() { return modify<" << field.component_name << ">()->value; }\n";
                out << "    const " << it->second.first << "& " << field.field_name << "() const { return get<" << field.component_name << ">().value; }\n\n";
            }
        }

        if (entity.class_name == "player") {
            out << "    uint8_t& held_slot() { return modify<api::ecs::com::entities::held_slot>()->hotbar_slot; }\n";
            out << "    const uint8_t& held_slot() const { return get<api::ecs::com::entities::held_slot>().hotbar_slot; }\n\n";

            out << "    api::ecs::com::entities::experience& experience() { return *modify<api::ecs::com::entities::experience>(); }\n";
            out << "    const api::ecs::com::entities::experience& experience() const { return get<api::ecs::com::entities::experience>(); }\n\n";

            out << "    int32_t& food() { return modify<api::ecs::com::entities::food>()->value; }\n";
            out << "    const int32_t& food() const { return get<api::ecs::com::entities::food>().value; }\n\n";

            out << "    float& saturation() { return modify<api::ecs::com::entities::saturation>()->value; }\n";
            out << "    const float& saturation() const { return get<api::ecs::com::entities::saturation>().value; }\n\n";

            out << "    std::unordered_map<uint32_t, base_objects::slot>& inventory() { return modify<api::ecs::com::entities::inventory>()->get(); }\n";
            out << "    const std::unordered_map<uint32_t, base_objects::slot>& inventory() const { return get<api::ecs::com::entities::inventory>().get(); }\n\n";

            out << "    std::unordered_map<std::string, std::unordered_map<uint32_t, base_objects::slot>>& custom_inventory() { return modify<api::ecs::com::entities::custom_inventory>()->get(); }\n";
            out << "    const std::unordered_map<std::string, std::unordered_map<uint32_t, base_objects::slot>>& custom_inventory() const { return get<api::ecs::com::entities::custom_inventory>().get(); }\n\n";
        }

        out << "};\n\n";
    }

    out << "}\n";
}

void generate_factory_file(const std::string& components_path, const std::string& path, const std::vector<EntityInfo>& entities) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Error opening factory output file: " << path << std::endl;
        exit(1);
    }

    out << "// Generated by entity_proxy_gen tool. Provides a factory for creating entities by ID.\n";
    out << "#pragma once\n\n";
    out << "#include <src/api/ecs.hpp>\n";
    out << "#include \"" << components_path << "\"\n";
    out << "#include <optional>\n\n";
    out << "namespace copper_server::generated::entity {\n";
    out << "    void register_entities();\n";
    out << "}\n";
}

void generate_factory_cpp_file(const std::string& components_path, const std::string& path, const std::vector<EntityInfo>& entities) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Error opening factory output file: " << path << std::endl;
        exit(1);
    }

    out << "// Generated by entity_proxy_gen tool. Provides a factory for creating entities by ID.\n";
    out << "#include \"" << components_path << "\"\n";
    out << "#include <optional>\n";
    out << "#include <src/api/ecs/base_components.hpp>\n";
    out << "#include <src/base_objects/shared_client_data.hpp>\n\n";
    out << "#include <src/util/reflect/api/packets/chat_type.hpp>\n";
    out << "#include <src/util/reflect/base_objects/block_entity.hpp>\n";
    out << "#include <src/util/reflect/base_objects/component.hpp>\n";
    out << "#include <src/util/reflect/base_objects/dye_color.hpp>\n";
    out << "#include <src/util/reflect/base_objects/entity/metadata.hpp>\n";
    out << "#include <src/util/reflect/base_objects/parsers.hpp>\n";
    out << "#include <src/util/reflect/base_objects/particle_data.hpp>\n";
    out << "#include <src/util/reflect/calculations.hpp>\n\n";
    out << "#include <src/api/ecs/entity_definition.hpp>\n\n";

    out << "namespace copper_server::generated::entity {\n\n";
    out << "    void register_entities() {\n";

    for (const auto& entity : entities) {
        out << "        api::ecs::initialization::make_entity_definition(\"" << entity.entity_name << "\")\n";
        out << "            .add_locked<api::ecs::com::entities::position>()\n";
        out << "            .add_locked<api::ecs::com::entities::motion>()\n";
        out << "            .add_locked<api::ecs::com::entities::bounding_box>()\n";
        out << "            .add_locked<api::ecs::com::entities::gravity>()\n";
        out << "            .add_locked<api::ecs::com::entities::on_ground>()\n";
        out << "            .add_locked<api::ecs::com::entities::rotation>()\n";
        out << "            .add_locked<api::ecs::com::entities::head_rotation>()\n";
        out << "            .add_locked<api::ecs::com::entities::protocol_id>()\n";
        out << "            .add_locked<api::ecs::com::entities::uuid>()\n";
        out << "            .add_locked(api::ecs::com::entities::entity_type{" << entity.id << "})\n";
        out << "            .add_locked<api::ecs::com::entities::nbt>()\n";
        out << "            .add_locked<api::ecs::com::entities::server_nbt>()\n";
        out << "            .add_locked<api::ecs::com::entities::spectating_players>()\n";
        out << "            .add_locked<api::ecs::com::entities::world_syncing>()\n";
        out << "            .add_locked<api::ecs::com::entities::ride_entity>()\n";
        out << "            .add_locked<api::ecs::com::entities::ride_by_entity>()\n";
        out << "            .add_locked<api::ecs::com::entities::attached_to>()\n";
        out << "            .add_locked<api::ecs::com::entities::attached>()\n";
        out << "            .add_locked<api::ecs::com::entities::effects>()";
        for (const auto& field : entity.metadata_fields)
            out << "\n            .add_reset_on_remove<" << field.component_name << ">()";
        out << ";\n";
    }

    out << "    }\n";
    out << "}\n";
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <input_json> <out_components.hpp> <out_components_to_packet.hpp> <out_wrappers.hpp> <out_factory.hpp> <out_factory.cpp>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string components_file = argv[2];
    std::string components_to_packet_file = argv[3];
    std::string wrappers_file = argv[4];
    std::string factory_file = argv[5];
    std::string factory_cpp_file = argv[6];


    std::filesystem::create_directories(std::filesystem::path(components_file).parent_path());
    std::filesystem::create_directories(std::filesystem::path(components_to_packet_file).parent_path());
    std::filesystem::create_directories(std::filesystem::path(wrappers_file).parent_path());
    std::filesystem::create_directories(std::filesystem::path(factory_file).parent_path());
    std::filesystem::create_directories(std::filesystem::path(factory_cpp_file).parent_path());

    // --- Phase 1: Discovery ---
    pt::ptree root;
    try {
        pt::read_json(input_file, root);
    } catch (const pt::json_parser_error& e) {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return 1;
    }

    std::vector<EntityInfo> all_entities;
    std::map<std::string, MetadataField> unique_components;

    std::set<std::string> class_name_add;
    for (const auto& entity_pair : root) {
        EntityInfo current_entity;
        current_entity.entity_name = entity_pair.first;
        current_entity.class_name = sanitize_name(entity_pair.first);
        current_entity.id = entity_pair.second.get<int>("id");
        try {
            for (const auto& metadata_item : entity_pair.second.get_child("metadata")) {
                MetadataField field;
                field.field_name = boost::to_lower_copy(metadata_item.second.get<std::string>("field_name"));
                field.type_name = metadata_item.second.get<std::string>("type_name");
                field.network_id = metadata_item.second.get<int>("network_id");
                field.component_name = field.field_name;
                if (field.field_name == current_entity.class_name)
                    field.field_name = "get_" + field.field_name;

                if (auto it = unique_components.find(field.component_name); it == unique_components.end()) {
                    current_entity.metadata_fields.push_back(field);
                    unique_components[field.component_name] = std::move(field);
                } else {
                    if (it->second.type_name != field.type_name || it->second.network_id != field.network_id) {
                        class_name_add.emplace(field.component_name);
                        field.component_name = current_entity.class_name + "_" + field.component_name;
                        current_entity.metadata_fields.push_back(field);
                        unique_components[field.component_name] = std::move(field);
                    } else
                        current_entity.metadata_fields.push_back(std::move(field));
                }
            }
        } catch (const pt::ptree_bad_path& path) {
            std::cerr << "Skipping component...\n";
        }

        all_entities.push_back(current_entity);
    }
    for (auto& entity : all_entities) {
        for (auto& field : entity.metadata_fields) {
            if (class_name_add.find(field.component_name) != class_name_add.end()) {
                field.component_name = entity.class_name + "_" + field.component_name;
                unique_components[field.component_name] = field;
            }
            field.component_name = "com::" + field.component_name;
        }
    }
    for (auto& it : class_name_add)
        unique_components.erase(it);

    std::cout << "Found " << all_entities.size() << " entities and " << unique_components.size() << " unique components.\n";

    std::cout << "Generating components file to " << components_file << "...\n";
    generate_components_file(components_file, unique_components);

    std::cout << "Generating components to packet conversion file to " << components_to_packet_file << "...\n";
    generate_components_to_packet_file(components_file, components_to_packet_file, unique_components);

    std::vector<MetadataField> player_fields;
    player_fields.push_back(MetadataField{"api::ecs::com::entities::held_slot"});
    player_fields.push_back(MetadataField{"api::ecs::com::entities::experience"});
    player_fields.push_back(MetadataField{"api::ecs::com::entities::food"});
    player_fields.push_back(MetadataField{"api::ecs::com::entities::saturation"});
    player_fields.push_back(MetadataField{"api::ecs::com::entities::inventory"});
    player_fields.push_back(MetadataField{"api::ecs::com::entities::custom_inventory"});
    player_fields.push_back(MetadataField{"api::ecs::com::entities::assigned_player"});

    for (auto& entity : all_entities)
        if (entity.class_name == "player")
            entity.metadata_fields.insert(entity.metadata_fields.begin(), player_fields.begin(), player_fields.end());


    std::cout << "Generating wrappers file to " << wrappers_file << "...\n";
    generate_wrappers_file(components_file, wrappers_file, all_entities);

    std::cout << "Generating factory header file to " << factory_file << "...\n";
    generate_factory_file(components_file, factory_file, all_entities);

    std::cout << "Generating factory source file to " << factory_cpp_file << "...\n";
    generate_factory_cpp_file(components_file, factory_cpp_file, all_entities);

    return 0;
}
