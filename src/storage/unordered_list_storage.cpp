#include <fstream>
#include <src/storage/unordered_list_storage.hpp>

namespace copper_server::storage {
    unordered_list_storage::unordered_list_storage(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path.parent_path());
            std::ofstream file;
            file.open(path, std::ios::out);
            file.close();
            _is_loaded = true;
            return;
        }
        std::ifstream file;
        file.open(path, std::ios::in);
        if (!file.is_open())
            return;
        data.set([&](auto& value) {
            for (std::string line; std::getline(file, line);)
                value.insert(line);
        });
        file.close();
        _is_loaded = true;
    }

    void unordered_list_storage::add(const std::string& set_value) {
        bool save = false;
        data.set([&](auto& value) {
            if (!value.contains(set_value)) {
                value.insert(set_value);
                save = true;
            }
        });
        if (save) {
            std::ofstream file;
            file.open(path, std::ios::out | std::ios::app);
            file << set_value << std::endl;
            file.flush();
            file.close();
        }
    }

    bool unordered_list_storage::contains(const std::string& check_value) {
        return data.get([&](auto& value) {
            return value.contains(check_value);
        });
    }

    void unordered_list_storage::remove(const std::string& rem_value) {
        bool save = false;
        data.set([&](auto& value) {
            auto it = value.find(rem_value);
            if (it != value.end()) {
                value.erase(it);
                save = true;
            }
        });

        if (save) {
            std::ofstream file;
            file.open(path, std::ios::out | std::ios::trunc);
            data.get([&](auto& value) {
                for (const auto& line : value)
                    file << line << '\n';
            });
            file.flush();
            file.close();
        }
    }

    std::unordered_set<std::string> unordered_list_storage::entrys(size_t max_items, bool& max_reached) {
        return data.get([&](auto& value) {
            if (value.size() <= max_items)
                return value;
            std::unordered_set<std::string> entrys;
            entrys.reserve(max_items);
            size_t i = 0;
            for (const auto& entry : value) {
                if (i++ == max_items) {
                    max_reached = 0;
                    break;
                }
                entrys.insert(entry);
            }
            return entrys;
        });
    }

    void unordered_list_storage::clear() {
        data.set([&](auto& value) {
            value.clear();
        });
        std::ofstream file;
        file.open(path, std::ios::out | std::ios::trunc);
        file.flush();
        file.close();
    }
}
