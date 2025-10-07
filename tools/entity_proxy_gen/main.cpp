/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>

namespace pt = boost::property_tree;

struct MetadataField {
    std::string field_name;
    int network_id;
    std::string type_name;
};

std::string sanitize_name(std::string name) {
    boost::replace_first(name, "minecraft:", "");
    return name;
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

void generate_getter(std::ofstream& out, std::ofstream& out_impl, const MetadataField& field, std::string_view entity_name) {
    std::string lower_field_name = boost::to_lower_copy(field.field_name);
    if (lower_field_name == entity_name)
        lower_field_name.push_back('_');
    auto it = type_map.find(field.type_name);
    if (it == type_map.end()) {
        std::cerr << "Warning: No type mapping for " << field.type_name << std::endl;
        return;
    }
    const auto& type_pair = it->second;
    const std::string& cpp_type = type_pair.first;
    const std::string& metadata_type = type_pair.second;

    out << "        " << cpp_type << "& " << lower_field_name << "();\n";

    out_impl << "    " << cpp_type << "& " << entity_name << "::" << lower_field_name << "() {\n";
    out_impl << "        auto it = e.metadata.find(\"" << field.field_name << "\");\n";
    if (metadata_type != cpp_type){
        out_impl << "        if (it == e.metadata.end()) {\n";
        out_impl << "            auto& res = e.metadata[\"" << field.field_name << "\"];\n";
        out_impl << "            res.value = " << metadata_type << "{};\n"; 
        out_impl << "            return std::get<" << metadata_type << ">(res.value).value;\n";
        out_impl << "        }\n";
        out_impl << "        return std::get<" << metadata_type << ">(it->second.value).value;\n";
    }else{
        out_impl << "        if (it == e.metadata.end()) {\n";
        out_impl << "            auto& res = e.metadata[\"" << field.field_name << "\"];\n";
        out_impl << "            res.value = " << metadata_type << "{};\n"; 
        out_impl << "            return std::get<" << metadata_type << ">(res.value);\n";
        out_impl << "        }\n";
        out_impl << "        return std::get<" << metadata_type << ">(it->second.value);\n";
    }
    out_impl << "    }\n\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_json> <output_header> <output_implementation>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    std::string output_file_bin = argv[3];

    pt::ptree root;
    try {
        pt::read_json(input_file, root);
    } catch (const pt::json_parser_error& e) {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return 1;
    }

    std::ofstream out(output_file);
    if (!out) {
        std::cerr << "Error opening output file: " << output_file << std::endl;
        return 1;
    }

    std::ofstream out_impl(output_file_bin);
    if (!out) {
        std::cerr << "Error opening output file: " << output_file_bin << std::endl;
        return 1;
    }


    out_impl << "// Generated by entity_proxy_gen tool\n";
    out_impl << "#include \"" + output_file + "\"\n";

    out << "// Generated by entity_proxy_gen tool\n";
    out << "#pragma once\n\n";
    out << "#include <functional>\n";
    out << "#include <string_view>\n";
    out << "#include <src/base_objects/entity.hpp>\n";


    out << "namespace copper_server::api::entity_proxy {\n\n";
    out_impl << "namespace copper_server::api::entity_proxy {\n\n";


    std::stringstream global_iterate_all;
    global_iterate_all << "    void iterate_all(base_objects::entity& e, std::move_only_function<void(uint8_t index, base_objects::entity_metadata&)>&& func) {\n";
    global_iterate_all << "        switch(e.get_entity_type_id()) {\n";


    for (const auto& entity_pair : root) {
        std::string entity_name = sanitize_name(entity_pair.first);
        out << "    struct " << entity_name << " {\n";
        out << "        base_objects::entity& e;\n\n";
        out << "        " << entity_name << "(base_objects::entity& e): e(e) {}\n\n";

        std::vector<MetadataField> fields;
        try {
            global_iterate_all << "        case " << entity_pair.second.get<int>("id") << ": " << entity_name << "(e).iterate_all(std::move(func)); break;\n";
            for (const auto& metadata_item : entity_pair.second.get_child("metadata")) {
                MetadataField field;
                field.field_name = metadata_item.second.get<std::string>("field_name");
                field.network_id = metadata_item.second.get<int>("network_id");
                field.type_name = metadata_item.second.get<std::string>("type_name");
                fields.push_back(field);
                generate_getter(out, out_impl, field, entity_name);
            }
        } catch (const pt::ptree_bad_path& e) {
            std::cerr << "Warning: Could not find metadata for " << entity_name << ". " << e.what() << std::endl;
        }


        out << "        void iterate_all(std::move_only_function<void(uint8_t index, base_objects::entity_metadata&)>&& func);\n";
        out_impl << "    void " << entity_name << "::iterate_all(std::move_only_function<void(uint8_t index, base_objects::entity_metadata&)>&& func) {\n";
        for (const auto& field : fields)
            out_impl << "        if (auto it = e.metadata.find(\"" << field.field_name << "\"); it != e.metadata.end()) func(" << field.network_id << ", it->second);\n";
        out_impl << "    }\n\n";


        out << "        static int16_t get_index_of(std::string_view field_name);\n";
        out_impl << "    int16_t " << entity_name << "::get_index_of(std::string_view field_name) {\n";
        for (const auto& field : fields)
            out_impl << "        if (field_name == \"" << field.field_name << "\") return " << field.network_id << ";\n";
        out_impl << "        return -1;\n";
        out_impl << "    }\n\n";

        out << "    };\n\n";
    }

    out << "    void iterate_all(base_objects::entity& e, std::move_only_function<void(uint8_t index, base_objects::entity_metadata&)>&& func);\n";


    out << "}\n";
    global_iterate_all << "        default: break;\n";
    global_iterate_all << "        }\n";
    out_impl << global_iterate_all.str() << "    }\n}\n";

    std::cout << "Successfully generated entity proxies to " << output_file << std::endl;

    return 0;
}
