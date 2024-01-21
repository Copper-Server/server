#pragma once
#include <unordered_map>
#include "item.hpp"
namespace mcCore {
	class Container {
		std::pair< uint8_t,item>* slot;
		uint64_t used_slots:8;
		uint64_t max_slots : 8;
		void add_size() {
			std::pair< uint8_t, item>* new_item = new std::pair< uint8_t, item>[used_slots+1];
			if (used_slots == 0) {
				used_slots++;
				slot = new_item;
			}
			for (uint8_t i = 0; i < used_slots; i++)
				new_item[i] = slot[i];
			delete[] slot;
			slot = new_item;
			used_slots++;
		}
		uint8_t find(uint8_t slot_id) {
			for (uint8_t i = 0; i < used_slots; i++)
				if (slot->first == slot_id)
					return i;
			return -1;
		}


		uint64_t associated_entity_id;
		uint64_t associated_entity_id;

	public:
		Container(uint8_t MaxSlots) {
			max_slots = MaxSlots;
		}
		Container(const Container& copy) {
			used_slots = copy.used_slots;
			std::pair< uint8_t, item>* slot = new std::pair< uint8_t, item>[used_slots];
			for (uint8_t i = 0; i < used_slots; i++)
				slot[i] = copy.slot[i];
			max_slots = copy.max_slots;
			associated_entity_id = copy.associated_entity_id;
		}
		~Container() {
			if (slot)
				delete slot;
		}
		std::pair<bool,uint8_t> free_slot() {
			for (uint8_t i = 0; i < max_slots; i++)
				if (!exist(i))
					return { true,i };
			return { false,0 };
		}

		bool exist(uint8_t slot_id) {
			for (uint8_t i = 0; i < used_slots; i++)
				if (slot->first == slot_id)
					return true;
			return false;
		}
		uint8_t usedSlots() {
			return used_slots;
		}
		void clear() {
			used_slots = 0;
			if (slot)
				delete slot;
		}
		void clear(item it) {
		rem_it:
			std::list<uint8_t> remove_slots;

			for (uint8_t i = 0; i < used_slots; i++) {
				auto& items = slot[i];
				if (items.second == it)
					remove_slots.push_back(items.first);
			}


			for (auto i : remove_slots)
				remove(i);
		}
		bool add(item it) {
			int64_t it_count = it.stack;
			for (uint8_t i = 0; i < used_slots; i++){
				auto& items = slot[i];
				if (it_count <= 0)
					return true;
				if (items.second.is_equal_type(it)) {
					int64_t add_res = (int64_t)(items.second.getItemData().max_stack - items.second.stack);
					if (add_res>0) {
						if (add_res - it_count >= 0) {
							items.second.stack += it_count;
							return true;
						}
						else {
							items.second.stack += add_res;
							it_count -= add_res;
						}
					}
				}
			}
			if (used_slots >= max_slots)
				return false;
			auto find_slot = free_slot();
			if (!find_slot.first)
				return false;
			set(find_slot.second,it);
			return true;
		}
		bool remove(item it) {
			int64_t it_count = it.stack;
			std::list<uint8_t> remove_slots;

			for(uint8_t i =0;i< used_slots;i++){
				auto& items = slot[i];
				if (it_count <= 0)
					goto remove_items;

				if (items.second.is_equal_type(it)) {
					int64_t rem_res = items.second.stack;

					if (rem_res > 0) {
						if (rem_res - it_count >= 0) {
							if((items.second.stack -= it_count )==0)
								remove_slots.push_back(items.first);
							goto remove_items;
						}
						else if (it_count - rem_res > 0) {
							items.second.stack += rem_res;
							remove_slots.push_back(items.first);
							it_count -= rem_res;
						}
					}
				}
			}
			if (remove_slots.size()) {
			remove_items:
				for (auto rit : remove_slots)
					remove(rit);
				return true;
			}


			return false;
		}
		void remove(uint8_t slot_id) {
			if (!exist(slot_id))
				return;
			if (!--used_slots) {
				delete[] slot;
				return;
			}

			std::pair< uint8_t, item>* new_item = new std::pair< uint8_t, item>[used_slots];
			
			uint8_t j = 0;
			for (uint8_t i = 0; i <= used_slots; i++)
				if (slot[i].first != slot_id)
					new_item[j++] = slot[i];
			delete[] slot;
			slot = new_item;
		}
		void set(uint8_t slot_id, item it) {
			if (slot_id >= max_slots)
				return;
			else if (exist(slot_id)) {
				slot[find(slot_id)] = { slot_id,it };
			}
			else if(used_slots + 1 < max_slots){
				add_size();
				slot[used_slots++] = { slot_id,it };
			}
		}
	};
}