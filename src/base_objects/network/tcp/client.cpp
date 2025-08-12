/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/network/tcp/client.hpp>
#include <src/log.hpp>

namespace copper_server::base_objects::network::tcp {
    response client::on_switch() {
        return response::empty();
    }

    //will return nullptr if Redefine not needed
    client::~client() {}

    void client::log_console(const std::string& prefix, const list_array<uint8_t>& data, size_t size) {
        //std::string output = prefix + " ";
        //output.reserve(size * 2);
        //static const char hex_chars[] = "0123456789ABCDEF";
        //for (size_t i = 0; i < size; i++) {
        //    output.push_back(hex_chars[data[i] >> 4]);
        //    output.push_back(hex_chars[data[i] & 0xF]);
        //}
        //log::debug("Debug tools", output);
    }
}