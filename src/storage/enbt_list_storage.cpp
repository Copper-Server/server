/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <filesystem>
#include <library/enbt/io.hpp>
#include <library/fast_task/include/files.hpp>
#include <src/storage/enbt_list_storage.hpp>
namespace copper_server::storage {
    enbt_list_storage::enbt_list_storage(const std::filesystem::path& path)
        : path(path) {
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path.parent_path());
            fast_task::files::async_iofstream file(
                path,
                fast_task::files::open_mode::write,
                fast_task::files::on_open_action::open,
                fast_task::files::_sync_flags{}
            );
            _is_loaded = true;
            return;
        }
        fast_task::files::async_iofstream file(
            path,
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            return;
        data.set([&](auto& value) {
            while (!file.eof()) {
                std::string key = (std::string)enbt::io_helper::read_string(file);
                if (file.eof())
                    break;
                value[key] = enbt::io_helper::read_token(file);
            }
        });
        _is_loaded = true;
    }

    void enbt_list_storage::add(const std::string& key, const enbt::value& enbt) {
        bool save = false;
        data.set([&](auto& value) {
            if (value.find(key) == value.end()) {
                value[key] = enbt;
                save = true;
            }
        });
        if (save) {
            fast_task::files::async_iofstream file(
                path,
                fast_task::files::open_mode::append,
                fast_task::files::on_open_action::open,
                fast_task::files::_sync_flags{}
            );
            enbt::io_helper::write_string(file, key);
            enbt::io_helper::write_token(file, enbt);
            file.flush();
        }
    }

    void enbt_list_storage::update(const std::string& key, const enbt::value& enbt) {
        bool save = false;
        data.set([&](auto& value) {
            auto it = value.find(key);
            if (it != value.end()) {
                it->second = enbt;
                save = true;
            }
        });

        if (save) {
            fast_task::files::async_iofstream file(
                path,
                fast_task::files::open_mode::write,
                fast_task::files::on_open_action::always_new,
                fast_task::files::_sync_flags{}
            );
            data.get([&](auto& list) {
                for (const auto& [key, value] : list) {
                    enbt::io_helper::write_string(file, key);
                    enbt::io_helper::write_token(file, value);
                }
            });
            file.flush();
        }
    }

    std::optional<enbt::value> enbt_list_storage::get(const std::string& key) {
        return data.get([&](auto& value) -> std::optional<enbt::value> {
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
            fast_task::files::async_iofstream file(
                path,
                fast_task::files::open_mode::write,
                fast_task::files::on_open_action::always_new,
                fast_task::files::_sync_flags{}
            );
            data.get([&](auto& list) {
                for (const auto& [key, value] : list) {
                    enbt::io_helper::write_string(file, key);
                    enbt::io_helper::write_token(file, value);
                }
            });
            file.flush();
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
}