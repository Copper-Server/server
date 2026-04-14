/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stacktrace>
#include <unordered_set>
#include <vector>

std::filesystem::path find_common_base(const std::filesystem::path& p1, const std::filesystem::path& p2) {
    auto it1 = p1.begin();
    auto it2 = p2.begin();

    auto end1 = p1.end();
    auto end2 = p2.end();

    std::filesystem::path common_path;

    while (it1 != end1 && it2 != end2 && *it1 == *it2) {
        common_path /= *it1;
        ++it1;
        ++it2;
    }

    return common_path;
}

std::filesystem::path get_output_file(const std::filesystem::path& input_file, const std::filesystem::path& output_path) {
    return output_path / std::filesystem::relative(input_file, find_common_base(input_file, output_path));
}

struct EnumInfo {
    std::string qualified_name;
    std::vector<std::pair<std::string, std::string>> values; // (name, value)
    bool is_class_enum = false;
    bool is_struct_enum = false;
};

constexpr std::string_view trim_view(std::string_view sv) {
    const auto first = sv.find_first_not_of(" \t\n\r");
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = sv.find_last_not_of(" \t\n\r");
    return sv.substr(first, last - first + 1);
}

void prepare_template_entry(const std::unordered_set<std::string>& concepts, std::string& str) {
    if (str.empty()) {
        return;
    }

    std::vector<std::string> final_names;
    std::string_view full_view(str);
    size_t start_pos = 0;

    // Iterate through the string, splitting by commas without creating intermediate substrings.
    while (start_pos < full_view.length()) {
        size_t end_pos = full_view.find(',', start_pos);
        if (end_pos == std::string_view::npos) {
            end_pos = full_view.length();
        }

        // --- Process each parameter segment using a string_view ---
        std::string_view param_view = trim_view(full_view.substr(start_pos, end_pos - start_pos));

        if (param_view.empty()) {
            start_pos = end_pos + 1;
            continue;
        }

        // 0. Remove default values (anything after '=')
        size_t equals_pos = param_view.find('=');
        if (equals_pos != std::string_view::npos) {
            param_view = trim_view(param_view.substr(0, equals_pos));
        }

        if (param_view.empty()) {
            start_pos = end_pos + 1;
            continue;
        }

        // 1. Remove `class` or `typename` prefixes.
        // In C++20, this can be simplified with param_view.starts_with("...").
        if (param_view.rfind("class ", 0) == 0) {
            param_view.remove_prefix(6);
        } else if (param_view.rfind("typename ", 0) == 0) {
            param_view.remove_prefix(9);
        } else
            for (auto& it : concepts)
                if (param_view.rfind(it, 0) == 0)
                    param_view.remove_prefix(it.size());

        param_view = trim_view(param_view); // Re-trim in case of extra spaces.

        // 2. Extract the final parameter name.
        std::string name;
        size_t ellipsis_pos = param_view.find("...");
        if (ellipsis_pos != std::string_view::npos) {
            // It's a parameter pack: `... Name` becomes `Name...`
            std::string_view identifier = trim_view(param_view.substr(ellipsis_pos + 3));
            name = std::string(identifier) + "...";
        } else {
            // It's a regular parameter: `const T& value` becomes `value`
            // Or a type-only parameter: `T` remains `T`
            size_t last_space = param_view.find_last_of(" \t\n\r*&");
            if (last_space != std::string_view::npos) {
                name = std::string(trim_view(param_view.substr(last_space + 1)));
            } else {
                name = std::string(param_view);
            }
        }

        if (!name.empty()) {
            final_names.push_back(std::move(name));
        }

        start_pos = end_pos + 1;
    }

    // --- Join the results into the final string ---
    if (final_names.empty()) {
        str.clear();
        return;
    }

    // Build the final string efficiently.
    std::string result;
    result.reserve(str.length()); // Reserve approximate capacity to avoid reallocations.
    result += final_names[0];
    for (size_t i = 1; i < final_names.size(); ++i) {
        result += ", ";
        result += final_names[i];
    }

    str = result;
}

void strip_template_defaults(std::string& str) {
    if (str.empty())
        return;

    std::string result;
    result.reserve(str.length());

    size_t start_pos = 0;
    while (start_pos < str.length()) {
        size_t comma_pos = str.find(',', start_pos);
        if (comma_pos == std::string::npos) {
            comma_pos = str.length();
        }

        // Extract parameter segment
        std::string_view param = trim_view(std::string_view(str).substr(start_pos, comma_pos - start_pos));

        // Find and remove default value (everything from '=' onwards)
        size_t equals_pos = param.find('=');
        if (equals_pos != std::string_view::npos) {
            param = trim_view(param.substr(0, equals_pos));
        }

        // Add to result with proper separator
        if (!result.empty() && !param.empty()) {
            result += ", ";
        }
        result += param;

        start_pos = comma_pos + 1;
    }

    str = result;
}

void prepare_template_head(const std::unordered_set<std::string>& concepts, std::string& str) {
    if (str.empty())
        return;

    // First, strip default values from template parameters
    strip_template_defaults(str);

    std::vector<std::string_view> tokens;
    std::string_view full_view(str);
    std::string_view delimiters = "<>, \t\r\n"; // Delimiters

    size_t current_pos = 0;
    while (current_pos < full_view.length()) {
        size_t delimiter_pos = full_view.find_first_of(delimiters, current_pos);

        if (delimiter_pos != std::string_view::npos) {
            if (delimiter_pos > current_pos) {
                tokens.push_back(full_view.substr(current_pos, delimiter_pos - current_pos));
            }
            tokens.push_back(full_view.substr(delimiter_pos, 1));
            current_pos = delimiter_pos + 1;
        } else {
            tokens.push_back(full_view.substr(current_pos));
            break;
        }
    }

    // Replace concepts with "class"
    bool was_modified = false;
    for (auto& token : tokens) {
        if (concepts.count(std::string(token))) {
            token = "class";
            was_modified = true;
        }
    }

    if (was_modified) {
        std::string result;
        result.reserve(str.length());
        for (const auto& token : tokens) {
            result += token;
        }
        str = std::move(result);
    }
}

// Remove leading/trailing whitespace
void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
}

void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(),
            s.end());
}

int process_file(std::ofstream& output_file, const std::filesystem::path& header_path) {
    std::stringstream ss;
    {
        std::ifstream header_file(header_path);
        if (!header_file) {
            std::cerr << "Failed to open header file: " << header_path << std::endl;
            return 1;
        }
        ss << header_file.rdbuf();
    }

    try {
        std::unordered_set<std::string> concepts_set;

        constexpr bool debug_out = false;
        std::string line;
        bool in_struct = false;
        bool in_enum = false;
        bool in_template = false;
        bool template_defined = false;
        bool skipping_function_body = false;
        int enum_brace_depth = 0;
        int function_brace_depth = 0;
        std::string struct_name;
        std::string template_params;
        std::vector<std::string> fields;
        std::vector<std::vector<std::string>> fields_stack;
        std::vector<std::string> struct_stack;
        std::vector<std::string> namespace_stack;
        std::vector<std::string> cached_output;
        EnumInfo current_enum;
        size_t skip_scopes = 0;

        auto build_fn = [&]() {
            for (const auto& ns : namespace_stack)
                if (ns == "std")
                    return;

            std::ostringstream func;
            std::string real_struct_name = struct_name;
            std::string tmpl_decl, tmpl_type, tmpl_args;
            bool is_template;
            if (is_template = struct_name.rfind("TEMPLATE|", 0) == 0; is_template) {
                size_t bar1 = struct_name.find('|');
                size_t bar2 = struct_name.find('|', bar1 + 1);
                tmpl_args = struct_name.substr(bar1 + 1, bar2 - bar1 - 1);
                prepare_template_head(concepts_set, tmpl_args);
                if (tmpl_args.empty()) {
                    tmpl_decl = "template<class FN>";
                } else {
                    tmpl_decl = "template<" + tmpl_args + ", class FN>";
                }

                std::string tmpl_arg_pass = struct_name.substr(bar1 + 1, bar2 - bar1 - 1);
                prepare_template_entry(concepts_set, tmpl_arg_pass);
                ltrim(tmpl_arg_pass);
                tmpl_type = struct_name.substr(bar2 + 1);
                real_struct_name = tmpl_type + "<" + tmpl_arg_pass + ">";
            } else
                tmpl_decl = "template<class FN>";


            //meta_for_type_s
            func << "template<" << tmpl_args << ">";
            func << "struct meta_for_type_s<";
            for (const auto& ns : namespace_stack)
                if (!ns.empty())
                    func << ns << "::";
            for (const auto& s : struct_stack)
                func << s << "::";
            func << real_struct_name << "> {\n";

            func << "using meta_type = ";
            for (const auto& ns : namespace_stack)
                if (!ns.empty())
                    func
                        << ns << "::";
            for (const auto& s : struct_stack)
                func << s << "::";
            func << real_struct_name << ";\n";

            //for_each_field
            func << "template<class FN>static constexpr void for_each_field([[maybe_unused]] meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  fn(obj." << f << ");\n";
            func << "}\n";

            //for_each_field const
            func << "template<class FN>static constexpr void for_each_field([[maybe_unused]] const meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  fn(obj." << f << ");\n";
            func << "}\n";


            //for_each_type_s
            func << "template<class FN>static constexpr void for_each_type([[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields) {
                func << "  fn.template operator()<decltype(std::declval<meta_type>()." << f << ")>();\n";
            }
            func << "}\n";

            //for_each_field_with_name
            func << "template<class FN>static constexpr void for_each_field_with_name([[maybe_unused]] meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  fn(obj." << f << ", \"" << f << "\");\n";
            func << "}\n";

            //for_each_field_with_name const
            func << "template<class FN>static constexpr void for_each_field_with_name([[maybe_unused]] const meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  fn(obj." << f << ", \"" << f << "\");\n";
            func << "}\n";

            //visit_field
            func << "template<class FN>static constexpr void visit_field([[maybe_unused]]std::string_view name, [[maybe_unused]] meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  if(name == \"" << f << "\") { fn(obj." << f << "); return; }\n";
            func << "}\n";

            //visit_field const
            func << "template<class FN>static constexpr void visit_field([[maybe_unused]]std::string_view name, [[maybe_unused]] const meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  if(name == \"" << f << "\") { fn(obj." << f << "); return; }\n";
            func << "}\n";

            //visit_field_with_name const
            func << "template<class FN>static constexpr void visit_field_with_name([[maybe_unused]]std::string_view name, [[maybe_unused]] const meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  if(name == \"" << f << "\") { fn(obj." << f << ", \"" << f << "\"); return; }\n";
            func << "}\n";

            //visit_field(index)
            func << "template<class FN>static constexpr void visit_field(size_t index, [[maybe_unused]] meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            if (fields.size()) {
                func << "  switch(index) {\n";
                for (size_t i = 0; i < fields.size(); ++i)
                    func << "    case " << i << ": fn(obj." << fields[i] << "); return;\n";
                func << "  }\n";
            }
            func << "}\n";

            //visit_field const(index)
            func << "template<class FN>static constexpr void visit_field(size_t index, [[maybe_unused]] const meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            if (fields.size()) {
                func << "  switch(index) {\n";
                for (size_t i = 0; i < fields.size(); ++i)
                    func << "    case " << i << ": fn(obj." << fields[i] << "); return;\n";
                func << "  }\n";
            }
            func << "}\n";

            //visit_field_with_name(index)
            func << "template<class FN>static constexpr void visit_field_with_name(size_t index, [[maybe_unused]] meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            if (fields.size()) {
                func << "  switch(index) {\n";
                for (size_t i = 0; i < fields.size(); ++i)
                    func << "    case " << i << ": fn(obj." << fields[i] << ", \"" << fields[i] << "\"); return;\n";
                func << "  }\n";
            }
            func << "}\n";

            //visit_field_with_name const(index)
            func << "template<class FN>static constexpr void visit_field_with_name(size_t index, [[maybe_unused]] const meta_type& obj, [[maybe_unused]] FN&& fn){\n";
            if (fields.size()) {
                func << "  switch(index) {\n";
                for (size_t i = 0; i < fields.size(); ++i)
                    func << "    case " << i << ": fn(obj." << fields[i] << ", \"" << fields[i] << "\"); return;\n";
                func << "  }\n";
            }
            func << "}\n";

            //visit_field(index)
            func << "template<class FN>static constexpr void visit_field(size_t index,  [[maybe_unused]] FN&& fn){\n";
            if (fields.size()) {
                func << "  switch(index) {\n";
                for (size_t i = 0; i < fields.size(); ++i)
                    func << "    case " << i << ": fn.template operator()<decltype(std::declval<meta_type>()." << fields[i] << ")>(); return;\n";
                func << "  }\n";
            }
            func << "}\n";

            //visit_field(name)
            func << "template<class FN>static constexpr void visit_field([[maybe_unused]]std::string_view name, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields) {
                func << "  if(name == \"" << f << "\") { fn.template operator()<decltype(std::declval<meta_type>()." << f << ")>(); return; }\n";
            }
            func << "}\n";

            //visit_field const(name)
            func << "template<class FN>static constexpr void visit(size_t index, [[maybe_unused]] FN&& fn){\n";
            if (fields.size()) {
                func << "  switch(index) {\n";
                for (size_t i = 0; i < fields.size(); ++i) {
                    func << "    case " << i << ": fn.template operator()<decltype(std::declval<meta_type>()." << fields[i] << ")>(); return;\n";
                }
                func << "  }\n";
            }
            func << "}\n";

            //visit_field_with_name (name)
            func << "template<class FN>static constexpr void visit_field_with_name([[maybe_unused]]std::string_view name, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields) {
                func << "  if(name == \"" << f << "\") { fn.template operator()<decltype(std::declval<meta_type>()." << f << ")>(\"" << f << "\"); return; }\n";
            }
            func << "}\n";

            //visit_field_with_name const (name)
            func << "template<class FN>static constexpr void visit_field_with_name(size_t index, [[maybe_unused]] FN&& fn){\n";
            if (fields.size()) {
                func << "  switch(index) {\n";
                for (size_t i = 0; i < fields.size(); ++i) {
                    func << "    case " << i << ": fn.template operator()<decltype(std::declval<meta_type>()." << fields[i] << ")>(\"" << fields[i] << "\"); return;\n";
                }
                func << "  }\n";
            }
            func << "}\n";
            //type_name

            if (!is_template) {
                func << "consteval std::string_view type_name() { return \"";
                for (const auto& ns : namespace_stack)
                    if (!ns.empty())
                        func << ns << "::";
                for (const auto& s : struct_stack)
                    func << s << "::";
                func << real_struct_name << "\"; }\n";
            } else {
                func << "static consteval std::string_view type_name() { return type_name_compile_time<meta_type>(); }\n";
            }
            func << "static constexpr inline size_t fields_count = " << fields.size() << ";\n";

            func << "};\n";


            cached_output.push_back(func.str());
        };

        auto build_enum = [&]() {
            std::ostringstream enm;
            auto& e = current_enum;
            enm << "template<>struct enum_data<" << e.qualified_name << "> {";
            enm << "using item = std::pair<std::string_view, " << e.qualified_name << ">;\n";

            enm << "static constexpr inline std::array<item, " << e.values.size() << "> values = {";
            for (size_t i = 0; i < e.values.size(); ++i)
                enm << "item{\"" << e.values[i].first << "\", " << e.qualified_name << "::" << e.values[i].first << "}" << (i < e.values.size() - 1 ? ", " : "");
            enm << "};";
            enm << "};\n";
            enm << "template<>consteval std::string_view type_name<" << e.qualified_name << ">() { return \"" << e.qualified_name << "\"; }\n";
            cached_output.push_back(enm.str());
        };

        std::cerr << header_path << "\n";
        std::istringstream iss(std::move(ss.str()));
        while (std::getline(iss, line)) {
            ltrim(line);
            rtrim(line);
            if (line.empty())
                continue;
            if (line.rfind("//") == 0) {
                if (line.rfind("//reflect_map skip_begin") == 0) {
                    ++skip_scopes;
                } else if (line.rfind("//reflect_map skip_end") == 0 && skip_scopes != 0) {
                    --skip_scopes;
                } else
                    continue;
            }
            if (skip_scopes != 0)
                continue;

            if (line.rfind("concept ", 0) == 0) {
                std::string_view concept_name = std::string_view(line).substr(8);
                auto it = concepts_set.emplace(concept_name.substr(0, concept_name.find_first_of("<>= \t\r")));
                template_params.clear();
                in_template = false;
                template_defined = false;
                if constexpr (debug_out)
                    std::cerr << "SSS0 " << line << std::endl;
                continue;
            }

            // --- Function body skipping logic ---
            if (skipping_function_body) {
                function_brace_depth += std::count(line.begin(), line.end(), '{');
                function_brace_depth -= std::count(line.begin(), line.end(), '}');
                if (function_brace_depth <= 0)
                    skipping_function_body = false;
                if constexpr (debug_out)
                    std::cerr << "SSS1 " << line << std::endl;
                continue;
            }
            if (auto fn_detect = line.find(')'); fn_detect != std::string::npos) {
                if (line.find(';') != std::string::npos) {
                    template_params.clear();
                    template_defined = false;
                    in_template = false;
                    if constexpr (debug_out)
                        std::cerr << "SSS2 " << line << std::endl;
                    continue;
                }
                if (line.find('{', fn_detect) == std::string::npos) {
                    // Peek ahead to see if next non-empty line starts with '{'
                    std::streampos prev_pos = iss.tellg();
                    std::string peek_line;
                    bool found_brace = false;
                    while (std::getline(iss, peek_line)) {
                        ltrim(peek_line);
                        if (!peek_line.empty()) {
                            if (peek_line.find('{') != std::string::npos) {
                                found_brace = true;
                                break;
                            }

                            if (!peek_line.rfind("requires ", 0) == 0)
                                break;
                        }
                    }
                    if (found_brace) {
                        skipping_function_body = true;
                        template_params.clear();
                        template_defined = false;
                        in_template = false;
                        function_brace_depth = 1;
                        function_brace_depth += std::count(peek_line.begin(), peek_line.end(), '{') - 1;
                        function_brace_depth -= std::count(peek_line.begin(), peek_line.end(), '}');
                        if constexpr (debug_out)
                            std::cerr << "SSS3 " << line << std::endl;
                        continue;
                    } else {
                        // Not a function body, rewind
                        if (iss.good())
                            iss.seekg(prev_pos);
                        if constexpr (debug_out)
                            std::cerr << "skipped line: " << peek_line << std::endl;
                    }
                } else {
                    skipping_function_body = true;
                    template_params.clear();
                    template_defined = false;
                    in_template = false;
                    function_brace_depth = 1;
                    function_brace_depth += std::count(line.begin(), line.end(), '{') - 1;
                    function_brace_depth -= std::count(line.begin(), line.end(), '}');
                    continue;
                }
            }

            if (line.rfind("template <", 0) == 0) {
                in_template = true;
                template_params = line.substr(std::string("template <").size());
                size_t end = template_params.find('>');
                if (end != std::string::npos)
                    template_params = template_params.substr(0, end);
                ltrim(template_params);
                rtrim(template_params);
                template_defined = true;
                if constexpr (debug_out)
                    std::cerr << "SSS4 " << line << std::endl;
                continue;
            }

            if (template_defined) {
                if (line.rfind("struct ", 0) != 0) {
                    template_params.clear();
                    in_template = false;
                    if constexpr (debug_out)
                        std::cerr << "SSS5 " << line << std::endl;
                    continue;
                }
                template_defined = false;
            }

            if (in_enum) {
                enum_brace_depth += std::count(line.begin(), line.end(), '{');
                enum_brace_depth -= std::count(line.begin(), line.end(), '}');
                // Parse enum values
                if (size_t comment = line.find("//"); comment != std::string::npos) // Remove trailing comment if any
                    line = line.substr(0, comment);
                std::string enum_line = line;
                ltrim(enum_line);
                rtrim(enum_line);
                if (!enum_line.empty() && enum_line != "{" && enum_line != "};" && enum_line != "}") {
                    // Remove trailing comma or semicolon
                    if (enum_line.back() == ',' || enum_line.back() == ';')
                        enum_line.pop_back();
                    // Split by '=' if present
                    size_t eq = enum_line.find('=');
                    std::string name, value;
                    if (eq != std::string::npos) {
                        name = enum_line.substr(0, eq);
                        value = enum_line.substr(eq + 1);
                        rtrim(name);
                        ltrim(value);
                    } else {
                        name = enum_line;
                        value = "";
                    }
                    rtrim(name);
                    if (!name.empty())
                        current_enum.values.emplace_back(name, value);
                }
                if (enum_brace_depth <= 0 && line.find('}') != std::string::npos && line.find(';') != std::string::npos) {
                    // End of enum
                    build_enum();
                    current_enum = EnumInfo{};
                    in_enum = false;
                }
                if constexpr (debug_out)
                    std::cerr << "SSS6 " << line << std::endl;
                continue;
            }
            if (line.rfind("enum ", 0) == 0) {
                // Enter enum, count braces
                in_enum = true;
                enum_brace_depth = 0;
                enum_brace_depth += std::count(line.begin(), line.end(), '{');
                enum_brace_depth -= std::count(line.begin(), line.end(), '}');
                // Parse enum name
                size_t name_start = std::string("enum ").size();
                if (line.find("class ", name_start) == name_start) {
                    name_start += std::string("class ").size();
                    current_enum.is_class_enum = true;
                } else if (line.find("struct ", name_start) == name_start) {
                    name_start += std::string("struct ").size();
                    current_enum.is_struct_enum = true;
                }
                size_t name_end = line.find_first_of(":{", name_start);
                if (name_end == std::string::npos)
                    name_end = line.size();
                std::string enum_name = line.substr(name_start, name_end - name_start);
                ltrim(enum_name);
                rtrim(enum_name);
                // Build qualified name
                std::ostringstream qname;
                for (const auto& ns : namespace_stack)
                    if (!ns.empty())
                        qname << ns << "::";
                for (const auto& s : struct_stack)
                    qname << s << "::";
                if (in_struct)
                    qname << struct_name << "::";
                qname << enum_name;
                current_enum.qualified_name = qname.str();
                current_enum.values.clear();
                // If enum is one-liner, exit immediately
                if (enum_brace_depth <= 0 && line.find('}') != std::string::npos && line.find(';') != std::string::npos) {
                    build_enum();
                    current_enum = EnumInfo{};
                    in_enum = false;
                }
                if constexpr (debug_out)
                    std::cerr << "SSS7 " << line << std::endl;
                continue;
            }

            // Handle namespace open
            if (line.rfind("namespace ", 0) == 0) {
                size_t ns_start = std::string("namespace ").size();
                size_t ns_end = line.find('{', ns_start);
                std::string ns_name = line.substr(ns_start, ns_end - ns_start);
                ltrim(ns_name);
                rtrim(ns_name);
                namespace_stack.push_back(ns_name);
                if constexpr (debug_out)
                    std::cerr << "SSS8 " << line << std::endl;
                continue;
            }
            // Handle namespace close
            if (line == "}") {
                if (!namespace_stack.empty()) {
                    namespace_stack.pop_back();
                    if constexpr (debug_out)
                        std::cerr << "SSS9 " << line << std::endl;
                    continue;
                }
            }
            // Handle struct open
            if (line.rfind("struct ", 0) == 0) {
                if (line.find('{') == std::string::npos) {
                    if constexpr (debug_out)
                        std::cerr << "SSS10 " << line << std::endl;
                    continue;
                }
                if (line.find_first_of("()") != std::string::npos) {
                    if constexpr (debug_out)
                        std::cerr << "SSS11 " << line << std::endl;
                    continue;
                }
                bool complete_struct = line.find('};') != std::string::npos;
                size_t name_start = std::string("struct ").size();
                size_t name_end = name_start;
                while (
                    name_end < line.size()
                    && (std::isalnum(line[name_end])
                        || line[name_end] == '_'
                        || line[name_end] == '<'
                        || line[name_end] == '>'
                        || (line[name_end] == ':'
                            && name_end + 1 < line.size()
                            && line[name_end + 1] == ':'))) {
                    if (line[name_end] == ':' && line[name_end + 1] == ':')
                        name_end += 2;
                    else
                        ++name_end;
                }
                std::string check_struct_name = line.substr(name_start, name_end - name_start);
                ltrim(check_struct_name);
                rtrim(check_struct_name);

                if (check_struct_name.find('<') != std::string::npos || check_struct_name.find('>') != std::string::npos) {
                    if constexpr (debug_out) {
                        std::cerr << "Skipping struct with template parameters: " << check_struct_name << std::endl;
                        std::cerr << "namespace_stack ";
                        for (auto& it : namespace_stack)
                            std::cerr << it << "::";
                        std::cerr << std::endl;
                    }

                    template_params.clear();
                    in_template = false;
                    template_defined = false;
                    if constexpr (debug_out)
                        std::cerr << "SSS12 " << line << std::endl;
                    continue;
                }
                if (in_struct && !struct_name.empty()) {
                    struct_stack.push_back(std::move(struct_name));
                    fields_stack.push_back(std::move(fields));
                    fields.clear();
                }
                in_struct = true;
                struct_name = check_struct_name;
                if (in_template) {
                    struct_name = "TEMPLATE|" + template_params + "|" + struct_name;
                    in_template = false;
                    template_defined = false;
                    template_params.clear();
                }
                if (complete_struct) {
                    line = "};";
                } else {
                    if constexpr (debug_out)
                        std::cerr << "SSS13 " << line << std::endl;
                    continue;
                }
            }

            // Handle struct close
            if (in_struct && line == "};") {
                build_fn();
                if (!struct_stack.empty()) {
                    struct_name = std::move(struct_stack.back());
                    struct_stack.pop_back();
                    fields = std::move(fields_stack.back());
                    fields_stack.pop_back();
                    in_struct = true;
                } else {
                    in_struct = false;
                    struct_name.clear();
                    fields.clear();
                }
                continue;
            }
            // Collect fields
            if (in_struct) {
                // Ignore using and static fields
                if (line.rfind("using ", 0) == 0 || line.rfind("static ", 0) == 0) {
                    template_params.clear();
                    in_template = false;
                    template_defined = false;
                    if constexpr (debug_out)
                        std::cerr << "SSS14 " << line << std::endl;
                    continue;
                }
                size_t semi = line.find(';');
                if (semi != std::string::npos && line.find('(') == std::string::npos) {
                    if (size_t comment = line.find("//"); comment != std::string::npos) // Remove trailing comment if any
                        line = line.substr(0, comment);

                    line = line.substr(0, semi);
                    line = line.substr(0, line.find("=")); //remove assignment if any
                    rtrim(line);

                    std::string field;
                    if (size_t colon = line.find(" : "); colon != std::string::npos) { //bit field
                        size_t name_end = line.find_last_not_of(" \t", colon - 1);
                        size_t name_start = line.find_last_of(" \t", name_end);
                        if (name_start != std::string::npos)
                            field = line.substr(name_start + 1, name_end - name_start);
                        else
                            field = line.substr(0, name_end);
                    } else if (size_t array = line.find("["); array != std::string::npos) { //array field
                        size_t name_end = line.find_last_not_of(" \t", array - 1);
                        size_t name_start = line.find_last_of(" \t", name_end);
                        if (name_start != std::string::npos)
                            field = line.substr(name_start + 1, name_end - name_start);
                        else
                            field = line.substr(0, name_end);
                    } else {
                        size_t last = line.find_last_not_of(" \t");
                        if (last != std::string::npos) {
                            size_t first = line.find_last_of(" \t", last);
                            field = line.substr(first + 1, last - first);
                        }
                    }

                    if (field.find_first_of("<>") == std::string::npos && !field.empty()) //filter field name
                        fields.push_back(field);
                }
            }

            if constexpr (debug_out)
                std::cerr << "SSS15 " << line << std::endl;
        }

        for (auto& it : namespace_stack) {
            std::cerr << "namespace " << it << " \n";
        }
        for (auto& it : fields_stack) {
            std::cerr << "fields{\n";
            for (auto& f : it) {
                std::cerr << "field " << f << " \n";
            }
            std::cerr << "}\n";
        }
        for (auto& it : struct_stack) {
            std::cerr << "struct " << it << " \n";
        }
        if (!struct_name.empty()) {
            std::cerr << "struct " << struct_name << " \n";
        }
        if (!template_params.empty())
            std::cerr << "template_params " << template_params << " \n";
        for (const auto& out : cached_output)
            output_file << out;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to build resource: " << header_path << ", unexected error: " << ex.what()
                  << ", stack trace " << std::stacktrace::current() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Failed to build resource: " << header_path << ", unexected error "
                  << ", stack trace " << std::stacktrace::current() << std::endl;
        return 1;
    }
    return 0;
}

struct need_to_update {
    std::filesystem::path header;
    std::filesystem::path output;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <output_path> <headers>\n"
                  << std::endl;
        return 1;
    }
    std::filesystem::path output_path = argv[1];
    std::filesystem::create_directories(output_path);
    std::vector<std::filesystem::path> outputs;
    std::vector<std::filesystem::path> headers;


    std::vector<need_to_update> to_update;

    {
        std::string_view headers_ = argv[2];
        while (headers_.size()) {
            auto next = headers_.find(' ');
            if (next != headers_.npos) {
                headers.emplace_back(headers_.substr(0, next));
                outputs.emplace_back(get_output_file(headers.back(), output_path));
                headers_ = headers_.substr(next + 1);
            } else {
                headers.emplace_back(headers_);
                break;
            }
        }
    }
    if (std::filesystem::exists(output_path)) {
        auto tool_last_write = std::filesystem::last_write_time(argv[0]);
        try {
            size_t max = headers.size();
            for (size_t i = 0; i < max; ++i) {
                bool need_update = !std::filesystem::exists(outputs[i]);
                if (!need_update) {
                    auto output_last_w = std::filesystem::last_write_time(outputs[i]);
                    need_update = std::filesystem::last_write_time(headers[i]) > output_last_w || output_last_w < tool_last_write;
                } else
                    std::filesystem::create_directories(outputs[i].parent_path());

                if (need_update) {
                    need_to_update update;
                    update.header = std::move(headers[i]);
                    update.output = std::move(outputs[i]);
                    to_update.emplace_back(std::move(update));
                }
            }
            if (to_update.empty()) {
                std::cout << "reflect_map: all done." << std::endl;
                return 0;
            }
        } catch (const std::filesystem::filesystem_error& err) {
            std::cerr << "Failed to get headers and output last_write time, reason: " << err.what();
            return 1;
        }
        headers.clear();
        outputs.clear();
    }

    for (auto& [header, output] : to_update) {
        std::cout << "reflect_map: processing " << header << " -> " << output << std::endl;
        std::ofstream output_file(output, std::ios::trunc);
        if (!output_file) {
            std::cerr << "Failed to open output file: " << output << std::endl;
            return 1;
        }
        output_file << "// Generated by reflect_map tool\n";
        output_file << "#pragma once\n";
        output_file << "#include <string>\n";
        output_file << "namespace copper_server::reflect{\n";
        if (process_file(output_file, header))
            return 1;
        output_file << "}\n";
    }
    std::cout << "reflect_map: complete." << std::endl;
    return 0;
}
