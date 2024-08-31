#pragma once
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#pragma pack(push)
#pragma pack(1)

namespace crafted_craft {
    namespace storage {
        class world_data;
        struct sub_chunk_data;
        union block_data;
    }

    namespace base_objects {
        struct block_entity;

        struct full_block_data {
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
            uint64_t light_pass : 6;             //-1 => unpassable
            uint64_t emit_light : 6;             // 0=> do not emit light
            uint64_t flame_resistance : 10;      //-1 => full resist
            uint64_t can_transmit_redstone : 2;  // 0 - no, 1 - yes, 2 - custom logic
            uint64_t emit_redstone_strength : 5; // 0=> do not emit redstone signal
            uint64_t _unused___ : 6;

            bool in_block(float x, float y, float z) const {
                return false; // hitbox->in(x, y, z);
            }

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

            std::function<void(storage::world_data&, storage::sub_chunk_data&, storage::block_data& data, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z)> on_tick;
            std::function<void(storage::world_data&, storage::sub_chunk_data&, block_entity& data, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z)> as_entity_on_tick;

            full_block_data()
                : break_resistance(get_max_uint64_t_value<11>()),
                  explode_resistance(get_max_uint64_t_value<11>()),
                  move_weight(-1),
                  light_pass(-1),
                  emit_light(0),
                  flame_resistance(-1),
                  can_transmit_redstone(0),
                  emit_redstone_strength(0),
                  _unused___(0) {
            }

            full_block_data(uint16_t breakResist, uint16_t explodeResist, uint8_t moveWeight, uint8_t lightPass, uint8_t emitLight, uint16_t flameResist, uint8_t canTransmitRedstone, uint8_t emitRedstoneStrength) {
                break_resistance = get_max_uint64_t_value<11>() < breakResist ? get_max_uint64_t_value<11>() : breakResist;
                explode_resistance = get_max_uint64_t_value<11>() < explodeResist ? get_max_uint64_t_value<11>() : explodeResist;
                move_weight = get_max_uint64_t_value<7>() < moveWeight ? get_max_uint64_t_value<7>() : moveWeight;
                light_pass = get_max_uint64_t_value<6>() < lightPass ? get_max_uint64_t_value<6>() : lightPass;
                emit_light = get_max_uint64_t_value<6>() < emitLight ? get_max_uint64_t_value<6>() : emitLight;
                flame_resistance = get_max_uint64_t_value<10>() < flameResist ? get_max_uint64_t_value<10>() : flameResist;
                can_transmit_redstone = get_max_uint64_t_value<2>() < canTransmitRedstone ? get_max_uint64_t_value<2>() : canTransmitRedstone;
                emit_redstone_strength = get_max_uint64_t_value<5>() < emitRedstoneStrength ? get_max_uint64_t_value<5>() : emitRedstoneStrength;
                _unused___ = 0;
            }

        private:
        };

        typedef uint16_t block_id_t;

        class block {
            static std::unordered_map<std::string, list_array<block>> tags;
            static std::unordered_map<block_id_t, full_block_data> full_block_data_;
            static block_id_t block_adder;

        public:
            static void initialize();

            static block_id_t addNewBlock(const full_block_data& new_block) {
                struct {
                    block_id_t id : 15;
                    block_id_t max_id : 15 = 0x7FFF;
                } bound_check;

                bound_check.id = block_adder + 1;
                if (bound_check.id == bound_check.max_id)
                    throw std::out_of_range("Blocks count out of range, block can't added");
                full_block_data_[block_adder++] = new_block;
            }

            bool inTagFamily(std::string tag) const {
                if (tags.contains(tag))
                    for (auto& it : tags[tag])
                        if (it == *this)
                            return true;
                return false;
            }

            void joinTagFamily(std::string tag) {
                auto& it = tags[tag];
                if (std::find(it.begin(), it.end(), *this) == it.end())
                    it.push_back(*this);
            }

            void outTagFamily(std::string tag) {
                auto& it = tags[tag];
                it.remove_one([this](auto& it) { return it == *this; });
            }

            block_id_t id : 15;

            block(block_id_t block_id = 0)
                : id(block_id) {
            }

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

            const full_block_data& getStaticData() const {
                return full_block_data_[id];
            }

            int16_t getMoveWeight() const {
                return full_block_data_[id].move_weight;
            }

            struct TickAnswer {
                enum class Behavior : uint8_t {
                    done,
                    _break,
                    _remove,
                    break_box,
                    remove_box,

                    add_to_be_ticked_next,

                    check_can_move_and_move_blocks,
                    check_can_move_and_move_blocks_then_add_new_blocks,

                    move_blocks,
                    move_blocks_then_add_new_blocks,

                    update_now,
                    update_now_except_ourself,
                    update_now_except_ticked,

                    clone
                };
                enum class PosMode : uint8_t {
                    as_pos,
                    as_box,
                    as_tunnels
                };
                Behavior behavior : 4;
                PosMode mode : 2;
                uint8_t x : 4;
                uint8_t y : 4;
                uint8_t z : 4;
                uint8_t X : 4;
                uint8_t Y : 4;
                uint8_t Z : 4;
                int8_t move_x : 5;
                int8_t move_y : 5;
                int8_t move_z : 5;
                block* block;

                TickAnswer(Behavior beh, PosMode pm = PosMode::as_pos, uint8_t xbs = 0, uint8_t ybs = 0, uint8_t zbs = 0, uint8_t Xbs = 0, uint8_t Ybs = 0, uint8_t Zbs = 0, uint8_t mx = 0, uint8_t my = 0, uint8_t mz = 0) {
                    behavior = beh;
                    mode = pm;
                    x = xbs;
                    y = ybs;
                    z = zbs;
                    X = Xbs;
                    Y = Ybs;
                    Z = Zbs;
                    move_x = mx;
                    move_y = my;
                    move_z = mz;
                    block = nullptr;
                }

                TickAnswer(Behavior beh, base_objects::block bl, PosMode pm = PosMode::as_tunnels, uint8_t xbs = 0, uint8_t ybs = 0, uint8_t zbs = 0, uint8_t Xbs = 0, uint8_t Ybs = 0, uint8_t Zbs = 0, uint8_t mx = 0, uint8_t my = 0, uint8_t mz = 0) {
                    behavior = beh;
                    mode = pm;
                    x = xbs;
                    y = ybs;
                    z = zbs;
                    X = Xbs;
                    Y = Ybs;
                    Z = Zbs;
                    move_x = mx;
                    move_y = my;
                    move_z = mz;
                    block = new base_objects::block(bl);
                }
            };
            enum class TickReason : uint8_t {
                present,
                next_tick,
                moved,
                near_changed,
                removed,
                eaten
            };

            bool operator==(const block& b) const {
                return id == b.id;
            }

            bool operator!=(const block& b) const {
                return id != b.id;
            }

            TickAnswer Tick(TickReason reason) {
                return TickAnswer(TickAnswer::Behavior::done);
            }
        };

        struct local_block_pos {
            uint8_t x : 4;
            uint8_t y : 4;
            uint8_t z : 4;
        };

        struct block_entity {
            ENBT nbt;
            int32_t type;
            block block_id;
            int8_t x : 4;
            int8_t y : 4;
            int8_t z : 4;
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
