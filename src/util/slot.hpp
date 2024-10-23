#ifndef SRC_UTIL_SLOT
#define SRC_UTIL_SLOT
#include "../base_objects/slot.hpp"
#include <boost/json.hpp>

namespace crafted_craft::util {
    base_objects::slot_component::unified parse_component(const std::string& component_name, boost::json::value& item);
    base_objects::slot_data parse_slot(boost::json::object& item);
    boost::json::object serialize_slot(const base_objects::slot_data& item);
}

#endif /* SRC_UTIL_SLOT */
