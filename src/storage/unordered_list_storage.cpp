#include <library/fast_task/src/files/files.hpp>
#include <src/storage/unordered_list_storage.hpp>

namespace copper_server::storage {
    unordered_list_storage::unordered_list_storage(const std::filesystem::path& path) {
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
            for (std::string line; std::getline(file, line);)
                value.insert(line);
        });
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
            fast_task::files::async_iofstream file(
                path,
                fast_task::files::open_mode::append,
                fast_task::files::on_open_action::open,
                fast_task::files::_sync_flags{}
            );
            file << set_value << std::endl;
            file.flush();
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
            fast_task::files::async_iofstream file(
                path,
                fast_task::files::open_mode::write,
                fast_task::files::on_open_action::truncate_exists,
                fast_task::files::_sync_flags{}
            );
            data.get([&](auto& value) {
                for (const auto& line : value)
                    file << line << '\n';
            });
            file.flush();
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
        fast_task::files::async_iofstream file(
            path,
            fast_task::files::open_mode::write,
            fast_task::files::on_open_action::truncate_exists,
            fast_task::files::_sync_flags{}
        );
        file.flush();
    }
}
