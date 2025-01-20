#ifndef SRC_BASE_OBJECTS_DATA_PACKS_KNOWN_PACK
#define SRC_BASE_OBJECTS_DATA_PACKS_KNOWN_PACK
#include <string>

namespace copper_server::base_objects::data_packs {
    struct known_pack {
        std::string namespace_;
        std::string id;
        std::string version;
    };
}
#endif /* SRC_BASE_OBJECTS_DATA_PACKS_KNOWN_PACK */
