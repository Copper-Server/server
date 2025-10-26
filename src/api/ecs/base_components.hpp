#ifndef SRC_API_ECS_BASE_COMPONENTS
#define SRC_API_ECS_BASE_COMPONENTS
#include <library/list_array.hpp>
#include <memory>
#include <src/api/ecs.hpp>
#include <src/api/entity.hpp>
#include <src/base_objects/entity/animation.hpp>
#include <src/base_objects/entity/event.hpp>
#include <src/base_objects/entity/metadata.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/slot.hpp>
#include <src/base_objects/weather.hpp>
#include <src/base_objects/world/block_action.hpp>
#include <src/util/calculations.hpp>

namespace copper_server {
    namespace storage {
        class world_data;
        class chunk_data;
    }

    namespace base_objects {
        struct shared_client_data;
        using client_data_holder = std::shared_ptr<shared_client_data>;

        namespace world {
            struct sub_chunk_data;
            class storage;
        }


        struct block;
        struct block_entity;
        struct block_entity_ref;
        struct const_block_entity_ref;
    }
}

namespace copper_server::api::ecs::com {
    struct inventory { //some entities
        std::unique_ptr<std::unordered_map<uint32_t, base_objects::slot_data>> value;

        inventory()
            : value(std::make_unique<std::unordered_map<uint32_t, base_objects::slot_data>>()) {}

        inventory(inventory&&) noexcept = default;
        inventory& operator=(inventory&&) noexcept = default;

        inventory(const inventory& other) {
            if (other.value)
                value = std::make_unique<std::unordered_map<uint32_t, base_objects::slot_data>>(*other.value);
        }

        inventory& operator=(const inventory& other) {
            if (this != &other) {
                if (other.value) {
                    if (!value) {
                        value = std::make_unique<std::unordered_map<uint32_t, base_objects::slot_data>>(*other.value);
                    } else
                        *value = *other.value;
                } else
                    value.reset();
            }
            return *this;
        }

        std::unordered_map<uint32_t, base_objects::slot_data>& get() {
            return *value;
        }

        const std::unordered_map<uint32_t, base_objects::slot_data>& get() const {
            return *value;
        }
    };

    struct custom_inventory { //player only
        std::unique_ptr<std::unordered_map<std::string, std::unordered_map<uint32_t, base_objects::slot_data>>> value;

        custom_inventory()
            : value(std::make_unique<std::unordered_map<std::string, std::unordered_map<uint32_t, base_objects::slot_data>>>()) {}

        custom_inventory(custom_inventory&&) noexcept = default;
        custom_inventory& operator=(custom_inventory&&) noexcept = default;

        custom_inventory(const custom_inventory& other) {
            if (other.value)
                value = std::make_unique<std::unordered_map<std::string, std::unordered_map<uint32_t, base_objects::slot_data>>>(*other.value);
        }

        custom_inventory& operator=(const custom_inventory& other) {
            if (this != &other) {
                if (other.value) {
                    if (!value) {
                        value = std::make_unique<std::unordered_map<std::string, std::unordered_map<uint32_t, base_objects::slot_data>>>(*other.value);
                    } else
                        *value = *other.value;
                } else
                    value.reset();
            }
            return *this;
        }

        std::unordered_map<std::string, std::unordered_map<uint32_t, base_objects::slot_data>>& get() {
            return *value;
        }

        const std::unordered_map<std::string, std::unordered_map<uint32_t, base_objects::slot_data>>& get() const {
            return *value;
        }
    };

    struct entity_type {
        int32_t type;

        const api::entity_data& const_data() const {
            return api::entity_data::get_entity(type);
        }

        void tick(ecs::entity self) const {
            auto processor = api::entity_data::get_entity(type).processor;
            if (processor->on_tick)
                processor->on_tick(self);
        }
    };

    struct assigned_player { //player only
        base_objects::client_data_holder player;
    };

    struct spectating_players {
        list_array<base_objects::client_data_holder> players;
    };

    struct world_syncing {
        bit_list_array<> processed_chunks;
        base_objects::cubic_bounds_chunk_radius processing_region;
        uint64_t assigned_world_id = (uint64_t)-1;
        storage::world_data* world = nullptr;
        std::optional<std::string> weak_reference;
        uint32_t attached_to_distance = 0;
        uint32_t inactivity_counter = 0;
        uint16_t keep_alive_ticks = 0; //used for handling entity animation
        bool on_ground : 1 = true;
        bool is_sleeping : 1 = false;
        bool is_sneaking : 1 = false;
        bool is_sprinting : 1 = false;
        bool inactivity_immune : 1 = false; //set if entity type is wither or if nbt set {PersistenceRequired: 1b}
        bool despawn_immune : 1 = false;    //set if entity is not despawning naturally
        enum class state_e : uint8_t {      //this controls inactivity_counter and their ai
            init = 0,
            player_near,
            player_far,
            no_player,             //outside player zone
            scheduled_for_despawn, //after second chance no_player or inactivity_counter reached entitys max counter
        } state : 3
            = state_e::init;

        bool mark_chunk(int64_t pos_x, int64_t pos_z, bool loaded) {
            if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                return false;
            if (!processing_region.in_bounds(pos_x, pos_z))
                return false;

            int64_t offset_x = pos_x - (processing_region.center_x - processing_region.radius);
            int64_t offset_z = pos_z - (processing_region.center_z - processing_region.radius);
            size_t index = offset_z * (processing_region.radius + processing_region.radius + 1) + offset_x;
            processed_chunks.set(index, loaded);
            return true;
        }

        bool chunk_in_bounds(int64_t pos_x, int64_t pos_z) const {
            if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                return false;
            return processing_region.in_bounds(pos_x, pos_z);
        }

        bool chunk_processed(int64_t pos_x, int64_t pos_z) const {
            if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                return false;

            if (!processing_region.in_bounds(pos_x, pos_z))
                return false;
            uint64_t offset_x = pos_x - (processing_region.center_x - processing_region.radius);
            uint64_t offset_z = pos_z - (processing_region.center_z - processing_region.radius);


            size_t index = offset_z * (processing_region.radius + processing_region.radius + 1) + offset_x;
            return processed_chunks.at(index);
        }

        void update_processing(int32_t center_x, int32_t center_z, uint8_t render_distance) {
            if (processing_region.center_x == center_x && processing_region.center_z == center_z && processing_region.radius == render_distance)
                return;
            auto new_processing_diameter = 2 * render_distance + 7;
            bit_list_array<> new_processing_data(new_processing_diameter * new_processing_diameter);

            auto processing_diameter = (processing_region.radius + processing_region.radius + 1);
            int32_t new_radius = new_processing_diameter / 2;

            for (int32_t dz = 0; dz < new_processing_diameter; ++dz) {
                for (int32_t dx = 0; dx < new_processing_diameter; ++dx) {
                    // World coordinates for this chunk in the new area
                    int32_t chunk_x = center_x - new_radius + dx;
                    int32_t chunk_z = center_z - new_radius + dz;

                    // Map to old area offsets
                    int64_t old_offset_x = chunk_x - (processing_region.center_x - processing_region.radius);
                    int64_t old_offset_z = chunk_z - (processing_region.center_z - processing_region.radius);

                    // If the chunk was loaded in the old area, copy its bit
                    if (old_offset_x >= 0 && old_offset_x < processing_diameter && old_offset_z >= 0 && old_offset_z < processing_diameter) {
                        size_t old_index = old_offset_z * processing_diameter + old_offset_x;
                        size_t new_index = dz * new_processing_diameter + dx;
                        if (processed_chunks[old_index])
                            new_processing_data.set(new_index, true);
                    }
                }
            }
            processing_region = {center_x, center_z, new_radius};
            processed_chunks = std::move(new_processing_data);
        }

        void update_render_distance(uint8_t render_distance) {
            update_processing((int32_t)processing_region.center_x, (int32_t)processing_region.center_z, render_distance);
        }

        void flush_processing() {
            auto diameter = processing_region.radius + processing_region.radius + 1;
            processed_chunks = bit_list_array<>(diameter * diameter);
        }

        template <class FN>
        void for_each_processing(FN&& fn) const {
            auto diameter = processing_region.radius + processing_region.radius + 1;
            auto x_offset_pre = processing_region.center_x - processing_region.radius;
            auto z_offset_pre = processing_region.center_z - processing_region.radius;
            processing_region.enum_points_from_center([&](auto x, auto z) {
                fn(x, z, processed_chunks.at((z - z_offset_pre) * diameter + (x - x_offset_pre)));
            });
        }
    };

    struct dead_mark {}; //optional

    struct ride_entity {
        std::optional<entity> other;
    };

    struct ride_by_entity {
        list_array<entity> ride_by;
    };

    struct attached_to {
        std::optional<std::variant<entity, enbt::raw_uuid>> other;
    };

    struct attached {
        list_array<std::variant<entity, enbt::raw_uuid>> ride_by_entity;
    };

    struct effects {
        struct effect {
            uint32_t duration = 0;
            uint32_t id = 0;
            uint8_t amplifier = 1;
            bool ambient : 1 = false;
            bool particles : 1 = true;
            bool show_icon : 1 = true;
            bool use_blend : 1 = false; //for darkness

            auto operator<=>(const effect&) const = default;
        };

        std::unique_ptr<std::unordered_map<uint32_t, list_array<effect>>> hidden_effects_; //effects with lower amplifier than active effect but longer duration
        std::unique_ptr<std::unordered_map<uint32_t, effect>> active_effects_;

        effects() : hidden_effects_(std::make_unique<std::unordered_map<uint32_t, list_array<effect>>>()),
                    active_effects_(std::make_unique<std::unordered_map<uint32_t, effect>>()) {}

        effects(effects&&) noexcept = default;
        effects& operator=(effects&&) noexcept = default;

        effects(const effects& other) {
            if (other.hidden_effects_)
                hidden_effects_ = std::make_unique<std::unordered_map<uint32_t, list_array<effect>>>(*other.hidden_effects_);
            if (other.active_effects_)
                active_effects_ = std::make_unique<std::unordered_map<uint32_t, effect>>(*other.active_effects_);
        }

        effects& operator=(const effects& other) {
            if (this != &other) {
                if (other.hidden_effects_) {
                    if (!hidden_effects_) {
                        hidden_effects_ = std::make_unique<std::unordered_map<uint32_t, list_array<effect>>>(*other.hidden_effects_);
                    } else
                        *hidden_effects_ = *other.hidden_effects_;
                } else if (other.active_effects_) {
                    if (!active_effects_) {
                        active_effects_ = std::make_unique<std::unordered_map<uint32_t, effect>>(*other.active_effects_);
                    } else
                        *active_effects_ = *other.active_effects_;
                } else
                    active_effects_.reset();
            }
            return *this;
        }

        std::unordered_map<uint32_t, list_array<effect>>& hidden_effects() {
            return *hidden_effects_;
        }

        const std::unordered_map<uint32_t, list_array<effect>>& hidden_effects() const {
            return *hidden_effects_;
        }

        std::unordered_map<uint32_t, effect>& active_effects() {
            return *active_effects_;
        }

        const std::unordered_map<uint32_t, effect>& active_effects() const {
            return *active_effects_;
        }
    };

    struct uuid {
        enbt::raw_uuid id;
    };

    struct nbt {
        std::unique_ptr<enbt::compound> value;

        nbt()
            : value(std::make_unique<enbt::compound>()) {}

        nbt(nbt&&) noexcept = default;
        nbt& operator=(nbt&&) noexcept = default;

        nbt(const nbt& other) {
            if (other.value)
                value = std::make_unique<enbt::compound>(*other.value);
        }

        nbt& operator=(const nbt& other) {
            if (this != &other) {
                if (other.value) {
                    if (!value) {
                        value = std::make_unique<enbt::compound>(*other.value);
                    } else
                        *value = *other.value;
                } else
                    value.reset();
            }
            return *this;
        }

        enbt::compound& get() {
            return *value;
        }

        const enbt::compound& get() const {
            return *value;
        }
    };

    struct server_nbt {
        std::unique_ptr<enbt::compound> value;

        server_nbt()
            : value(std::make_unique<enbt::compound>()) {}

        server_nbt(server_nbt&&) noexcept = default;
        server_nbt& operator=(server_nbt&&) noexcept = default;

        server_nbt(const server_nbt& other) {
            if (other.value)
                value = std::make_unique<enbt::compound>(*other.value);
        }

        server_nbt& operator=(const server_nbt& other) {
            if (this != &other) {
                if (other.value) {
                    if (!value) {
                        value = std::make_unique<enbt::compound>(*other.value);
                    } else
                        *value = *other.value;
                } else
                    value.reset();
            }
            return *this;
        }

        enbt::compound& get() {
            return *value;
        }

        const enbt::compound& get() const {
            return *value;
        }
    };

    struct protocol_id {
        int32_t value = 0;
    };

    struct position : public util::vector {
        using util::vector::xyz;
        using util::vector::operator=;
    };

    struct rotation : public util::angle_deg {
        using util::angle_deg::angle_deg;
        using util::angle_deg::operator=;
    };

    struct head_rotation : public util::angle_deg {
        using util::angle_deg::angle_deg;
        using util::angle_deg::operator=;
    };

    struct motion : public util::vector {
        using util::vector::xyz;
        using util::vector::operator=;
    };

    struct on_ground {
        bool value = false;
    };

    struct bounding_box {
        double xz;
        double y;
    };

    struct gravity {
        double value = -0.0784;
    };

    struct virtual_entity {
        bool value = false;
    };

    struct experience {
        int32_t level = 0;
        int32_t exp = 0;

        static int32_t calculate_required_experience(int32_t level) {
            // clang-format off
            int32_t required_exp = level;
            switch(level){
                case  0:case  1:case  2:case  3:case  4:case  5:case 6: case  7:
                case  8:case  9:case 10:case 11:case 12:case 13:case 14:case 15:
                    required_exp = required_exp * 2 + 7;
                    break;
                case 16:case 17:case 18:case 19:case 20:case 21:case 22:case 23:
                case 24:case 25:case 26:case 27:case 28:case 29:case 30:
                    required_exp = required_exp * 5 - 38;
                    break;
                default:
                    required_exp = required_exp * 9 - 158;
                    break;
            }
            // clang-format on
            return required_exp;
        }

        static int32_t calculate_experience_from_level(int32_t level) {
            // clang-format off
            int32_t required_exp = level;
            switch(level){
                case  0:case  1:case  2:case  3:case  4:case  5:case 6: case  7:
                case  8:case  9:case 10:case 11:case 12:case 13:case 14:case 15:
                    required_exp = required_exp * required_exp + 6 * required_exp;
                    break;
                case 16:case 17:case 18:case 19:case 20:case 21:case 22:case 23:
                case 24:case 25:case 26:case 27:case 28:case 29:case 30:case 31:
                    required_exp = int32_t(2.5 * required_exp * required_exp - 40.5 * required_exp + 360);
                    break;
                default:
                    required_exp = int32_t(4.5 * required_exp * required_exp - 162.5 * required_exp + 2220);
                break;
            }
            // clang-format on
            return required_exp;
        }

        int32_t get_level() const {
            return level;
        }

        void set_level(int32_t new_level) {
            int32_t old_lvl = level;
            level = new_level;
            double progress_old = 1.0 / calculate_required_experience(old_lvl) * get_experience();
            set_experience(int32_t(progress_old * calculate_required_experience(new_level)));
        }

        void add_level(int32_t lvl) {
            set_level(get_level() + lvl);
        }

        void reduce_level(int32_t lvl) {
            set_level(get_level() - lvl);
        }

        int32_t get_experience() const {
            return exp;
        }

        void set_experience(int32_t _exp) {
            int32_t add_levels = 0;

            auto required_exp = calculate_required_experience(get_level());
            for (; required_exp <= _exp; required_exp = calculate_required_experience(get_level())) {
                _exp -= required_exp;
                ++add_levels;
            }

            int32_t levels = get_level() + add_levels;
            while (_exp < 0 && levels) {
                levels--;
                _exp += calculate_required_experience(levels);
            }

            exp = _exp;
            level = levels;
        }

        int32_t get_total_experience() {
            return calculate_experience_from_level(level) + exp;
        }

        float get_progress() {
            return float((1.0 / calculate_required_experience(get_level())) * exp);
        }

        void add_experience(int32_t experience) {
            set_experience(get_experience() + experience);
        }

        void reduce_experience(int32_t experience) {
            set_experience(get_experience() - experience);
        }
    };

    struct held_slot {
        uint8_t hotbar_slot;
    };

    struct saturation {
        float value;
    };

    struct food {
        int32_t value;
    };
}
#endif /* SRC_API_ECS_BASE_COMPONENTS */
