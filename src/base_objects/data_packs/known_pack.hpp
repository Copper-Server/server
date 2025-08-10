/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
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
