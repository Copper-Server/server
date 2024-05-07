#include "enbt_list_storage.hpp"
#include <filesystem>
#include <fstream>

namespace crafted_craft {
    namespace storage {
        enbt_list_storage::enbt_list_storage(const std::string& path)
            : path(path) {
            std::filesystem::path p(path);
            if (!std::filesystem::exists(p)) {
                std::filesystem::create_directories(p.parent_path());
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
            while (!file.eof()) {
                std::string key = (std::string)ENBTHelper::ReadString(file);
                data[key] = ENBTHelper::ReadToken(file);
            }
            _is_loaded = true;
            file.close();
        }

        void enbt_list_storage::add(const std::string& key, const ENBT& enbt) {
            if (data.find(key) == data.end()) {
                data[key] = enbt;
                std::fstream file;
                file.open(path, std::ios::out | std::ios::app);
                ENBTHelper::WriteString(file, key);
                ENBTHelper::WriteToken(file, enbt);
                file.flush();
                file.close();
            }
        }

        std::optional<ENBT> enbt_list_storage::get(const std::string& key) {
            auto it = data.find(key);
            if (it != data.end())
                return std::optional(it->second);
            return std::nullopt;
        }

        bool enbt_list_storage::contains(const std::string& value) {
            return data.contains(value);
        }

        void enbt_list_storage::remove(const std::string& key) {
            if (data.find(key) != data.end()) {
                data.erase(key);
                std::fstream file;
                file.open(path, std::ios::out | std::ios::trunc);
                for (const auto& [key, value] : data) {
                    ENBTHelper::WriteString(file, key);
                    ENBTHelper::WriteToken(file, value);
                }
                file.flush();
                file.close();
            }
        }

        std::list<std::string> enbt_list_storage::keys() {
            std::list<std::string> keys;
            for (const auto& [key, value] : data)
                keys.push_back(key);
            return keys;
        }

    } // namespace storage

} // namespace crafted_craft