#ifndef SRC_BASE_OBJECTS_BLOCK_STATE_PROVIDER
#define SRC_BASE_OBJECTS_BLOCK_STATE_PROVIDER
#include <functional>
#include <library/enbt/enbt.hpp>
#include <src/base_objects/block.hpp>

namespace copper_server::base_objects {
    struct block_state_provider_generator {
        using handler = std::function<block(const enbt::compound_const_ref& config, enbt::compound& local_state)>;

        std::function<block()> process_provider(const enbt::compound_const_ref& provider_config) const {
            auto& handler = handlers.at(normalize_name((std::string)provider_config["type"]));
            return {[handler = handler, provider_config = provider_config, state = enbt::compound()]() mutable {
                return handler(provider_config, state);
            }};
        }

        void register_handler(const std::string& name, handler handler) {
            handlers[normalize_name(name)] = std::move(handler);
        }

        void unregister_handler(const std::string& name) {
            handlers.erase(normalize_name(name));
        }

        const handler& get_handler(const std::string& name) const {
            return handlers.at(normalize_name(name));
        }

        void reset_handlers() {
            handlers.clear();
        }

        bool has_handler(const std::string& name) const {
            return handlers.find(normalize_name(name)) != handlers.end();
        }

    private:
        static std::string normalize_name(const std::string& name) {
            if (name.find(':') != std::string::npos)
                return name;
            else
                return "minecraft:" + name;
        }

        std::unordered_map<std::string, handler> handlers;
    };
}
#endif /* SRC_BASE_OBJECTS_BLOCK_STATE_PROVIDER */
