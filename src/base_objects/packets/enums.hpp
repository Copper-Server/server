#ifndef SRC_BASE_OBJECTS_PACKETS_ENUMS
#define SRC_BASE_OBJECTS_PACKETS_ENUMS
#include <cstdint>

namespace copper_server::base_objects::packets {
    enum class game_event_id : uint8_t {     //[Argument values]
        no_respawn_block_available = 0,      //none
        raining_begin = 1,                   //none
        raining_end = 2,                     //none
        gamemode_change = 3,                 //0 - survival, 1 - creative, 2 - adventure, 3 - spectator
        win_game = 4,                        //0 - respawn, 1 - roll credits and respawn
        demo_event = 5,                      //0 - welcome, 101 - movement, 102 - jump, 103 - inventory, 104 - end screen
        arrow_hit_player = 6,                //none
        rain_level_change = 7,               //from 0 to 1
        thunder_level_change = 8,            //from 0 to 1
        puffer_fish_sting = 9,               //none
        guardian_appear = 10,                //none
        disable_respawn_screen = 11,         //0 and 1
        enable_limited_crafting = 12,        //0 and 1
        start_waiting_for_level_chunks = 13, //none
    };

    enum class world_event_id : uint32_t {
        dispenser_dispenses = 1000,
        dispenser_dispense_fail = 1001,
        dispenser_shoots = 1002,
        firework_shot = 1004,
        fire_extinguished = 1009,
        play_record = 1010,
        stop_record = 1011,
        ghast_warn = 1015,
        ghast_shoots = 1016,
        ender_dragon_shoots = 1017,
        blaze_shoots = 1018,
        ie_attacks_wooden_door = 1019,
        zombie_attacks_iron_door = 1020,
        zombie_breaks_wooden_door = 1021,
        wither_breaks_block = 1022,
        wither_spawned = 1023,
        wither_shoots = 1024,
        bat_takes_of = 1025,
        zombie_infects = 1026,
        zombie_villager_converted = 1027,
        ender_dragon_dies = 1028,
        anvil_destroyed = 1029,
        anvil_used = 1030,
        anvil_lands = 1031,
        portal_travel = 1032,
        chorus_flower_grows = 1033,
        chorus_flower_dies = 1034,
        brewing_stand_brews = 1035,
        end_portal_created = 1038,
        phantom_bites = 1039,
        zombie_converts_to_drowned = 1040,
        husk_converts_to_zombie = 1041,
        grindstone_used = 1042,
        book_page_turned = 1043,
        smithing_table_used = 1044,
        pointed_dripstone_landing = 1045,
        lava_dripping_on_cauldron_from_dripstone = 1046,
        water_dripping_on_cauldron_from_dripstone = 1047,
        skeleton_converts_to_stray = 1048,
        crafter_successfully_crafts_item = 1049,
        crafter_fails_to_craft_item = 1050,

        composter_composts = 1500,
        lava_converts_block = 1501,
        redstone_torch_burns_out = 1502,
        ender_eye_placed_in_end_portal_frame = 1503,
        fluid_drips_from_dripstone = 1504,
        bone_meal_particles_and_sound = 1505,
        dispenser_activation_smoke = 2000,
        block_break_and_sound = 2001,
        splash_potion_particle_effect = 2002,
        eye_of_ender_entity_break_animation = 2003,
        spawner_spawns_mob = 2004,
        dragon_breath = 2006,
        instant_splash_potion = 2007,
        ender_dragon_destroys_block = 2008,
        wet_sponge_vaporizes = 2009,
        crafter_activation_smoke = 2010,
        bee_fertilizes_plant = 2011,
        turtle_egg_placed = 2012,
        smash_attack_mace = 2013,
        end_gateway_spawns = 3000,
        ender_dragon_resurrected = 3001,
        electric_spark = 3002,
        copper_apply_wax = 3003,
        copper_remove_wax = 3004,
        copper_scrape_oxidation = 3005,
        sculk_charge = 3006,
        sculk_shrieker_shriek = 3007,
        block_finished_brushing = 3008,
        sniffer_egg_cracks = 3009,
        trial_spawner_spawns_mob_at_spawner = 3011,
        trial_spawner_spawns_mob_at_spawn_location = 3012,
        trial_spawner_detects_player = 3013,
        trial_spawner_ejects_item = 3014,
        vault_activates = 3015,
        vault_deactivates = 3016,
        vault_ejects_item = 3017,
        cobweb_weaved = 3018,
        ominous_trial_spawner_detects_player = 3019,
        trial_spawner_turns_ominous = 3020,
        ominous_item_spawner_spawns_item = 3021,
    };
}

#endif /* SRC_BASE_OBJECTS_PACKETS_ENUMS */
