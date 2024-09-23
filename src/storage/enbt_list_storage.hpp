#ifndef SRC_STORAGE_ENBT_LIST_STORAGE
#define SRC_STORAGE_ENBT_LIST_STORAGE
#include "../library/enbt.hpp"
#include "../library/fast_task.hpp"
#include "../library/list_array.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace crafted_craft {
    namespace storage {
        class enbt_list_storage {
            fast_task::protected_value<std::unordered_map<std::string, enbt::value>> data;
            std::filesystem::path path;
            bool _is_loaded = false;

        public:
            enbt_list_storage(const std::filesystem::path& path);

            void add(const std::string& value, const enbt::value& enbt);
            void update(const std::string& value, const enbt::value& enbt);
            std::optional<enbt::value> get(const std::string& value);
            bool contains(const std::string& value);
            void remove(const std::string& value);

            list_array<std::string> keys(size_t max_items, bool& max_reached);

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

            size_t size() {
                return data.get([](auto& protected_val) {
                    return protected_val.size();
                });
            }

            template <class FN>
            void for_each(size_t max_items, FN&& iterator) const {
                for_each(max_items, std::forward<FN>(iterator), []() {});
            }

            bool is_loaded() const {
                return _is_loaded;
            }
        };
    } // namespace storage

} // namespace crafted_craft

#endif /* SRC_STORAGE_ENBT_LIST_STORAGE */
