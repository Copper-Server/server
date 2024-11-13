#ifndef SRC_API_INTERNAL_CONSOLE
#define SRC_API_INTERNAL_CONSOLE
#include <src/base_objects/virtual_client.hpp>

namespace copper_server::api::console {
    void register_virtual_client(base_objects::virtual_client& client);
    void unregister_virtual_client();
}

#endif /* SRC_API_INTERNAL_CONSOLE */
