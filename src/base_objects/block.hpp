#pragma once
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include "adaptived_id.hpp"
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


        uint64_t break_resistance : 25;   //-1 => unbreacable
        uint64_t explode_resistance : 22; //-1 => unexplobable

        uint64_t light_distance : 5;
        uint64_t flamenable : 1;
        uint64_t light_pass : 1;
        uint64_t can_have_nbt : 1;
        uint64_t can_ticked : 1;
        uint64_t can_transmit_redstone : 1;
        uint64_t redstone_streight : 4;
        uint64_t movable : 1;

        const HitBox* hitbox; //if nullptr => non full block

        bool in_block(float x, float y, float z) const {
            return hitbox->in(x, y, z);
        }

        const data::ClientAdaptived client_adaptived(McVersion user_version) const {
            for (uint16_t i = 0; i < variants_count; i++) {
                auto& vari = variants[i];
                if (vari.support_mc_from.Variant == user_version.Variant)
                    if (vari.support_mc_from <= user_version)
                        return vari;
            }
            return {nullptr, 0};
        }

        bool can_explode(uint32_t explode_streight) const {
            if (explode_resistance == explode_resistance_max)
                return false;
            return explode_resistance < explode_streight;
        }

        bool can_break(uint32_t break_streight) const { //use when in client side player break, server need calculate break tick long
            if (break_resistance == break_resistance_max)
                return false;
            return break_resistance < break_streight;
        }

        FullBlockData() {
            break_resistance = explode_resistance = light_distance = flamenable = light_pass = can_have_nbt = 0;
            hitbox = nullptr;
            variants = nullptr;
            variants_count = 0;
        }

        FullBlockData(const data::ClientAdaptivedContainer& ClientAdaptivedContainer, short hitboxID, uint32_t breakResist, uint32_t explodeResist, bool canHaveNbt, bool lightPass, bool Flamenable, uint8_t lightDistance) {
            break_resistance = break_resistance_max < breakResist ? break_resistance_max : breakResist;
            explode_resistance = explode_resistance_max < explodeResist ? explode_resistance_max : explodeResist;
            variants = ClientAdaptivedContainer.begin();
            variants_count = ClientAdaptivedContainer.size();
            hitbox = &HitBox::getHitBox(hitboxID);
            can_have_nbt = canHaveNbt;
            light_pass = lightPass;
            flamenable = Flamenable;
            light_distance = lightDistance;
        }

    private:
        const data::ClientAdaptived* variants;
        uint64_t variants_count : 16;
    };

    typedef uint16_t block_id_t;

    class Block {
        static std::unordered_map<std::string, list_array<Block>> tags;
        static std::unordered_map<block_id_t, FullBlockData> full_block_data;
        static block_id_t block_adder;
        ENBT nbt;

    public:
        block_id_t static addNewBlock(const FullBlockData& new_block) {
            if (block_adder > block_adder + 1)
                throw std::out_of_range("Blocks count out of short range, block cannont added");
            full_block_data[block_adder++] = new_block;
        }

        bool inTagFamily(std::string tag) const {
            if (tags.contains(tag))
                for (auto& it : tags[tag])
                    if (id_and_state_eq(it))
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

        block_id_t id;

        Block(block_id_t block_id, const ENBT& enbt)
            : id(block_id) {
            if (!can_has_nbt())
                throw std::invalid_argument("this block not hold nbt");
            nbt["nbt"] = enbt;
        }

        Block(block_id_t block_id, ENBT&& enbt)
            : id(block_id) {
            if (!can_has_nbt())
                throw std::invalid_argument("this block not hold nbt");
            nbt["nbt"] = std::move(enbt);
        }

        Block(block_id_t block_id = 0)
            : id(block_id) {
            nbt = nullptr;
        }

        Block(const Block& copy) {
            operator=(copy);
        }

        Block(Block&& move_me) noexcept {
            operator=(std::move(move_me));
        }

        Block& operator=(const Block& block) {
            nbt = nbt;
            id = block.id;
            return *this;
        }

        Block& operator=(Block&& block) noexcept {
            nbt = std::move(block.nbt);
            id = block.id;
            return *this;
        }

        const FullBlockData& getStaticData() const {
            return full_block_data[id];
        }

        bool can_has_nbt() const {
            return full_block_data[id].can_have_nbt;
        }

        bool has_nbt() const {
            return nbt.contains("nbt");
        }

        const ENBT& get_nbt() const {
            return nbt["nbt"];
        }

        bool has_state() const {
            return nbt.contains("state");
        }

        bool has_state(const char* state_name) const {
            if (nbt.contains("state"))
                return nbt["state"].contains(state_name);
            return false;
        }

        const ENBT& get_state() const {
            return nbt["state"];
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
            prezent,
            next_tick,
            moved,
            near_changed,
            removed,
            eated
        };

        bool id_and_state_eq(Block& b) const {
            return id == b.id && nbt["state"] == b.nbt["state"];
        }

        bool operator==(const Block& b) const {
            return id == b.id && nbt == b.nbt;
        }

        bool operator!=(const Block& b) const {
            return !operator==(b);
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
            using std::size_t;

            return ((hash<block_id_t>()(k.id) ^ (hash<const void*>()(&k.getStaticData()) << 2)) >> 2) ^ ((hash<const void*>()(&k.get_nbt()) << 1) >> 1) ^ (hash<const void*>()(&k.get_state()) << 1);
        }
    };
}

#pragma pack(pop)