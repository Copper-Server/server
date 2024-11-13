#ifndef SRC_API_BLOCK_STATE_PROVIDER
#define SRC_API_BLOCK_STATE_PROVIDER
#include <src/base_objects/block_state_provider.hpp>

namespace copper_server::api::block_state_provider {
    std::function<base_objects::block()> process_provider(const enbt::compound_const_ref& provider_config);
    void register_handler(const std::string& name, base_objects::block_state_provider_generator::handler handler);
    void unregister_handler(const std::string& name);
    const base_objects::block_state_provider_generator::handler& get_handler(const std::string& name);
    void reset_handlers();
    bool has_handler(const std::string& name);
    
    bool registered();
}

#endif /* SRC_API_BLOCK_STATE_PROVIDER */
