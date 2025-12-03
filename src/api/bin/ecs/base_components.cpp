#include <src/api/ecs/base_components.hpp>
#include <src/api/tags.hpp>

namespace copper_server::api::ecs::com {
    namespace block_entity {
        //tag_type::tag_type(const std::string& tag){
        //
        //}
    }

    namespace entities {
        tag_type::tag_type(const std::string& tag) {
            value = api::tags::get_tag_id(api::tags::get_tag_handle(api::tags::builtin_entry::entity_type, tag));
        }
    }
}