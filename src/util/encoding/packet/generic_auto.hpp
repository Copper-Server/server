
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

#define auto_define_packet_ops(packet_type)                                                                                                    \
    template struct packet_ops<packet_type>;                                                                                                   \
    template <>                                                                                                                                \
    base_objects::events::sync_event<packet_type&, base_objects::shared_client_data&>& packet_ops<packet_type>::send_viewer() {                \
        static base_objects::events::sync_event<packet_type&, base_objects::shared_client_data&> event;                                        \
        return event;                                                                                                                          \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    base_objects::events::sync_event_no_cancel<packet_type&, base_objects::shared_client_data&>& packet_ops<packet_type>::post_send_viewer() { \
        static base_objects::events::sync_event_no_cancel<packet_type&, base_objects::shared_client_data&> event;                              \
        return event;                                                                                                                          \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    base_objects::events::sync_event<packet_type&, base_objects::shared_client_data&>& packet_ops<packet_type>::receive_viewer() {             \
        static base_objects::events::sync_event<packet_type&, base_objects::shared_client_data&> event;                                        \
        return event;                                                                                                                          \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    base_objects::events::sync_event_single<packet_type&&, base_objects::shared_client_data&>& packet_ops<packet_type>::processor() {          \
        static base_objects::events::sync_event_single<packet_type&&, base_objects::shared_client_data&> event;                                \
        return event;                                                                                                                          \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    bool packet_ops<packet_type>::send(base_objects::shared_client_data& client, packet_type&& p) {                                            \
        return util::encoding::packet::make_send<packet_ops>(client, std::move(p));                                                            \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    base_objects::network::response packet_ops<packet_type>::client_encode(base_objects::shared_client_data& context, packet_type&& p) {       \
        return util::encoding::packet::make_encode<packet_ops>(context, std::move(p));                                                         \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    base_objects::network::response packet_ops<packet_type>::encode(packet_type&& p) {                                                         \
        base_objects::shared_client_data context;                                                                                              \
        return util::encoding::packet::make_encode<packet_ops>(context, std::move(p));                                                         \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    bool packet_ops<packet_type>::make_process(base_objects::shared_client_data& client, packet_type&& p) {                                    \
        return util::encoding::packet::decoder_make_process<packet_ops>(client, p);                                                            \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    packet_type packet_ops<packet_type>::decode(ArrayStream& stream) {                                                                         \
        base_objects::shared_client_data context;                                                                                              \
        return client_decode(context, stream);                                                                                                 \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    packet_type packet_ops<packet_type>::client_decode(base_objects::shared_client_data& context, ArrayStream& stream) {                       \
        packet_type res;                                                                                                                       \
        util::encoding::packet::decode_entry(context, stream, res, &res);                                                                      \
        return res;                                                                                                                            \
    }                                                                                                                                          \
    template <>                                                                                                                                \
    std::string packet_ops<packet_type>::stringize(const packet_type& p) {                                                                     \
        std::string res;                                                                                                                       \
        util::encoding::packet::sp::serialize_packet(res, 0, p);                                                                               \
        return res;                                                                                                                            \
    }

#endif /* SRC_UTIL_ENCODING_PACKET_AUTO */
