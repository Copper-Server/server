/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS
#define SRC_API_PACKETS
#include <array>
#include <library/enbt/enbt.hpp>
#include <src/base_objects/box.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/packets_help.hpp>
#include <src/base_objects/pallete_container.hpp>
#include <src/base_objects/parsers.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/calculations.hpp>
#define STRUCT__ struct //;
#define decl_variant(name, ...)                        \
    /*Spacing for reflect_map*/                        \
    STRUCT__ name : public std::variant<__VA_ARGS__> { \
        using base = std::variant<__VA_ARGS__>;        \
        using base::variant;                           \
        using base::operator=;                         \
    }

namespace copper_server {
    struct ArrayStream;

    namespace storage {
        class chunk_data;
        class world_data;
    }

    namespace base_objects {
        struct SharedClientData;
        class command_manager;
    }

    //this api allows users to handle clients and simulate them if needed, also supports serialization to string for debug purposes
    // note: because this api uses reflection under the hood, recommended to enable build cache to reduce the build time
    // the api implements the latest protocol implementation: 772(1.21.8)
    namespace api::packets {

        using base_objects::Angle;
        using base_objects::any_of;
        using base_objects::bitset_fixed;
        using base_objects::bool_or;
        using base_objects::compound_packet;
        using base_objects::depends_next;
        using base_objects::disconnect_after;
        using base_objects::enum_as;
        using base_objects::enum_as_flag;
        using base_objects::enum_item;
        using base_objects::enum_set;
        using base_objects::enum_switch;
        using base_objects::flag_item;
        using base_objects::flags_list;
        using base_objects::flags_list_from;
        using base_objects::for_each_type;
        using base_objects::id_set;
        using base_objects::identifier;
        using base_objects::ignored;
        using base_objects::item_depend;
        using base_objects::json_text_component;
        using base_objects::limited_num;
        using base_objects::list_array_depend;
        using base_objects::list_array_fixed;
        using base_objects::list_array_no_size;
        using base_objects::list_array_siz_from_packet;
        using base_objects::list_array_sized;
        using base_objects::list_array_sized_no_size;
        using base_objects::list_array_sized_siz_from_packet;
        using base_objects::no_size;
        using base_objects::optional_var_int32;
        using base_objects::optional_var_int64;
        using base_objects::or_;
        using base_objects::ordered_id;
        using base_objects::packet;
        using base_objects::packet_compress;
        using base_objects::partial_enum_switch;
        using base_objects::position;
        using base_objects::size_from_packet;
        using base_objects::size_source;
        using base_objects::sized_entry;
        using base_objects::string_sized;
        using base_objects::value_optional;
        using base_objects::var_int32;
        using base_objects::var_int64;

        //base_objects::box should always hold value
        struct chat_type {
            struct decoration {
                enum class param_e : uint8_t {
                    sender = 0,
                    target = 1,
                    content = 2
                };
                using enum param_e;
                std::string translation_key;
                list_array<base_objects::enum_as<param_e, base_objects::var_int32>> parameters;
                std::optional<Chat> style = std::nullopt;
            };

            decoration chat;
            decoration narration;
        };

        enum class gamemode_e : uint8_t {
            survival = 0,
            creative = 1,
            adventure = 2,
            spectator = 3,
        };

        enum class optional_gamemode_e : int8_t {
            undefined = -1,
            survival = 0,
            creative = 1,
            adventure = 2,
            spectator = 3,
        };
        enum class difficulty_e : uint8_t {
            peaceful = 0,
            easy = 1,
            normal = 2,
            hard = 3,
        };

        struct slot {
            depends_next<var_int32> count;
            var_int32::item id;
            var_int32 components_to_add;
            var_int32 components_to_remove;
            list_array_no_size<base_objects::component, &slot::components_to_add> to_add;
            list_array_no_size<var_int32::data_component_type, &slot::components_to_remove> to_remove;

            slot create(const base_objects::slot&);
        };

        struct slot_display {
            struct empty : public enum_item<0> {};

            struct any_fuel : public enum_item<1> {};

            struct item : public enum_item<2> {
                var_int32::item type;
            };

            struct item_stack : public enum_item<3> {
                slot item_stack;
            };

            struct tag : public enum_item<4> {
                identifier tag;
            };

            struct smithing_trim : public enum_item<5> {
                base_objects::box<slot_display> base;
                base_objects::box<slot_display> material;
                base_objects::box<slot_display> pattern;
            };

            struct with_remainder : public enum_item<6> {
                base_objects::box<slot_display> ingredient;
                base_objects::box<slot_display> remainder;
            };

            struct composite : public enum_item<7> {
                list_array<base_objects::box<slot_display>> ingredient;
            };

            enum_switch<
                var_int32,
                empty,
                any_fuel,
                item,
                item_stack,
                tag,
                smithing_trim,
                with_remainder,
                composite>
                display;
        };

        struct recipe_display {
            struct crafting_shapeless : public enum_item<0> {
                list_array<slot_display> ingredients;
                slot_display result;
                slot_display crafting_station;
            };

            struct crafting_shaped : public enum_item<1> {
                var_int32 width;
                var_int32 height;
                list_array<slot_display> ingredients;
                slot_display result;
                slot_display crafting_station;
            };

            struct furnace : public enum_item<2> {
                slot_display ingredient;
                slot_display fuel;
                slot_display result;
                slot_display crafting_station;
                var_int32 cooking_time;
                float experience;
            };

            struct stonecutter : public enum_item<3> {
                slot_display ingredient;
                slot_display result;
                slot_display crafting_station;
            };

            struct smithing : public enum_item<4> {
                slot_display template_;
                slot_display base;
                slot_display addition;
                slot_display result;
                slot_display crafting_station;
            };

            enum_switch<
                var_int32,
                crafting_shapeless,
                crafting_shaped,
                furnace,
                stonecutter,
                smithing>
                display;
        };

        struct particle_data {
            struct block : public enum_item<1> {
                var_int32::block_state id;
            };

            struct block_marker : public enum_item<2> {
                var_int32::block_state id;
            };

            struct dust : public enum_item<13> {
                int32_t rgb;
                float scale;
            };

            struct dust_color_transition : public enum_item<14> {
                int32_t from_rgb;
                int32_t to_rgb;
                float scale;
            };

            struct entity_effect : public enum_item<20> {
                int32_t argb;
            };

            struct falling_dust : public enum_item<28> {
                var_int32::block_state id;
            };

            struct tinted_leaves : public enum_item<35> {
                int32_t rgb;
            };

            struct sculk_charge : public enum_item<37> {
                float roll;
            };

            struct item : public enum_item<46> {
                slot item;
            };

            struct vibration : public enum_item<47> {

                struct block : public enum_item<0> {
                    position block_pos;
                };

                struct entity : public enum_item<1> {
                    var_int32 entity_id;
                    float eye_height;
                };

                partial_enum_switch<var_int32::position_source_type, block, entity> data;
                var_int32 travel_ticks;
            };

            struct trail : public enum_item<48> {
                double x;
                double y;
                double z;
                int32_t rgb;
                var_int32 duration;
            };

            struct shriek : public enum_item<102> {
                var_int32 delay;
            };

            struct dust_pillar : public enum_item<108> {
                var_int32::block_state id;
            };

            struct block_crumble : public enum_item<112> {
                var_int32::block_state id;
            };

            partial_enum_switch<
                var_int32::particle_type,
                block,
                block_marker,
                dust,
                dust_color_transition,
                entity_effect,
                falling_dust,
                tinted_leaves,
                sculk_charge,
                item,
                vibration,
                trail,
                shriek,
                dust_pillar,
                block_crumble>
                data;
        };

        struct teleport_flags {
            enum class flags_f {
                x_relative = 0x1,
                y_relative = 0x2,
                z_relative = 0x4,
                yaw_relative = 0x8,
                pitch_relative = 0x10,
                velocity_x_relative = 0x20,
                velocity_y_relative = 0x40,
                velocity_z_relative = 0x80,
                adjust_velocity_to_rotation = 0x100,
            };
            using enum flags_f;

            enum_as_flag<flags_f, int32_t> flags;
        };

        namespace client_bound {
            namespace status {
                struct status_response : public packet<0x00> {
                    string_sized<32767> json_response;
                };

                struct pong_response : public packet<0x01> {
                    uint64_t timestamp;
                };
            }

            decl_variant(
                status_packet,
                status::status_response,
                status::pong_response
            );

            namespace login {
                struct login_disconnect : public packet<0x00>, disconnect_after {
                    json_text_component reason;
                };

                struct hello : public packet<0x01> {
                    string_sized<20> server_id;
                    list_array<uint8_t> public_key;
                    list_array<uint8_t> verify_token;
                    bool should_authenticate;
                };

                struct login_finished : public packet<0x02> {

                    struct property {
                        string_sized<64> name;
                        string_sized<32767> value;
                        std::optional<string_sized<1024>> signature = std::nullopt;
                    };

                    enbt::raw_uuid uuid;
                    string_sized<16> user_name;
                    list_array_sized<property, 16> properties;
                };

                struct login_compression : public packet<0x03> {
                    packet_compress<var_int32> threshold;
                };

                struct custom_query : public packet<0x04> {
                    var_int32 query_message_id;
                    identifier channel;
                    list_array_sized_siz_from_packet<uint8_t, 1048576> payload;
                };

                struct cookie_request : public packet<0x05> {
                    identifier key;
                };
            }

            decl_variant(
                login_packet,
                login::login_disconnect,
                login::hello,
                login::login_finished,
                login::login_compression,
                login::custom_query,
                login::cookie_request
            );

            namespace configuration {
                struct cookie_request : public packet<0x00> {
                    identifier key;
                };

                struct custom_payload : public packet<0x01> {
                    identifier channel;
                    list_array_sized_siz_from_packet<uint8_t, 1048576> payload;
                };

                struct disconnect : public packet<0x02>, disconnect_after {
                    Chat reason;
                };

                struct finish_configuration : public packet<0x03> {};

                struct keep_alive : public packet<0x04> {
                    uint64_t keep_alive_id;
                };

                struct ping : public packet<0x05> {
                    ordered_id<int32_t, "ping"> ping_request_id;
                };

                struct reset_chat : public packet<0x06> {};

                struct registry_data : public packet<0x07> {

                    struct entry {
                        identifier entry_id;
                        std::optional<enbt::value> data = std::nullopt;
                    };

                    identifier registry_id;
                    list_array<entry> entries;
                };

                struct resource_pack_pop : public packet<0x08> {
                    std::optional<enbt::raw_uuid> uuid = std::nullopt;
                };

                struct resource_pack_push : public packet<0x09> {
                    enbt::raw_uuid uuid;
                    string_sized<32767> url;
                    string_sized<40> hash;
                    bool forced;
                    std::optional<Chat> prompt_message = std::nullopt;
                };

                struct store_cookie : public packet<0x0A> {
                    identifier key;
                    list_array_sized<uint8_t, 5120> payload;
                };

                struct transfer : public packet<0x0B> {
                    string_sized<32767> host;
                    var_int32 port;
                };

                struct update_enabled_features : public packet<0x0C> {
                    list_array<identifier> features;
                };

                struct update_tags : public packet<0x0D> {

                    struct tag {
                        identifier tag_name;
                        list_array<var_int32> values;
                    };

                    struct entry {
                        identifier registry_id;
                        list_array<tag> tags;
                    };

                    list_array<entry> entries;
                };

                struct select_known_packs : public packet<0x0E> {

                    struct pack {
                        string_sized<32767> pack_namespace;
                        string_sized<32767> id;
                        string_sized<32767> version;
                    };

                    list_array<pack> packs;
                };

                struct custom_report_details : public packet<0x0F> {

                    struct detail {
                        string_sized<128> title;
                        string_sized<4096> description;
                    };

                    list_array_sized<detail, 32> details;
                };

                struct server_links : public packet<0x10> {
                    enum class link_type : uint8_t {
                        bug_report = 0,
                        community_guidelines = 1,
                        support = 2,
                        status = 3,
                        feedback = 4,
                        community = 5,
                        website = 6,
                        forums = 7,
                        news = 8,
                        announcements = 9,
                    };
                    using enum link_type;

                    struct link {
                        or_<enum_as<link_type, var_int32>, Chat> label;
                        std::string url;
                    };

                    list_array<link> links;
                };

                struct clear_dialog : public packet<0x11> {};

                struct show_dialog : public packet<0x12> {
                    enbt::value dialog;
                };
            }

            decl_variant(
                configuration_packet,
                configuration::cookie_request,
                configuration::custom_payload,
                configuration::disconnect,
                configuration::finish_configuration,
                configuration::keep_alive,
                configuration::ping,
                configuration::reset_chat,
                configuration::registry_data,
                configuration::resource_pack_pop,
                configuration::resource_pack_push,
                configuration::store_cookie,
                configuration::transfer,
                configuration::update_enabled_features,
                configuration::update_tags,
                configuration::select_known_packs,
                configuration::custom_report_details,
                configuration::server_links,
                configuration::clear_dialog,
                configuration::show_dialog
            );


            struct play_packet;

            namespace play {
                struct bundle_delimiter : public compound_packet {
                    packet<0> begin;
                    list_array<play_packet> packets; //do not include here packet that switches to other state, it would not be called
                    packet<0> end;

                    bundle_delimiter();
                    bundle_delimiter(bundle_delimiter&&);
                    bundle_delimiter(const bundle_delimiter&);
                    bundle_delimiter(list_array<play_packet>&& mov);
                    bundle_delimiter(const list_array<play_packet>&& copy);
                };

                struct add_entity : public packet<0x01> {
                    var_int32 entity_id;
                    enbt::raw_uuid uuid;
                    var_int32::entity_type type;
                    double x;
                    double y;
                    double z;
                    Angle pitch;
                    Angle yaw;
                    Angle head_yaw;
                    var_int32 data;
                    short velocity_x;
                    short velocity_y;
                    short velocity_z;
                };

                struct animate : public packet<0x02> {
                    enum class animation_e : uint8_t {
                        swing_main_arm = 0,
                        unrecognized = 1,
                        leave_bed = 2,
                        swing_offhand = 3,
                        critical_hit = 4,
                        enchanted_hit = 5,
                    };
                    using enum animation_e;

                    var_int32 entity_id;
                    enum_as<animation_e, uint8_t> animation;
                };

                struct award_stats : public packet<0x03> {

                    struct statistic {
                        var_int32 category_id;
                        var_int32 statistic_id;
                        var_int32 value;
                    };

                    list_array<statistic> statistics;
                };

                struct block_changed_ack : public packet<0x04> {
                    var_int32 block_sequence_id;
                };

                struct block_destruction : public packet<0x05> {
                    var_int32 entity_id;
                    position location;
                    uint8_t destroy_stage;
                };

                struct block_entity_data : public packet<0x06> {
                    position location;
                    var_int32::block_entity_type type;
                    enbt::value data;
                };

                struct block_event : public packet<0x07> {
                    position location;
                    uint8_t action_id;
                    uint8_t action_param;
                    var_int32::block_type block;
                };

                struct block_update : public packet<0x08> {
                    position location;
                    var_int32::block_state block;
                };

                struct boss_event : public packet<0x09> {
                    enbt::raw_uuid uuid;

                    struct add : public enum_item<0> {
                        Chat title;
                        float health;
                        var_int32 color;
                        var_int32 division;
                        uint8_t flags;
                    };

                    struct remove : public enum_item<1> {};

                    struct update_health : public enum_item<2> {
                        float health;
                    };

                    struct update_title : public enum_item<3> {
                        Chat title;
                    };

                    struct update_style : public enum_item<4> {
                        var_int32 color;
                        var_int32 division;
                    };

                    struct update_flags : public enum_item<5> {
                        uint8_t flags;
                    };

                    enum_switch<
                        var_int32,
                        add,
                        remove,
                        update_health,
                        update_title,
                        update_style,
                        update_flags>
                        action;
                };

                struct change_difficulty : public packet<0x0A> {
                    enum_as<difficulty_e, uint8_t> difficulty;
                    bool is_locked;
                };

                struct chunk_batch_finished : public packet<0x0B> {
                    var_int32 batch_size;
                };

                struct chunk_batch_start : public packet<0x0C> {};

                struct chunks_biomes : public packet<0x0D> {
                    int32_t z;
                    int32_t x;
                    sized_entry<list_array_no_size<base_objects::pallete_container_biome, size_source::get_world_chunks_height>, var_int32> sections_of_biomes;

                    static chunks_biomes create(const storage::chunk_data&);
                };

                struct clear_titles : public packet<0x0E> {
                    bool reset;
                };

                struct command_suggestions : public packet<0x0F> {

                    struct match {
                        string_sized<32767> set;
                        std::optional<Chat> tooltip = std::nullopt;
                    };

                    var_int32 suggestion_transaction_id;
                    var_int32 start;
                    var_int32 length;
                    list_array<match> matches;
                };

                struct commands : public packet<0x10> {
                    struct node {
                        struct root_node : public flag_item<0, 0x3, 1> {};

                        struct literal_node : public flag_item<1, 0x3, 1> {
                            string_sized<32767> name;
                        };

                        struct argument_node : public flag_item<2, 0x3, 1> {
                            string_sized<32767> name;
                            base_objects::command_parser type;
                        };

                        struct is_executable : public flag_item<0x04, 0x04, -1> {};

                        struct redirect_node : public flag_item<8, 0x8, 0> {
                            var_int32 node;
                        };

                        struct suggestions_type : public flag_item<0x10, 0x10, 2> {
                            identifier name;
                        };

                        struct is_restricted : public flag_item<0x20, 0x20, -2> {};

                        int8_t flags;
                        list_array<var_int32> children;
                        flags_list_from<
                            node,
                            int8_t,
                            &node::flags,
                            literal_node,
                            root_node,
                            argument_node,
                            is_executable,
                            redirect_node,
                            suggestions_type,
                            is_restricted>
                            flags_values;
                    };

                    list_array<node> nodes;
                    var_int32 root_index;

                    static commands create(const base_objects::command_manager& manager);
                };

                struct container_close : public packet<0x11> {
                    var_int32 windows_id;
                };

                struct container_set_content : public packet<0x12> {
                    var_int32 windows_id;
                    var_int32 state_id;
                    list_array<slot> inventory_data;
                    slot carried_item;
                };

                struct container_set_data : public packet<0x13> {
                    var_int32 windows_id;

                    struct furnace {
                        enum class property_e : uint8_t {
                            fuel_left = 0,
                            max_fuel = 1,
                            progress = 2,
                            max_progress = 3,
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct enchantment_table {
                        enum class property_e : uint8_t {
                            level_requirement_top = 0,
                            level_requirement_middle = 1,
                            level_requirement_bottom = 2,
                            enchantment_seed = 3,
                            enchantment_id_top = 4,
                            enchantment_id_middle = 5,
                            enchantment_id_bottom = 6,
                            enchantment_lvl_top = 7,
                            enchantment_lvl_middle = 8,
                            enchantment_lvl_bottom = 9,
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct beacon {
                        enum class property_e : uint8_t {
                            power_level = 0,
                            first_potion = 1,
                            second_potion = 2,
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct anvil {
                        enum class property_e : uint8_t {
                            repair_cost = 0,
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct brewing_stand {
                        enum class property_e : uint8_t {
                            brew_time = 0, //400-0
                            fuel_left = 1, //0-20
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct stonecutter {
                        enum class property_e : uint8_t {
                            selected_recipe = 0, //-1 = none
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct loom {
                        enum class property_e : uint8_t {
                            selected_pattern = 0, //0 = base
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct lectern {
                        enum class property_e : uint8_t {
                            page_number = 0,
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct smithing_table {
                        enum class property_e : uint8_t {
                            has_recipe_error = 0, // 0>= == false, 0< == true
                        };
                        using enum property_e;
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct other {
                        short property;
                        short value;
                    };

                    any_of<
                        other,
                        anvil,
                        beacon,
                        brewing_stand,
                        furnace,
                        enchantment_table,
                        stonecutter,
                        loom,
                        lectern,
                        smithing_table>
                        value;
                };

                struct container_set_slot : public packet<0x14> {
                    var_int32 windows_id;
                    var_int32 state_id;
                    short slot;
                    struct slot carried_item;
                };

                struct cookie_request : public packet<0x15> {
                    identifier key;
                };

                struct cooldown : public packet<0x16> {
                    identifier group;
                    var_int32 ticks;
                };

                struct custom_chat_completions : public packet<0x17> {
                    enum class suggestion_e : uint8_t {
                        add = 0,
                        remove = 1,
                        set = 2,
                    };
                    using enum suggestion_e;
                    enum_as<suggestion_e, var_int32> suggestion;
                    list_array<string_sized<32767>> entries;
                };

                struct custom_payload : public packet<0x18> {
                    identifier channel;
                    list_array_sized_siz_from_packet<uint8_t, 1048576> payload;
                };

                struct damage_event : public packet<0x19> {
                    var_int32 entity_id;
                    optional_var_int32 source_damage_type_id = std::nullopt;
                    optional_var_int32 source_entity_id = std::nullopt;
                    optional_var_int32 source_direct_entity_id = std::nullopt;
                    std::optional<util::VECTOR> source_pos = std::nullopt;
                };

                struct debug_sample : public packet<0x1A> {
                    list_array<int64_t> sample;
                    var_int32 sample_type;
                };

                struct delete_chat : public packet<0x1B> {
                    var_int32 message_id;
                    std::optional<std::array<uint8_t, 256>> signature = std::nullopt;
                };

                struct disconnect : public packet<0x1C>, disconnect_after {
                    Chat reason;
                };

                struct disguised_chat : public packet<0x1D> {
                    Chat message;
                    or_<var_int32::chat_type, chat_type> type;
                    Chat sender;
                    std::optional<Chat> target_name = std::nullopt;
                };

                struct entity_event : public packet<0x1E> {
                    var_int32 entity_id;
                    int8_t status;
                };

                struct entity_position_sync : public packet<0x1F> {
                    var_int32 entity_id;
                    double x;
                    double y;
                    double z;
                    double velocity_x;
                    double velocity_y;
                    double velocity_z;
                    float yaw;
                    float pitch;
                    bool on_ground;
                };

                struct explode : public packet<0x20> {

                    struct player_delta_velocity_t {
                        double x;
                        double y;
                        double z;
                    };

                    double x;
                    double y;
                    double z;
                    std::optional<player_delta_velocity_t> player_delta_velocity = std::nullopt;
                    particle_data particle;
                    or_<var_int32::sound_event, base_objects::sound_event> sound;
                };

                struct forget_level_chunk : public packet<0x21> {
                    int32_t z;
                    int32_t x;
                };

                struct game_event : public packet<0x22> {

                    struct no_respawn_block_available : public enum_item<0> {
                        float _ignored = 0.0f;
                    };

                    struct raining_begin : public enum_item<1> {
                        float _ignored = 0.0f;
                    };

                    struct raining_end : public enum_item<2> {
                        float _ignored = 0.0f;
                    };

                    struct gamemode_change : public enum_item<3> {
                        enum_as<gamemode_e, float> gamemode;
                    };

                    struct win_game : public enum_item<4> {
                        float roll_credits; //true/false
                    };

                    struct demo_event : public enum_item<5> {
                        enum class event_e : uint8_t {
                            welcome = 0,
                            movement_controls = 101,
                            jump_controls = 102,
                            inventory_controls = 103,
                            demo_over = 104,
                        };
                        using enum event_e;
                        enum_as<event_e, float> event;
                    };

                    struct arrow_hit_player : public enum_item<6> {
                        float _ignored = 0.0f;
                    };

                    struct rain_level_change : public enum_item<7> {
                        float level;
                    };

                    struct thunder_level_change : public enum_item<8> {
                        float level;
                    };

                    struct puffer_fish_sting_sound : public enum_item<9> {
                        float _ignored = 0.0f;
                    };

                    struct guardian_appear_animation : public enum_item<10> {
                        float _ignored = 0.0f;
                    };

                    struct respawn_screen_mode : public enum_item<11> {
                        float enabled; //true/false
                    };

                    struct limited_crafting_mode : public enum_item<12> {
                        float enabled; //true/false
                    };

                    struct wait_for_level_chunks : public enum_item<13> {
                        float _ignored = 0.0f;
                    };

                    enum_switch<
                        var_int32,
                        no_respawn_block_available,
                        raining_begin,
                        raining_end,
                        gamemode_change,
                        win_game,
                        demo_event,
                        arrow_hit_player,
                        rain_level_change,
                        thunder_level_change,
                        puffer_fish_sting_sound,
                        guardian_appear_animation,
                        respawn_screen_mode,
                        limited_crafting_mode,
                        wait_for_level_chunks>
                        event;
                };

                struct horse_screen_open : public packet<0x23> {
                    var_int32 window_id;
                    var_int32 columns_count;
                    int32_t entity_id;
                };

                struct hurt_animation : public packet<0x24> {
                    var_int32 entity_id;
                    float yaw;
                };

                struct initialize_border : public packet<0x25> {
                    double x;
                    double z;
                    double old_diameter;
                    double new_diameter;
                    var_int64 speed_ms;
                    var_int32 portal_teleport_boundary;
                    var_int32 warning_blocks;
                    var_int32 warning_time;
                };

                struct keep_alive : public packet<0x26> {
                    uint64_t keep_alive_id;
                };

                struct level_chunk_with_light : public packet<0x27> {
                    struct height_map {
                        enum class type_e : uint8_t {
                            world_surface = 1,
                            ocean_floor = 3,
                            motion_blocking = 4,
                            motion_blocking_no_leaves = 5,
                        };
                        using enum type_e;
                        enum_as<type_e, var_int32> type;
                        base_objects::pallete_data_height_map pallete_data;
                    };

                    struct section {
                        uint16_t block_count;
                        base_objects::pallete_container_block block_states;
                        base_objects::pallete_container_biome biomes;
                    };

                    struct block_entity {
                        uint8_t xz;
                        short y;
                        var_int32::block_entity_type type;
                        enbt::value data;
                    };

                    int32_t x;
                    int32_t z;
                    list_array<height_map> height_maps;
                    sized_entry<list_array_no_size<section, size_source::get_world_chunks_height>, var_int32> sections;
                    list_array<block_entity> block_entities;

                    list_array<uint64_t> sky_light_mask;
                    list_array<uint64_t> block_light_mask;
                    list_array<uint64_t> empty_sky_light_mask;
                    list_array<uint64_t> empty_block_light_mask;
                    list_array<list_array_fixed<uint8_t, 2048>> sky_light;
                    list_array<list_array_fixed<uint8_t, 2048>> block_light;

                    static level_chunk_with_light create(const storage::chunk_data&, const storage::world_data&);
                };

                struct level_event : public packet<0x28> {
                    enum class event_id : uint16_t {
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
                        zombie_attacks_wooden_door = 1019,
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
                    enum_as<event_id, int32_t> event;
                    position location;
                    int32_t data;
                    bool disable_volume;
                };

                struct level_particles : public packet<0x29> {
                    bool long_distance;
                    bool always_visible;
                    double x;
                    double y;
                    double z;
                    float offset_x;
                    float offset_y;
                    float offset_z;
                    float max_speed;
                    int32_t particle_count;
                    particle_data particle;
                };

                struct light_update : public packet<0x2A> {
                    int32_t x;
                    int32_t z;
                    list_array<uint64_t> sky_light_mask;
                    list_array<uint64_t> block_light_mask;
                    list_array<uint64_t> empty_sky_light_mask;
                    list_array<uint64_t> empty_block_light_mask;
                    list_array<list_array_fixed<uint8_t, 2048>> sky_light;
                    list_array<list_array_fixed<uint8_t, 2048>> block_light;


                    static light_update create(const storage::chunk_data&);
                };

                struct login : public packet<0x2B> {

                    struct death_location_t {
                        identifier world;
                        position location;
                    };

                    int32_t entity_id;
                    bool is_hardcore;
                    list_array<identifier> dimension_names;
                    var_int32 max_players;
                    var_int32 view_distance;
                    var_int32 simulation_distance;
                    bool reduced_debug_info;
                    bool respawn_screen;
                    bool limited_crafting_enabled;
                    var_int32::dimension_type dimension_type;
                    identifier dimension_name;
                    int64_t seed_hashed;
                    enum_as<gamemode_e, uint8_t> gamemode;
                    enum_as<optional_gamemode_e, int8_t> prev_gamemode;
                    bool world_is_debug;
                    bool world_is_flat;
                    std::optional<death_location_t> death_location = std::nullopt;
                    var_int32 portal_cooldown;
                    var_int32 sea_level;
                    bool enforce_secure_chat;
                };

                struct map_item_data : public packet<0x2C> {

                    struct icon {
                        enum class type_e : uint8_t {
                            white_arrow = 0,
                            green_arrow = 1,
                            red_arrow = 2,
                            blue_arrow = 3,
                            white_cross = 4,
                            red_pointer = 5,
                            white_circle = 6,
                            small_white_circle = 7,
                            mansion = 8,
                            monument = 9,
                            white_banner = 10,
                            orange_banner = 11,
                            magenta_banner = 12,
                            light_blue_banner = 13,
                            yellow_banner = 14,
                            lime_banner = 15,
                            pink_banner = 16,
                            gray_banner = 17,
                            light_gray_banner = 18,
                            cyan_banner = 19,
                            purple_banner = 20,
                            blue_banner = 21,
                            brown_banner = 22,
                            green_banner = 23,
                            red_banner = 24,
                            black_banner = 25,
                            treasure_marker = 26,
                            desert_village = 27,
                            plains_village = 28,
                            savanna_village = 29,
                            snowy_village = 30,
                            taiga_village = 31,
                            jungle_temple = 32,
                            swamp_hut = 33,
                            trial_chambers = 34,
                        };
                        enum_as<type_e, var_int32> type;
                        limited_num<int8_t, -128, 127> x;
                        limited_num<int8_t, -128, 127> z;
                        limited_num<int8_t, 0, 15> dir;
                        std::optional<Chat> name = std::nullopt;
                    };

                    struct color_patch {
                        depends_next<uint8_t> columns;
                        uint8_t rows;
                        uint8_t x;
                        uint8_t z;
                        list_array_no_size<uint8_t, &color_patch::columns, &color_patch::rows> data; //255 color pallete
                    };

                    var_int32 map_id;
                    int8_t scale;
                    bool is_locked;
                    std::optional<list_array<icon>> icons = std::nullopt;
                    color_patch patch;
                };

                struct merchant_offers : public packet<0x2D> {

                    struct trade {
                        struct trade_item {
                            var_int32::item item_id;
                            var_int32 item_count;
                            list_array<base_objects::component> components;
                        };

                        trade_item input_0;
                        slot output;
                        std::optional<trade_item> input_1 = std::nullopt;
                        bool trade_disabled;
                        int trade_uses;
                        int max_trade_uses;
                        int xp;
                        int special_price;
                        float price_multiplier;
                        int demand;
                    };

                    var_int32 window_id;
                    list_array<trade> trades;
                    var_int32 villager_level;
                    var_int32 experience;
                    bool is_regular_villager;
                    bool can_restock;
                };

                struct move_entity_pos : public packet<0x2E> {
                    var_int32 entity_id;
                    short delta_x;
                    short delta_y;
                    short delta_z;
                    bool on_ground;
                };

                struct move_entity_pos_rot : public packet<0x2F> {
                    var_int32 entity_id;
                    short delta_x;
                    short delta_y;
                    short delta_z;
                    Angle yaw;
                    Angle pitch;
                    bool on_ground;
                };

                struct move_minecart_along_track : public packet<0x30> {

                    struct step {
                        double x;
                        double y;
                        double z;
                        double velocity_x;
                        double velocity_y;
                        double velocity_z;
                        Angle yaw;
                        Angle pitch;
                        float weight;
                    };

                    var_int32 entity_id;
                    list_array<step> steps;
                };

                struct move_entity_rot : public packet<0x31> {
                    var_int32 entity_id;
                    Angle yaw;
                    Angle pitch;
                    bool on_ground;
                };

                struct move_vehicle : public packet<0x32> {
                    double x;    //absolute
                    double y;    //absolute
                    double z;    //absolute
                    Angle yaw;   //absolute
                    Angle pitch; //absolute
                };

                struct open_book : public packet<0x33> {
                    enum class hand_e : uint8_t {
                        main = 0,
                        off = 1,
                    };
                    enum_as<hand_e, var_int32> hand;
                };

                struct open_screen : public packet<0x34> {
                    var_int32 window_id;
                    var_int32::menu window_type;
                    Chat window_title;
                };

                struct open_sign_editor : public packet<0x35> {
                    position location;
                    bool is_front_text;
                };

                struct ping : public packet<0x36> {
                    int32_t id;
                };

                struct pong_response : public packet<0x37> {
                    uint64_t id;
                };

                struct place_ghost_recipe : public packet<0x38> {
                    var_int32 window_id;
                    recipe_display display;
                };

                struct player_abilities : public packet<0x39> {
                    enum class flags_f : uint8_t {
                        invulnerable = 0x1,
                        flying = 0x2,
                        allow_flying = 0x4,
                        creative_mode = 0x8,
                    };
                    using enum flags_f;

                    enum_as_flag<flags_f, uint8_t> flags;
                    float flying_speed;
                    float fov_modifier;
                };

                struct player_chat : public packet<0x3A> {
                    struct previous_message {
                        value_optional<var_int32, std::array<uint8_t, 256>> message_id_or_signature;
                    };

                    struct no_filter : public enum_item<0> {};

                    struct fully_filtered : public enum_item<1> {};

                    struct partially_filtered : public enum_item<2> {
                        bit_list_array<uint64_t> filtered_characters;
                    };

                    var_int32 global_index;
                    enbt::raw_uuid sender;
                    var_int32 index;
                    std::optional<std::array<uint8_t, 256>> signature = std::nullopt;
                    string_sized<256> message;
                    uint64_t timestamp;
                    uint64_t salt;
                    list_array_sized<previous_message, 20> previous_messages;
                    std::optional<Chat> unsigned_content = std::nullopt;
                    enum_switch<var_int32, no_filter, fully_filtered, partially_filtered> filter;
                    or_<var_int32::chat_type, chat_type> type;
                    Chat sender_name;
                    std::optional<Chat> target_name = std::nullopt;
                };

                struct player_combat_end : public packet<0x3B> {
                    var_int32 duration;
                };

                struct player_combat_enter : public packet<0x3C> {};

                struct player_combat_kill : public packet<0x3D> {
                    var_int32 player_id;
                    Chat message;
                };

                struct player_info_remove : public packet<0x3E> {
                    list_array<enbt::raw_uuid> uuids;
                };

                struct player_info_update : public packet<0x3F> {
                    struct add_player {
                        string_sized<16> name;

                        struct property {
                            string_sized<64> name;
                            string_sized<32767> value;
                            std::optional<string_sized<1024>> signature = std::nullopt;
                        };

                        list_array_sized<property, 16> properties;
                    };

                    struct initialize_chat {
                        enbt::raw_uuid chat_session_id;
                        uint64_t pub_key_expiries_timestamp;
                        list_array_fixed<uint8_t, 512> public_key;
                        list_array_fixed<uint8_t, 4096> public_signature;
                    };

                    struct set_gamemode {
                        var_int32 gamemode;
                    };

                    struct listed {
                        bool should;
                    };

                    struct set_ping {
                        var_int32 milliseconds;
                    };

                    struct set_display_name {
                        std::optional<Chat> name = std::nullopt;
                    };

                    struct set_hat_visible {
                        bool visible;
                    };

                    struct set_list_priority {
                        var_int32 level;
                    };

                    struct header {
                        enbt::raw_uuid uuid;
                    };

                    enum_set<
                        header,
                        add_player,
                        initialize_chat,
                        set_gamemode,
                        listed,
                        set_ping,
                        set_display_name,
                        set_list_priority,
                        set_hat_visible>
                        actions;
                };

                struct player_look_at : public packet<0x40> {
                    enum class using_position_e : uint8_t {
                        feet = 0,
                        eyes = 1,
                    };
                    using enum using_position_e;

                    struct entity_target {
                        int32_t entity_id;
                        enum_as<using_position_e, var_int32> using_position;
                    };

                    enum_as<using_position_e, var_int32> using_position;
                    double target_x;
                    double target_y;
                    double target_z;
                    std::optional<entity_target> entity = std::nullopt;
                };

                struct player_position : public packet<0x41> {
                    ordered_id<var_int32, "sync/player_position"> teleport_id;
                    double x;
                    double y;
                    double z;
                    double velocity_x;
                    double velocity_y;
                    double velocity_z;
                    float yaw;
                    float pitch;
                    teleport_flags flags;
                };

                struct player_rotation : public packet<0x42> {
                    float yaw;
                    float pitch;
                };

                struct recipe_book_add : public packet<0x43> {

                    struct recipe {
                        enum class flags_f : uint8_t {
                            show_notification = 0x1,
                            highlight_as_new = 0x2,
                        };
                        using enum flags_f;

                        var_int32::recipe recipe_id;
                        recipe_display display;
                        var_int32 group_id;
                        var_int32 category_id;
                        std::optional<list_array<id_set<var_int32::item>>> ingredients = std::nullopt;
                        enum_as_flag<flags_f, int8_t> flags;
                    };

                    list_array<recipe> recipes;
                    bool replace;
                };

                struct recipe_book_remove : public packet<0x44> {
                    list_array<var_int32::recipe> recipe_ids;
                };

                struct recipe_book_settings : public packet<0x45> {
                    bool crafting_recipe_open;
                    bool crafting_recipe_filter_active;
                    bool smelting_recipe_open;
                    bool smelting_recipe_filter_active;
                    bool blast_recipe_open;
                    bool blast_recipe_filter_active;
                    bool smoker_recipe_open;
                    bool smoker_recipe_filter_active;
                };

                struct remove_entities : public packet<0x46> {
                    list_array<var_int32> entity_ids;
                };

                struct remove_mob_effect : public packet<0x47> {
                    var_int32 entity_id;
                    var_int32::mob_effect effect_id;
                };

                struct reset_score : public packet<0x48> {
                    string_sized<32767> entity_name;
                    std::optional<string_sized<32767>> objective_name = std::nullopt;
                };

                struct resource_pack_pop : public packet<0x49> {
                    std::optional<enbt::raw_uuid> uuid = std::nullopt;
                };

                struct resource_pack_push : public packet<0x4A> {
                    enbt::raw_uuid uuid;
                    string_sized<32767> url;
                    string_sized<40> hash; //0 or 40, other values waste bandwidth
                    bool forced;
                    std::optional<Chat> prompt_message = std::nullopt;
                };

                struct respawn : public packet<0x4B> {

                    struct death_location_t {
                        identifier dimension_name;
                        position location;
                    };

                    var_int32::dimension_type dimension_type;
                    identifier dimension_name;
                    uint64_t seed_hashed;
                    enum_as<gamemode_e, uint8_t> gamemode;
                    enum_as<optional_gamemode_e, int8_t> previous_gamemode;
                    bool is_debug;
                    bool is_flat;
                    std::optional<death_location_t> death_location = std::nullopt;
                    var_int32 portal_cooldown;
                    var_int32 sea_level;
                    enum class flags_f : uint8_t {
                        keep_attributes = 0x1,
                        keep_metadata = 0x2,
                    };
                    using enum flags_f;

                    enum_as_flag<flags_f, uint8_t> flags;
                };

                struct rotate_head : public packet<0x4C> {
                    var_int32 entity_id;
                    Angle head_yaw; //new angle
                };

                struct section_blocks_update : public packet<0x4D> {

                    struct position_t {
                        uint64_t x : 22;
                        uint64_t z : 22;
                        uint64_t y : 20;

                        uint64_t to_packet() const;

                        static position_t from_packet(uint64_t value);
                    };

                    struct block_entry {
                        uint32_t block_state : 20;
                        uint32_t local_x : 4;
                        uint32_t local_z : 4;
                        uint32_t local_y : 4;

                        var_int64 to_packet() const;

                        static block_entry from_packet(var_int64 value);
                    };

                    position_t position;
                    list_array<block_entry> block;
                };

                struct select_advancements_tab : public packet<0x4E> {
                    std::optional<identifier> id = std::nullopt;
                };

                struct server_data : public packet<0x4F> {
                    Chat motd;
                    std::optional<list_array<uint8_t>> icon_png = std::nullopt;
                };

                struct set_action_bar_text : public packet<0x50> {
                    Chat text;
                };

                struct set_border_center : public packet<0x51> {
                    double x;
                    double z;
                };

                struct set_border_lerp_size : public packet<0x52> {
                    double old_diameter;
                    double new_diameter;
                    var_int64 speed_milliseconds;
                };

                struct set_border_size : public packet<0x53> {
                    double diameter;
                };

                struct set_border_warning_delay : public packet<0x54> {
                    var_int32 warn_time;
                };

                struct set_border_warning_distance : public packet<0x55> {
                    var_int32 meters;
                };

                struct set_camera : public packet<0x56> {
                    var_int32 entity_id;
                };

                struct set_chunk_cache_center : public packet<0x57> {
                    var_int32 x;
                    var_int32 z;
                };

                struct set_chunk_cache_radius : public packet<0x58> {
                    var_int32 distance;
                };

                struct set_cursor_item : public packet<0x59> {
                    slot item;
                };

                struct set_default_spawn_position : public packet<0x5A> {
                    position location;
                    float angle;
                };

                struct set_display_objective : public packet<0x5B> {
                    enum class position_e : uint8_t {
                        list = 0,
                        sidebar = 1,
                        below_name = 2,
                        team_white = 4,
                        team_orange = 5,
                        team_magenta = 6,
                        team_light_blue = 7,
                        team_yellow = 8,
                        team_lime = 9,
                        team_pink = 10,
                        team_gray = 11,
                        team_light_gray = 12,
                        team_cyan = 13,
                        team_purple = 14,
                        team_blue = 15,
                        team_brown = 16,
                        team_green = 17,
                        team_red = 18,
                        team_black = 19,
                    };
                    using enum position_e;
                    enum_as<position_e, var_int32> position;
                    string_sized<32767> name;
                };

                struct set_entity_data : public packet<0x5C> {
                    var_int32 entity_id;
                    list_array_siz_from_packet<uint8_t> metadata;
                };

                struct set_entity_link : public packet<0x5D> {
                    int32_t attached_entity_id;
                    int32_t holding_entity_id;
                };

                struct set_entity_motion : public packet<0x5E> {
                    var_int32 entity_id;
                    int16_t velocity_x;
                    int16_t velocity_y;
                    int16_t velocity_z;
                };

                struct set_equipment : public packet<0x5F> {

                    struct equipment {
                        enum class slot_place_e {
                            main_hand = 0,
                            off_hand = 1,
                            boots = 2,
                            leggings = 3,
                            chestplate = 4,
                            helmet = 5,
                            body = 6,
                            saddle = 7,
                        };
                        ignored<bool> has_next_item = false;

                        item_depend<enum_as<slot_place_e, uint8_t>, 0x80, &equipment::has_next_item> slot_place;
                        slot item;
                    };

                    var_int32 entity_id;
                    list_array_depend<equipment> equipments;
                };

                struct set_experience : public packet<0x60> {
                    limited_num<float, 0.0f, 1.0f> bar;
                    var_int32 level;
                    var_int32 total_experience;
                };

                struct set_health : public packet<0x61> {
                    float health;
                    var_int32 food;
                    limited_num<float, 0.0f, 5.0f> saturation;
                };

                struct set_held_slot : public packet<0x62> {
                    var_int32 slot;
                };

                struct set_objective : public packet<0x63> {

                    struct blank : public enum_item<0> {};

                    struct styled : public enum_item<1> {
                        enbt::compound styling;
                    };

                    struct fixed : public enum_item<2> {
                        Chat content;
                    };

                    struct create : public enum_item<0> {
                        Chat name;
                        var_int32 type; //0 numbers, 1 - hearts
                        std::optional<enum_switch<var_int32, blank, styled, fixed>> default_format = std::nullopt;
                    };

                    struct remove : public enum_item<1> {};

                    struct update : public enum_item<2> {
                        Chat name;
                        var_int32 type; //0 numbers, 1 - hearts
                        std::optional<enum_switch<var_int32, blank, styled, fixed>> default_format = std::nullopt;
                    };

                    string_sized<32767> name;
                    enum_switch<int8_t, create, remove, update> mode;
                };

                struct set_passengers : public packet<0x64> {
                    var_int32 entity_id;
                    list_array<var_int32> passengers;
                };

                struct set_player_inventory : public packet<0x65> {
                    var_int32 slot;
                    struct slot data;
                };

                struct set_player_team : public packet<0x66> {
                    enum class friendly_f : int8_t {
                        allow_friendly_fire = 0x1,
                        can_see_invisible = 0x2,
                    };

                    enum class name_tag_visibility_e {
                        always,
                        never,
                        hide_for_others,
                        hide_for_own,
                    };

                    enum class collision_rule_e {
                        always,
                        never,
                        push_for_others,
                        push_for_own,
                    };

                    struct create : public enum_item<0> {
                        Chat display_name;
                        enum_as_flag<friendly_f, int8_t> friendly;
                        enum_as<name_tag_visibility_e, var_int32> name_tag_visibility;
                        enum_as<collision_rule_e, var_int32> collision_rule;
                        var_int32 team_color;
                        Chat prefix;
                        Chat suffix;
                        list_array<string_sized<32767>> entries;
                    };

                    struct remove : public enum_item<1> {};

                    struct update : public enum_item<2> {
                        Chat display_name;
                        enum_as_flag<friendly_f, int8_t> friendly;
                        enum_as<name_tag_visibility_e, var_int32> name_tag_visibility;
                        enum_as<collision_rule_e, var_int32> collision_rule;
                        var_int32 team_color;
                        Chat prefix;
                        Chat suffix;
                    };

                    struct add_entries : public enum_item<3> {
                        list_array<string_sized<32767>> entries;
                    };

                    struct remove_entries : public enum_item<4> {
                        list_array<string_sized<32767>> entries;
                    };

                    string_sized<32767> name;
                    enum_switch<int8_t, create, remove, update, add_entries, remove_entries> mode;
                };

                struct set_score : public packet<0x67> {

                    struct blank : public enum_item<0> {};

                    struct styled : public enum_item<1> {
                        enbt::compound styling;
                    };

                    struct fixed : public enum_item<2> {
                        Chat content;
                    };

                    string_sized<32767> entry_name;
                    string_sized<32767> objective_name;
                    var_int32 value;
                    std::optional<Chat> name = std::nullopt;
                    std::optional<enum_switch<var_int32, blank, styled, fixed>> default_format = std::nullopt;
                };

                struct set_simulation_distance : public packet<0x68> {
                    var_int32 distance;
                };

                struct set_subtitle_text : public packet<0x69> {
                    Chat text;
                };

                struct set_time : public packet<0x6A> {
                    uint64_t world_age;
                    uint64_t time_of_day;
                    bool time_of_day_increment;
                };

                struct set_title_text : public packet<0x6B> {
                    Chat text;
                };

                struct set_titles_animation : public packet<0x6C> {
                    int32_t fade_in;
                    int32_t stay;
                    int32_t fadeout;
                };

                struct sound_entity : public packet<0x6D> {
                    or_<var_int32::sound_event, base_objects::sound_event> sound;
                    var_int32 category;
                    var_int32 entity_id;
                    float volume;
                    float pitch;
                    int64_t seed;
                };

                struct sound : public packet<0x6E> {
                    or_<var_int32::sound_event, base_objects::sound_event> sound;
                    var_int32 category;
                    int32_t x;
                    int32_t y;
                    int32_t z;
                    float volume;
                    float pitch;
                    int64_t seed;
                };

                struct start_configuration : public packet<0x6F> {};

                struct stop_sound : public packet<0x70> {

                    struct source : public flag_item<0x1, 0x1, 1> {
                        var_int32 source;
                    };

                    struct sound_name : public flag_item<0x2, 0x2, 2> {
                        identifier name;
                    };

                    flags_list<int8_t, source, sound_name> flags;
                };

                struct store_cookie : public packet<0x71> {
                    identifier key;
                    list_array_sized<uint8_t, 5120> payload;
                };

                struct system_chat : public packet<0x72> {
                    Chat content;
                    bool is_overlay = false;
                };

                struct tab_list : public packet<0x73> {
                    Chat header;
                    Chat footer;
                };

                struct tag_query : public packet<0x74> {
                    var_int32 tag_query_id; //managed by client
                    enbt::value nbt;
                };

                struct take_item_entity : public packet<0x75> {
                    var_int32 collected_entity_id;
                    var_int32 collectors_entity_id;
                    var_int32 items_count;
                };

                struct teleport_entity : public packet<0x76> {
                    var_int32 entity_id;
                    double x;
                    double y;
                    double z;
                    double velocity_x;
                    double velocity_y;
                    double velocity_z;
                    float yaw;
                    float pitch;
                    teleport_flags flags;
                    bool on_ground;
                };

                struct test_instance_block_status : public packet<0x77> {

                    struct volume_t {
                        double x;
                        double y;
                        double z;
                    };

                    Chat status;
                    std::optional<volume_t> volume = std::nullopt;
                };

                struct ticking_state : public packet<0x78> {
                    float tick_rate;
                    bool is_frozen;
                };

                struct ticking_step : public packet<0x79> {
                    var_int32 steps = 1;
                };

                struct transfer : public packet<0x7A> {
                    std::string host;
                    var_int32 port;
                };

                struct update_advancements : public packet<0x7B> {

                    struct display {
                        struct background_texture : public flag_item<0x1, 0x1, 1> {
                            identifier texture;
                        };

                        struct show_toast : public flag_item<0x2, 0x2, 2> {};

                        struct hidden : public flag_item<0x3, 0x3, 3> {};

                        Chat title;
                        Chat description;
                        slot icon;
                        var_int32 frame_type;
                        flags_list<int32_t, background_texture, show_toast, hidden> flags;
                        float x_cord;
                        float y_cord;
                    };

                    struct advancement {
                        std::optional<identifier> parent_id = std::nullopt;
                        std::optional<display> display = std::nullopt;
                        list_array<list_array<string_sized<32767>>> nested_requirements;
                        bool send_telemetry = false;
                    };

                    struct progress {
                        identifier criterion;
                        std::optional<int64_t> date_of_archiving = std::nullopt;
                    };

                    struct advancement_mapping {
                        identifier key;
                        advancement value;
                    };

                    struct progress_mapping {
                        identifier key;
                        list_array<progress> value;
                    };

                    bool clear_prev;
                    list_array<advancement_mapping> advancement_mappings;
                    list_array<identifier> remove_advancements;
                    list_array<progress_mapping> progress_mappings;
                    bool show;
                };

                struct update_attributes : public packet<0x7C> {
                    //clients execute add operation first then add_percent and at the end multiply
                    enum class operation_e {
                        add = 0,
                        add_percent = 1,
                        multiply = 2
                    };

                    struct modifier {
                        identifier id;
                        double amount;
                        enum_as<operation_e, int8_t> operation;
                    };

                    struct property {
                        var_int32::attribute id;
                        double value;
                        list_array<modifier> modifiers;
                    };

                    var_int32 entity_id;
                    list_array<property> properties;
                };

                struct update_mob_effect : public packet<0x7D> {
                    enum class flags_f : int8_t {
                        is_ambient = 0x1,
                        show_particles = 0x2,
                        show_icon = 0x4,
                        blend = 0x8,
                    };

                    var_int32 entity_id;
                    var_int32::mob_effect effect;
                    var_int32 amplifier;
                    var_int32 duration;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct update_recipes : public packet<0x7E> {

                    struct property {
                        identifier set_id;
                        list_array<var_int32::item> items;
                    };

                    struct stonecuter_recipe {
                        id_set<var_int32::item> ingredients;
                        slot_display item;
                    };

                    list_array<property> property_sets;
                    list_array<stonecuter_recipe> stonecuter_recipes;
                };

                struct update_tags : public packet<0x7F> {

                    struct tag {
                        identifier tag_name;
                        list_array<var_int32> values;
                    };

                    struct entry {
                        identifier registry_id;
                        list_array<tag> tags;
                    };

                    list_array<entry> entries;
                };

                struct projectile_power : public packet<0x80> {
                    var_int32 entity_id;
                    double power;
                };

                struct custom_report_details : public packet<0x81> {

                    struct detail {
                        string_sized<128> title;
                        string_sized<4096> description;
                    };

                    list_array_sized<detail, 32> details;
                };

                struct server_links : public packet<0x82> {
                    enum class link_type : uint8_t {
                        bug_report = 0,
                        community_guidelines = 1,
                        support = 2,
                        status = 3,
                        feedback = 4,
                        community = 5,
                        website = 6,
                        forums = 7,
                        news = 8,
                        announcements = 9,
                    };
                    using enum link_type;

                    struct link {
                        bool_or<enum_as<link_type, var_int32>, Chat> label;
                        std::string url;
                    };

                    list_array<link> links;
                };

                struct waypoint : public packet<0x83> {
                    enum class operation_e {
                        track = 0,
                        untrack = 1,
                        update = 2,
                    };

                    struct color_t {
                        uint8_t r;
                        uint8_t g;
                        uint8_t b;
                    };

                    struct here : public enum_item<0> {};

                    struct near : public enum_item<1> {
                        var_int32 x;
                        var_int32 y;
                        var_int32 z;
                    };

                    struct far : public enum_item<2> {
                        var_int32 x;
                        var_int32 z;
                    };

                    struct far_away : public enum_item<3> {
                        float azimuth;
                    };

                    bool_or<enbt::raw_uuid, std::string> id;
                    identifier icon_style; //assets path
                    std::optional<color_t> color = std::nullopt;
                    enum_switch<var_int32, here, near, far, far_away> type;
                };

                struct clear_dialog : public packet<0x84> {};

                struct show_dialog : public packet<0x85> {
                    or_<var_int32::dialog, enbt::value> dialog;
                };
            }

            decl_variant(
                play_packet,
                play::bundle_delimiter,
                play::add_entity,
                play::animate,
                play::award_stats,
                play::block_changed_ack,
                play::block_destruction,
                play::block_entity_data,
                play::block_event,
                play::block_update,
                play::boss_event,
                play::change_difficulty,
                play::chunk_batch_finished,
                play::chunk_batch_start,
                play::chunks_biomes,
                play::clear_titles,
                play::command_suggestions,
                play::commands,
                play::container_close,
                play::container_set_content,
                play::container_set_data,
                play::container_set_slot,
                play::cookie_request,
                play::cooldown,
                play::custom_chat_completions,
                play::custom_payload,
                play::damage_event,
                play::debug_sample,
                play::delete_chat,
                play::disconnect,
                play::disguised_chat,
                play::entity_event,
                play::entity_position_sync,
                play::explode,
                play::forget_level_chunk,
                play::game_event,
                play::horse_screen_open,
                play::hurt_animation,
                play::initialize_border,
                play::keep_alive,
                play::level_chunk_with_light,
                play::level_event,
                play::level_particles,
                play::light_update,
                play::login,
                play::map_item_data,
                play::merchant_offers,
                play::move_entity_pos,
                play::move_entity_pos_rot,
                play::move_minecart_along_track,
                play::move_entity_rot,
                play::move_vehicle,
                play::open_book,
                play::open_screen,
                play::open_sign_editor,
                play::ping,
                play::pong_response,
                play::place_ghost_recipe,
                play::player_abilities,
                play::player_chat,
                play::player_combat_end,
                play::player_combat_enter,
                play::player_combat_kill,
                play::player_info_remove,
                play::player_info_update,
                play::player_look_at,
                play::player_position,
                play::player_rotation,
                play::recipe_book_add,
                play::recipe_book_remove,
                play::recipe_book_settings,
                play::remove_entities,
                play::remove_mob_effect,
                play::reset_score,
                play::resource_pack_pop,
                play::resource_pack_push,
                play::respawn,
                play::rotate_head,
                play::section_blocks_update,
                play::select_advancements_tab,
                play::server_data,
                play::set_action_bar_text,
                play::set_border_center,
                play::set_border_lerp_size,
                play::set_border_size,
                play::set_border_warning_delay,
                play::set_border_warning_distance,
                play::set_camera,
                play::set_chunk_cache_center,
                play::set_chunk_cache_radius,
                play::set_cursor_item,
                play::set_default_spawn_position,
                play::set_display_objective,
                play::set_entity_data,
                play::set_entity_link,
                play::set_entity_motion,
                play::set_equipment,
                play::set_experience,
                play::set_health,
                play::set_held_slot,
                play::set_objective,
                play::set_passengers,
                play::set_player_inventory,
                play::set_player_team,
                play::set_score,
                play::set_simulation_distance,
                play::set_subtitle_text,
                play::set_time,
                play::set_title_text,
                play::set_titles_animation,
                play::sound_entity,
                play::sound,
                play::start_configuration,
                play::stop_sound,
                play::store_cookie,
                play::system_chat,
                play::tab_list,
                play::tag_query,
                play::take_item_entity,
                play::teleport_entity,
                play::test_instance_block_status,
                play::ticking_state,
                play::ticking_step,
                play::transfer,
                play::update_advancements,
                play::update_attributes,
                play::update_mob_effect,
                play::update_recipes,
                play::update_tags,
                play::projectile_power,
                play::custom_report_details,
                play::server_links,
                play::waypoint,
                play::clear_dialog,
                play::show_dialog
            );
        }

        decl_variant(
            client_bound_packet,
            client_bound::status_packet,
            client_bound::login_packet,
            client_bound::configuration_packet,
            client_bound::play_packet
        );

        namespace server_bound {
            namespace handshake {
                struct intention : public packet<0x00> {
                    enum class intent_e : uint8_t {
                        status = 1,
                        login = 2,
                        transfer = 3
                    };
                    var_int32 protocol_version;
                    string_sized<255> server_address;
                    uint16_t server_port;
                    enum_as<intent_e, var_int32> intent;
                };
            }

            decl_variant(handshake_packet, handshake::intention);

            namespace status {
                struct status_request : public packet<0x00> {};

                struct ping_response : public packet<0x01> {
                    uint64_t timestamp;
                };
            }

            decl_variant(status_packet, status::status_request, status::ping_response);

            namespace login {
                struct hello : public packet<0x00> {
                    string_sized<16> name;
                    enbt::raw_uuid uuid;
                };

                struct key : public packet<0x01> {
                    list_array<uint8_t> shared_secret;
                    list_array<uint8_t> verify_token;
                };

                struct custom_query_answer : public packet<0x02> {
                    var_int32 query_message_id;
                    list_array_sized_siz_from_packet<uint8_t, 32767> payload;
                };

                struct login_acknowledged : public packet<0x03>, base_objects::switches_to::configuration {};

                struct cookie_response : public packet<0x04> {
                    identifier key;
                    std::optional<list_array_sized<uint8_t, 5120>> payload = std::nullopt;
                };
            }

            decl_variant(
                login_packet,
                login::hello,
                login::key,
                login::custom_query_answer,
                login::login_acknowledged,
                login::cookie_response
            );

            namespace configuration {
                struct client_information : public packet<0x00> {
                    enum class chat_mode_e : uint8_t {
                        disabled = 0,
                        commands_only = 1,
                        hidden = 2,
                    };
                    enum class displayer_skin_parts_f : uint8_t {
                        cape = 0x1,
                        jacket = 0x2,
                        left_sleeve = 0x4,
                        right_sleeve = 0x8,
                        left_pants = 0x10,
                        right_pants = 0x20,
                        hat = 0x40,
                        _unused = 0x80
                    };
                    enum class main_hand_e : uint8_t {
                        left = 0,
                        right = 1
                    };
                    enum class particle_status_e : uint8_t {
                        all = 0,
                        decreased = 1,
                        minimal = 2,
                    };
                    string_sized<16> locale;
                    uint8_t view_distance;
                    enum_as<chat_mode_e, var_int32> chat_mode;
                    bool enable_chat_colors;
                    enum_as_flag<displayer_skin_parts_f, uint8_t> displayed_skin_parts;
                    enum_as<main_hand_e, var_int32> main_hand;
                    bool enable_text_filtering;
                    bool allow_server_listings;
                    enum_as<particle_status_e, var_int32> particle_status;
                };

                struct cookie_response : public packet<0x01> {
                    identifier key;
                    std::optional<list_array_sized<uint8_t, 5120>> payload = std::nullopt;
                };

                struct custom_payload : public packet<0x02> {
                    identifier channel;
                    list_array_sized_siz_from_packet<uint8_t, 32767> payload;
                };

                struct finish_configuration : public packet<0x03>, base_objects::switches_to::play {};

                struct keep_alive : public packet<0x04> {
                    uint64_t keep_alive_id;
                };

                struct pong : public packet<0x05> {
                    ordered_id<int32_t, "ping"> ping_request_id;
                };

                struct resource_pack : public packet<0x06> {
                    enum class result_e : uint8_t {
                        success = 0,
                        declined = 1,
                        download_failed = 2,
                        accepted = 3,
                        downloaded = 4,
                        invalid_url = 5,
                        reload_failed = 6,
                        discarded = 7
                    };
                    using enum result_e;
                    enbt::raw_uuid uuid;
                    enum_as<result_e, var_int32> result;
                };

                struct select_known_packs : public packet<0x07> {

                    struct pack {
                        std::string _namespace;
                        std::string id;
                        std::string version;
                    };

                    list_array<pack> packs;
                };

                struct custom_click_action : public packet<0x08> {
                    identifier id;
                    enbt::value payload;
                };
            }

            decl_variant(
                configuration_packet,
                configuration::client_information,
                configuration::cookie_response,
                configuration::custom_payload,
                configuration::finish_configuration,
                configuration::keep_alive,
                configuration::pong,
                configuration::resource_pack,
                configuration::select_known_packs,
                configuration::custom_click_action
            );

            namespace play {
                struct accept_teleportation : public packet<0x00> {
                    ordered_id<var_int32, "sync/player_position"> teleport_id;
                };

                struct block_entity_tag_query : public packet<0x01> {
                    var_int32 tag_query_id; //managed by client
                    position location;
                };

                struct bundle_item_selected : public packet<0x02> {
                    var_int32 bundle_slot;
                    var_int32 item_slot;
                };

                struct change_difficulty : public packet<0x03> {
                    enum_as<difficulty_e, uint8_t> difficulty;
                };

                struct change_gamemode : public packet<0x04> {
                    enum_as<gamemode_e, uint8_t> gamemode;
                };

                struct chat_ack : public packet<0x05> {
                    var_int32 count;
                };

                struct chat_command : public packet<0x06> {
                    string_sized<32767> command;
                };

                struct chat_command_signed : public packet<0x07> {

                    struct argument_signature {
                        string_sized<16> argument_name;
                        std::array<uint8_t, 256> signature;
                    };

                    string_sized<32767> command;
                    uint64_t timestamp;
                    uint64_t salt;
                    list_array_sized<argument_signature, 8> argument_signatures;
                    var_int32 message_count;
                    bitset_fixed<20> acknowledged;
                    uint8_t check_sum;
                };

                struct chat : public packet<0x08> {
                    string_sized<256> message;
                    uint64_t timestamp;
                    uint64_t salt;
                    std::optional<std::array<uint8_t, 256>> signature = std::nullopt;
                    var_int32 message_count;
                    bitset_fixed<20> acknowledged;
                    uint8_t check_sum;
                };

                struct chat_session_update : public packet<0x09> {
                    enbt::raw_uuid uuid;
                    uint64_t expiries_at;
                    list_array_sized<uint8_t, 512> public_key;
                    list_array_sized<uint8_t, 4096> key_signature;
                };

                struct chunk_batch_received : public packet<0x0A> {
                    float chunks_per_tick;
                };

                struct client_command : public packet<0x0B> {
                    enum class action_id_e : uint8_t {
                        perform_respawn = 0,
                        request_stats = 1,
                    };
                    using enum action_id_e;
                    enum_as<action_id_e, var_int32> action_id;
                };

                struct client_tick_end : public packet<0x0C> {};

                struct client_information : public packet<0x0D> {
                    enum class chat_mode_e : uint8_t {
                        disabled = 0,
                        commands_only = 1,
                        hidden = 2,
                    };
                    enum class displayer_skin_parts_f : uint8_t {
                        cape = 0x1,
                        jacket = 0x2,
                        left_sleeve = 0x4,
                        right_sleeve = 0x8,
                        left_pants = 0x10,
                        right_pants = 0x20,
                        hat = 0x40,
                        _unused = 0x80
                    };
                    enum class main_hand_e : uint8_t {
                        left = 0,
                        right = 1
                    };
                    enum class particle_status_e : uint8_t {
                        all = 0,
                        decreased = 1,
                        minimal = 2,
                    };
                    string_sized<16> locale;
                    uint8_t view_distance;
                    enum_as<chat_mode_e, var_int32> chat_mode;
                    bool enable_chat_colors;
                    enum_as_flag<displayer_skin_parts_f, uint8_t> displayer_skin_parts;
                    enum_as<main_hand_e, var_int32> main_hand;
                    bool enable_text_filtering;
                    bool allow_server_listings;
                    enum_as<particle_status_e, var_int32> particle_status;
                };

                struct command_suggestion : public packet<0x0E> {
                    var_int32 suggestion_transaction_id; //managed by client
                    string_sized<32500> command_text;
                };

                struct configuration_acknowledged : public packet<0x0F>, base_objects::switches_to::configuration {};

                struct container_button_click : public packet<0x10> {
                    var_int32 window_id;
                    var_int32 button_id;
                };

                struct container_click : public packet<0x11> {
                    struct hashed_slot_data {
                        var_int32::item item_id;
                        var_int32 count;

                        struct component {
                            var_int32::data_component_type type;
                            int32_t crc32_hash;
                        };

                        list_array<component> add_components;
                        list_array<var_int32::data_component_type> remove_components;
                    };

                    struct changed_slot {
                        short slot;
                        std::optional<hashed_slot_data> data = std::nullopt;
                    };

                    var_int32 window_id;
                    var_int32 state_id;
                    short slot;
                    int8_t button;
                    var_int32 mode;
                    list_array_sized<changed_slot, 128> changed;
                    std::optional<hashed_slot_data> carry_item = std::nullopt;
                };

                struct container_close : public packet<0x12> {
                    var_int32 window_id;
                };

                struct container_slot_state_changed : public packet<0x13> {
                    var_int32 slot_id;
                    var_int32 window_id;
                    bool state;
                };

                struct cookie_response : public packet<0x14> {
                    identifier key;
                    std::optional<list_array_sized<uint8_t, 5120>> payload = std::nullopt;
                };

                struct custom_payload : public packet<0x15> {
                    identifier channel;
                    list_array_sized_siz_from_packet<uint8_t, 32767> payload;
                };

                struct debug_sample_subscription : public packet<0x16> {
                    var_int32 sample_type;
                };

                struct edit_book : public packet<0x17> {
                    var_int32 slot;
                    list_array_sized<string_sized<1024>, 100> entries;
                    std::optional<string_sized<32>> title = std::nullopt;
                };

                struct entity_tag_query : public packet<0x18> {
                    var_int32 tag_query_id; //managed by client
                    var_int32 entity_id;
                };

                struct interact : public packet<0x19> {
                    var_int32 entity_id;
                    enum class hand_e : uint8_t {
                        main = 0,
                        off = 1
                    };

                    struct interact_ : public enum_item<0> {
                        enum_as<hand_e, var_int32> hand;
                    };

                    struct attack : public enum_item<1> {};

                    struct interact_at : public enum_item<2> {
                        float x;
                        float y;
                        float z;
                        enum_as<hand_e, var_int32> hand;
                    };

                    enum_switch<var_int32, interact_, attack, interact_at> type;
                    bool sneak_key_pressed;
                };

                struct jigsaw_generate : public packet<0x1A> {
                    position location;
                    var_int32 levels;
                    bool keep_jigsaws;
                };

                struct keep_alive : public packet<0x1B> {
                    uint64_t id;
                };

                struct lock_difficulty : public packet<0x1C> {
                    bool is_locked;
                };

                struct move_player_pos : public packet<0x1D> {
                    enum class flags_f : uint8_t {
                        on_ground = 1,
                        push_against_wall = 2
                    };
                    using enum flags_f;

                    double x;
                    double y;
                    double z;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct move_player_pos_rot : public packet<0x1E> {
                    enum class flags_f : uint8_t {
                        on_ground = 1,
                        push_against_wall = 2
                    };
                    using enum flags_f;

                    double x;
                    double y;
                    double z;
                    float yaw;
                    float pitch;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct move_player_rot : public packet<0x1F> {
                    enum class flags_f : uint8_t {
                        on_ground = 1,
                        push_against_wall = 2
                    };
                    using enum flags_f;

                    float yaw;
                    float pitch;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct move_player_status_only : public packet<0x20> {
                    enum class flags_f : uint8_t {
                        on_ground = 1,
                        push_against_wall = 2
                    };
                    using enum flags_f;

                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct move_vehicle : public packet<0x21> {
                    double x;
                    double y;
                    double z;
                    float yaw;
                    float pitch;
                    bool on_ground;
                };

                struct paddle_boat : public packet<0x22> {
                    bool left_paddle_turning;
                    bool right_paddle_turning;
                };

                struct pick_item_from_block : public packet<0x23> {
                    position location;
                    bool include_data;
                };

                struct pick_item_from_entity : public packet<0x24> {
                    var_int32 entity_id;
                    bool include_data;
                };

                struct ping_request : public packet<0x25> {
                    uint64_t payload;
                };

                struct place_recipe : public packet<0x26> {
                    var_int32 windows_id;
                    var_int32::recipe recipe_id;
                    bool make_all;
                };

                struct player_abilities : public packet<0x27> {
                    enum class flags_f : uint8_t {
                        flying = 2
                    };
                    using enum flags_f;

                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct player_action : public packet<0x28> {
                    enum class status_e : uint8_t {
                        digging_start = 0,
                        digging_canceled = 1,
                        digging_finished = 2,
                        drop_item_stack = 3,
                        drop_item = 4,
                        right_click_item = 5,
                        swap_item_in_hand = 6,
                    };
                    enum class face_e : uint8_t {
                        bottom = 0,
                        top = 1,
                        north = 2,
                        south = 3,
                        west = 4,
                        east = 5,
                    };
                    enum_as<status_e, var_int32> status;
                    position location;
                    enum_as<face_e, int8_t> face;
                    var_int32 block_sequence_id;
                };

                struct player_command : public packet<0x29> {
                    enum class action_e : uint8_t {
                        leave_bed = 0,
                        start_sprinting = 1,
                        stop_sprinting = 2,
                        horse_jump_start = 3,
                        horse_jump_stop = 4,
                        inventory_vehicle_open = 5,
                        elytra_fly = 6,
                    };
                    var_int32 entity_id;
                    enum_as<action_e, var_int32> action;
                    var_int32 jump_boost;
                };

                struct player_input : public packet<0x2A> {
                    enum class status_f : uint8_t {
                        forward = 1,
                        backward = 2,
                        left = 4,
                        right = 8,
                        jump = 16,
                        sneak = 32,
                        sprint = 64,
                    };
                    using enum status_f;
                    enum_as_flag<status_f, uint8_t> face;
                };

                struct player_loaded : public packet<0x2B> {};

                struct pong : public packet<0x2C> {
                    int32_t id;
                };

                struct recipe_book_change_settings : public packet<0x2D> {
                    enum class book_type_e : uint8_t {
                        crafting = 0,
                        furnace = 1,
                        blast_furnace = 2,
                        smoker = 3,
                    };
                    using enum book_type_e;

                    enum_as<book_type_e, var_int32> book_type;
                    bool book_open;
                    bool filter_active;
                };

                struct recipe_book_seen_recipe : public packet<0x2E> {
                    var_int32::recipe recipe_id;
                };

                struct rename_item : public packet<0x2F> {
                    string_sized<32767> new_name;
                };

                struct resource_pack : public packet<0x30> {
                    enum class result_e : uint8_t {
                        success = 0,
                        declined = 1,
                        download_failed = 2,
                        accepted = 3,
                        downloaded = 4,
                        invalid_url = 5,
                        reload_failed = 6,
                        discarded = 7
                    };
                    using enum result_e;
                    enbt::raw_uuid uuid;
                    enum_as<result_e, var_int32> result;
                };

                struct seen_advancements : public packet<0x31> {

                    struct opened_tab : public enum_item<0> {
                        identifier tab_id;
                    };

                    struct closed_screen : public enum_item<1> {};

                    enum_switch<var_int32, opened_tab, closed_screen> action;
                };

                struct select_trade : public packet<0x32> {
                    var_int32 selected_slot;
                };

                struct set_beacon : public packet<0x33> {
                    std::optional<var_int32::mob_effect> primary_effect = std::nullopt;
                    std::optional<var_int32::mob_effect> secondary_effect = std::nullopt;
                };

                struct set_carried_item : public packet<0x34> {
                    short slot;
                };

                struct set_command_block : public packet<0x35> {
                    enum class mode_e : uint8_t {
                        chain = 0,
                        repeating = 1,
                        impulse = 2,
                    };

                    enum class flags_f : uint8_t {
                        track_output = 1,
                        is_conditional = 2,
                        automatic = 4,
                    };
                    using enum flags_f;

                    position location;
                    string_sized<32767> command;
                    enum_as<mode_e, var_int32> mode;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct set_command_minecart : public packet<0x36> {
                    var_int32 entity_id;
                    string_sized<32767> command;
                    bool track_output;
                };

                struct set_creative_mode_slot : public packet<0x37> {
                    short slot;
                    struct slot item;
                };

                struct set_jigsaw_block : public packet<0x38> {
                    position location;
                    identifier name;
                    identifier target;
                    identifier pool;
                    string_sized<32767> final_state;
                    string_sized<32767> joint_state;
                    var_int32 selection_priority;
                    var_int32 placement_priority;
                };

                struct set_structure_block : public packet<0x39> {
                    enum class mirror_side_e : uint8_t {
                        none = 0,
                        left_right = 1,
                        front_back = 2,
                    };
                    enum class rotation_e : uint8_t {
                        none = 0,
                        clockwise_90 = 1,
                        clockwise_180 = 2,
                        counterclockwise_90 = 3,
                    };

                    struct ignore_entities : public flag_item<1, 1, 1> {};

                    struct show_air : public flag_item<2, 2, 2> {};

                    struct show_bounding_block : public flag_item<4, 4, 3> {};

                    struct strict_placement : public flag_item<8, 8, 4> {};

                    position location;
                    var_int32 action;
                    var_int32 mode;
                    string_sized<32767> name;
                    limited_num<int8_t, -48, 48> offset_x;
                    limited_num<int8_t, -48, 48> offset_y;
                    limited_num<int8_t, -48, 48> offset_z;
                    limited_num<int8_t, 0, 48> size_x;
                    limited_num<int8_t, 0, 48> size_y;
                    limited_num<int8_t, 0, 48> size_z;
                    enum_as<mirror_side_e, var_int32> mirror_side;
                    enum_as<rotation_e, var_int32> rotation;
                    string_sized<128> metadata;
                    limited_num<float, 0.0f, 1.0f> integrity;
                    var_int64 seed;
                    flags_list<
                        int8_t,
                        ignore_entities,
                        show_air,
                        show_bounding_block,
                        strict_placement>
                        flags;
                };

                struct set_test_block : public packet<0x3A> {
                    enum class mode_e : uint8_t {
                        start = 0,
                        log = 1,
                        fail = 2,
                        accept = 3,
                    };
                    position location;
                    enum_as<mode_e, var_int32> mode;
                    std::string message;
                };

                struct sign_update : public packet<0x3B> {
                    position location;
                    bool is_front_text;
                    std::array<string_sized<384>, 4> lines;
                };

                struct swing : public packet<0x3C> {
                    enum class hand_e : uint8_t {
                        main = 0,
                        off = 1,
                    };
                    enum_as<hand_e, var_int32> hand;
                };

                struct teleport_to_entity : public packet<0x3D> {
                    enbt::raw_uuid uuid;
                };

                struct test_instance_block_action : public packet<0x3E> {
                    enum class action_e : uint8_t {
                        init = 0,
                        query = 1,
                        set = 2,
                        reset = 3,
                        save = 4,
                        export_ = 5,
                        run = 6,
                    };
                    enum class rotation_e : uint8_t {
                        none = 0,
                        clockwise_90 = 1,
                        clockwise_180 = 2,
                        counterclockwise_90 = 3,
                    };
                    enum class status_e : uint8_t {
                        cleared = 0,
                        running = 1,
                        finished = 2,
                    };
                    position location;
                    enum_as<action_e, var_int32> action;
                    std::optional<var_int32::test_instance> test_id = std::nullopt;
                    var_int32 size_x;
                    var_int32 size_y;
                    var_int32 size_z;
                    enum_as<rotation_e, var_int32> rotation;
                    bool ignore_entities;
                    enum_as<status_e, var_int32> status;
                    std::optional<Chat> error_message = std::nullopt;
                };

                struct use_item_on : public packet<0x3F> {
                    enum class hand_e : uint8_t {
                        main = 0,
                        off = 1,
                    };
                    enum_as<hand_e, var_int32> hand;
                    position location;
                    var_int32 face;
                    limited_num<float, 0.0f, 1.0f> cursor_x;
                    limited_num<float, 0.0f, 1.0f> cursor_y;
                    limited_num<float, 0.0f, 1.0f> cursor_z;
                    bool inside_block;
                    bool world_border_hit;
                    var_int32 block_sequence_id;
                };

                struct use_item : public packet<0x40> {
                    enum class hand_e : uint8_t {
                        main = 0,
                        off = 1,
                    };
                    enum_as<hand_e, var_int32> hand;
                    var_int32 block_sequence_id;
                    float yaw;
                    float pitch;
                };

                struct custom_click_action : public packet<0x41> {
                    identifier id;
                    enbt::value payload;
                };
            }

            decl_variant(play_packet, play::accept_teleportation, play::block_entity_tag_query, play::bundle_item_selected, play::change_difficulty, play::change_gamemode, play::chat_ack, play::chat_command, play::chat_command_signed, play::chat, play::chat_session_update, play::chunk_batch_received, play::client_command, play::client_tick_end, play::client_information, play::command_suggestion, play::configuration_acknowledged, play::container_button_click, play::container_click, play::container_close, play::container_slot_state_changed, play::cookie_response, play::custom_payload, play::debug_sample_subscription, play::edit_book, play::entity_tag_query, play::interact, play::jigsaw_generate, play::keep_alive, play::lock_difficulty, play::move_player_pos, play::move_player_pos_rot, play::move_player_rot, play::move_player_status_only, play::move_vehicle, play::paddle_boat, play::pick_item_from_block, play::pick_item_from_entity, play::ping_request, play::place_recipe, play::player_abilities, play::player_action, play::player_command, play::player_input, play::player_loaded, play::pong, play::recipe_book_change_settings, play::recipe_book_seen_recipe, play::rename_item, play::resource_pack, play::seen_advancements, play::select_trade, play::set_beacon, play::set_carried_item, play::set_command_block, play::set_command_minecart, play::set_creative_mode_slot, play::set_jigsaw_block, play::set_structure_block, play::set_test_block, play::sign_update, play::swing, play::teleport_to_entity, play::test_instance_block_action, play::use_item_on, play::use_item, play::custom_click_action);
        }

        decl_variant(
            server_bound_packet,
            server_bound::handshake_packet,
            server_bound::status_packet,
            server_bound::login_packet,
            server_bound::configuration_packet,
            server_bound::play_packet
        );


        bool send(base_objects::SharedClientData& client, client_bound_packet&&);
        base_objects::network::response internal_encode(base_objects::SharedClientData& client, client_bound_packet&&);
        base_objects::network::response internal_encode(base_objects::SharedClientData& client, server_bound_packet&&);
        base_objects::network::response encode(client_bound_packet&& packet);
        base_objects::network::response encode(server_bound_packet&& packet);

        bool decode(base_objects::SharedClientData& context, ArrayStream&);
        bool make_process(base_objects::SharedClientData& context, server_bound_packet&&);

        client_bound::status_packet decode_client_status(ArrayStream&);
        client_bound::login_packet decode_client_login(ArrayStream&);
        client_bound::configuration_packet decode_client_configuration(ArrayStream&);
        client_bound::play_packet decode_client_play(ArrayStream&);

        server_bound::handshake_packet decode_server_handshake(ArrayStream&);
        server_bound::status_packet decode_server_status(ArrayStream&);
        server_bound::login_packet decode_server_login(ArrayStream&);
        server_bound::configuration_packet decode_server_configuration(ArrayStream&);
        server_bound::play_packet decode_server_play(ArrayStream&);


        client_bound::status_packet decode_client_status(base_objects::SharedClientData& context, ArrayStream&);
        client_bound::login_packet decode_client_login(base_objects::SharedClientData& context, ArrayStream&);
        client_bound::configuration_packet decode_client_configuration(base_objects::SharedClientData& context, ArrayStream&);
        client_bound::play_packet decode_client_play(base_objects::SharedClientData& context, ArrayStream&);

        server_bound::handshake_packet decode_server_handshake(base_objects::SharedClientData& context, ArrayStream&);
        server_bound::status_packet decode_server_status(base_objects::SharedClientData& context, ArrayStream&);
        server_bound::login_packet decode_server_login(base_objects::SharedClientData& context, ArrayStream&);
        server_bound::configuration_packet decode_server_configuration(base_objects::SharedClientData& context, ArrayStream&);
        server_bound::play_packet decode_server_play(base_objects::SharedClientData& context, ArrayStream&);


        std::string stringize_packet(const client_bound::status_packet&);
        std::string stringize_packet(const client_bound::login_packet&);
        std::string stringize_packet(const client_bound::configuration_packet&);
        std::string stringize_packet(const client_bound::play_packet&);
        std::string stringize_packet(const server_bound::handshake_packet&);
        std::string stringize_packet(const server_bound::status_packet&);
        std::string stringize_packet(const server_bound::login_packet&);
        std::string stringize_packet(const server_bound::configuration_packet&);
        std::string stringize_packet(const server_bound::play_packet&);
        std::string stringize_packet(const client_bound_packet&);
        std::string stringize_packet(const server_bound_packet&);

        namespace __internal {
            base_objects::events::sync_event<client_bound::status_packet&, base_objects::SharedClientData&>& client_viewer_s(size_t id);
            base_objects::events::sync_event<client_bound::login_packet&, base_objects::SharedClientData&>& client_viewer_l(size_t id);
            base_objects::events::sync_event<client_bound::configuration_packet&, base_objects::SharedClientData&>& client_viewer_c(size_t id);
            base_objects::events::sync_event<client_bound::play_packet&, base_objects::SharedClientData&>& client_viewer_p(size_t id);

            base_objects::events::sync_event<client_bound::status_packet&, base_objects::SharedClientData&>& client_viewer_mode_s();
            base_objects::events::sync_event<client_bound::login_packet&, base_objects::SharedClientData&>& client_viewer_mode_l();
            base_objects::events::sync_event<client_bound::configuration_packet&, base_objects::SharedClientData&>& client_viewer_mode_c();
            base_objects::events::sync_event<client_bound::play_packet&, base_objects::SharedClientData&>& client_viewer_mode_p();
            base_objects::events::sync_event<client_bound_packet&, base_objects::SharedClientData&>& client_viewer_mode();

            base_objects::events::sync_event<server_bound::handshake_packet&, base_objects::SharedClientData&>& server_viewer_h(size_t id);
            base_objects::events::sync_event<server_bound::status_packet&, base_objects::SharedClientData&>& server_viewer_s(size_t id);
            base_objects::events::sync_event<server_bound::login_packet&, base_objects::SharedClientData&>& server_viewer_l(size_t id);
            base_objects::events::sync_event<server_bound::configuration_packet&, base_objects::SharedClientData&>& server_viewer_c(size_t id);
            base_objects::events::sync_event<server_bound::play_packet&, base_objects::SharedClientData&>& server_viewer_p(size_t id);

            base_objects::events::sync_event<server_bound::handshake_packet&, base_objects::SharedClientData&>& server_viewer_mode_h();
            base_objects::events::sync_event<server_bound::status_packet&, base_objects::SharedClientData&>& server_viewer_mode_s();
            base_objects::events::sync_event<server_bound::login_packet&, base_objects::SharedClientData&>& server_viewer_mode_l();
            base_objects::events::sync_event<server_bound::configuration_packet&, base_objects::SharedClientData&>& server_viewer_mode_c();
            base_objects::events::sync_event<server_bound::play_packet&, base_objects::SharedClientData&>& server_viewer_mode_p();
            base_objects::events::sync_event<server_bound_packet&, base_objects::SharedClientData&>& server_viewer_mode();

            base_objects::events::sync_event_no_cancel<client_bound::status_packet&, base_objects::SharedClientData&>& client_post_send_viewer_s(size_t id);
            base_objects::events::sync_event_no_cancel<client_bound::login_packet&, base_objects::SharedClientData&>& client_post_send_viewer_l(size_t id);
            base_objects::events::sync_event_no_cancel<client_bound::configuration_packet&, base_objects::SharedClientData&>& client_post_send_viewer_c(size_t id);
            base_objects::events::sync_event_no_cancel<client_bound::play_packet&, base_objects::SharedClientData&>& client_post_send_viewer_p(size_t id);

            base_objects::events::sync_event_no_cancel<client_bound::status_packet&, base_objects::SharedClientData&>& client_post_send_viewer_mode_s();
            base_objects::events::sync_event_no_cancel<client_bound::login_packet&, base_objects::SharedClientData&>& client_post_send_viewer_mode_l();
            base_objects::events::sync_event_no_cancel<client_bound::configuration_packet&, base_objects::SharedClientData&>& client_post_send_viewer_mode_c();
            base_objects::events::sync_event_no_cancel<client_bound::play_packet&, base_objects::SharedClientData&>& client_post_send_viewer_mode_p();
            base_objects::events::sync_event_no_cancel<client_bound_packet&, base_objects::SharedClientData&>& client_post_send_viewer_mode();

            base_objects::events::sync_event_single<server_bound::handshake_packet&&, base_objects::SharedClientData&>& server_processor_h(size_t id);
            base_objects::events::sync_event_single<server_bound::status_packet&&, base_objects::SharedClientData&>& server_processor_s(size_t id);
            base_objects::events::sync_event_single<server_bound::login_packet&&, base_objects::SharedClientData&>& server_processor_l(size_t id);
            base_objects::events::sync_event_single<server_bound::configuration_packet&&, base_objects::SharedClientData&>& server_processor_c(size_t id);
            base_objects::events::sync_event_single<server_bound::play_packet&&, base_objects::SharedClientData&>& server_processor_p(size_t id);
        }

        namespace events {
            template <class Packet>
                requires(
                    std::is_constructible_v<server_bound_packet, Packet>
                    || std::is_constructible_v<client_bound_packet, Packet>
                    || std::is_constructible_v<server_bound::handshake_packet, Packet>
                    || std::is_constructible_v<server_bound::status_packet, Packet>
                    || std::is_constructible_v<server_bound::login_packet, Packet>
                    || std::is_constructible_v<server_bound::configuration_packet, Packet>
                    || std::is_constructible_v<server_bound::play_packet, Packet>
                    || std::is_constructible_v<client_bound::status_packet, Packet>
                    || std::is_constructible_v<client_bound::login_packet, Packet>
                    || std::is_constructible_v<client_bound::configuration_packet, Packet>
                    || std::is_constructible_v<client_bound::play_packet, Packet>
                )
            auto& viewer() {
                if constexpr (std::is_same_v<server_bound_packet, Packet>) {
                    return __internal::server_viewer_mode();
                } else if constexpr (std::is_same_v<server_bound::handshake_packet, Packet>) {
                    return __internal::server_viewer_mode_h();
                } else if constexpr (std::is_same_v<server_bound::status_packet, Packet>) {
                    return __internal::server_viewer_mode_s();
                } else if constexpr (std::is_same_v<server_bound::login_packet, Packet>) {
                    return __internal::server_viewer_mode_l();
                } else if constexpr (std::is_same_v<server_bound::configuration_packet, Packet>) {
                    return __internal::server_viewer_mode_c();
                } else if constexpr (std::is_same_v<server_bound::play_packet, Packet>) {
                    return __internal::server_viewer_mode_p();
                } else if constexpr (std::is_constructible_v<server_bound::handshake_packet, Packet>) {
                    return __internal::server_viewer_h(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<server_bound::status_packet, Packet>) {
                    return __internal::server_viewer_s(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<server_bound::login_packet, Packet>) {
                    return __internal::server_viewer_l(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, Packet>) {
                    return __internal::server_viewer_c(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<server_bound::play_packet, Packet>) {
                    return __internal::server_viewer_p(Packet::packet_id::value);
                } else if constexpr (std::is_same_v<client_bound_packet, Packet>) {
                    return __internal::client_viewer_mode();
                } else if constexpr (std::is_same_v<client_bound::status_packet, Packet>) {
                    return __internal::client_viewer_mode_s();
                } else if constexpr (std::is_same_v<client_bound::login_packet, Packet>) {
                    return __internal::client_viewer_mode_l();
                } else if constexpr (std::is_same_v<client_bound::configuration_packet, Packet>) {
                    return __internal::client_viewer_mode_c();
                } else if constexpr (std::is_same_v<client_bound::play_packet, Packet>) {
                    return __internal::client_viewer_mode_p();
                } else if constexpr (std::is_constructible_v<client_bound::status_packet, Packet>) {
                    return __internal::client_viewer_s(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<client_bound::login_packet, Packet>) {
                    return __internal::client_viewer_l(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<client_bound::configuration_packet, Packet>) {
                    return __internal::client_viewer_c(Packet::packet_id::value);
                } else
                    return __internal::client_viewer_p(Packet::packet_id::value);
            }

            template <class Packet>
                requires(
                    std::is_same_v<client_bound_packet, Packet>
                    || std::is_constructible_v<client_bound::status_packet, Packet>
                    || std::is_constructible_v<client_bound::login_packet, Packet>
                    || std::is_constructible_v<client_bound::configuration_packet, Packet>
                    || std::is_constructible_v<client_bound::play_packet, Packet>
                )
            auto& viewer_post_send() {
                if constexpr (std::is_same_v<client_bound_packet, Packet>) {
                    return __internal::client_post_send_viewer_mode();
                } else if constexpr (std::is_same_v<client_bound::status_packet, Packet>) {
                    return __internal::client_post_send_viewer_mode_s();
                } else if constexpr (std::is_same_v<client_bound::login_packet, Packet>) {
                    return __internal::client_post_send_viewer_mode_l();
                } else if constexpr (std::is_same_v<client_bound::configuration_packet, Packet>) {
                    return __internal::client_post_send_viewer_mode_c();
                } else if constexpr (std::is_same_v<client_bound::play_packet, Packet>) {
                    return __internal::client_post_send_viewer_mode_p();
                } else if constexpr (std::is_constructible_v<client_bound::status_packet, Packet>) {
                    return __internal::client_post_send_viewer_s(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<client_bound::login_packet, Packet>) {
                    return __internal::client_post_send_viewer_l(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<client_bound::configuration_packet, Packet>) {
                    return __internal::client_post_send_viewer_c(Packet::packet_id::value);
                } else
                    return __internal::client_post_send_viewer_p(Packet::packet_id::value);
            }

            template <class Packet>
            auto& processor() {
                if constexpr (std::is_constructible_v<server_bound::handshake_packet, Packet>) {
                    return __internal::server_processor_h(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<server_bound::status_packet, Packet>) {
                    return __internal::server_processor_s(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<server_bound::login_packet, Packet>) {
                    return __internal::server_processor_l(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, Packet>) {
                    return __internal::server_processor_c(Packet::packet_id::value);
                } else if constexpr (std::is_constructible_v<server_bound::play_packet, Packet>) {
                    return __internal::server_processor_p(Packet::packet_id::value);
                }
            }

            extern base_objects::events::sync_event<base_objects::SharedClientData&> client_state_changed;
        }
    }
}

inline copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::api::packets::client_bound_packet&& packet) {
    copper_server::api::packets::send(client, std::move(packet));
    return client;
}

inline copper_server::api::packets::teleport_flags::flags_f operator|(copper_server::api::packets::teleport_flags::flags_f a, copper_server::api::packets::teleport_flags::flags_f b) {
    return copper_server::api::packets::teleport_flags::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::base_objects::switches_to::status);
copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::base_objects::switches_to::login);
copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::base_objects::switches_to::configuration);
copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::base_objects::switches_to::play);

inline copper_server::api::packets::client_bound::play::player_abilities::flags_f operator|(copper_server::api::packets::client_bound::play::player_abilities::flags_f a, copper_server::api::packets::client_bound::play::player_abilities::flags_f b) {
    return copper_server::api::packets::client_bound::play::player_abilities::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::client_bound::play::recipe_book_add::recipe::flags_f operator|(copper_server::api::packets::client_bound::play::recipe_book_add::recipe::flags_f a, copper_server::api::packets::client_bound::play::recipe_book_add::recipe::flags_f b) {
    return copper_server::api::packets::client_bound::play::recipe_book_add::recipe::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::client_bound::play::respawn::flags_f operator|(copper_server::api::packets::client_bound::play::respawn::flags_f a, copper_server::api::packets::client_bound::play::respawn::flags_f b) {
    return copper_server::api::packets::client_bound::play::respawn::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::client_bound::play::set_player_team::friendly_f operator|(copper_server::api::packets::client_bound::play::set_player_team::friendly_f a, copper_server::api::packets::client_bound::play::set_player_team::friendly_f b) {
    return copper_server::api::packets::client_bound::play::set_player_team::friendly_f(static_cast<int8_t>(a) | static_cast<int8_t>(b));
}

inline copper_server::api::packets::client_bound::play::update_mob_effect::flags_f operator|(copper_server::api::packets::client_bound::play::update_mob_effect::flags_f a, copper_server::api::packets::client_bound::play::update_mob_effect::flags_f b) {
    return copper_server::api::packets::client_bound::play::update_mob_effect::flags_f(static_cast<int8_t>(a) | static_cast<int8_t>(b));
}

inline copper_server::api::packets::server_bound::play::client_information::displayer_skin_parts_f operator|(copper_server::api::packets::server_bound::play::client_information::displayer_skin_parts_f a, copper_server::api::packets::server_bound::play::client_information::displayer_skin_parts_f b) {
    return copper_server::api::packets::server_bound::play::client_information::displayer_skin_parts_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::move_player_pos::flags_f operator|(copper_server::api::packets::server_bound::play::move_player_pos::flags_f a, copper_server::api::packets::server_bound::play::move_player_pos::flags_f b) {
    return copper_server::api::packets::server_bound::play::move_player_pos::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::move_player_pos_rot::flags_f operator|(copper_server::api::packets::server_bound::play::move_player_pos_rot::flags_f a, copper_server::api::packets::server_bound::play::move_player_pos_rot::flags_f b) {
    return copper_server::api::packets::server_bound::play::move_player_pos_rot::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::move_player_rot::flags_f operator|(copper_server::api::packets::server_bound::play::move_player_rot::flags_f a, copper_server::api::packets::server_bound::play::move_player_rot::flags_f b) {
    return copper_server::api::packets::server_bound::play::move_player_rot::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::move_player_status_only::flags_f operator|(copper_server::api::packets::server_bound::play::move_player_status_only::flags_f a, copper_server::api::packets::server_bound::play::move_player_status_only::flags_f b) {
    return copper_server::api::packets::server_bound::play::move_player_status_only::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::player_input::status_f operator|(copper_server::api::packets::server_bound::play::player_input::status_f a, copper_server::api::packets::server_bound::play::player_input::status_f b) {
    return copper_server::api::packets::server_bound::play::player_input::status_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::set_command_block::flags_f operator|(copper_server::api::packets::server_bound::play::set_command_block::flags_f a, copper_server::api::packets::server_bound::play::set_command_block::flags_f b) {
    return copper_server::api::packets::server_bound::play::set_command_block::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

#undef decl_variant
#undef STRUCT__
#endif /* SRC_API_PACKETS */
