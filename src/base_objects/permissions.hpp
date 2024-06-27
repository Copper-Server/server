#ifndef SRC_BASE_OBJECTS_PERMISSIONS
#define SRC_BASE_OBJECTS_PERMISSIONS
#include "../library/list_array.hpp"
#include <string>

namespace crafted_craft {
    namespace base_objects {
        struct permissions_object {
            std::string permission_tag;
            std::string description;
            int8_t permission_level = -1; //if -1 then this permission just a tag
            bool instant_grant = false;   //if true then this permission will be granted without checking others

            bool operator==(const permissions_object& other) const {
                return permission_tag == other.permission_tag;
            }

            bool operator!=(const permissions_object& other) const {
                return permission_tag != other.permission_tag;
            }

            bool operator>(const permissions_object& other) const {
                return permission_tag > other.permission_tag;
            }

            bool operator<(const permissions_object& other) const {
                return permission_tag < other.permission_tag;
            }

            bool operator>=(const permissions_object& other) const {
                return permission_tag >= other.permission_tag;
            }

            bool operator<=(const permissions_object& other) const {
                return permission_tag <= other.permission_tag;
            }
        };
    }
}

namespace std {
    template <>
    struct hash<crafted_craft::base_objects::permissions_object> {
        size_t operator()(const crafted_craft::base_objects::permissions_object& obj) const {
            return hash<string>()(obj.permission_tag);
        }
    };
}

#endif /* SRC_BASE_OBJECTS_PERMISSIONS */
