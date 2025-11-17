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
#include <stacktrace>
#include <string>
#include <vector>

namespace pt = boost::property_tree;


std::map<std::string, std::string> type_map = {
    {"BannerPatternsComponent", "base_objects::component::banner_patterns"},
    {"BlockPos", "util::xyz<int32_t>"},
    {"BlockState", "base_objects::block"},
    {"ComponentMap", "std::unordered_map<int32_t, base_objects::component>"},
    {"ContainerLock", "base_objects::item_predicate"},
    {"DefaultedList<ItemStack>", "base_objects::container_sized<"},
    {"Either<CreakingEntity, UUID>", "api::ecs::entity_ref"},
    {"Identifier", "base_objects::identifier"},
    {"Inventory", "base_objects::slot_data"}, //TODO investigate more
    {"ItemStack", "base_objects::slot_data"},
    {"LazyEntityReference<LivingEntity>", "api::ecs::entity_ref"},
    {"List<BeehiveBlockEntity$Bee>", "list_array<api::ecs::entity>"},
    {"List<ItemStack>", "list_array<base_objects::slot_data>"},
    {"List<TestInstanceBlockEntity$Error>", "list_array<base_objects::block_entity::test_instance::error>"},
    {"ProfileComponent", "base_objects::component::profile"},
    {"Pool<spawner.MobSpawnerEntry>", "base_objects::pool<base_objects::block_entity::mob_spawner_entry>"}, //TODO implement
    {"Reference2IntOpenHashMap<RegistryKey<Recipe<?>>>", "std::unordered_map<api::id::recipe, int32_t>"},
    {"RegistryKey<TestInstance>", "api::id::test_instance_type"},
    {"RegistryKey<LootTable>", "api::id::loot_table"},
    {"RegistryKey<StructurePool>", "api::id::worldgen__structure_pool_element"},
    {"RegistryEntry<spawner.TrialSpawnerConfig>", "api::id::trial_spawner_config"}, //TODO add support for new configurations
    {"RegistryEntry<jukebox.JukeboxSong>", "api::id::jukebox_song"},
    {"RegistryEntry<effect.StatusEffect>", "api::id::mob_effect"},
    {"SculkCatalystBlockEntity$Listener", "base_objects::block_entity::sculk_catalyst::listener"},
    {"Set<UUID>", "std::unordered_set<base_objects::uuid>"},
    {"Sherds", "std::array<api::id::item, 4>"},
    {"SignText", "std::array<base_objects::chat, 5>"},
    {"String", "std::string"},
    {"TestBlockMode", "base_objects::block_entity::test::mode"},
    {"Text", "base_objects::chat"},
    {"Vec3i", "util::xyz<int32_t>"},
    {"Vibrations$VibrationListener", "base_objects::vibration_listener"},
    {"spawner.MobSpawnerEntry", "base_objects::block_entity::mob_spawner_entry"}, //TODO implement
    {"ViewerCountManager", "base_objects::viewer_count_manager"},
    {"boolean", "bool"},
    {"double", "double"},
    {"float", "float"},
    {"int", "int32_t"},
    {"int[]", "std::array<int32_t, "},
    {"long", "int64_t"},
};

std::string camel_to_snake(const std::string& input) {
    if (input.empty())
        return "";

    std::string result;
    result.reserve(input.length() + 5);

    for (size_t i = 0; i < input.length(); ++i) {
        char current_char = input[i];

        if (std::isupper(current_char)) {
            if (i > 0) {
                bool prev_is_upper = std::isupper(input[i - 1]);
                bool next_is_lower = (i + 1 < input.length()) && std::islower(input[i + 1]);

                if (!prev_is_upper || next_is_lower)
                    result += '_';
            }
            result += std::tolower(current_char);
        } else
            result += current_char;
    }
    return result;
}

bool is_sized_required(const std::string& java_type) {
    if (type_map.count(java_type)) {
        const auto& mapped = type_map.at(java_type);
        return (mapped.starts_with("std::array<") || mapped.starts_with("base_objects::container_sized<")) && !mapped.ends_with('>');
    }
    return false;
}

bool is_optional(const std::string& java_type) {
    return java_type.starts_with("Optional<");
}

std::string get_inner_type(const std::string& java_type) {
    auto start = java_type.find('<');
    auto end = java_type.rfind('>');
    if (start == std::string::npos || end == std::string::npos)
        return java_type;
    return java_type.substr(start + 1, end - start - 1);
}

std::string get_cpp_type(const pt::ptree& field_node) {
    std::string java_type = field_node.get<std::string>("type");
    bool as_optional = false;
    if (auto preservation_opt = field_node.get_optional<std::string>("preservation"))
        as_optional = *preservation_opt == "OPTIONAL";

    if (is_optional(java_type))
        return "std::optional<" + type_map.at(get_inner_type(java_type)) + ">";

    std::string result;
    if (type_map.count(java_type)) {
        std::string cpp_type = type_map.at(java_type);
        if (is_sized_required(java_type)) {
            auto size_opt = field_node.get_optional<int>("size");
            if (!size_opt)
                size_opt = field_node.get_optional<int>("default_size");
            if (size_opt)
                result = cpp_type + std::to_string(*size_opt) + ">";
            else
                result = cpp_type + "/*FIXME: UNKNOWN SIZE*/>";
        } else
            result = cpp_type;
    }
    if (field_node.get_child_optional("enum_values"))
        result = field_node.get<std::string>("name") + "_e";
    else if (result.empty())
        return "/* FIXME unknown_type: " + java_type + " */";

    if (as_optional)
        if (!result.starts_with("list_array<"))
            return "std::optional<" + result + ">";
    return result;
}

std::string get_field_name(const std::string& name) {
    if (name == "auto")
        return "is_auto";
    else
        return camel_to_snake(name);
}

std::string sanitize_name(const std::string& name) {
    if (name.starts_with("minecraft:"))
        return name.substr(10);
    return name;
}

std::string get_enum_member_name(const pt::ptree& enum_value) {
    std::string name = enum_value.get<std::string>("name");
    if (name.starts_with("translation{")) {
        auto id_opt = enum_value.get_optional<std::string>("id");
        if (!id_opt || id_opt->empty()) {
            throw std::runtime_error("Enum with translation key name '" + name + "' is missing a required 'id' field.");
        }
        name = *id_opt;
    }
    boost::algorithm::to_lower(name);
    if (!name.empty() && isdigit(name[0])) {
        name = "_" + name;
    }
    return name;
}

std::string get_enum_nbt_string(const pt::ptree& enum_value) {
    std::string name = enum_value.get<std::string>("name");
    if (name.starts_with("translation{")) {
        auto id_opt = enum_value.get_optional<std::string>("id");
        if (!id_opt || id_opt->empty()) {
            throw std::runtime_error("Enum with translation key name '" + name + "' is missing a required 'id' field.");
        }
        name = *id_opt;
    }
    boost::algorithm::to_upper(name);
    if (!name.empty() && isdigit(name[0])) {
        name = "_" + name;
    }
    return name;
}

void generate_struct_fields(std::ostream& out, const pt::ptree& fields_node, int indent_level);
void generate_to_nbt_body(std::ostream& out, const std::string& parent_accessor, const std::string& prev_structs, const std::string& stream_name, const pt::ptree& fields_node, int indent_level);
void generate_from_nbt_body(std::ostream& out, const std::string& result_accessor, const std::string& prev_structs, const pt::ptree& fields_node, int indent_level);

void generate_struct_fields(std::ostream& out, const pt::ptree& fields_node, int indent_level) {
    std::string indent(indent_level * 4, ' ');
    for (const auto& pair : fields_node) {
        const pt::ptree& field = pair.second;
        std::string field_name = get_field_name(field.get<std::string>("name"));
        std::string java_type = field.get<std::string>("type");
        auto nbt_name = field.get_optional<std::string>("nbt_name");
        bool not_preserved = nbt_name && *nbt_name == "_____UNKNOWN_____";


        if (field.get_child_optional("nested_fields") && !type_map.contains(java_type)) {
            out << indent << "struct " << field_name << "_t {\n";
            generate_struct_fields(out, field.get_child("nested_fields"), indent_level + 1);
            out << indent << "} " << field_name << (not_preserved ? "; //not preserved\n" : ";\n");
        } else if (field.get_child_optional("enum_values")) {
            out << indent << "enum class " << field_name << "_e {\n";
            int enum_index_counter = 0;
            for (const auto& enum_pair : field.get_child("enum_values")) {
                const pt::ptree& enum_value = enum_pair.second;
                std::string member_name = get_enum_member_name(enum_value);
                int index = enum_value.get_optional<int>("index").value_or(enum_index_counter);
                out << indent << "    " << member_name << " = " << index << ",\n";
                enum_index_counter++;
            }
            out << indent << "};" << "\n";
            out << indent << get_cpp_type(field) << " " << field_name << (not_preserved ? "; //not preserved\n" : ";\n");
        } else {
            out << indent << get_cpp_type(field) << " " << field_name << (not_preserved ? "; //not preserved\n" : ";\n");
        }
    }
}

void generate_to_nbt_body(std::ostream& out, const std::string& parent_accessor, const std::string& prev_structs, const std::string& stream_name, const pt::ptree& fields_node, int indent_level) {
    std::string indent(indent_level * 4, ' ');
    for (const auto& pair : fields_node) {
        const pt::ptree& field = pair.second;
        std::string field_name = get_field_name(field.get<std::string>("name"));
        auto nbt_name_opt = field.get_optional<std::string>("nbt_name");
        std::string preservation = field.get<std::string>("preservation");
        std::string java_type = field.get<std::string>("type");
        std::string current_accessor = parent_accessor + "." + field_name;

        auto nested_fields_opt = field.get_child_optional("nested_fields");
        if (!nbt_name_opt && nested_fields_opt && type_map.contains(java_type)) {
            std::string base_accessor = current_accessor;
            bool is_opt = is_optional(java_type) || preservation == "OPTIONAL";
            if (is_opt) {
                out << indent << "if (" << base_accessor << ") {\n";
                std::string indent_inner((indent_level + 1) * 4, ' ');
                for (const auto& nested_pair : *nested_fields_opt) {
                    const pt::ptree& nested_field = nested_pair.second;
                    auto nested_nbt_name_opt = nested_field.get_optional<std::string>("nbt_name");
                    if (!nested_nbt_name_opt || *nested_nbt_name_opt == "_____UNKNOWN_____")
                        continue;
                    std::string nested_field_name = get_field_name(nested_field.get<std::string>("name"));
                    std::string nested_value_accessor = base_accessor + "->" + nested_field_name;
                    out << indent_inner << stream_name << ".write(\"" << *nested_nbt_name_opt << "\", [this](util::nbt_write_stream& stream) { util::encoding::nbt::serialize_entry(stream, " << nested_value_accessor << "); });\n";
                }
                out << indent << "}\n";
            } else {
                for (const auto& nested_pair : *nested_fields_opt) {
                    const pt::ptree& nested_field = nested_pair.second;
                    auto nested_nbt_name_opt = nested_field.get_optional<std::string>("nbt_name");
                    if (!nested_nbt_name_opt || *nested_nbt_name_opt == "_____UNKNOWN_____")
                        continue;
                    std::string nested_field_name = get_field_name(nested_field.get<std::string>("name"));
                    std::string nested_value_accessor = base_accessor + "." + nested_field_name;
                    out << indent << stream_name << ".write(\"" << *nested_nbt_name_opt << "\", [this](util::nbt_write_stream& stream) { util::encoding::nbt::serialize_entry(stream, " << nested_value_accessor << "); });\n";
                }
            }
            continue;
        }

        if (!nbt_name_opt || *nbt_name_opt == "_____UNKNOWN_____")
            continue;

        if (preservation == "OPTIONAL") {
            std::string condition = java_type.starts_with("List<") ? current_accessor + ".size()" : current_accessor;
            out << indent << "if (" << condition << ")\n"
                << indent << "    ";
        } else
            out << indent;

        out << stream_name << ".write(\"" << *nbt_name_opt << "\", [this](util::nbt_write_stream& stream) {";


        auto nbt_override_opt = field.get_optional<std::string>("nbt_type_override");
        if (nbt_override_opt) {
            std::string override_type_str = *nbt_override_opt;
            std::string accessor = (is_optional(java_type) || preservation == "OPTIONAL") ? ("(*" + current_accessor + ")") : current_accessor;
            if (override_type_str == "int[3]" && java_type == "BlockPos") {
                out << "\n"
                    << indent << "    std::array<int32_t, 3> temp_arr = {" << accessor << ".x, " << accessor << ".y, " << accessor << ".z};\n"
                    << indent << "    util::encoding::nbt::serialize_entry(stream, temp_arr);\n"
                    << indent << "});\n";
            } else {
                out << " /* FIXME: Unhandled nbt_type_override */ util::encoding::nbt::serialize_entry(stream, " << (is_optional(java_type) || preservation == "OPTIONAL" ? "*" : "") + current_accessor << "); });\n";
            }
        } else if (field.get_child_optional("nested_fields") && !type_map.contains(java_type)) {
            out << "\n"
                << indent << "    auto compound_stream = stream.write_compound();\n";
            generate_to_nbt_body(out, current_accessor, prev_structs + "::" + field_name + "_t", "compound_stream", field.get_child("nested_fields"), indent_level + 1);
            out << indent << "});\n";
        } else if (auto enums = field.get_child_optional("enum_values")) {
            if (auto default_value = field.get_child_optional("enum_value_sample")) {
                out << "\n"
                    << indent << "    switch (" << (is_optional(java_type) ? "*" : "") << current_accessor << ") {";

                for (const auto& enum_pair : *enums) {
                    std::string member_name = get_enum_member_name(enum_pair.second);
                    std::string nbt_string = get_enum_nbt_string(enum_pair.second);
                    out << "\n"
                        << indent << "    case " << prev_structs << (prev_structs.empty() ? "" : "::") << field_name << "_e::" << member_name << ": stream.write(\"" << nbt_string << "\"); break;";
                }
                out << "\n"
                    << indent << "    default: stream.write(\"" << default_value->get_value<std::string>() << "\"); break;";
                out << "\n"
                    << indent << "    }\n";
            } else
                out << "\n"
                    << indent << "stream.write(static_cast<int>(" << (is_optional(java_type) ? "*" : "") << current_accessor << "));\n";
            out << indent << "});\n";
        } else if (java_type.starts_with("List<") || java_type == "ComponentMap") {
            std::string item_accessor = java_type == "ComponentMap" ? "item.second" : "item";
            out << "stream.write_list(" << current_accessor << ".size(), util::nbt_type::tag_compound).iterable(" << current_accessor
                << ", [this](auto& item, util::nbt_write_stream& item_stream) { util::encoding::nbt::serialize_entry(item_stream, " << item_accessor << "); }); });\n";
        } else {
            std::string value_accessor = is_optional(java_type) || preservation == "OPTIONAL" ? "*" + current_accessor : current_accessor;
            out << " util::encoding::nbt::serialize_entry(stream, " << value_accessor << "); });\n";
        }
    }
}

void generate_from_nbt_body(std::ostream& out, const std::string& result_accessor, const std::string& prev_structs, const pt::ptree& fields_node, int indent_level) {
    std::string indent(indent_level * 4, ' ');
    for (const auto& pair : fields_node) {
        const pt::ptree& field = pair.second;
        std::string field_name = get_field_name(field.get<std::string>("name"));
        auto nbt_name_opt = field.get_optional<std::string>("nbt_name");
        auto nested_fields_opt = field.get_child_optional("nested_fields");
        std::string preservation = field.get<std::string>("preservation");
        std::string java_type = field.get<std::string>("type");
        std::string current_accessor = result_accessor + "." + field_name;


        if (!nbt_name_opt && nested_fields_opt && type_map.contains(java_type)) {
            bool is_opt = is_optional(java_type) || preservation == "OPTIONAL";
            // Iterate through the nested JSON fields (x, y, z).
            for (const auto& nested_pair : *nested_fields_opt) {
                const pt::ptree& nested_field = nested_pair.second;
                auto nested_nbt_name_opt = nested_field.get_optional<std::string>("nbt_name");
                if (!nested_nbt_name_opt || *nested_nbt_name_opt == "_____UNKNOWN_____")
                    continue;

                std::string nested_field_name = get_field_name(nested_field.get<std::string>("name"));
                std::string nested_value_accessor = current_accessor;
                if (is_opt)
                    nested_value_accessor += "->";
                else
                    nested_value_accessor += ".";
                nested_value_accessor += nested_field_name;

                // Use 'collect' for optional and 'collect_required' for required fields.
                std::string collect_method = is_opt ? "collect" : "collect_required";

                out << "\n"
                    << indent << "." << collect_method << "(\"" << *nested_nbt_name_opt << "\", [&ref](util::nbt_read_stream& stream) {";
                // If the parent object is optional, we must create it before setting a member.
                if (is_opt) {
                    out << "\n"
                        << indent << "    if (!" << current_accessor << ") " << current_accessor << ".emplace();";
                }
                out << "\n"
                    << indent << "    util::encoding::nbt::deserialize_entry(" << nested_value_accessor << ", stream);";
                out << "\n"
                    << indent << "})";
            }
            continue; // Skip the original logic for this field.
        }

        if (!nbt_name_opt || *nbt_name_opt == "_____UNKNOWN_____")
            continue;


        std::string collect_method;
        if (java_type.starts_with("List<") || java_type == "ComponentMap")
            collect_method = (preservation == "REQUIRED" || preservation == "REQUIRED_DEFAULT_EMPTY") ? "collect_iterate_required" : "collect_iterate";
        else
            collect_method = (preservation == "REQUIRED" || preservation == "REQUIRED_DEFAULT_EMPTY") ? "collect_required" : "collect";

        out << "\n"
            << indent << "." << collect_method << "(\"" << *nbt_name_opt << "\", [&ref](util::nbt_read_stream& stream) {";


        auto nbt_override_opt = field.get_optional<std::string>("nbt_type_override");
        if (nbt_override_opt) {
            std::string override_type_str = *nbt_override_opt;
            if (override_type_str == "int[3]" && java_type == "BlockPos") {
                out << "\n"
                    << indent << "    std::array<int32_t, 3> temp_arr;";
                out << "\n"
                    << indent << "    util::encoding::nbt::deserialize_entry(temp_arr, stream);";
                if (preservation == "OPTIONAL") {
                    out << "\n"
                        << indent << "    " << current_accessor << ".emplace(temp_arr[0], temp_arr[1], temp_arr[2]);";
                } else {
                    out << "\n"
                        << indent << "    " << current_accessor << " = {temp_arr[0], temp_arr[1], temp_arr[2]};";
                }
            } else {
                out << " /* FIXME: Unhandled nbt_type_override */ util::encoding::nbt::deserialize_entry(" << current_accessor << ", stream);";
            }
            out << "\n"
                << indent << "})";
        } else if (nested_fields_opt && !type_map.contains(java_type)) {
            out << "\n"
                << indent << "    util::nbt_collection::compound_flex flex;\n"
                << indent << "    flex";
            generate_from_nbt_body(out, current_accessor, prev_structs + "::" + field_name + "_t", *nested_fields_opt, indent_level + 1);
            out << "\n"
                << indent << "        .make_collect(stream);\n"
                << indent << "})";
        } else if (auto enums = field.get_child_optional("enum_values")) {
            if (auto default_value = field.get_child_optional("enum_value_sample")) {
                out << "\n"
                    << indent << "    std::string nbt_val;\n";
                out << indent << "    stream.read_into(nbt_val);";
                bool first = true;
                for (const auto& enum_pair : *enums) {
                    std::string member_name = get_enum_member_name(enum_pair.second);
                    std::string nbt_string = get_enum_nbt_string(enum_pair.second);
                    out << "\n"
                        << indent << "    ";
                    if (first) {
                        first = false;
                    } else
                        out << "else ";
                    out << "if (nbt_val == \"" << nbt_string << "\") " << current_accessor << " = " << prev_structs << (prev_structs.empty() ? "" : "::") << field_name << "_e::" << member_name << ";";
                }
                auto def = default_value->get_value<std::string>();
                boost::algorithm::to_lower(def);
                out
                    << "\n"
                    << indent << "    " << (enums->size() ? "else " : "") << current_accessor << " = " << prev_structs << (prev_structs.empty() ? "" : "::") << field_name << "_e::" << def << ";";
            } else {
                out << "\n"
                    << indent << "    int nbt_val;\n";
                out << indent << "    stream.read_into(nbt_val);\n";
                out << indent << "    " << current_accessor << " = static_cast<" << prev_structs << (prev_structs.empty() ? "" : "::") << field_name << "_e>(nbt_val);";
            }
            out << "\n"
                << indent << "})";
        } else if (java_type.starts_with("List<")) {
            out << " " << current_accessor << ".push_back({}); util::encoding::nbt::deserialize_entry(" << current_accessor << ".back(), stream); })";
        } else if (java_type == "ComponentMap") {
            out << " base_objects::component component; util::encoding::nbt::deserialize_entry(component, stream); "
                << current_accessor << "[component.get_id()] = std::move(component); })";
        } else
            out << " util::encoding::nbt::deserialize_entry(" << current_accessor << ", stream); })";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_json> <out_block_entities.hpp> <out_block_entities.cpp>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string out_header = argv[2];
    std::string out_source = argv[3];


    std::filesystem::create_directories(std::filesystem::path(out_header).parent_path());
    std::filesystem::create_directories(std::filesystem::path(out_source).parent_path());

    if (std::filesystem::exists(out_header) && std::filesystem::exists(out_source)) {
        try {
            auto tool_last_write = std::filesystem::last_write_time(argv[0]);
            auto output_last_header_w = std::filesystem::last_write_time(out_header);
            auto output_last_source_w = std::filesystem::last_write_time(out_source);
            auto input_last_w = std::filesystem::last_write_time(input_file);

            if (tool_last_write < input_last_w)
                if (output_last_header_w < input_last_w)
                    if (output_last_source_w < input_last_w)
                        return 0;
        } catch (const std::filesystem::filesystem_error& err) {
            std::cerr << "Failed to get headers and output last write time, reason: " << err.what();
            return 1;
        }
    }

    pt::ptree root;
    try {
        pt::read_json(input_file, root);
    } catch (const pt::json_parser_error& e) {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return 1;
    }
    std::ofstream header_file(out_header);
    std::ofstream source_file(out_source);

    const std::string file_header =
        R"(/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * This file is generated automatically. Any changes will be overwritten.
 */
)";
    header_file << file_header
                << "\n#pragma once\n\n"
                   "#include <src/api/id.hpp>\n"
                   "#include <src/base_objects/block_entity.hpp>\n"
                   "#include <src/base_objects/chat.hpp>\n"
                   "#include <src/base_objects/component.hpp>\n"
                   "#include <src/base_objects/container.hpp>\n"
                   "#include <src/base_objects/item_predicate.hpp>\n"
                   "#include <src/base_objects/pool.hpp>\n"
                   "#include <src/base_objects/slot.hpp>\n"
                   "#include <src/base_objects/uuid.hpp>\n"
                   "\n"
                   "namespace copper_server::util {\n"
                   "    class nbt_read_stream;\n"
                   "    class nbt_write_stream;\n"
                   "}\n"
                   "\n"
                   "namespace copper_server::generated::block_entity {\n";
    source_file << file_header
                << "\n#include \""
                << out_header << "\"\n"
                                 "\n"
                                 "#include <src/util/reflect.hpp>\n"
                                 "#include <src/util/reflect/api/packets/slot.hpp>\n"
                                 "#include <src/util/reflect/api/packets/types.hpp>\n"
                                 "#include <src/util/reflect/base_objects/block_entity.hpp>\n"
                                 "#include <src/util/reflect/base_objects/component.hpp>\n"
                                 "#include <src/util/reflect/base_objects/dye_color.hpp>\n"


                                 "\n"
                                 "#include <src/util/nbt_stream.hpp>\n"
                                 "#include <src/util/encoding/nbt/serialization.hpp>\n"
                                 "#include <src/util/encoding/nbt/deserialization.hpp>\n"
                                 "\n"
                                 "namespace copper_server::generated::block_entity {\n\n";

    try {
        std::string from_nbt_select;

        for (const auto& pair : root) {
            if (pair.first.starts_with("data:"))
                continue;

            const std::string struct_name = sanitize_name(pair.first);
            const pt::ptree& block_node = pair.second;
            const auto& fields_node_opt = block_node.get_child_optional("fields");
            if (!fields_node_opt)
                continue;

            header_file << "    struct " << struct_name << " : public base_objects::block_entity {\n";
            generate_struct_fields(header_file, *fields_node_opt, 2);
            header_file << "\n";
            header_file << "        virtual ~" << struct_name << "() = default;\n";
            header_file << "        void to_nbt(util::nbt_write_stream& stream) override;\n";
            header_file << "        static std::unique_ptr<base_objects::block_entity> from_nbt(util::nbt_read_stream& stream);\n";
            header_file << "        std::unique_ptr<base_objects::block_entity> clone() const override;\n";
            header_file << "    };\n\n";

            source_file << "    void " << struct_name << "::to_nbt(util::nbt_write_stream& stream) {\n";
            source_file << "        auto compound_stream = stream.write_compound();\n";
            source_file << "        to_nbt_base_data(compound_stream);\n";
            generate_to_nbt_body(source_file, "(*this)", struct_name, "compound_stream", *fields_node_opt, 2);
            source_file << "    }\n";

            source_file << "    std::unique_ptr<base_objects::block_entity> " << struct_name << "::from_nbt(util::nbt_read_stream& stream) {\n";
            source_file << "        std::unique_ptr<" << struct_name << "> result = std::make_unique<" << struct_name << ">();\n";
            source_file << "        " << struct_name << "& ref = *result;\n";
            source_file << "        util::nbt_collection::compound_flex read_flex;\n";
            source_file << "        ref.from_nbt_base_data(read_flex);\n";
            source_file << "        read_flex";
            generate_from_nbt_body(source_file, "ref", struct_name, *fields_node_opt, 3);
            source_file << "\n            .make_collect(stream);\n";
            source_file << "        return result;\n";
            source_file << "    }\n";
            source_file << "    std::unique_ptr<base_objects::block_entity> " << struct_name << "::clone() const {\n";
            source_file << "        return std::make_unique<" << struct_name << ">(*this);\n";
            source_file << "    }\n";
            from_nbt_select += "            case " + std::to_string(block_node.get<int>("id")) + ": return " + struct_name + "::from_nbt(stream);\n";
        }

        header_file << "    std::unique_ptr<base_objects::block_entity> from_nbt(base_objects::block_id_t id, util::nbt_read_stream& stream);\n";

        // clang-format off
        source_file <<
            "    std::unique_ptr<base_objects::block_entity> from_nbt(base_objects::block id, util::nbt_read_stream& stream){\n"
            "        if (id.is_block_entity()){\n"
            "            switch (id.block_entity_id()){\n"
                               << from_nbt_select << 
                            "            default: return {};\n"
            "            }\n"
            "        } else\n"
            "            return {};\n"
            "    }\n";
        // clang-format on
        header_file << "} // namespace copper_server::generated::block_entity\n";
        source_file << "} // namespace copper_server::generated::block_entity\n";

        std::cout << "Successfully generated C++ files:\n- " << out_header << "\n- " << out_source << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Failed to build resource: " << input_file << ", unexected error: " << ex.what()
                  << ", stack trace " << std::stacktrace::current() << std::endl;
        return 1;
    }
    return 0;
}
