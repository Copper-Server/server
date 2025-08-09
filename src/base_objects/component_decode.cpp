#include <src/api/packets.hpp> //for reflection //TODO change reflect_map to be more abstract;
#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/calculations.hpp> //for reflection //TODO change reflect_map to be more abstract;
#include <src/util/reflect.hpp>

namespace copper_server::base_objects {
    
    component component::parse_component(const std::string& name, const enbt::value& item) {
        return {};//TODO
    }
}