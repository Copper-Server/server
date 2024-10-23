#pragma once
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include "shared_string.hpp"
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#pragma pack(push)
#pragma pack(1)

namespace crafted_craft {
    namespace storage {
        class world_data;
        struct sub_chunk_data;
    }

    namespace base_objects {
        union block;
        typedef uint16_t block_id_t;

        class static_block_data {
            struct block_state_hash {
                size_t operator()(const std::unordered_map<std::string, std::string>& value) const noexcept {
                    size_t result = 0;
                    std::hash<std::string> string_hasher;
                    for (auto& [key, value] : value) {
                        result ^= string_hasher(key) & string_hasher(value);
                    }
                    return result;
                }
            };
        public:


            template <size_t size>
            static consteval uint64_t get_max_uint64_t_value() {
                struct {
                    uint64_t val : size = -1;
                } tmp;
                return tmp.val;
            }


            uint64_t break_resistance : 11;      //-1 => unbreakable, to get real value divide by 100
            uint64_t explode_resistance : 11;    //-1 => unexplodable
            uint64_t move_weight : 7;            //-1 => unmovable
            uint64_t light_pass_block : 6;       //-1 => unpassable
            uint64_t emit_light : 6;             // 0=> do not emit light
            uint64_t flame_resistance : 10;      //-1 => full resist
            uint64_t can_transmit_redstone : 2;  // 0 - no, 1 - yes, 2 - custom logic
            uint64_t emit_redstone_strength : 5; // 0=> do not emit redstone signal
            uint64_t is_block_entity : 1;        // 0=> no, 1=> yes
            uint64_t is_solid : 1;               // 0=> no, 1=> yes, used for height_map
            uint64_t motion_blocking : 1;        // 0=> no, 1=> yes, used for height_map
            uint64_t _unused___ : 3;

            bool can_explode(uint32_t explode_strength) const {
                if (explode_resistance == get_max_uint64_t_value<11>())
                    return false;
                return explode_resistance < explode_strength;
            }

            bool can_break(uint32_t break_strength) const { //use when in client side player break, server need calculate break tick long
                if (break_resistance == get_max_uint64_t_value<11>())
                    return false;
                return break_resistance < break_strength;
            }

            //on tick first checks `is_block_entity` and if true, checks `as_entity_on_tick` if one of them false/undefined then checks `on_tick`, if undefined then do nothing
            std::function<void(storage::world_data&, storage::sub_chunk_data&, block& data, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked)> on_tick;
            std::function<void(storage::world_data&, storage::sub_chunk_data&, block& data, enbt::value& extended_data, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked)> as_entity_on_tick;

            std::unordered_map<std::string, std::unordered_set<std::string>> states;

            boost::bimaps::bimap<
                boost::bimaps::unordered_set_of<block_id_t, std::hash<block_id_t>>,
                boost::bimaps::unordered_set_of<std::unordered_map<std::string, std::string>, block_state_hash>>
                assigned_states;
            block_id_t default_state = 0;
            enbt::compound defintion;
            base_objects::shared_string name;

            list_array<base_objects::shared_string> block_aliases; //string block ids(checks from first to last, if none found in `initialize_blocks()` throws) implicitly uses id first


            enum class tick_opt : uint16_t {
                undefined,
                block_tickable,
                entity_tickable,
                no_tick
            } tickable;

            bool is_tickable() const {
                switch (tickable) {
                case tick_opt::block_tickable:
                case tick_opt::entity_tickable:
                    return true;
                case tick_opt::undefined:
                    if (on_tick)
                        return true;
                    else if (as_entity_on_tick)
                        return true;
                    else
                        return false;
                default:
                case tick_opt::no_tick:
                    return false;
                }
            }

            bool is_tickable() {
                switch (tickable) {
                case tick_opt::block_tickable:
                case tick_opt::entity_tickable:
                    return true;
                case tick_opt::undefined:
                    tickable = resolve_tickable();
                    return tickable != tick_opt::no_tick;
                default:
                case tick_opt::no_tick:
                    return false;
                }
            }

            tick_opt resolve_tickable() const {
                if (on_tick)
                    return tick_opt::block_tickable;
                if (as_entity_on_tick)
                    return tick_opt::entity_tickable;
                return tick_opt::no_tick;
            }

            tick_opt get_tickable() {
                if (tickable == tick_opt::undefined)
                    tickable = resolve_tickable();
                return tickable;
            }

            static_block_data()
                : break_resistance(get_max_uint64_t_value<11>()),
                  explode_resistance(get_max_uint64_t_value<11>()),
                  move_weight(-1),
                  light_pass_block(-1),
                  emit_light(0),
                  flame_resistance(-1),
                  can_transmit_redstone(0),
                  emit_redstone_strength(0),
                  is_block_entity(0),
                  is_solid(0),
                  motion_blocking(0),
                  _unused___(0) {
            }

            static_block_data(uint16_t break_resist, uint16_t explode_resist, uint8_t move_weight, uint8_t light_pass_block, uint8_t emit_light, uint16_t flame_resist, uint8_t can_transmit_redstone, uint8_t emit_redstone_strength, bool is_block_entity, bool is_solid, bool motion_blocking) {
                this->break_resistance = get_max_uint64_t_value<11>() < break_resist ? get_max_uint64_t_value<11>() : break_resist;
                this->explode_resistance = get_max_uint64_t_value<11>() < explode_resist ? get_max_uint64_t_value<11>() : explode_resist;
                this->move_weight = get_max_uint64_t_value<7>() < move_weight ? get_max_uint64_t_value<7>() : move_weight;
                this->light_pass_block = get_max_uint64_t_value<6>() < light_pass_block ? get_max_uint64_t_value<6>() : light_pass_block;
                this->emit_light = get_max_uint64_t_value<6>() < emit_light ? get_max_uint64_t_value<6>() : emit_light;
                this->flame_resistance = get_max_uint64_t_value<10>() < flame_resist ? get_max_uint64_t_value<10>() : flame_resist;
                this->can_transmit_redstone = get_max_uint64_t_value<2>() < can_transmit_redstone ? get_max_uint64_t_value<2>() : can_transmit_redstone;
                this->emit_redstone_strength = get_max_uint64_t_value<5>() < emit_redstone_strength ? get_max_uint64_t_value<5>() : emit_redstone_strength;
                this->is_block_entity = is_block_entity;
                this->is_solid = is_solid;
                this->motion_blocking = motion_blocking;
                this->_unused___ = 0;
            }

            //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
            static void reset_blocks();      //INTERNAL
            static void initialize_blocks(); //INTERNAL, used to assign internal_block_aliases ids from block_aliases


            static std::unordered_map<block_id_t, std::unordered_map<uint32_t, block_id_t>> internal_block_aliases; //local id -> protocol id -> block id
            static std::unordered_map<block_id_t, std::unordered_map<std::string, uint32_t>> internal_block_aliases_protocol;
        };

        union block {
            using tick_opt = static_block_data::tick_opt;

            static void initialize();

            static block_id_t addNewStatelessBlock(static_block_data&& new_block) {
                if (named_full_block_data.contains(new_block.name))
                    throw std::runtime_error("Block with " + new_block.name.get() + " name already defined.");

                struct {
                    block_id_t id : 15;
                } bound_check;

                bound_check.id = full_block_data_.size();
                if (size_t(bound_check.id) != full_block_data_.size())
                    throw std::out_of_range("Blocks count out of range, block can't added");
                auto block_ = std::make_shared<static_block_data>(std::move(new_block));
                named_full_block_data[block_->name] = block_;
                full_block_data_.emplace_back(block_);
                return bound_check.id;
            }

            static void access_full_block_data(std::function<void(std::vector<std::shared_ptr<static_block_data>>&, std::unordered_map<base_objects::shared_string, std::shared_ptr<static_block_data>>&)> access) {
                access(full_block_data_, named_full_block_data);
            }

            static block_id_t addNewStatelessBlock(const static_block_data& new_block) {
                return addNewStatelessBlock(static_block_data(new_block));
            }


            struct {
                base_objects::block_id_t id : 15;
                uint16_t block_state_data : 15;
                tick_opt tickable : 2;
            };

            uint32_t raw;

            block(block_id_t block_id = 0, uint16_t block_state_data = 0)
                : id(block_id), block_state_data(block_state_data), tickable(tick_opt::undefined) {}

            block(const block& copy) {
                operator=(copy);
            }

            block(block&& move_me) noexcept {
                operator=(std::move(move_me));
            }

            block& operator=(const block& block) {
                id = block.id;
                return *this;
            }

            block& operator=(block&& block) noexcept {
                id = block.id;
                return *this;
            }

            const static_block_data& getStaticData() const {
                return *full_block_data_.at(id);
            }

            static const static_block_data& getStaticData(block_id_t id) {
                return *full_block_data_.at(id);
            }

            bool operator==(const block& b) const {
                return id == b.id;
            }

            bool operator!=(const block& b) const {
                return id != b.id;
            }

            void tick(storage::world_data&, storage::sub_chunk_data& sub_chunk, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked);

            static tick_opt resolve_tickable(base_objects::block_id_t block_id);
            bool is_tickable();
            bool is_tickable() const;

            bool is_block_entity() const {
                return getStaticData().is_block_entity;
            }

            static static_block_data& get_block(const base_objects::shared_string& name) {
                return *named_full_block_data.at(name);
            }

        private:
            static std::unordered_map<base_objects::shared_string, std::shared_ptr<static_block_data>> named_full_block_data;
            static std::vector<std::shared_ptr<static_block_data>> full_block_data_;
        };

        struct block_entity {
            block block;
            enbt::value data;
        };

        using full_block_data = std::variant<block, block_entity>;

        struct local_block_pos {
            uint8_t x : 4;
            uint8_t y : 4;
            uint8_t z : 4;
        };

        union compressed_block_state {
            struct {
                uint64_t blockStateId : 52;
                uint64_t blockLocalX : 4;
                uint64_t blockLocalZ : 4;
                uint64_t blockLocalY : 4;
            };

            uint64_t value;
        };

        struct block_hash {
            std::size_t operator()(const block& k) const {
                using std::hash;
                return hash<block_id_t>()(k.id);
            }
        };
    }
}

#pragma pack(pop)
