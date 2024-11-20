#ifndef SRC_API_ENTITY_ID_MAP
#define SRC_API_ENTITY_ID_MAP
#include <library/enbt/enbt.hpp>

namespace copper_server::api::entity_id_map {
    std::pair<int32_t, enbt::raw_uuid> allocate_id();
    int32_t allocate_id(const enbt::raw_uuid& uuid);
    void remove_id(int32_t id);
    int32_t remove_id(const enbt::raw_uuid& uuid);
    int32_t get_id(const enbt::raw_uuid& uuid);
    enbt::raw_uuid get_uuid(int32_t id);
    bool has_id(int32_t id);
    bool has_uuid(const enbt::raw_uuid& uuid);
}

#endif /* SRC_API_ENTITY_ID_MAP */
