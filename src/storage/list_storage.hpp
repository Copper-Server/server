#ifndef SRC_STORAGE_LIST_STORAGE
#define SRC_STORAGE_LIST_STORAGE
#include <filesystem>
#include <library/fast_task.hpp>
#include <library/list_array.hpp>
#include <string>

namespace copper_server::storage {
    class list_storage {
        fast_task::protected_value<list_array<std::string>> data;
        std::filesystem::path path;
        bool _is_loaded = false;

    public:
        list_storage(const std::filesystem::path& path);

        void add(const std::string& value);
        bool contains(const std::string& value);
        void remove(const std::string& value);

        list_array<std::string> entrys(size_t max_items, bool& max_reached);

        template <class FN, class MAX_REACHED_CALLBACK>
        void for_each(size_t max_items, FN&& iterator, MAX_REACHED_CALLBACK&& max_reached_callback) const {
            data.get([&iterator, max_items, &max_reached_callback](auto& protected_val) {
                size_t i = 0;
                for (auto& it : protected_val) {
                    if (i++ == max_items) {
                        max_reached_callback();
                        break;
                    }
                    iterator(it);
                }
            });
        }

        template <class FN>
        void for_each(size_t max_items, FN&& iterator) const {
            for_each(max_items, std::forward<FN>(iterator), []() {});
        }

        template <class FN>
        void for_each(FN&& iterator) const {
            data.get([&iterator](auto& protected_val) {
                for (auto& it : protected_val)
                    iterator(it);
            });
        }

        size_t size() {
            return data.get([](auto& protected_val) {
                return protected_val.size();
            });
        }

        void clear();

        bool is_loaded() const {
            return _is_loaded;
        }
    };
}
#endif /* SRC_STORAGE_LIST_STORAGE */
