#pragma once
#include <string>
#include <unordered_map>
#include "entity.hpp"
#include "adaptived_id.hpp"
namespace mcCore {



	union item_use_data {
		uint32_t none;
		struct {
			uint16_t hunger_resore;
			uint16_t saturation_restore;
		} eat;
		uint32_t summmon_command_id;

	};
	class item_data {
		enum class UseMode : uint64_t;
		static std::unordered_map<uint32_t, const char*> use_summon_command;
		const data::ClientAdaptived* variants;
		uint64_t furnace_product_id : 32;	//-1 -> cannont use in furnace
		uint64_t furnace_burn_time : 15;
		uint64_t use_data : 32;

		uint64_t variants_count : 16;

		item_data(int32_t FurnaceProductID, uint16_t FurnaceBurnTime, bool CanBurn, uint8_t MaxStack, const data::ClientAdaptivedContainer& NamedVariants, UseMode UseMode, item_use_data UseData, uint16_t use_coldown) {
			furnace_product_id = FurnaceProductID;
			furnace_burn_time = FurnaceBurnTime;
			variants_count = NamedVariants.size();
			variants = NamedVariants.begin();
			use_mode = UseMode;
			use_data = UseData.none;
			use_couldown = use_coldown;
			can_burn = CanBurn;
			max_stack = MaxStack;
		}
		item_data() {
			variants = nullptr;
			furnace_product_id = -1;
			furnace_burn_time = 0;
			variants_count = 0;
			use_mode = UseMode::none;
			use_data = 0;
			use_couldown = 0;
			can_burn = 0;
			max_stack = 0;
		}
		static std::unordered_map<uint32_t, item_data> all_items;
		static uint32_t item_adder_data;
	public:
		static item_data undefined;
		UseMode use_mode : 2;
		uint64_t max_stack : 8;
		uint64_t use_couldown : 16;
		uint64_t can_burn : 1;

		enum class UseMode : uint64_t {
			none,
			eat,
			spawn_entity
		};

		bool is_undefined() const {
			return
				use_mode == UseMode::none &&
				furnace_product_id == -1 &&
				!furnace_burn_time &&
				!variants &&
				!variants_count &&
				!use_data &&
				!use_couldown &&
				!can_burn &&
				!max_stack
				;
		}

		const const data::ClientAdaptived& namedID(McVersion user_version) {
			for (uint16_t i = 0; i < variants_count; i++)
				if(variants[i].support_mc_from.Variant == user_version.Variant)
					if (variants[i].support_mc_from <= user_version)
						return variants[i];
		}

		item_use_data UseData() const{
			item_use_data res;
			res.none = use_data;
			return res;
		}
		const char const* UseSummonCommand() {
			if (use_mode != UseMode::spawn_entity)
				return nullptr;
			return use_summon_command[UseData().summmon_command_id];
		}

		static item_data& getItemData(uint32_t id) {
			return all_items[id];
		}
		static uint32_t CreateItem(int32_t FurnaceProductID, uint16_t FurnaceBurnTime, bool CanBurn, uint8_t MaxStack, const data::ClientAdaptivedContainer& NamedVariants, UseMode UseMode, item_use_data UseData, uint16_t use_coldown) {
			all_items[item_adder_data] = { FurnaceProductID, FurnaceBurnTime, CanBurn, MaxStack, NamedVariants, UseMode, UseData, use_coldown };
			return item_adder_data++;
		}
	};
	uint32_t item_data::item_adder_data = 0;
	std::unordered_map<uint32_t, item_data> item_data::all_items;
	std::unordered_map<uint32_t, const char*> use_summon_command;
	item_data item_data::undefined;


	class item {
		char* nbt;
		uint64_t id:32;
	public:
		item() {
			nbt = nullptr;
			id = 0;
			stack = 0;
		}
		item(uint32_t ID, uint8_t STACK, const char const* NBT = nullptr,size_t nbt_len = 0) {
			id = ID;
			stack = STACK;
			if (NBT) {
				char* nbt = new char[nbt_len];
				strcpy_s(nbt, nbt_len, NBT);
			}
			else
				nbt = nullptr;
		}
		~item() {
			if(nbt)
				delete[] nbt;
		}
		uint32_t ID() {
			return id;
		}
		uint64_t stack : 8;
		item_data& getItemData() {
			return item_data::getItemData(id);
		}
		bool is_max() {
			return item_data::getItemData(id).max_stack > stack;
		}
		item_data::UseMode UseMode() {
			return item_data::getItemData(id).use_mode;
		}
		const char const* getNbt() {
			return nbt;
		}
		void set_nbt(const char const* NBT, size_t nbt_len) {
			if (NBT) {
				char* nbt = new char[nbt_len];
				strcpy_s(nbt, nbt_len, NBT);
			}
			else if(nbt)
			{
				delete[] nbt;
				nbt = nullptr;
			}
		}

		bool is_equal_type(item it) {
			if (!nbt == !it.nbt && id == it.id)
			{
				if (nbt)
					if (strcmp(nbt, it.nbt))
						return false;
				return true;
			}
			else
				return false;
		}

		bool operator==(item it) {
			if (stack != it.stack)
				return false;
			return is_equal_type(it);
		}
	};
}