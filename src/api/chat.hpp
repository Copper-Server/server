/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_CHAT
#define SRC_API_CHAT
#include <library/list_array.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/chat.hpp>
#include <string>

namespace copper_server::base_objects{
    struct shared_client_data;
}
namespace copper_server::api::chat {
    base_objects::events::sync_event_single<const std::string&, bit_list_array<uint64_t>&, base_objects::shared_client_data&>& chat_filter();
    base_objects::events::sync_event_single<std::optional<Chat>&, base_objects::shared_client_data&>& custom_name_provider();
    base_objects::events::sync_event_single<std::optional<Chat>&, const std::string&, base_objects::shared_client_data&>& custom_content_provider();
}


#endif /* SRC_API_CHAT */
