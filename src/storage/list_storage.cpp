#include "list_storage.hpp"
#include <fstream>

namespace crafted_craft {
    namespace storage {
        list_storage::list_storage(const std::string& path)
            : path(path) {
            std::fstream file;
            file.open(path, std::ios::in);
            for (std::string line; std::getline(file, line);)
                data.insert(line);
            file.flush();
            file.close();
            _is_loaded = true;
        }

        void list_storage::add(const std::string& value) {
            if (!data.contains(value)) {
                data.insert(value);

                std::fstream file;
                file.open(path, std::ios::out | std::ios::app);
                file << value << std::endl;
                file.flush();
                file.close();
            }
        }

        bool list_storage::contains(const std::string& value) {
            return data.contains(value);
        }

        void list_storage::remove(const std::string& value) {
            if (this->contains(value)) {
                this->data.erase(value);
                std::fstream file;
                file.open(path, std::ios::out | std::ios::trunc);
                for (const auto& line : this->data)
                    file << line << std::endl;
                file.flush();
                file.close();
            }
        }

        std::list<std::string> list_storage::entrys() {
            std::list<std::string> entrys;
            for (const auto& entry : data)
                entrys.push_back(entry);
            return entrys;
        }

    } // namespace storage

} // namespace crafted_craft
