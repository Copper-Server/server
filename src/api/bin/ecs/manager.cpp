#include <src/api/bin/ecs/manager.hpp>

namespace copper_server::api::ecs {
    manager& manager::instance() {
        static manager self;
        return self;
    }
}