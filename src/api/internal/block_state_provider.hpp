#ifndef SRC_API_INTERNAL_BLOCK_STATE_PROVIDER
#define SRC_API_INTERNAL_BLOCK_STATE_PROVIDER
#include "../../base_objects/block_state_provider.hpp"

namespace crafted_craft::api::block_state_provider {
    void register_generator(base_objects::block_state_provider_generator& processor);
    void unregister_generator();
}

#endif /* SRC_API_INTERNAL_BLOCK_STATE_PROVIDER */
