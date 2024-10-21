#ifndef SRC_UTIL_SLOT_READER
#define SRC_UTIL_SLOT_READER
#include "../base_objects/slot.hpp"
#include <boost/json.hpp>

namespace crafted_craft::util {
    base_objects::slot_component::unified parse_component(const std::string& component_name, boost::json::value& item);
    base_objects::slot_data parse_slot(boost::json::object& item);
}

#endif /* SRC_UTIL_SLOT_READER */
