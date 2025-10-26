/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/chat.hpp>

namespace copper_server::api::chat {
    base_objects::events::sync_event_single<const std::string&, bit_list_array<uint64_t>&, base_objects::shared_client_data&>& chat_filter() {
        static base_objects::events::sync_event_single<const std::string&, bit_list_array<uint64_t>&, base_objects::shared_client_data&> res;
        return res;
    }

    base_objects::events::sync_event_single<std::optional<base_objects::chat>&, base_objects::shared_client_data&>& custom_name_provider() {
        static base_objects::events::sync_event_single<std::optional<base_objects::chat>&, base_objects::shared_client_data&> res;
        return res;
    }

    base_objects::events::sync_event_single<std::optional<base_objects::chat>&, const std::string&, base_objects::shared_client_data&>& custom_content_provider() {
        static base_objects::events::sync_event_single<std::optional<base_objects::chat>&, const std::string&, base_objects::shared_client_data&> res;
        return res;
    }
}