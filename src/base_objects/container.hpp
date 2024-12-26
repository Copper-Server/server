#ifndef SRC_BASE_OBJECTS_CONTAINER
#define SRC_BASE_OBJECTS_CONTAINER
#include <src/base_objects/slot.hpp>
#include <src/registers.hpp>
#include <unordered_map>

namespace copper_server::base_objects {


    class Container {
        std::unordered_map<uint8_t, slot_data> slots;
        uint64_t max_slots : 8;

        uint8_t find(uint8_t slot_id) {
            auto end = slots.end();
            for (auto it = slots.begin(); it != end; it++)
                if (it->second.id == slot_id)
                    return it->first;
            return -1;
        }

        int32_t associated_entity_id;

    public:
        Container(uint8_t MaxSlots) {
            max_slots = MaxSlots;
        }

        Container(const Container& copy) {
            slots = copy.slots;
            max_slots = copy.max_slots;
            associated_entity_id = copy.associated_entity_id;
        }

        Container(Container&& copy) {
            slots = std::move(copy.slots);
            max_slots = copy.max_slots;
            associated_entity_id = copy.associated_entity_id;
        }

        std::pair<bool, uint8_t> free_slot() {
            for (uint8_t i = 0; i < max_slots; i++)
                if (!exist(i))
                    return {true, i};
            return {false, 0};
        }

        bool exist(uint8_t slot_id) {
            return slots.find(slot_id) != slots.end();
        }

        uint8_t usedSlots() {
            return slots.size();
        }

        void clear() {
            slots.clear();
        }

        void clear(int32_t id, size_t count = (size_t)-1) {
            std::list<uint8_t> remove_slots;

            auto end = slots.end();
            for (auto it = slots.begin(); it != end; it++) {
                auto& slot_ = *it;
                if (slot_.second.id == id) {
                    if (count == (size_t)-1)
                        remove_slots.push_back(slot_.first);
                    else if (slot_.second.count <= count) {
                        count -= slot_.second.count;
                        remove_slots.push_back(slot_.first);
                    } else {
                        slot_.second.count -= count;
                        break;
                    }
                }
            }

            for (auto i : remove_slots)
                remove(i);
        }

        template <class _FN>
        void clear(_FN selector) {
            std::list<uint8_t> remove_slots;
            for (uint8_t i = 0; i < used_slots; i++) {
                auto& slot_ = slots[i];
                if (selector(slot_.second))
                    remove_slots.push_back(slot_.first);
            }

            for (auto i : remove_slots)
                remove(i);
        }

        bool add(slot_data item) {
            int64_t to_add = item.count;
            auto item_data_ptr = registers::itemList.find(item.id);
            if (item_data_ptr == registers::itemList.end())
                return false;
            auto& item_data = *item_data_ptr->second;
            if (item_data.max_count == 0)
                return false;

            auto end = slots.end();
            for (auto it = slots.begin(); it != end; it++) {
                auto& slot_ = *it;
                if (to_add <= 0)
                    return true;
                if (slot_.second.id == item.id) {
                    if (slot_.second == item) {
                        int64_t add_res = int64_t(item_data.max_count) - slot_.second.count;
                        if (add_res > 0) {
                            add_res = std::min(add_res, to_add);
                            slot_.second.count += add_res;
                            to_add -= add_res;
                        }
                    }
                }
            }
            if (slots.size() >= max_slots)
                return false;
            auto find_slot = free_slot();
            if (!find_slot.first)
                return false;
            slots[find_slot.second] = item;
            slots[find_slot.second].count = to_add;
            return true;
        }

        bool remove(slot_data item, bool remove_all = false) {
            int64_t it_count = remove_all ? INT64_MAX : item.count;
            std::list<uint8_t> remove_slots;

            auto end = slots.end();
            for (auto it = slots.begin(); it != end; it++) {
                auto& slot_ = *it;
                if (it_count <= 0)
                    break;

                if (slot_.second.id == item.id) {
                    if (slot_.second == item) {
                        int64_t rem_res = slot_.second.count;

                        if (rem_res > 0) {
                            rem_res = std::min(rem_res, it_count);
                            slot_.second.count -= rem_res;
                            it_count -= rem_res;
                            if (slot_.second.count == 0)
                                remove_slots.push_back(slot_.first);
                        }
                    }
                }
            }
            if (remove_slots.size()) {
            remove_slots:
                for (auto rit : remove_slots)
                    remove(rit);
                return true;
            }
            return false;
        }

        void remove(uint8_t slot_id) {
            slots.erase(slot_id);
        }

        void set(uint8_t slot_id, slot it) {
            if (it)
                slots[slot_id] = *it;
            else
                slots.erase(slot_id);
        }
    };
}


#endif /* SRC_BASE_OBJECTS_CONTAINER */
