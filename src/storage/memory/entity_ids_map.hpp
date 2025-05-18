#ifndef SRC_STORAGE_MEMORY_ENTITY_IDS_MAP
#define SRC_STORAGE_MEMORY_ENTITY_IDS_MAP
#include <library/enbt/enbt.hpp>
#include <library/fast_task.hpp>
#include <shared_mutex>
#include <src/base_objects/atomic_holder.hpp>

namespace copper_server::base_objects {
    struct entity;
    using entity_ref = atomic_holder<entity>;
}
namespace copper_server::storage::memory {
    //this storage utilizes only positive values and leaves negative for plugin usage
    // negative ids may used for breaking block animation
    //
    //this means the theoretical maximum of entities is INT32_MAX
    class entity_ids_map_storage {

        struct id_s {
            std::vector<int32_t> id;
            enbt::raw_uuid uuid;
            base_objects::entity_ref assigned_entity;
        };

        using id_sp = std::shared_ptr<id_s>;

        std::unordered_map<int32_t, id_sp> ids_l;
        std::unordered_map<enbt::raw_uuid, id_sp> ids_r;
        fast_task::task_mutex mutex;
        int32_t id_allocator = 0;

        int32_t id_increment();
        uint8_t allocate_special(uint8_t required_ids);

    public:
        //generates random UUID allocates, guarantees uniqueness
        [[nodiscard]] std::pair<int32_t, enbt::raw_uuid> allocate_id();
        [[nodiscard]] std::pair<int32_t, enbt::raw_uuid> allocate_special_sequence(uint8_t required_ids);
        [[nodiscard]] int32_t allocate_id(const enbt::raw_uuid& uuid);
        [[nodiscard]] int32_t allocate_special_sequence(const enbt::raw_uuid& uuid, uint8_t required_ids);
        void remove_id(int32_t id);
        [[nodiscard]] int32_t remove_id(const enbt::raw_uuid& uuid);
        [[nodiscard]] int32_t get_id(const enbt::raw_uuid& uuid);
        [[nodiscard]] enbt::raw_uuid get_uuid(int32_t id);
        void assign_entity(int32_t id, base_objects::entity_ref entity);
        void assign_entity(const enbt::raw_uuid& uuid, base_objects::entity_ref entity);
        [[nodiscard]] base_objects::entity_ref get_entity(int32_t id);
        [[nodiscard]] base_objects::entity_ref get_entity(const enbt::raw_uuid& uuid);
        [[nodiscard]] bool has_id(int32_t id);
        [[nodiscard]] bool has_uuid(const enbt::raw_uuid& uuid);
    };
}
#endif /* SRC_STORAGE_MEMORY_ENTITY_IDS_MAP */
