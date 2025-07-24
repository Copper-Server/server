#ifndef SRC_API_NEW_PACKETS
#define SRC_API_NEW_PACKETS
#include <array>
#include <library/enbt/enbt.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/packets_help.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/slot.hpp>

#define decl_variant(name, ...)                      \
    struct name : public std::variant<__VA_ARGS__> { \
        using std::variant<__VA_ARGS__>::variant;    \
        using base = std::variant<__VA_ARGS__>;      \
    }

namespace copper_server {
    struct ArrayStream;

    namespace base_objects {
        struct SharedClientData;
    }

    namespace api::new_packets {
        using base_objects::Angle;
        using base_objects::any_of;
        using base_objects::bitset_fixed;
        using base_objects::compound_packet;
        using base_objects::enum_as;
        using base_objects::enum_as_flag;
        using base_objects::enum_item;
        using base_objects::enum_switch;
        using base_objects::flags_item;
        using base_objects::flags_list;
        using base_objects::flags_list_from;
        using base_objects::for_each_type;
        using base_objects::identifier;
        using base_objects::json_text_component;
        using base_objects::limited_num;
        using base_objects::no_size;
        using base_objects::optional_var_int32;
        using base_objects::optional_var_int64;
        using base_objects::or_;
        using base_objects::ordered_id;
        using base_objects::packet;
        using base_objects::packet_preprocess;
        using base_objects::position;
        using base_objects::size_from_packet;
        using base_objects::string_sized;
        using base_objects::unordered_id;
        using base_objects::value_optional;
        using base_objects::var_int32;
        using base_objects::var_int64;
        using base_objects::vector_no_size;
        using base_objects::vector_siz_from_packet;
        using base_objects::vector_sized;
        using base_objects::vector_sized_no_size;
        using base_objects::vector_sized_siz_from_packet;

        struct chat_type {
            struct decoration {
                enum class param_e {
                    sender = 0,
                    target = 1,
                    content = 2
                };
                std::string translation_key;
                std::vector<base_objects::enum_as<param_e, base_objects::var_int32>> parameters;
                std::optional<Chat> style;
            };

            decoration chat;
            decoration narration;
        };

        struct sound_event {
            base_objects::identifier sound_id;
            std::optional<float> fixed_range;
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

        namespace client_bound {
            namespace login {
                struct login_disconnect : public packet<0x00> {
                    json_text_component reason;
                };

                struct hello : public packet<0x01>, packet_preprocess {
                    string_sized<20> server_id;
                    std::vector<uint8_t> public_key;
                    std::vector<uint8_t> verify_token;
                    bool should_authenticate;
                };

                struct login_finished : public packet<0x02>, packet_preprocess {
                    struct property : public packet_preprocess {
                        string_sized<64> name;
                        string_sized<32767> value;
                        std::optional<string_sized<1024>> signature;
                    };

                    enbt::raw_uuid uuid;
                    string_sized<16> user_name;
                    vector_sized<property, 16> properties;
                };

                struct login_compression : public packet<0x03> {
                    var_int32 threshold;
                };

                struct custom_query : public packet<0x04>, packet_preprocess {
                    ordered_id<var_int32> message_id;
                    identifier channel;
                    vector_sized_no_size<uint8_t, 1048576> user_name;
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

                struct custom_payload : public packet<0x01>, packet_preprocess {
                    identifier channel;
                    vector_sized_no_size<uint8_t, 1048576> user_name;
                };

                struct disconnect : public packet<0x02> {
                    Chat reason;
                };

                struct finish_configuration : public packet<0x03> {};

                struct keep_alive : public packet<0x04> {
                    uint64_t keep_alive_id;
                };

                struct ping : public packet<0x05> {
                    unordered_id<int32_t> ping_request_id;
                };

                struct reset_chat : public packet<0x06> {};

                struct registry_data : public packet<0x07> {
                    struct entry {
                        identifier entry_id;
                        std::optional<enbt::value> data;
                    };

                    identifier registry_id;
                    std::vector<entry> entries;
                };

                struct resource_pack_pop : public packet<0x08> {
                    std::optional<enbt::raw_uuid> uuid;
                };

                struct resource_pack_push : public packet<0x09>, packet_preprocess {
                    std::optional<enbt::raw_uuid> uuid;
                    string_sized<32767> url;
                    string_sized<40> hash;
                    bool forced;
                    std::optional<Chat> prompt_message;
                };

                struct store_cookie : public packet<0x0A>, packet_preprocess {
                    identifier key;
                    vector_sized<uint8_t, 5120> payload;
                };

                struct transfer : public packet<0x0B>, packet_preprocess {
                    string_sized<32767> host;
                    var_int32 port;
                };

                struct update_enabled_features : public packet<0x0C> {
                    std::vector<identifier> features;
                };

                struct update_tags : public packet<0x0D> {
                    struct tag {
                        identifier tag_name;
                        std::vector<var_int32> values;
                    };

                    identifier registry_id;
                    std::vector<tag> tags;
                };

                struct select_known_packs : public packet<0x0E>, packet_preprocess {
                    struct pack : public packet_preprocess {
                        string_sized<32767> pack_namespace;
                        string_sized<32767> id;
                        string_sized<32767> version;
                    };

                    std::vector<pack> packs;
                };

                struct custom_report_details : public packet<0x0F>, packet_preprocess {
                    struct detail : public packet_preprocess {
                        string_sized<128> title;
                        string_sized<4096> description;
                    };

                    vector_sized<detail, 32> details;
                };

                struct server_links : public packet<0x10> {
                    enum class link_type : int32_t {
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

                    struct link {
                        or_<enum_as<link_type, var_int32>, Chat> label;
                        std::string url;
                    };

                    std::vector<link> links;
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
                    packet<0x00> begin;
                    std::vector<play_packet> packets;
                    packet<0x00> end;
                };

                struct add_entity : public packet<0x01> {
                    var_int32 entity_id;
                    enbt::raw_uuid uuid;
                    var_int32 type;
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
                    enum animation_e : uint8_t {
                        swing_main_arm = 0,
                        unrecognized = 1,
                        leave_bed = 2,
                        swing_offhand = 3,
                        critical_hit = 4,
                        enchanted_hit = 5,
                    };

                    var_int32 entity_id;
                    enum_as<animation_e, uint8_t> animation;
                };

                struct award_stats : public packet<0x03> {
                    struct statistic {
                        var_int32 category_id;
                        var_int32 statistic_id;
                        var_int32 value;
                    };

                    std::vector<statistic> statistics;
                };

                struct block_changed_ack : public packet<0x04> {
                    unordered_id<var_int32> sequence_id;
                };

                struct block_destruction : public packet<0x05> {
                    var_int32 entity_id;
                    position location;
                    uint8_t destroy_stage;
                };

                struct block_entity_data : public packet<0x06> {
                    position location;
                    var_int32 type;
                    enbt::value data;
                };

                struct block_event : public packet<0x07> {
                    position location;
                    uint8_t action_id;
                    uint8_t action_param;
                    var_int32 block_type;
                };

                struct block_update : public packet<0x08> {
                    position location;
                    var_int32 block_state_id;
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

                struct chunk_biomes : public packet<0x0D> {
                    int32_t z;
                    int32_t x;
                    std::vector<uint8_t> data;
                };

                struct clear_titles : public packet<0x0E> {
                    bool reset;
                };

                struct command_suggestions_response : public packet<0x0F>, packet_preprocess {
                    struct match : public packet_preprocess {
                        string_sized<32767> set;
                        std::optional<Chat> tooltip;
                    };

                    unordered_id<var_int32> command_transaction_id;
                    var_int32 start;
                    var_int32 length;
                    std::vector<match> matches;
                };

                struct commands : public packet<0x10>, packet_preprocess {
                    struct node : public packet_preprocess {
                        uint8_t flags;
                        std::vector<var_int32> children;

                        struct redirect_node : public flags_item<8, 0x8, 0> {
                            var_int32 node;
                        };

                        struct literal_node : public flags_item<2, 0x3, 1>, packet_preprocess {
                            string_sized<32767> name;
                        };

                        struct argument_node : public flags_item<3, 0x3, 1>, packet_preprocess {
                            string_sized<32767> name;

                            template <class T>
                            struct min_max {
                                struct Min : public flags_item<1, 1, 0> {
                                    T val;
                                };

                                struct Max : public flags_item<2, 2, 1> {
                                    T val;
                                };

                                flags_list<uint8_t, Min, Max> values;
                            };

                            struct brigadier__bool : public enum_item<0> {};

                            struct brigadier__float : public enum_item<1>, min_max<float> {};

                            struct brigadier__double : public enum_item<2>, min_max<double> {};

                            struct brigadier__integer : public enum_item<3>, min_max<int32_t> {};

                            struct brigadier__long : public enum_item<4>, min_max<int64_t> {};

                            struct brigadier__string : public enum_item<5> {
                                enum behavior_e {
                                    single_word,
                                    quotable_phrase,
                                    greedy_phrase
                                };

                                enum_as<behavior_e, var_int32> behavior_e;
                            };

                            struct brigadier__entity : public enum_item<6> {
                                struct only_one_entity : public flags_item<1, 1, -1> {};

                                struct only_players : public flags_item<2, 2, -1> {};

                                flags_list<uint8_t, only_one_entity, only_players> flag;
                            };

                            struct minecraft__game_profile : public enum_item<7> {};

                            struct minecraft__block_pos : public enum_item<8> {};

                            struct minecraft__column_pos : public enum_item<9> {};

                            struct minecraft__vec3 : public enum_item<10> {};

                            struct minecraft__vec2 : public enum_item<11> {};

                            struct minecraft__block_state : public enum_item<12> {};

                            struct minecraft__block_predicate : public enum_item<13> {};

                            struct minecraft__item_stack : public enum_item<14> {};

                            struct minecraft__item_predicate : public enum_item<15> {};

                            struct minecraft__color : public enum_item<16> {};

                            struct minecraft__hex_color : public enum_item<17> {};

                            struct minecraft__component : public enum_item<18> {};

                            struct minecraft__style : public enum_item<19> {};

                            struct minecraft__message : public enum_item<20> {};

                            struct minecraft__nbt_compound_tag : public enum_item<21> {};

                            struct minecraft__nbt_tag : public enum_item<22> {};

                            struct minecraft__nbt_path : public enum_item<23> {};

                            struct minecraft__objective : public enum_item<24> {};

                            struct minecraft__objective_criteria : public enum_item<25> {};

                            struct minecraft__operation : public enum_item<26> {};

                            struct minecraft__particle : public enum_item<27> {};

                            struct minecraft__angle : public enum_item<28> {};

                            struct minecraft__rotation : public enum_item<29> {};

                            struct minecraft__scoreboard_slot : public enum_item<30> {};

                            struct minecraft__score_holder : public enum_item<31> {};

                            struct minecraft__swizzle : public enum_item<32> {};

                            struct minecraft__team : public enum_item<33> {};

                            struct minecraft__item_slot : public enum_item<34> {};

                            struct minecraft__item_slots : public enum_item<35> {};

                            struct minecraft__resource_location : public enum_item<36> {};

                            struct minecraft__function : public enum_item<37> {};

                            struct minecraft__entity_anchor : public enum_item<38> {};

                            struct minecraft__int_range : public enum_item<39> {};

                            struct minecraft__float_range : public enum_item<40> {};

                            struct minecraft__dimension : public enum_item<41> {};

                            struct minecraft__gamemode : public enum_item<42> {};

                            struct minecraft__time : public enum_item<43> {
                                int32_t min_duration;
                            };

                            struct minecraft__resource_or_tag : public enum_item<44> {
                                identifier registry;
                            };

                            struct minecraft__resource_or_tag_key : public enum_item<45> {
                                identifier registry;
                            };

                            struct minecraft__resource : public enum_item<46> {
                                identifier registry;
                            };

                            struct minecraft__resource_key : public enum_item<47> {
                                identifier registry;
                            };

                            struct minecraft__resource_selector : public enum_item<48> {
                                identifier registry;
                            };

                            struct minecraft__template_mirror : public enum_item<49> {};

                            struct minecraft__template_rotation : public enum_item<50> {};

                            struct minecraft__heightmap : public enum_item<51> {};

                            struct minecraft__loot_table : public enum_item<52> {};

                            struct minecraft__loot_predicate : public enum_item<53> {};

                            struct minecraft__loot_modifier : public enum_item<54> {};

                            struct minecraft__dialog : public enum_item<55> {};

                            struct minecraft__uuid : public enum_item<56> {};

                            enum_switch<
                                var_int32,
                                brigadier__bool,
                                brigadier__float,
                                brigadier__double,
                                brigadier__integer,
                                brigadier__long,
                                brigadier__string,
                                brigadier__entity,
                                minecraft__game_profile,
                                minecraft__block_pos,
                                minecraft__column_pos,
                                minecraft__vec3,
                                minecraft__vec2,
                                minecraft__block_state,
                                minecraft__block_predicate,
                                minecraft__item_stack,
                                minecraft__item_predicate,
                                minecraft__color,
                                minecraft__hex_color,
                                minecraft__component,
                                minecraft__style,
                                minecraft__message,
                                minecraft__nbt_compound_tag,
                                minecraft__nbt_tag,
                                minecraft__nbt_path,
                                minecraft__objective,
                                minecraft__objective_criteria,
                                minecraft__operation,
                                minecraft__particle,
                                minecraft__angle,
                                minecraft__rotation,
                                minecraft__scoreboard_slot,
                                minecraft__score_holder,
                                minecraft__swizzle,
                                minecraft__team,
                                minecraft__item_slot,
                                minecraft__item_slots,
                                minecraft__resource_location,
                                minecraft__function,
                                minecraft__entity_anchor,
                                minecraft__int_range,
                                minecraft__float_range,
                                minecraft__dimension,
                                minecraft__gamemode,
                                minecraft__time,
                                minecraft__resource_or_tag,
                                minecraft__resource_or_tag_key,
                                minecraft__resource,
                                minecraft__resource_key,
                                minecraft__resource_selector,
                                minecraft__template_mirror,
                                minecraft__template_rotation,
                                minecraft__heightmap,
                                minecraft__loot_table,
                                minecraft__loot_predicate,
                                minecraft__loot_modifier,
                                minecraft__dialog,
                                minecraft__uuid>
                                type;
                        };

                        struct is_executable : public flags_item<0x04, 0x04, -1> {};

                        struct suggestions_type : public flags_item<0x10, 0x10, 2> {
                            identifier name;
                        };

                        struct is_restricted : public flags_item<0x20, 0x20, -2> {};

                        flags_list_from<
                            node,
                            uint8_t,
                            &node::flags,
                            literal_node,
                            argument_node,
                            is_executable,
                            redirect_node,
                            suggestions_type,
                            is_restricted>
                            flags_values;
                    };

                    std::vector<node> nodes;
                    var_int32 root_index;
                };

                struct container_close : public packet<0x11> {
                    var_int32 windows_id;
                };

                struct container_set_content : public packet<0x12> {
                    var_int32 windows_id;
                    var_int32 state_id;
                    std::vector<base_objects::slot> inventory_data;
                    base_objects::slot carried_item;
                };

                struct container_set_data : public packet<0x13> {
                    var_int32 windows_id;

                    struct furnace {
                        enum class property_e {
                            fuel_left = 0,
                            max_fuel = 1,
                            progress = 2,
                            max_progress = 3,
                        };
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct enchantment_table {
                        enum class property_e {
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
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct beacon {
                        enum class property_e {
                            power_level = 0,
                            first_potion = 1,
                            second_potion = 2,
                        };
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct anvil {
                        enum class property_e {
                            repair_cost = 0,
                        };
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct brewing_stand {
                        enum class property_e {
                            brew_time = 0, //400-0
                            fuel_left = 1, //0-20
                        };
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct stonecutter {
                        enum class property_e {
                            selected_recipe = 0, //-1 = none
                        };
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct loom {
                        enum class property_e {
                            selected_pattern = 0, //0 = base
                        };
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct lectern {
                        enum class property_e {
                            page_number = 0,
                        };
                        enum_as<property_e, short> property;
                        short value;
                    };

                    struct smithing_table {
                        enum class property_e {
                            has_recipe_error = 0, // 0>= == false, 0< == true
                        };
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
                    base_objects::slot carried_item;
                };

                struct cookie_request : public packet<0x15> {
                    identifier key;
                };

                struct cooldown : public packet<0x16> {
                    identifier group;
                    var_int32 ticks;
                };

                struct custom_chat_completions : public packet<0x17>, packet_preprocess {
                    enum class suggestion_e {
                        add = 0,
                        remove = 1,
                        set = 2,
                    };
                    enum_as<suggestion_e, var_int32> suggestion;
                    std::vector<string_sized<32767>> entries;
                };

                struct custom_payload : public packet<0x18>, packet_preprocess {
                    identifier channel;
                    vector_sized_no_size<uint8_t, 1048576> user_name;
                };

                struct damage_event : public packet<0x19> {
                    struct position_double {
                        double x;
                        double y;
                        double z;
                    };

                    var_int32 entity_id;
                    optional_var_int32 source_damage_type_id;
                    optional_var_int32 source_entity_id;
                    optional_var_int32 source_direct_entity_id;
                    std::optional<position_double> source_pos;
                };

                struct debug_sample : public packet<0x1A> {
                    std::vector<int64_t> sample;
                    var_int32 sample_type;
                };

                struct delete_chat : public packet<0x1B> {
                    var_int32 message_id;
                    std::optional<std::array<uint8_t, 256>> signature;
                };

                struct disconnect : public packet<0x1C> {
                    Chat reason;
                };

                struct disguised_chat : public packet<0x1D> {
                    Chat message;
                    or_<var_int32, chat_type> type;
                    Chat sender;
                    std::optional<Chat> target_name;
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
                    std::optional<player_delta_velocity_t> player_delta_velocity;
                    var_int32 particle_id;
                    vector_no_size<uint8_t> particle_data;
                    or_<var_int32, sound_event> sound;
                };

                struct forget_level_chunk : public packet<0x21> {
                    int32_t z;
                    int32_t x;
                };

                struct game_event : public packet<0x22> {
                    struct no_respawn_block_available : public enum_item<0> {
                        float _ignored;
                    };

                    struct raining_begin : public enum_item<1> {
                        float _ignored;
                    };

                    struct raining_end : public enum_item<2> {
                        float _ignored;
                    };

                    struct gamemode_change : public enum_item<3> {
                        enum_as<gamemode_e, float> gamemode;
                    };

                    struct win_game : public enum_item<4> {
                        float roll_credits; //true/false
                    };

                    struct demo_event : public enum_item<5> {
                        enum class event_e {
                            welcome = 0,
                            movement_controls = 101,
                            jump_controls = 102,
                            inventory_controls = 103,
                            demo_over = 104,
                        };
                        enum_as<event_e, float> event;
                    };

                    struct arrow_hit_player : public enum_item<6> {
                        float _ignored;
                    };

                    struct rain_level_change : public enum_item<7> {
                        float level;
                    };

                    struct thunder_level_change : public enum_item<8> {
                        float level;
                    };

                    struct puffer_fish_sting_sound : public enum_item<9> {
                        float _ignored;
                    };

                    struct guardian_appear_animation : public enum_item<10> {
                        float _ignored;
                    };

                    struct respawn_screen_mode : public enum_item<11> {
                        float enabled; //true/false
                    };

                    struct limited_crafting_mode : public enum_item<12> {
                        float enabled; //true/false
                    };

                    struct wait_for_level_chunks : public enum_item<13> {
                        float _ignored;
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
                    int32_t x;
                    int32_t z;
                    //TODO chunk data + light
                };

                struct level_event : public packet<0x28> {
                    enum class event_id {
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
                    var_int32 particle_id;
                    vector_no_size<uint8_t> particle_data;
                };

                struct light_update : public packet<0x2A> {
                    int32_t x;
                    int32_t z;
                    //TODO light
                };

                struct login : public packet<0x2B> {
                    struct death_location_t {
                        identifier world;
                        position location;
                    };

                    int32_t entity_id;
                    bool is_hardcore;
                    std::vector<identifier> dimension_names;
                    var_int32 max_players;
                    var_int32 view_distance;
                    var_int32 simulation_distance;
                    bool reduced_debug_info;
                    bool respawn_screen;
                    bool limited_crafting_enabled;
                    var_int32 dimension_type;
                    identifier dimension_name;
                    int64_t seed_hashed;
                    enum_as<gamemode_e, uint8_t> gamemode;
                    enum_as<optional_gamemode_e, int8_t> prev_gamemode;
                    bool world_is_debug;
                    bool world_is_flat;
                    std::optional<death_location_t> death_location;
                    var_int32 portal_cooldown;
                    var_int32 sea_level;
                    bool enforce_secure_chat;
                };

                struct map_item_data : public packet<0x2C>, packet_preprocess {
                    struct icon : public packet_preprocess {
                        enum class type_e {
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
                        std::optional<Chat> name;
                    };

                    struct color_patch {
                        uint8_t rows;
                        uint8_t x;
                        uint8_t z;
                        vector_no_size<uint8_t> data;
                    };

                    var_int32 map_id;
                    int8_t scale;
                    bool is_locked;
                    std::optional<std::vector<icon>> icons;
                    value_optional<uint8_t, color_patch> columns;
                };

                struct merchant_offers : public packet<0x2D> {
                    //TODO
                };

                struct move_entity_pos : public packet<0x2E> {
                    //TODO
                };

                struct move_entity_pos_rot : public packet<0x2F> {
                    //TODO
                };

                struct move_minecart_along_track : public packet<0x30> {
                    //TODO
                };

                struct move_entity_rot : public packet<0x31> {
                    //TODO
                };

                struct move_vehicle : public packet<0x32> {
                    //TODO
                };

                struct open_book : public packet<0x33> {
                    //TODO
                };

                struct open_screen : public packet<0x34> {
                    //TODO
                };

                struct open_sign_editor : public packet<0x35> {
                    //TODO
                };

                struct ping : public packet<0x36> {
                    //TODO
                };

                struct pong_response : public packet<0x37> {
                    //TODO
                };

                struct place_ghost_recipe : public packet<0x38> {
                    //TODO
                };

                struct player_abilities : public packet<0x39> {
                    //TODO
                };

                struct player_chat : public packet<0x3A> {
                    //TODO
                };

                struct player_combat_end : public packet<0x3B> {
                    //TODO
                };

                struct player_combat_enter : public packet<0x3C> {
                    //TODO
                };

                struct player_combat_kill : public packet<0x3D> {
                    //TODO
                };

                struct player_info_remove : public packet<0x3E> {
                    //TODO
                };

                struct player_info_update : public packet<0x3F> {
                    //TODO
                };

                struct player_look_at : public packet<0x40> {
                    //TODO
                };

                struct player_position : public packet<0x41> {
                    //TODO
                };

                struct player_rotation : public packet<0x42> {
                    //TODO
                };

                struct recipe_book_add : public packet<0x43> {
                    //TODO
                };

                struct recipe_book_remove : public packet<0x44> {
                    //TODO
                };

                struct recipe_book_settings : public packet<0x45> {
                    //TODO
                };

                struct remove_entities : public packet<0x46> {
                    //TODO
                };

                struct remove_mob_effect : public packet<0x47> {
                    //TODO
                };

                struct reset_score : public packet<0x48> {
                    //TODO
                };

                struct resource_pack_pop : public packet<0x49> {
                    //TODO
                };

                struct resource_pack_push : public packet<0x4A> {
                    //TODO
                };

                struct respawn : public packet<0x4B> {
                    //TODO
                };

                struct rotate_head : public packet<0x4C> {
                    //TODO
                };

                struct section_blocks_update : public packet<0x4D> {
                    //TODO
                };

                struct select_advancements_tab : public packet<0x4E> {
                    //TODO
                };

                struct server_data : public packet<0x4F> {
                    //TODO
                };

                struct set_action_bar_text : public packet<0x50> {
                    //TODO
                };

                struct set_border_center : public packet<0x51> {
                    //TODO
                };

                struct set_border_lerp_size : public packet<0x52> {
                    //TODO
                };

                struct set_border_size : public packet<0x53> {
                    //TODO
                };

                struct set_border_warning_delay : public packet<0x54> {
                    //TODO
                };

                struct set_border_warning_distance : public packet<0x55> {
                    //TODO
                };

                struct set_camera : public packet<0x56> {
                    //TODO
                };

                struct set_chunk_cache_center : public packet<0x57> {
                    //TODO
                };

                struct set_chunk_cache_radius : public packet<0x58> {
                    //TODO
                };

                struct set_cursor_item : public packet<0x59> {
                    //TODO
                };

                struct set_default_spawn_position : public packet<0x5A> {
                    //TODO
                };

                struct set_display_objective : public packet<0x5B> {
                    //TODO
                };

                struct set_entity_data : public packet<0x5C> {
                    //TODO
                };

                struct set_entity_link : public packet<0x5D> {
                    //TODO
                };

                struct set_entity_motion : public packet<0x5E> {
                    //TODO
                };

                struct set_equipment : public packet<0x5F> {
                    //TODO
                };

                struct set_experience : public packet<0x60> {
                    //TODO
                };

                struct set_health : public packet<0x61> {
                    //TODO
                };

                struct set_held_slot : public packet<0x62> {
                    //TODO
                };

                struct set_objective : public packet<0x63> {
                    //TODO
                };

                struct set_passengers : public packet<0x64> {
                    //TODO
                };

                struct set_player_inventory : public packet<0x65> {
                    //TODO
                };

                struct set_player_team : public packet<0x66> {
                    //TODO
                };

                struct set_score : public packet<0x67> {
                    //TODO
                };

                struct set_simulation_distance : public packet<0x68> {
                    //TODO
                };

                struct set_subtitle_text : public packet<0x69> {
                    //TODO
                };

                struct set_time : public packet<0x6A> {
                    //TODO
                };

                struct set_title_text : public packet<0x6B> {
                    //TODO
                };

                struct set_titles_animation : public packet<0x6C> {
                    //TODO
                };

                struct sound_entity : public packet<0x6D> {
                    //TODO
                };

                struct sound : public packet<0x6E> {
                    //TODO
                };

                struct start_configuration : public packet<0x6F> {
                    //TODO
                };

                struct stop_sound : public packet<0x70> {
                    //TODO
                };

                struct store_cookie : public packet<0x71> {
                    //TODO
                };

                struct system_chat : public packet<0x72> {
                    Chat content;
                    bool is_overlay;
                };

                struct tab_list : public packet<0x73> {
                    //TODO
                };

                struct tag_query : public packet<0x74> {
                    //TODO
                };

                struct take_item_entity : public packet<0x75> {
                    //TODO
                };

                struct teleport_entity : public packet<0x76> {
                    //TODO
                };

                struct test_instance_block_status : public packet<0x77> {
                    //TODO
                };

                struct ticking_state : public packet<0x78> {
                    //TODO
                };

                struct ticking_step : public packet<0x79> {
                    //TODO
                };

                struct transfer : public packet<0x7A> {
                    //TODO
                };

                struct update_advancements : public packet<0x7B> {
                    //TODO
                };

                struct update_attributes : public packet<0x7C> {
                    //TODO
                };

                struct update_mob_effect : public packet<0x7D> {
                    //TODO
                };

                struct update_recipes : public packet<0x7E> {
                    //TODO
                };

                struct update_tags : public packet<0x7F> {
                    //TODO
                };

                struct projectile_power : public packet<0x80> {
                    //TODO
                };

                struct custom_report_details : public packet<0x81> {
                    //TODO
                };

                struct server_links : public packet<0x82> {
                    //TODO
                };

                struct waypoint : public packet<0x83> {
                    //TODO
                };

                struct clear_dialog : public packet<0x84> {};

                struct show_dialog : public packet<0x85> {
                    //TODO
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
                play::chunk_biomes,
                play::clear_titles,
                play::command_suggestions_response,
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
            client_bound::login_packet,
            client_bound::configuration_packet,
            client_bound::play_packet
        );

        namespace server_bound {
            namespace login {
                struct hello : public packet<0x00>, packet_preprocess {
                    string_sized<16> name;
                    enbt::raw_uuid uuid;
                };

                struct key : public packet<0x01> {
                    std::vector<uint8_t> shared_secret;
                    std::vector<uint8_t> verify_token;
                };

                struct custom_query_answer : public packet<0x02>, packet_preprocess {
                    ordered_id<var_int32> message_id;
                    vector_sized_siz_from_packet<uint8_t, 32767> user_name;
                };

                struct login_acknowledged : public packet<0x03> {};

                struct cookie_response : public packet<0x04>, packet_preprocess {
                    identifier key;
                    std::optional<vector_sized<uint8_t, 5120>> payload;
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
                struct client_information : public packet<0x00>, packet_preprocess {
                    enum class chat_mode_e {
                        disabled = 0,
                        commands_only = 1,
                        hidden = 2,
                    };
                    enum class displayer_skin_parts_f {
                        cape = 0x1,
                        jacket = 0x2,
                        left_sleeve = 0x4,
                        right_sleeve = 0x8,
                        left_pants = 0x10,
                        right_pants = 0x20,
                        hat = 0x40,
                        _unused = 0x80
                    };
                    enum class main_hand_e {
                        left = 0,
                        right = 1
                    };
                    enum class particle_status_e {
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

                struct cookie_response : public packet<0x01>, packet_preprocess {
                    identifier key;
                    std::optional<vector_sized<uint8_t, 5120>> payload;
                };

                struct custom_payload : public packet<0x02>, packet_preprocess {
                    identifier channel;
                    vector_sized_siz_from_packet<uint8_t, 32767> user_name;
                };

                struct finish_configuration : public packet<0x03> {};

                struct keep_alive : public packet<0x04> {
                    uint64_t keep_alive_id;
                };

                struct pong : public packet<0x05> {
                    unordered_id<int32_t> ping_request_id;
                };

                struct resource_pack : public packet<0x06> {
                    enum class result_e {
                        success = 0,
                        declined = 1,
                        download_failed = 2,
                        accepted = 3,
                        downloaded = 4,
                        invalid_url = 5,
                        reload_failed = 6,
                        discarded = 7
                    };
                    enbt::raw_uuid uuid;
                    enum_as<result_e, var_int32> result;
                };

                struct select_known_packs : public packet<0x07> {
                    struct pack {
                        std::string _namespace;
                        std::string id;
                        std::string version;
                    };

                    std::vector<pack> packs;
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
                    ordered_id<var_int32> teleport_id;
                };

                struct block_entity_tag_query : public packet<0x01> {
                    ordered_id<var_int32> block_entity_tag_query_id;
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

                struct chat_command : public packet<0x06>, packet_preprocess {
                    string_sized<32767> command;
                };

                struct chat_command_signed : public packet<0x07>, packet_preprocess {
                    struct argument_signature : public packet_preprocess {
                        string_sized<16> argument_name;
                        std::array<uint8_t, 256> signature;
                    };

                    string_sized<32767> command;
                    uint64_t timestamp;
                    uint64_t salt;
                    vector_sized<argument_signature, 8> argument_signatures;
                    var_int32 message_count;
                    bitset_fixed<20> acknowledged;
                    uint8_t check_sum;
                };

                struct chat : public packet<0x08>, packet_preprocess {
                    string_sized<256> command;
                    uint64_t timestamp;
                    uint64_t salt;
                    std::optional<std::array<uint8_t, 256>> signature;
                    var_int32 message_count;
                    bitset_fixed<20> acknowledged;
                    uint8_t check_sum;
                };

                struct chat_session_update : public packet<0x09>, packet_preprocess {
                    enbt::raw_uuid uuid;
                    uint64_t expiries_at;
                    vector_sized<uint8_t, 512> public_key;
                    vector_sized<uint8_t, 4096> key_signature;
                };

                struct chunk_batch_received : public packet<0x0A> {
                    float chunks_per_tick;
                };

                struct client_command : public packet<0x0B> {
                    enum class action_id_e {
                        perform_respawn = 0,
                        request_stats = 1,
                    };
                    enum_as<action_id_e, var_int32> action_id;
                };

                struct client_tick_end : public packet<0x0C> {};

                struct client_information : public packet<0x0D>, packet_preprocess {
                    enum class chat_mode_e {
                        disabled = 0,
                        commands_only = 1,
                        hidden = 2,
                    };
                    enum class displayer_skin_parts_f {
                        cape = 0x1,
                        jacket = 0x2,
                        left_sleeve = 0x4,
                        right_sleeve = 0x8,
                        left_pants = 0x10,
                        right_pants = 0x20,
                        hat = 0x40,
                        _unused = 0x80
                    };
                    enum class main_hand_e {
                        left = 0,
                        right = 1
                    };
                    enum class particle_status_e {
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

                struct command_suggestion : public packet<0x0E>, packet_preprocess {
                    unordered_id<var_int32> suggestion_transaction_id;
                    string_sized<32500> locale;
                };

                struct configuration_acknowledged : public packet<0x0F> {};

                struct container_button_click : public packet<0x10> {
                    var_int32 window_id;
                    var_int32 button_id;
                };

                struct container_click : public packet<0x11>, packet_preprocess {
                    struct hashed_slot_data {
                        var_int32 item_id;
                        var_int32 count;

                        struct component {
                            var_int32 type;
                            int32_t crc32_hash;
                        };

                        std::vector<component> add_components;
                        std::vector<var_int32> remove_components;
                    };

                    struct hashed_slot : public std::optional<hashed_slot_data> {
                        using std::optional<hashed_slot_data>::optional;
                    };

                    struct changed_slot {
                        short slot;
                        hashed_slot data;
                    };

                    var_int32 window_id;
                    var_int32 state_id;
                    short slot;
                    int8_t button;
                    var_int32 mode;
                    vector_sized<changed_slot, 128> changed;
                    hashed_slot carry_item;
                };

                struct container_close : public packet<0x12> {
                    var_int32 window_id;
                };

                struct container_slot_state_changed : public packet<0x13> {
                    var_int32 slot_id;
                    var_int32 window_id;
                    bool state;
                };

                struct cookie_response : public packet<0x14>, packet_preprocess {
                    identifier key;
                    std::optional<vector_sized<uint8_t, 5120>> payload;
                };

                struct custom_payload : public packet<0x15>, packet_preprocess {
                    identifier channel;
                    vector_sized_siz_from_packet<uint8_t, 32767> user_name;
                };

                struct debug_sample_subscription : public packet<0x16> {
                    var_int32 sample_type;
                };

                struct edit_book : public packet<0x17>, packet_preprocess {
                    var_int32 slot;
                    vector_sized<string_sized<1024>, 100> entries;
                    std::optional<string_sized<32>> title;
                };

                struct entity_tag_query : public packet<0x18> {
                    ordered_id<var_int32> entity_tag_transaction_id;
                    var_int32 entity_id;
                };

                struct interact : public packet<0x19> {
                    var_int32 entity_id;
                    enum class hand_e {
                        main = 0,
                        off = 1
                    };

                    struct interact_ : public enum_item<0> {
                        enum_as<hand_e, var_int32> hand;
                    };

                    struct attack : public enum_item<1> {
                    };

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
                    enum flags_f {
                        on_ground = 1,
                        push_against_wall = 2
                    };

                    double x;
                    double y;
                    double z;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct move_player_pos_rot : public packet<0x1D> {
                    enum flags_f {
                        on_ground = 1,
                        push_against_wall = 2
                    };

                    double x;
                    double y;
                    double z;
                    float yaw;
                    float pitch;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct move_player_rot : public packet<0x1F> {
                    enum flags_f {
                        on_ground = 1,
                        push_against_wall = 2
                    };

                    float yaw;
                    float pitch;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct move_player_status_only : public packet<0x20> {
                    enum flags_f {
                        on_ground = 1,
                        push_against_wall = 2
                    };

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
                    var_int32 recipe_id;
                    bool make_all;
                };

                struct player_abilities : public packet<0x27> {
                    enum flags_f {
                        flying = 2
                    };

                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct player_action : public packet<0x28> {
                    enum class status_e {
                        digging_start = 0,
                        digging_canceled = 1,
                        digging_finished = 2,
                        drop_item_stack = 3,
                        drop_item = 4,
                        right_click_item = 5,
                        swap_item_in_hand = 6,
                    };
                    enum class face_e {
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
                    var_int32 sequence;
                };

                struct player_command : public packet<0x29> {
                    enum class action_e {
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
                    enum class status_f {
                        forward = 1,
                        backward = 2,
                        left = 4,
                        right = 8,
                        jump = 16,
                        sneak = 32,
                        sprint = 64,
                    };
                    enum_as_flag<status_f, uint8_t> face;
                };

                struct player_loaded : public packet<0x2B> {
                };

                struct pong : public packet<0x2C> {
                    int32_t id;
                };

                struct recipe_book_change_settings : public packet<0x2D> {
                    enum class book_type_e {
                        crafting = 0,
                        furnace = 1,
                        blast_furnace = 2,
                        smoker = 3,
                    };

                    enum_as<book_type_e, var_int32> book_type;
                    bool book_open;
                    bool filter_active;
                };

                struct recipe_book_seen_recipe : public packet<0x2E> {
                    var_int32 recipe_id;
                };

                struct rename_item : public packet<0x2F>, packet_preprocess {
                    string_sized<32767> new_name;
                };

                struct resource_pack : public packet<0x30> {
                    enum class result_e {
                        success = 0,
                        declined = 1,
                        download_failed = 2,
                        accepted = 3,
                        downloaded = 4,
                        invalid_url = 5,
                        reload_failed = 6,
                        discarded = 7
                    };
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
                    std::optional<var_int32> primary_effect;
                    std::optional<var_int32> secondary_effect;
                };

                struct set_carried_item : public packet<0x34> {
                    short slot;
                };

                struct set_command_block : public packet<0x35>, packet_preprocess {
                    enum class mode_e {
                        chain = 0,
                        repeating = 1,
                        impulse = 2,
                    };

                    enum flags_f {
                        track_output = 1,
                        is_conditional = 2,
                        automatic = 4,
                    };

                    position location;
                    string_sized<32767> command;
                    enum_as<mode_e, var_int32> mode;
                    enum_as_flag<flags_f, int8_t> flags;
                };

                struct set_command_minecart : public packet<0x36>, packet_preprocess {
                    var_int32 entity_id;
                    string_sized<32767> command;
                    bool track_output;
                };

                struct set_creative_mode_slot : public packet<0x37> {
                    short slot;
                    base_objects::slot item;
                };

                struct set_jigsaw_block : public packet<0x38>, packet_preprocess {
                    position location;
                    identifier name;
                    identifier target;
                    identifier pool;
                    string_sized<32767> final_state;
                    string_sized<32767> joint_state;
                    var_int32 selection_priority;
                    var_int32 placement_priority;
                };

                struct set_structure_block : public packet<0x39>, packet_preprocess {
                    enum class mirror_side_e {
                        none = 0,
                        left_right = 1,
                        front_back = 2,
                    };
                    enum class rotation_e {
                        none = 0,
                        clockwise_90 = 1,
                        clockwise_180 = 2,
                        counterclockwise_90 = 3,
                    };

                    struct ignore_entities : public flags_item<1, 1, 1> {};

                    struct show_air : public flags_item<2, 2, 2> {};

                    struct show_bounding_block : public flags_item<4, 4, 3> {};

                    struct strict_placement : public flags_item<8, 8, 4> {};

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
                    enum class mode_e {
                        start = 0,
                        log = 1,
                        fail = 2,
                        accept = 3,
                    };
                    position location;
                    enum_as<mode_e, var_int32> mode;
                    std::string message;
                };

                struct sign_update : public packet<0x3B>, packet_preprocess {
                    position location;
                    bool is_front_text;
                    std::array<string_sized<384>, 4> lines;
                };

                struct swing : public packet<0x3C> {
                    enum class hand_e {
                        main = 0,
                        off = 1,
                    };
                    enum_as<hand_e, var_int32> hand;
                };

                struct teleport_to_entity : public packet<0x3D> {
                    enbt::raw_uuid uuid;
                };

                struct test_instance_block_action : public packet<0x3E> {
                    enum class action_e {
                        init = 0,
                        query = 1,
                        set = 2,
                        reset = 3,
                        save = 4,
                        export_ = 5,
                        run = 6,
                    };
                    enum class rotation_e {
                        none = 0,
                        clockwise_90 = 1,
                        clockwise_180 = 2,
                        counterclockwise_90 = 3,
                    };
                    enum class status_e {
                        cleared = 0,
                        running = 1,
                        finished = 2,
                    };
                    position location;
                    enum_as<action_e, var_int32> action;
                    std::optional<var_int32> test_id;
                    var_int32 size_x;
                    var_int32 size_y;
                    var_int32 size_z;
                    enum_as<rotation_e, var_int32> rotation;
                    bool ignore_entities;
                    enum_as<status_e, var_int32> status;
                    std::optional<Chat> error_message;
                };

                struct use_item_on : public packet<0x3F>, packet_preprocess {
                    enum class hand_e {
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
                    unordered_id<var_int32> sequence_id;
                };

                struct use_item : public packet<0x40> {
                    enum class hand_e {
                        main = 0,
                        off = 1,
                    };
                    enum_as<hand_e, var_int32> hand;
                    unordered_id<var_int32> sequence_id;
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
            server_bound::login_packet,
            server_bound::configuration_packet,
            server_bound::play_packet
        );


        bool send(base_objects::SharedClientData& client, client_bound_packet&&);
        base_objects::network::response encode(client_bound_packet&& packet);

        bool decode(base_objects::SharedClientData& context, ArrayStream&);
        client_bound::login_packet decode_client_login(ArrayStream&);
        client_bound::configuration_packet decode_client_configuration(ArrayStream&);
        client_bound::play_packet decode_client_play(ArrayStream&);

        server_bound::login_packet decode_server_login(ArrayStream&);
        server_bound::configuration_packet decode_server_configuration(ArrayStream&);
        server_bound::play_packet decode_server_play(ArrayStream&);
        

        client_bound::login_packet decode_client_login(base_objects::SharedClientData& context, ArrayStream&);
        client_bound::configuration_packet decode_client_configuration(base_objects::SharedClientData& context, ArrayStream&);
        client_bound::play_packet decode_client_play(base_objects::SharedClientData& context, ArrayStream&);

        server_bound::login_packet decode_server_login(base_objects::SharedClientData& context, ArrayStream&);
        server_bound::configuration_packet decode_server_configuration(base_objects::SharedClientData& context, ArrayStream&);
        server_bound::play_packet decode_server_play(base_objects::SharedClientData& context, ArrayStream&);


        std::string stringize_packet(const client_bound_packet&);
        std::string stringize_packet(const server_bound_packet&);

        namespace __internal {
            base_objects::events::event_register_id register_client_viewer(uint8_t mode, size_t id, base_objects::events::sync_event<client_bound_packet&>::function&&);
            base_objects::events::event_register_id register_server_viewer(uint8_t mode, size_t id, base_objects::events::sync_event<server_bound_packet&>::function&&);
            void unregister_client_viewer(uint8_t mode, size_t id, base_objects::events::event_register_id);
            void unregister_server_viewer(uint8_t mode, size_t id, base_objects::events::event_register_id);

            base_objects::events::event_register_id register_server_processor(uint8_t mode, size_t id, std::function<void(server_bound_packet&)>&&);
            void unregister_server_processor(base_objects::events::event_register_id);
        }

        template <class Packet>
            requires(std::is_constructible_v<client_bound::login_packet, Packet> || std::is_constructible_v<client_bound::configuration_packet, Packet> || std::is_constructible_v<client_bound::play_packet, Packet>)
        base_objects::events::event_register_id register_viewer_client_bound(auto&& fn) {
            size_t mode;
            if constexpr (std::is_constructible_v<client_bound::login_packet, Packet>) {
                mode = 0;
            } else if constexpr (std::is_constructible_v<client_bound::configuration_packet, Packet>) {
                mode = 1;
            } else
                mode = 2;

            return __internal::register_client_viewer(
                mode,
                Packet::packet_id::value,
                [&](auto& packet) {
                    return std::visit(
                        [&fn](auto& it) -> bool {
                            if constexpr (std::is_same_v<Packet, std::decay_t<decltype(it)>>)
                                return fn(it);
                            return true;
                        }
                    );
                }
            );
        }

        template <class Packet>
            requires(std::is_constructible_v<server_bound::login_packet, Packet> || std::is_constructible_v<server_bound::configuration_packet, Packet> || std::is_constructible_v<server_bound::play_packet, Packet>)
        base_objects::events::event_register_id register_viewer_server_bound(auto&& fn) {
            size_t mode;
            if constexpr (std::is_constructible_v<server_bound::login_packet, Packet>) {
                mode = 0;
            } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, Packet>) {
                mode = 1;
            } else
                mode = 2;

            return __internal::register_server_viewer(
                mode,
                Packet::packet_id::value,
                [&](auto& packet) {
                    return std::visit(
                        [&fn](auto& it) -> bool {
                            if constexpr (std::is_same_v<Packet, std::decay_t<decltype(it)>>)
                                return fn(it);
                            return true;
                        }
                    );
                }
            );
        }

        template <class Packet>
            requires(std::is_constructible_v<client_bound::login_packet, Packet> || std::is_constructible_v<client_bound::configuration_packet, Packet> || std::is_constructible_v<client_bound::play_packet, Packet>)
        base_objects::events::event_register_id register_viewer_client_bound(base_objects::events::event_register_id id) {
            size_t mode;
            if constexpr (std::is_constructible_v<client_bound::login_packet, Packet>) {
                mode = 0;
            } else if constexpr (std::is_constructible_v<client_bound::configuration_packet, Packet>) {
                mode = 1;
            } else
                mode = 2;

            return __internal::unregister_client_viewer(
                mode,
                Packet::packet_id::value,
                id
            );
        }

        //could be registered only once packet
        template <class Packet>
            requires(std::is_constructible_v<server_bound::login_packet, Packet> || std::is_constructible_v<server_bound::configuration_packet, Packet> || std::is_constructible_v<server_bound::play_packet, Packet>)
        base_objects::events::event_register_id unregister_viewer_server_bound(base_objects::events::event_register_id id) {
            size_t mode;
            if constexpr (std::is_constructible_v<server_bound::login_packet, Packet>) {
                mode = 0;
            } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, Packet>) {
                mode = 1;
            } else
                mode = 2;

            return __internal::unregister_server_viewer(
                mode,
                Packet::packet_id,
                id
            );
        }

        template <class Packet>
            requires(std::is_constructible_v<server_bound::login_packet, Packet> || std::is_constructible_v<server_bound::configuration_packet, Packet> || std::is_constructible_v<server_bound::play_packet, Packet>)
        base_objects::events::event_register_id register_server_bound_processor(auto&& fn) {
            size_t mode;
            if constexpr (std::is_constructible_v<server_bound::login_packet, Packet>) {
                mode = 0;
            } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, Packet>) {
                mode = 1;
            } else
                mode = 2;

            return __internal::register_server_processor(
                mode,
                Packet::packet_id::value,
                [&](auto& packet) {
                    return std::visit(
                        [&fn](auto& it) {
                            if constexpr (std::is_same_v<Packet, std::decay_t<decltype(it)>>)
                                return fn(it);
                        }
                    );
                }
            );
        }

        inline void unregister_server_bound_processor(base_objects::events::event_register_id id) {
            __internal::unregister_server_processor(id);
        }
    }

    inline base_objects::SharedClientData& operator<<(base_objects::SharedClientData& client, api::new_packets::client_bound_packet&& packet){
        api::new_packets::send(client, std::move(packet));
        return client;
    }
}

#undef decl_variant
#endif /* SRC_API_NEW_PACKETS */
