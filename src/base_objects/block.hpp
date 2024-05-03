#pragma once
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include "hitbox.hpp"
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#pragma pack(push)
#pragma pack(1)

namespace crafted_craft {
    struct FullBlockData {
        static constexpr uint64_t break_resistance_max = []() {
            struct {
                uint64_t val : 25;
            } tmp;

            tmp.val = -1;
            return tmp.val;
        }();
        static constexpr uint64_t explode_resistance_max = []() {
            struct {
                uint64_t val : 22;
            } tmp;

            tmp.val = -1;
            return tmp.val;
        }();


        uint64_t break_resistance : 25;   //-1 => unbreakable
        uint64_t explode_resistance : 22; //-1 => un-explodable

        uint64_t light_distance : 5;
        uint64_t flammable : 1;
        uint64_t light_pass : 1;
        uint64_t can_have_nbt : 1;
        uint64_t can_ticked : 1;
        uint64_t can_transmit_redstone : 1;
        uint64_t redstone_strength : 4;
        uint64_t movable : 1;

        const HitBox* hitbox; //if nullptr => non full block, if -1 => full block, other => custom hitbox

        bool in_block(float x, float y, float z) const {
            return hitbox->in(x, y, z);
        }

        bool can_explode(uint32_t explode_strength) const {
            if (explode_resistance == explode_resistance_max)
                return false;
            return explode_resistance < explode_strength;
        }

        bool can_break(uint32_t break_strength) const { //use when in client side player break, server need calculate break tick long
            if (break_resistance == break_resistance_max)
                return false;
            return break_resistance < break_strength;
        }

        FullBlockData() {
            break_resistance = explode_resistance = light_distance = flammable = light_pass = can_have_nbt = 0;
            hitbox = nullptr;
        }

        FullBlockData(short hitboxID, uint32_t breakResist, uint32_t explodeResist, bool canHaveNbt, bool lightPass, bool Flammable, uint8_t lightDistance) {
            break_resistance = break_resistance_max < breakResist ? break_resistance_max : breakResist;
            explode_resistance = explode_resistance_max < explodeResist ? explode_resistance_max : explodeResist;
            hitbox = &HitBox::getHitBox(hitboxID);
            can_have_nbt = canHaveNbt;
            light_pass = lightPass;
            flammable = Flammable;
            light_distance = lightDistance;
        }
    };

    typedef uint16_t block_id_t;

    class Block {
        static std::unordered_map<std::string, list_array<Block>> tags;
        static std::unordered_map<block_id_t, FullBlockData> full_block_data;
        static block_id_t block_adder;

    public:
        block_id_t static addNewBlock(const FullBlockData& new_block) {
            struct {
                block_id_t id : 15;
            } bound_check;

            bound_check.id = block_adder + 1;
            if (bound_check.id == 0)
                throw std::out_of_range("Blocks count out of short range, block can't added");
            full_block_data[block_adder++] = new_block;
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

        Block(block_id_t block_id = 0)
            : id(block_id) {
        }

        Block(const Block& copy) {
            operator=(copy);
        }

        Block(Block&& move_me) noexcept {
            operator=(std::move(move_me));
        }

        Block& operator=(const Block& block) {
            id = block.id;
            return *this;
        }

        Block& operator=(Block&& block) noexcept {
            id = block.id;
            return *this;
        }

        const FullBlockData& getStaticData() const {
            return full_block_data[id];
        }

        bool can_has_nbt() const {
            return full_block_data[id].can_have_nbt;
        }

        bool canMove() {
            return full_block_data[id].movable;
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
            Block* block;

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

            TickAnswer(Behavior beh, Block bl, PosMode pm = PosMode::as_tunnels, uint8_t xbs = 0, uint8_t ybs = 0, uint8_t zbs = 0, uint8_t Xbs = 0, uint8_t Ybs = 0, uint8_t Zbs = 0, uint8_t mx = 0, uint8_t my = 0, uint8_t mz = 0) {
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
                block = new Block(bl);
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

        bool operator==(const Block& b) const {
            return id == b.id;
        }

        bool operator!=(const Block& b) const {
            return id != b.id;
        }

        TickAnswer Tick(TickReason reason) {
            if (full_block_data[id].can_ticked) {
                return TickAnswer(TickAnswer::Behavior::done);
            } else
                return TickAnswer(TickAnswer::Behavior::done);
        }
    };

    union CompressedBlockState {
        struct {
            uint64_t blockStateId : 52;
            uint64_t blockLocalX : 4;
            uint64_t blockLocalZ : 4;
            uint64_t blockLocalY : 4;
        };

        uint64_t value;
    };

    struct BlockHash {
        std::size_t operator()(const Block& k) const {
            using std::hash;
            return hash<block_id_t>()(k.id);
        }
    };
}

#pragma pack(pop)