#ifndef SRC_BASE_OBJECTS_PERMISSIONS
#define SRC_BASE_OBJECTS_PERMISSIONS
#include "../library/list_array.hpp"
#include <string>

namespace crafted_craft {
    namespace base_objects {
        struct permissions_object {
            std::string permission_tag;
            std::string description;
            int8_t permission_level = -1;   //if -1 then this permission just a tag
            bool instant_grant : 1 = false; //if true then this permission will be granted without checking others
            bool reverse_mode : 1 = false;  //reverse check, if player has
            bool important : 1 = false;     //if check failed, then player not allowed to do this action, even instant_grant ignored

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

        struct permission_group {
            std::string group_name;
            list_array<std::string> permissions_tags;
        };
    }
}

namespace std {
    template <>
    struct hash<crafted_craft::base_objects::permissions_object> {
        size_t operator()(const crafted_craft::base_objects::permissions_object& obj) const {
            return hash<std::string>()(obj.permission_tag);
        }
    };
}

#endif /* SRC_BASE_OBJECTS_PERMISSIONS */
