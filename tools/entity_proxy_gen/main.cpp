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
    std::string field_name;
    std::string type_name;
    int32_t network_id;
};

struct EntityInfo {
    int32_t id;
    std::string original_name;
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
    {"Text", {"Chat", "base_objects::entity_metadata::text_component"}},
    {"Optional<Text>", {"std::optional<Chat>", "base_objects::entity_metadata::optional_text_component"}},
    {"BlockPos", {"base_objects::position", "base_objects::entity_metadata::position"}},
    {"Optional<BlockPos>", {"std::optional<base_objects::position>", "base_objects::entity_metadata::optional_position"}},
    {"EntityPose", {"base_objects::entity_metadata::entity_pose", "base_objects::entity_metadata::entity_pose"}},
    {"ParticleEffect", {"base_objects::entity_metadata::particle", "base_objects::entity_metadata::particle"}},
    {"List<ParticleEffect>", {"list_array<base_objects::entity_metadata::particle>", "base_objects::entity_metadata::particles"}},
    {"ItemStack", {"base_objects::slot", "base_objects::entity_metadata::slot"}},
    {"Direction", {"base_objects::entity_metadata::direction", "base_objects::entity_metadata::direction"}},
    {"Optional<LazyEntityReference<LivingEntity>>", {"std::optional<enbt::raw_uuid>", "base_objects::entity_metadata::optional_living_entity_reference"}},
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

    out << "// Generated by ecs_gen_tool. Contains all component type definitions.\n";
    out << "#pragma once\n\n";
    out << "#include <src/base_objects/entity_metadata.hpp>\n";
    out << "// Include your other custom types...\n";
    out << "#include <src/base_objects/types.hpp>\n\n";
    out << "namespace copper_server::generated::com {\n\n";

    out << "// --- Component Struct Definitions ---\n";
    for (const auto& pair : unique_components) {
        const std::string& component_name = pair.first;
        const MetadataField& field = pair.second;

        if (field.type_name == "Boolean") {
            out << "struct " << component_name << " {\n";
            out << "    static constexpr uint8_t network_id = " << field.network_id << ";\n";
            out << "};\n";
        } else {
            auto it = type_map.find(field.type_name);
            if (it != type_map.end()) {
                out << "struct " << component_name << " {\n";
                out << "    static constexpr uint8_t network_id = " << field.network_id << ";\n";
                out << "    " << it->second.first << " value;\n";
                out << "};\n";
            } else {
                std::cerr << "Warning: No C++ type mapping for component '" << component_name << "'. Skipping.\n";
            }
        }
    }

    out << "\n// --- Conversion to Network Variant ---\n";
    for (const auto& pair : unique_components) {
        const std::string& component_name = pair.first;
        const MetadataField& field = pair.second;

        if (field.type_name == "Boolean") {
            out << "inline base_objects::entity_metadata to_metadata(const " << component_name << "&) {\n";
            out << "    return base_objects::entity_metadata{ base_objects::entity_metadata::boolean{true} };\n";
            out << "}\n";
        } else {
            auto it = type_map.find(field.type_name);
            auto it_variant = type_map.find(it->second.first);
            if (it != type_map.end() && it_variant != type_map.end()) {
                out << "inline base_objects::entity_metadata to_metadata(const " << component_name << "& comp) {\n";
                if (it->second.first == it_variant->second.second) { // If the C++ type is the same as the variant type
                    out << "    return base_objects::entity_metadata{ comp.value };\n";
                } else {
                    out << "    return base_objects::entity_metadata{ " << it_variant->second.second << "{comp.value} };\n";
                }
                out << "}\n";
            }
        }
    }

    // Generate a tuple of all component types for easy iteration
    out << "\n// --- All Component Types Tuple ---\n";
    out << "using AllSyncableComponents = std::tuple<\n";
    size_t count = 0;
    for (const auto& pair : unique_components) {
        out << "    " << pair.first << (++count == unique_components.size() ? "" : ",") << "\n";
    }
    out << ">;\n";


    out << "\n} // namespace copper_server::generated::com\n";
}

void generate_wrappers_file(const std::string& components_path, const std::string& path, const std::vector<EntityInfo>& entities) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Error opening wrappers output file: " << path << std::endl;
        exit(1);
    }

    // Write the boilerplate EntityWrapper template
    out << "// Generated by ecs_gen_tool. Contains strongly-typed entity wrappers.\n";
    out << "#pragma once\n";
    out << "#include <src/api/ecs.hpp>\n";
    out << "#include \"" << components_path << "\"\n";
    out <<
        R"CODE(
#include <optional>
#include <type_traits>

namespace copper_server::generated::wrapper {

// --- Abstraction Layer ---

template <typename T, typename... List>
concept is_one_of = (std::is_same_v<T, List> || ...);

template <typename... CoreComponents>
class EntityWrapper {
protected:
    api::ecs::entity m_entity;

public:
    explicit EntityWrapper(api::ecs::entity e) : m_entity(e) {}

    [[nodiscard]] api::ecs::entity raw() const { return m_entity; }

    template <typename T>
        requires(is_one_of<T, CoreComponents...>)
    [[nodiscard]] const T& get() const { return m_entity.get<T>(); }

    template <typename T>
        requires(is_one_of<T, CoreComponents...>)
    [[nodiscard]] api::ecs::mutable_component<T> modify() { return m_entity.modify<T>(); }

    template <typename T, typename... Args>
    void add(Args&&... args) { m_entity.add<T>(std::forward<Args>(args)...); }
    
    template <typename T>
    void set(T&& component) { m_entity.set(std::move(component)); }

    template <typename T>
    void remove() {
        static_assert(!is_one_of<T, CoreComponents...>, "Cannot remove a core component from this entity wrapper.");
        m_entity.remove<T>();
    }

    template <typename T>
    [[nodiscard]] bool has() const { return m_entity.has<T>(); }
    void destroy() { m_entity.destroy(); }
    operator api::ecs::entity() const { return m_entity; }
};

template <typename WrapperType, typename... Comps>
bool check_all_components(api::ecs::entity e, std::tuple<Comps...>) {
    return (e.has<Comps>() && ...);
}

template <typename WrapperType>
std::optional<WrapperType> try_wrap_entity(api::ecs::entity e) {
    if (check_all_components(e, typename WrapperType::core_components_tuple{})) {
        return WrapperType(e);
    }
    return std::nullopt;
}

// --- Generated Entity Classes ---

)CODE";

    // Generate the specific entity classes
    for (const auto& entity : entities) {
        out << "class " << entity.class_name << " : public EntityWrapper<\n";
        for (size_t i = 0; i < entity.metadata_fields.size(); ++i) {
            out << "    com::" << to_lower_case(entity.metadata_fields[i].field_name) << (i == entity.metadata_fields.size() - 1 ? "" : ",") << "\n";
        }
        out << "> {\n";
        out << "public:\n";
        out << "    using core_components_tuple = std::tuple<\n";
        for (size_t i = 0; i < entity.metadata_fields.size(); ++i) {
            out << "        com::" << to_lower_case(entity.metadata_fields[i].field_name) << (i == entity.metadata_fields.size() - 1 ? "" : ",") << "\n";
        }
        out << "    >;\n";
        out << "    explicit " << entity.class_name << "(api::ecs::entity e) : EntityWrapper(e) {}\n\n";
        out << "    // --- Convenience Accessors ---\n";

        for (const auto& field : entity.metadata_fields) {
            std::string component_name = to_lower_case(field.field_name);
            std::string method_name = to_lower_case(field.field_name);

            if (field.type_name == "Boolean") {
                out << "    bool is_" << method_name << "() const { return m_entity.has<com::" << component_name << ">(); }\n";
                out << "    void set_" << method_name << "(bool v) { if(v) m_entity.add<com::" << component_name << ">(); else m_entity.remove<com::" << component_name << ">(); }\n\n";
            } else {
                auto it = type_map.find(field.type_name);
                if (it != type_map.end()) {
                    out << "    " << it->second.first << "& " << method_name << "() { return modify<com::" << component_name << ">()->value; }\n";
                    out << "    const " << it->second.first << "& " << method_name << "() const { return get<com::" << component_name << ">().value; }\n\n";
                }
            }
        }
        out << "};\n\n";
    }

    out << "} // namespace copper_server::generated::wrapper\n";
}

void generate_factory_file(const std::string& components_path, const std::string& path, const std::vector<EntityInfo>& entities) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Error opening factory output file: " << path << std::endl;
        exit(1);
    }

    out << "// Generated by ecs_gen_tool. Provides a factory for creating entities by ID.\n";
    out << "#pragma once\n\n";
    out << "#include <src/api/ecs.hpp>\n";
    out << "#include \"" << components_path << "\"\n";
    out << "#include <optional>\n\n";
    out << "namespace copper_server::generated::factory {\n\n";
    out << "inline std::optional<api::ecs::entity> create_entity_by_id(int entity_id, api::ecs::global_registry& registry) {\n";
    out << "    api::ecs::entity_recipe recipe;\n";
    out << "    bool found = true;\n";
    out << "    switch(entity_id) {\n";

    for (const auto& entity : entities) {
        out << "        case " << entity.id << ": // " << entity.original_name << "\n";
        for (const auto& field : entity.metadata_fields) {
            out << "            recipe.with<com::" << to_lower_case(field.field_name) << ">();\n";
        }
        out << "            break;\n";
    }

    out << "        default:\n";
    out << "            found = false;\n";
    out << "            break;\n";
    out << "    }\n\n";
    out << "    if (!found) {\n";
    out << "        return std::nullopt;\n";
    out << "    }\n\n";
    out << "    recipe.freeze();\n";
    out << "    return registry.create_entity_and_wait(recipe);\n";
    out << "}\n\n";
    out << "} // namespace copper_server::generated::factory\n";
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <input_json> <out_components.hpp> <out_wrappers.hpp> <out_factory.hpp>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string components_file = argv[2];
    std::string wrappers_file = argv[3];
    std::string factory_file = argv[4];


    std::filesystem::create_directories(std::filesystem::path(components_file).parent_path());
    std::filesystem::create_directories(std::filesystem::path(wrappers_file).parent_path());
    std::filesystem::create_directories(std::filesystem::path(factory_file).parent_path());

    // --- Phase 1: Discovery ---
    pt::ptree root;
    try {
        pt::read_json(input_file, root);
    } catch (const pt::json_parser_error& e) {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return 1;
    }

    std::vector<EntityInfo> all_entities;
    std::map<std::string, MetadataField> unique_components; // PascalCase name -> JSON type name

    for (const auto& entity_pair : root) {
        EntityInfo current_entity;
        current_entity.original_name = entity_pair.first;
        current_entity.class_name = sanitize_name(entity_pair.first);
        current_entity.id = entity_pair.second.get<int>("id");

        try {
            for (const auto& metadata_item : entity_pair.second.get_child("metadata")) {
                MetadataField field;
                field.field_name = metadata_item.second.get<std::string>("field_name");
                field.type_name = metadata_item.second.get<std::string>("type_name");
                field.network_id = metadata_item.second.get<int>("network_id");
                current_entity.metadata_fields.push_back(field);

                std::string component_name = boost::to_lower_copy(field.field_name);
                if (unique_components.find(component_name) == unique_components.end())
                    unique_components[component_name] = std::move(field);
            }
        } catch (const pt::ptree_bad_path&) {
            // It's ok if an entity has no metadata
        }
        all_entities.push_back(current_entity);
    }

    // --- Phase 2: Code Generation ---

    std::cout << "Found " << all_entities.size() << " entities and " << unique_components.size() << " unique components.\n";

    std::cout << "Generating components file to " << components_file << "...\n";
    generate_components_file(components_file, unique_components);

    std::cout << "Generating wrappers file to " << wrappers_file << "...\n";
    generate_wrappers_file(components_file, wrappers_file, all_entities);

    std::cout << "Generating factory file to " << factory_file << "...\n";
    generate_factory_file(components_file, factory_file, all_entities);

    std::cout << "Successfully generated all files.\n";

    return 0;
}
