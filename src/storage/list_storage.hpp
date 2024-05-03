#ifndef SRC_STORAGE_LIST_STORAGE
#define SRC_STORAGE_LIST_STORAGE
#include <string>
#include <unordered_set>

namespace crafted_craft {
    namespace storage {
        class list_storage {
            std::unordered_set<std::string> data;
            std::string path;
            bool _is_loaded = false;

        public:
            list_storage(const std::string& path);

            void add(const std::string& value);
            bool contains(const std::string& value);
            void remove(const std::string& value);

            std::list<std::string> entrys();

            bool is_loaded() const {
                return _is_loaded;
            }
        };
    } // namespace storage

} // namespace crafted_craft

#endif /* SRC_STORAGE_LIST_STORAGE */
