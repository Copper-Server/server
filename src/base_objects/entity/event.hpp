/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_ENTITY_ENTITY_EVENT
#define SRC_BASE_OBJECTS_ENTITY_ENTITY_EVENT
#include <cstdint>

namespace copper_server::base_objects {
    enum class entity_event : uint8_t {
        arrow_tipped_particle = 0,
        rabbit_rotated_jump = 1,
        spawner_resets_delay = 1, //to 200 ticks
        _undefined_2 = 2,
        projectile_collided = 3,
        entity_died = 3,
        evoker_fangs_attack = 4,
        ravager_attack = 4,
        hoglin_attack = 4,     //plays animation for 10 ticks and plays attack sound
        zoglin_attack = 4,     //plays animation for 10 ticks and plays attack sound
        warden_attack = 4,     //stop roar and play attack animation
        iron_golem_attack = 4, //attack and sound
        _undefined_5 = 5,
        taming_failed = 6,
        taming_succeeded = 7,
        wolf_shakes = 8, //water animation
        item_usage_finished = 9,
        sheep_eats_grass = 10,     //for 40 ticks
        minecart_tnt_ignites = 10, //without sound
        iron_golem_holds_poppy = 11,
        villager_hearts = 12,
        villager_angry = 13,
        villager_happy = 14,
        witch_particles = 15, //with chance of 0.075% each tick spawns 10 to 45 particles
        zombie_villager_cured = 16,
        firework_explosion = 17,
        animal_in_love = 18,
        allay_duplicates = 18,
        squid_rotated_more_2_pi_rad = 19,
        entity_existence_changed = 20, //silverfish enters/leaves block or spawner spawns entity
        guardian_attack_sound = 21,
        enable_reduced_debug_screen = 22,
        disable_reduced_debug_screen = 23,
        set_op_0 = 24,
        set_op_1 = 25,
        set_op_2 = 26,
        set_op_3 = 27,
        set_op_4 = 28,
        blocked_by_shield_sound_effect = 29,
        shield_broken_sound_effect = 30,
        hook_entity = 31,     //If the caught entity is the connected player, then cause them to be pulled toward the caster of the fishing rod.
        armor_stand_hit = 32, //also resets hit cooldown
        _undefined_33 = 33,
        iron_golem_hides_poppy = 34,
        totem_popped = 35,
        _undefined_36 = 36,
        _undefined_37 = 37,
        dolphin_locating_structure = 38, //sent when fed and located structure
        ravager_stunned = 39,            //for 40 ticks
        ocelot_taming_failed = 40,
        ocelot_taming_succeeded = 41,
        villager_splashes = 42,
        bad_omen_used = 43,
        _undefined_44 = 44,
        fox_chews_item = 45,
        entity_teleported = 46, //chorus fruit eaten or endermen
        item_broken_on_main_hand = 47,
        item_broken_on_off_hand = 48,
        item_broken_on_head = 49,
        item_broken_on_chest = 50,
        item_broken_on_legs = 51,
        item_broken_on_feet = 52,
        honey_block_slide = 53,
        landed_on_honey = 54,
        items_on_hands_swapped = 55,
        wolf_stops_shaking = 56,
        _undefined_57 = 57,
        goat_lows_head_for_ramming = 58,
        goat_rises_head = 59,
        death_smoke = 60,
        warden_shakes = 61,     //for 10 ticks
        warden_sonic_boom = 62, //animation only
        sniffer_digs = 63
    };
}
#endif /* SRC_BASE_OBJECTS_ENTITY_ENTITY_EVENT */
