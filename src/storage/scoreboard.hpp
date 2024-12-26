#ifndef SRC_STORAGE_SCOREBOARD
#define SRC_STORAGE_SCOREBOARD
#include <filesystem>
#include <src/base_objects/scoreboard.hpp>

namespace copper_server::storage {
    class scoreboard {
        std::filesystem::path path;

    public:
        scoreboard(const std::filesystem::path& path);
        base_objects::scoreboard data;

        void load();
        void save();
    };
}

#endif /* SRC_STORAGE_SCOREBOARD */
