/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <array>
#include <src/api/packets/debug_subscription_type.hpp>

namespace copper_server::api::packets {
    namespace deb_red_wire {
        using direction = debug_subscription_type::redstone_wire_orientations::direction;
        // --- Helper Data & Functions ---

        // a precomputed lookup table that defines the canonical order of the four
        // orthogonal directions for any given 'up' direction. this is the key to
        // creating a consistent mapping between the struct and the network ordinal.
        const std::array<std::array<direction, 4>, 6> orthogonals = {{
            // up = down (y-)
            {{direction::north, direction::east, direction::south, direction::west}},
            // up = up (y+)
            {{direction::north, direction::east, direction::south, direction::west}},
            // up = north (z-)
            {{direction::up, direction::east, direction::down, direction::west}},
            // up = south (z+)
            {{direction::up, direction::west, direction::down, direction::east}},
            // up = east (x+)
            {{direction::up, direction::south, direction::down, direction::north}},
            // up = west (x-)
            {{direction::up, direction::north, direction::down, direction::south}},
        }};

        // helper to get the array of orthogonal directions for a given 'up'.
        const std::array<direction, 4>& get_orthogonals(direction up) {
            return orthogonals[static_cast<uint8_t>(up)];
        }

        // helper to find the 2-bit index of a 'front' direction relative to its 'up'.
        uint8_t get_front_ortho_index(direction front, direction up) {
            const auto& orthogonals_ = get_orthogonals(up);
            for (uint8_t i = 0; i < 4; ++i) {
                if (orthogonals_[i] == front) {
                    return i;
                }
            }
            // this should never be reached if the 'up' and 'front' directions
            // provided are a valid, orthogonal pair.
            throw std::invalid_argument("front direction is not orthogonal to up direction");
        }

    }

    // --- wire_orientation member function implementations ---

    auto debug_subscription_type::redstone_wire_orientations::get_up() const -> direction {
        return static_cast<direction>(up_val);
    }

    auto debug_subscription_type::redstone_wire_orientations::get_bias() const -> side_bias {
        return static_cast<side_bias>(bias_val);
    }

    auto debug_subscription_type::redstone_wire_orientations::get_front() const -> direction {
        const auto& orthogonals = deb_red_wire::get_orthogonals(get_up());
        return orthogonals[front_ortho_idx];
    }

    void debug_subscription_type::redstone_wire_orientations::set_up(direction up) {
        up_val = static_cast<uint8_t>(up);
    }

    void debug_subscription_type::redstone_wire_orientations::set_bias(side_bias bias) {
        bias_val = static_cast<uint8_t>(bias);
    }

    void debug_subscription_type::redstone_wire_orientations::set_front(direction front) {
        front_ortho_idx = deb_red_wire::get_front_ortho_index(front, get_up());
    }

    uint8_t debug_subscription_type::redstone_wire_orientations::to_packet() const {
        uint8_t up_idx = up_val;
        uint8_t front_idx = front_ortho_idx;
        uint8_t bias_idx = bias_val;

        return (up_idx * 4 + front_idx) * 2 + bias_idx;
    }

    auto debug_subscription_type::redstone_wire_orientations::from_packet(uint8_t val) -> redstone_wire_orientations {
        if (val >= 48)
            throw std::out_of_range("Value must be between 0 and 47");
        redstone_wire_orientations orientation;
        orientation.bias_val = val % 2;
        uint8_t remainder = val / 2;
        orientation.front_ortho_idx = remainder % 4;
        remainder = remainder / 4;
        orientation.up_val = remainder;
        return orientation;
    }
}