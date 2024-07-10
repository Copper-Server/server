#include "enbt_list_storage.hpp"
#include <filesystem>
#include <fstream>

namespace crafted_craft {
    namespace storage {
        enbt_list_storage::enbt_list_storage(const std::filesystem::path& path)
            : path(path) {
            if (!std::filesystem::exists(path)) {
                std::filesystem::create_directories(path.parent_path());
                std::ofstream file;
                file.open(path, std::ios::out);
                file.close();
                _is_loaded = true;
                return;
            }
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open())
                return;
            data.set([&](auto& value) {
                while (!file.eof()) {
                    std::string key = (std::string)ENBTHelper::ReadString(file);
                    if (file.eof())
                        break;
                    value[key] = ENBTHelper::ReadToken(file);
                }
            });
            _is_loaded = true;
            file.close();
        }

        void enbt_list_storage::add(const std::string& key, const ENBT& enbt) {
            bool save = false;
            data.set([&](auto& value) {
                if (value.find(key) == value.end()) {
                    value[key] = enbt;
                    save = true;
                }
            });
            if (save) {
                std::fstream file;
                file.open(path, std::ios::out | std::ios::app | std::ios::binary);
                ENBTHelper::WriteString(file, key);
                ENBTHelper::WriteToken(file, enbt);
                file.flush();
                file.close();
            }
        }

        void enbt_list_storage::update(const std::string& key, const ENBT& enbt) {
            bool save = false;
            data.set([&](auto& value) {
                auto it = value.find(key);
                if (it != value.end()) {
                    it->second = enbt;
                    save = true;
                }
            });

            if (save) {
                std::fstream file;
                file.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
                data.get([&](auto& value) {
                    for (const auto& [key, value] : value) {
                        ENBTHelper::WriteString(file, key);
                        ENBTHelper::WriteToken(file, value);
                    }
                });
                file.flush();
                file.close();
            }
        }

        std::optional<ENBT> enbt_list_storage::get(const std::string& key) {
            return data.get([&](auto& value) -> std::optional<ENBT> {
                auto it = value.find(key);
                if (it != value.end())
                    return std::optional(it->second);
                return std::nullopt;
            });
        }

        bool enbt_list_storage::contains(const std::string& find_value) {
            return data.get([&](auto& value) {
                return value.contains(find_value);
            });
        }

        void enbt_list_storage::remove(const std::string& key) {
            bool save = false;
            data.set([&](auto& value) {
                auto it = value.find(key);
                if (it != value.end()) {
                    value.erase(it);
                    save = true;
                }
            });

            if (save) {
                std::fstream file;
                file.open(path, std::ios::out | std::ios::trunc);
                data.get([&](auto& value) {
                    for (const auto& [key, value] : value) {
                        ENBTHelper::WriteString(file, key);
                        ENBTHelper::WriteToken(file, value);
                    }
                });
                file.flush();
                file.close();
            }
        }

        list_array<std::string> enbt_list_storage::keys(size_t max_items, bool& max_reached) {
            return data.get([&](auto& value) {
                list_array<std::string> keys;
                size_t i = 0;
                keys.reserve(value.size());
                for (const auto& [key, value_] : value) {
                    if (i++ == max_items) {
                        max_reached = 0;
                        break;
                    }
                    keys.push_back(key);
                }
                return keys;
            });
        }

    } // namespace storage

} // namespace crafted_craft