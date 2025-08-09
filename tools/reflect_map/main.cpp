#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stacktrace>
#include <unordered_set>

struct EnumInfo {
    std::string qualified_name;
    std::vector<std::pair<std::string, std::string>> values; // (name, value)
};

/**
 * @brief Helper function to trim leading and trailing whitespace from a std::string_view.
 * @param sv The string_view to trim.
 * @return A new, trimmed string_view.
 */
constexpr std::string_view trim_view(std::string_view sv) {
    const auto first = sv.find_first_not_of(" \t\n\r");
    if (first == std::string_view::npos) {
        return {}; // Return an empty view if the string is all whitespace
    }
    const auto last = sv.find_last_not_of(" \t\n\r");
    return sv.substr(first, last - first + 1);
}

/**
 * @brief Refactors a C++ template parameter string to be more readable and concise.
 *
 * This function improves upon the original by using std::string_view for efficient,
 * allocation-free parsing and a single-pass approach to build the result. This enhances
 * both readability by simplifying the logic and performance by reducing overhead.
 *
 * Transformations performed:
 * 1. Removes `class` and `typename` keywords.
 * 2. Normalizes parameter packs from `... Name` to `Name...`.
 * 3. Extracts only the parameter name from a full declaration (e.g., `const T& name` -> `name`).
 * 4. Keeps type-only parameters (e.g., `T` -> `T`).
 *
 * @param str The template parameter string to modify in-place.
 * Example Input:  "class T, const T& min, class ...Args"
 * Example Output: "T, min, Args..."
 */
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

void prepare_template_head(const std::unordered_set<std::string>& concepts, std::string& str) {
    if (str.empty())
        return;

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
        bool enum_class = false;
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
        std::vector<std::string> output_funcs;
        std::vector<EnumInfo> enums;
        EnumInfo current_enum;

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
                tmpl_type = struct_name.substr(bar2 + 1) + "<" + tmpl_arg_pass + ">";
                real_struct_name = tmpl_type;
            } else
                tmpl_decl = "template<class FN>";

            //for_each_field
            func << tmpl_decl;
            func << "constexpr void for_each_field([[maybe_unused]] ";
            for (const auto& ns : namespace_stack)
                if (!ns.empty())
                    func << ns << "::";
            for (const auto& s : struct_stack)
                func << s << "::";
            func << real_struct_name << "& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  fn(obj." << f << ");\n";
            func << "}\n";

            //for_each_field const
            func << tmpl_decl;
            func << "constexpr void for_each_field([[maybe_unused]] const ";
            for (const auto& ns : namespace_stack)
                if (!ns.empty())
                    func << ns << "::";
            for (const auto& s : struct_stack)
                func << s << "::";
            func << real_struct_name << "& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  fn(obj." << f << ");\n";
            func << "}\n";

            //for_each_type_s
            func << "template<" << tmpl_args << ">";
            func << "struct for_each_type_s<";
            for (const auto& ns : namespace_stack)
                if (!ns.empty())
                    func << ns << "::";
            for (const auto& s : struct_stack)
                func << s << "::";
            func << real_struct_name << "> {\n";
            func << "template<class FN>static constexpr void each([[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields) {
                func << "  fn.template operator()<decltype(std::declval<";
                for (const auto& ns : namespace_stack)
                    if (!ns.empty())
                        func
                            << ns << "::";
                for (const auto& s : struct_stack)
                    func << s << "::";
                func << real_struct_name << ">()." << f << ")>();\n";
            }
            func << "}\n};\n";

            //for_each_type_with_name_s
            func << "template<" << tmpl_args << ">";
            func << "struct for_each_type_with_name_s<";
            for (const auto& ns : namespace_stack)
                if (!ns.empty())
                    func << ns << "::";
            for (const auto& s : struct_stack)
                func << s << "::";
            func << real_struct_name << "> {\n";

            func << "template<class FN>static constexpr void each([[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields) {
                func << "  fn.template operator()<decltype(std::declval<";
                for (const auto& ns : namespace_stack)
                    if (!ns.empty())
                        func
                            << ns << "::";
                for (const auto& s : struct_stack)
                    func << s << "::";
                func << real_struct_name << ">()." << f << ")>(\"" << f << "\");\n";
            }
            func << "}\n};\n";


            //for_each_field_with_name
            func << tmpl_decl;
            func << "constexpr void for_each_field_with_name([[maybe_unused]] ";
            for (const auto& ns : namespace_stack)
                if (!ns.empty())
                    func << ns << "::";
            for (const auto& s : struct_stack)
                func << s << "::";
            func << real_struct_name << "& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  fn(obj." << f << ", \"" << f << "\");\n";
            func << "}\n";

            //for_each_field_with_name const
            func << tmpl_decl;
            func << "constexpr void for_each_field_with_name([[maybe_unused]] const ";
            for (const auto& ns : namespace_stack)
                if (!ns.empty())
                    func << ns << "::";
            for (const auto& s : struct_stack)
                func << s << "::";
            func << real_struct_name << "& obj, [[maybe_unused]] FN&& fn){\n";
            for (const auto& f : fields)
                func << "  fn(obj." << f << ", \"" << f << "\");\n";
            func << "}\n";

            //type_name
            if (!is_template) {
                func << "template<>consteval std::string_view type_name<";
                for (const auto& ns : namespace_stack)
                    if (!ns.empty())
                        func << ns << "::";
                for (const auto& s : struct_stack)
                    func << s << "::";
                func << real_struct_name << ">() { return \"";
                for (const auto& ns : namespace_stack)
                    if (!ns.empty())
                        func << ns << "::";
                for (const auto& s : struct_stack)
                    func << s << "::";
                func << real_struct_name << "\"; }\n";
            }
            output_funcs.push_back(func.str());
        };


        std::istringstream iss(std::move(ss.str()));
        while (std::getline(iss, line)) {
            ltrim(line);
            rtrim(line);
            if (line.empty() || line.rfind("//") == 0) {
                continue; // Skip empty lines and comments
            }

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
                        std::cerr << peek_line << std::endl;
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
                    enums.push_back(current_enum);
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
                enum_class = false;
                if (line.find("class ", name_start) == name_start) {
                    name_start += std::string("class ").size();
                    enum_class = true;
                } else if (line.find("struct ", name_start) == name_start) {
                    name_start += std::string("struct ").size();
                    enum_class = true;
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

                    enums.push_back(current_enum);
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
                            && line[name_end + 1] == ':'))
                ) {
                    if (line[name_end] == ':' && line[name_end + 1] == ':')
                        name_end += 2;
                    else
                        ++name_end;
                }
                std::string check_struct_name = line.substr(name_start, name_end - name_start);
                ltrim(check_struct_name);
                rtrim(check_struct_name);

                if (check_struct_name.find('<') != std::string::npos || check_struct_name.find('>') != std::string::npos) {
                    std::cerr << "Skipping struct with template parameters: " << check_struct_name << std::endl;
                    std::cerr << "namespace_stack ";
                    for (auto& it : namespace_stack)
                        std::cerr << it << "::";
                    std::cerr << std::endl;

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
                    size_t colon = line.find(" : ");
                    if (colon != std::string::npos) { //bit field
                        size_t name_end = line.find_last_not_of(" \t", colon - 1);
                        size_t name_start = line.find_last_of(" \t", name_end);
                        field = line.substr(name_start + 1, name_end - name_start);
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
        if (!template_params.empty()) {
            std::cerr << "template_params " << template_params << " \n";
        }
        output_file << "// Generated code for header: " << header_path << "\n";
        // Output all generated functions
        for (const auto& f : output_funcs)
            output_file << f;
        for (const auto& e : enums) {
            output_file << "template<>struct enum_data<" << e.qualified_name << "> {";
            output_file << "using item = std::pair<std::string_view, " << e.qualified_name << ">;\n";

            output_file << "static constexpr inline std::array<item, " << e.values.size() << "> values = {";
            for (size_t i = 0; i < e.values.size(); ++i)
                output_file << "item{\"" << e.values[i].first << "\", " << e.qualified_name << "::" << e.values[i].first << "}" << (i < e.values.size() - 1 ? ", " : "");
            output_file << "};";
            output_file << "};\n";
            output_file << "template<>consteval std::string_view type_name<" << e.qualified_name << ">() { return \"" << e.qualified_name << "\"; }\n";
        }
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

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <output> <headers>\n"
                  << std::endl;
        return 1;
    }
    std::filesystem::path output_path = argv[1];
    std::vector<std::filesystem::path> headers;
    {
        std::string_view headers_ = argv[2];
        while (headers_.size()) {
            auto next = headers_.find(' ');
            if (next != headers_.npos) {
                headers.emplace_back(headers_.substr(0, next));
                headers_ = headers_.substr(next + 1);
            } else {
                headers.emplace_back(headers_);
                break;
            }
        }
    }
    if (std::filesystem::exists(output_path)) {
        try {
            bool need_update = false;
            for (const auto& header_path : headers) {
                need_update = std::filesystem::last_write_time(header_path) > std::filesystem::last_write_time(output_path);
                if (need_update)
                    break;
            }
            if (!need_update) {
                std::cout << "reflect_map: all done." << std::endl;
                return 0;
            }
        } catch (const std::filesystem::filesystem_error& err) {
            std::cerr << "Failed to get headers and output last_write time, reason: " << err.what();
            return 1;
        }
    }

    std::ofstream output_file(output_path, std::ios::trunc);
    if (!output_file) {
        std::cerr << "Failed to open output file: " << output_path << std::endl;
        return 1;
    }
    output_file << "// Generated by reflect_map tool\n";
    output_file << "#pragma once\n";
    output_file << "#include <string>\n";
    output_file << "namespace copper_server::reflect{\n";
    output_file << "template<class T>consteval std::string_view type_name();\n";
    output_file << "template<class T>struct enum_data{};\n";
    output_file << "template<class T>struct for_each_type_s{};\n";
    output_file << "template<class T>struct for_each_type_with_name_s{};\n";
    for (const auto& header_path : headers)
        if (process_file(output_file, header_path))
            return 1;
    output_file << "template<class T, class FN>constexpr void for_each_type(FN&& fn){for_each_type_s<T>::each(std::move(fn));}\n";
    output_file << "template<class T, class FN>constexpr void for_each_type_with_name(FN&& fn){for_each_type_with_name_s<T>::each(std::move(fn));}\n";
    output_file << "}\n";
    std::cout << "reflect_map: complete." << std::endl;
    return 0;
}
