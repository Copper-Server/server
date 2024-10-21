#include "../library/enbt.hpp"
#include "commands.hpp"

namespace crafted_craft {
    namespace base_objects {
        struct predicate_processor {
            using handler = std::function<bool(const enbt::compound_const_ref&, const command_context&)>;

            bool process_predicate(const enbt::compound_const_ref& predicate, const command_context& context) const {
                return handlers.at(normalize_name((std::string)predicate["condition"]))(predicate, context);
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
}