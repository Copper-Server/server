#include <filesystem>
#include <fstream>
#include <iostream>

// get input and output file names from command line
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <name> <input file>\n";
        return 1;
    }

    const char* _name = argv[1];
    std::filesystem::path input_file = argv[2];
    std::filesystem::path output_file = input_file.parent_path() / "embed" / (input_file.filename().string() + ".cc");
    std::filesystem::path header_file = input_file.parent_path() / "embed" / (input_file.filename().string() + ".hpp");
    auto file_size = std::filesystem::file_size(input_file);
    auto file_change_time = std::filesystem::last_write_time(input_file);

    std::filesystem::create_directories(output_file.parent_path());
    std::filesystem::create_directories(header_file.parent_path());

    if (std::filesystem::exists(output_file) && std::filesystem::exists(header_file)) {
        auto output_file_time = std::filesystem::last_write_time(output_file);
        auto header_file_time = std::filesystem::last_write_time(header_file);
        if (file_change_time < output_file_time && file_change_time < header_file_time)
            return 0;
    }
    std::cout << "Building resource: " << input_file << std::endl;

    std::ifstream in(input_file, std::ios::binary);
    if (!in) {
        std::cerr << "Error opening input file: " << input_file << "\n";
        return 1;
    }

    std::ofstream out(output_file, std::ios::binary);
    if (!out) {
        std::cerr << "Error opening output file: " << output_file << "\n";
        return 1;
    }

    std::ofstream header(header_file, std::ios::binary);
    if (!header) {
        std::cerr << "Error opening header file: " << header_file << "\n";
        return 1;
    }

    out << "#include \"" << input_file.filename().string() + ".hpp\"\n";
    out << "const std::array<uint8_t, " << file_size << "> " << _name << " = {\n    ";
    unsigned char c;
    while (in.get(reinterpret_cast<char&>(c)))
        out << "0x" << std::hex << static_cast<int>(c) << ", ";
    out << "\n};\n";


    header << "#pragma once\n\n";
    header << "#include<array>\n";
    header << "#include<cstdint>\n";
    header << "extern const std::array<uint8_t,  " << file_size << "> " << _name << ";\n";

    std::cout << "Resource built successfully: " << input_file << std::endl;
    return 0;
}