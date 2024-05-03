#ifndef SRC_STORAGE_ENBT_LIST_STORAGE
#define SRC_STORAGE_ENBT_LIST_STORAGE
#include "../library/enbt.hpp"
#include <optional>
#include <string>
#include <unordered_map>

namespace crafted_craft {
    namespace storage {
        class enbt_list_storage {
            std::unordered_map<std::string, ENBT> data;
            std::string path;
            bool _is_loaded = false;

        public:
            enbt_list_storage(const std::string& path);

            void add(const std::string& value, const ENBT& enbt);
            std::optional<ENBT> get(const std::string& value);
            bool contains(const std::string& value);
            void remove(const std::string& value);

            std::list<std::string> keys();

            bool is_loaded() const {
                return _is_loaded;
            }
        };
    } // namespace storage

} // namespace crafted_craft

#endif /* SRC_STORAGE_ENBT_LIST_STORAGE */
