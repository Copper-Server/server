
/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_ENCODING_PACKET_AUTO
#define SRC_UTIL_ENCODING_PACKET_AUTO

#include <src/util/encoding/packet/generic_decode.hpp>
#include <src/util/encoding/packet/generic_encode.hpp>
#include <src/util/encoding/packet/generic_stringize.hpp>

#define auto_define_packet_ops(packet)                                                                                               \
    template struct packet_ops<packet>;                                                                                              \
    template <>                                                                                                                      \
    base_objects::events::sync_event<packet&, base_objects::shared_client_data&>& packet_ops<packet>::send_viewer() {                \
        static base_objects::events::sync_event<packet&, base_objects::shared_client_data&> event;                                   \
        return event;                                                                                                                \
    }                                                                                                                                \
    template <>                                                                                                                      \
    base_objects::events::sync_event_no_cancel<packet&, base_objects::shared_client_data&>& packet_ops<packet>::post_send_viewer() { \
        static base_objects::events::sync_event_no_cancel<packet&, base_objects::shared_client_data&> event;                         \
        return event;                                                                                                                \
    }                                                                                                                                \
    template <>                                                                                                                      \
    base_objects::events::sync_event<packet&, base_objects::shared_client_data&>& packet_ops<packet>::receive_viewer() {             \
        static base_objects::events::sync_event<packet&, base_objects::shared_client_data&> event;                                   \
        return event;                                                                                                                \
    }                                                                                                                                \
    template <>                                                                                                                      \
    base_objects::events::sync_event_single<packet&&, base_objects::shared_client_data&>& packet_ops<packet>::processor() {          \
        static base_objects::events::sync_event_single<packet&&, base_objects::shared_client_data&> event;                           \
        return event;                                                                                                                \
    }                                                                                                                                \
    template <>                                                                                                                      \
    bool packet_ops<packet>::send(base_objects::shared_client_data& client, packet&& p) {                                            \
        return util::encoding::packet::make_send<packet_ops>(client, std::move(p));                                                  \
    }                                                                                                                                \
    template <>                                                                                                                      \
    base_objects::network::response packet_ops<packet>::client_encode(base_objects::shared_client_data& context, packet&& p) {       \
        return util::encoding::packet::make_encode<packet_ops>(context, std::move(p));                                               \
    }                                                                                                                                \
    template <>                                                                                                                      \
    base_objects::network::response packet_ops<packet>::encode(packet&& p) {                                                         \
        base_objects::shared_client_data context;                                                                                    \
        return util::encoding::packet::make_encode<packet_ops>(context, std::move(p));                                               \
    }                                                                                                                                \
    template <>                                                                                                                      \
    bool packet_ops<packet>::make_process(base_objects::shared_client_data& client, packet&& p) {                                    \
        return util::encoding::packet::decoder_make_process<packet_ops>(client, p);                                                  \
    }                                                                                                                                \
    template <>                                                                                                                      \
    packet packet_ops<packet>::decode(ArrayStream& stream) {                                                                         \
        base_objects::shared_client_data context;                                                                                    \
        return client_decode(context, stream);                                                                                       \
    }                                                                                                                                \
    template <>                                                                                                                      \
    packet packet_ops<packet>::client_decode(base_objects::shared_client_data& context, ArrayStream& stream) {                       \
        packet res;                                                                                                                  \
        util::encoding::packet::decode_entry(context, stream, res, &res);                                                            \
        return res;                                                                                                                  \
    }                                                                                                                                \
    template <>                                                                                                                      \
    std::string packet_ops<packet>::stringize(const packet& p) {                                                                     \
        std::string res;                                                                                                             \
        util::encoding::packet::sp::serialize_packet(res, 0, p);                                                                     \
        return res;                                                                                                                  \
    }

#endif /* SRC_UTIL_ENCODING_PACKET_AUTO */
