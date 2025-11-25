#ifndef SRC_API_BIN_ECS_DELETION_SYSTEM
#define SRC_API_BIN_ECS_DELETION_SYSTEM
#include <src/api/ecs.hpp>
#include <src/api/ecs/base_components.hpp>

namespace copper_server::api::ecs {
    //generic system to allow entity destroying with dependent entities
    struct deletion_system : public system_interface {
        using reads = dependent<com::dead_mark>;
        using writes = dependent<com::dead_mark>;

        void tick(world_local_registry& world) override {
            for (auto&& [view] : world.view().with<com::dead_mark>()) {
                auto current = view.current_entity();
                for (auto& child : current.get_flat_relation())
                    child.add<com::dead_mark>();//This is safe because if the entity removed, the ecs would ignore this operation 
                detail::queue_destroy_entity(current.id, current.generation);
            }
        }
    };
}

#endif /* SRC_API_BIN_ECS_DELETION_SYSTEM */
