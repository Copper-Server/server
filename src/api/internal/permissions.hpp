#ifndef SRC_API_INTERNAL_PERMISSIONS
#define SRC_API_INTERNAL_PERMISSIONS
#include <src/storage/permissions_manager.hpp>

namespace copper_server::api::permissions {
    void init_permissions(storage::permissions_manager& manager);
}

#endif /* SRC_API_INTERNAL_PERMISSIONS */
