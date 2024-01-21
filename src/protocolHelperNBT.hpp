#pragma once
#include "enbt.hpp"
class NBT {
	std::vector<uint8_t> nbt_data;
	template<class T>
	void insertValue(T val,size_t max = sizeof(T)) {
		uint8_t* proxy = (uint8_t*)&val;
		for (size_t i = 0; i < max; i++)
			nbt_data.push_back(proxy[i]);
	}
	void insertString(const uint8_t* val, size_t max) {
		for (size_t i = 0; i < max; i++)
			nbt_data.push_back(val[i]);
	}

	void IntegerInsert(ENBT& val, bool typ_ins) {
		switch (val.getLenType()) {
		case ENBT::Type_ID::LenType::Tiny:
			if (typ_ins)
				nbt_data.push_back(1);
			if (val.getTypeLenSigh())
				nbt_data.push_back(std::get<int8_t>(val.getExtern()));
			else
				throw std::exception("Unsuported tag");
			break;

		case ENBT::Type_ID::LenType::Short:
			if (typ_ins)
				nbt_data.push_back(2);
			if (val.getTypeLenSigh())
				insertValue(std::get<int16_t>(val.getExtern()));
			else
				throw std::exception("Unsuported tag");
			break;
		case ENBT::Type_ID::LenType::Default:
			if (typ_ins)
				nbt_data.push_back(3);
			if (val.getTypeLenSigh())
				insertValue(std::get<int32_t>(val.getExtern()));
			else
				throw std::exception("Unsuported tag");
			break;
		case ENBT::Type_ID::LenType::Long:
			if (typ_ins)
				nbt_data.push_back(4);
			if (val.getTypeLenSigh())
				insertValue(std::get<int64_t>(val.getExtern()));
			else
				throw std::exception("Unsuported tag");
			break;
		}
	}
	void FloatingInsert(ENBT& val, bool typ_ins) {
		switch (val.getLenType()) {
		case ENBT::Type_ID::LenType::Default:
			if (typ_ins)
				nbt_data.push_back(5);
			insertValue(std::get<float>(val.getExtern()));
			break;
		case ENBT::Type_ID::LenType::Long:
			if (typ_ins)
				nbt_data.push_back(6);
			insertValue(std::get<double>(val.getExtern()));
			break;
		}
	}

	void BuildCompoud(ENBT& comp) {
		EnbtInterator inter(comp);
		while (inter.canContinue()) {
			const char* name = inter.itemName();
			int16_t name_len = strnlen_s(name, INT16_MAX);
			ENBT& tmp = inter.next();
			InsertType(tmp.type_id());
			insertValue(name_len);
			insertString((uint8_t*)name, name_len);
			RecuirsiveBuilder(tmp, false);
		}
	}
	void InsertType(ENBT::Type_ID t) {
		switch (t.type)
		{
		case ENBT::Type_ID::Type::none:
				nbt_data.push_back(0);
			break;
		case ENBT::Type_ID::Type::integer:
			switch (t.length) {
			case ENBT::Type_ID::LenType::Tiny:
				nbt_data.push_back(1);
				break;
			case ENBT::Type_ID::LenType::Short:
				nbt_data.push_back(2);
				break;
			case ENBT::Type_ID::LenType::Default:
				nbt_data.push_back(3);
				break;
			case ENBT::Type_ID::LenType::Long:
				nbt_data.push_back(4);
				break;
			}
			break;
		case ENBT::Type_ID::Type::floating:
			switch (t.length) {
			case ENBT::Type_ID::LenType::Default:
				nbt_data.push_back(5);
				break;
			case ENBT::Type_ID::LenType::Long:
				nbt_data.push_back(6);
				break;
			}
			break;
		case ENBT::Type_ID::Type::utf8_str:
			nbt_data.push_back(8);
			break;
		case ENBT::Type_ID::Type::array:
			nbt_data.push_back(9);
			break;
		case ENBT::Type_ID::Type::compound:
			nbt_data.push_back(10);
			break;
		default:
			throw std::exception("Unsuported tag");
		}
	}
	void BuildArray(int32_t len, ENBT& arr) {
		insertValue(len);
		for (int32_t i = 0; i < len; i++) 
			RecuirsiveBuilder(arr[i],false);
	}
	void BuildArray(int32_t len, ENBT& enbt,bool insert_type,bool commpress) {
		if (enbt[0].getType() == ENBT::Type_ID::Type::integer && commpress) {
			if (enbt[0].getLenType() != ENBT::Type_ID::LenType::Short) {
				if (enbt[0].getLenType() == ENBT::Type_ID::LenType::Tiny) {
					if (enbt[0].getTypeLenSigh()) {
						if (insert_type)
							nbt_data.push_back(7);
						if (((int32_t)enbt.size()) != enbt.size())
							throw std::exception("Unsuported array len");
						BuildArray(enbt.size(), enbt);
						return;
					}
				}
			}
			if (enbt[0].getLenType() != ENBT::Type_ID::LenType::Default) {
				if (enbt[0].getLenType() == ENBT::Type_ID::LenType::Tiny) {
					if (enbt[0].getTypeLenSigh()) {
						if (insert_type)
							nbt_data.push_back(11);
						if (((int32_t)enbt.size()) != enbt.size())
							throw std::exception("Unsuported array len");
						BuildArray(enbt.size(), enbt);
						return;
					}
				}
			}
			if (enbt[0].getLenType() != ENBT::Type_ID::LenType::Long) {
				if (enbt[0].getLenType() == ENBT::Type_ID::LenType::Tiny) {
					if (enbt[0].getTypeLenSigh()) {
						if (insert_type)
							nbt_data.push_back(12);
						if (((int32_t)enbt.size()) != enbt.size())
							throw std::exception("Unsuported array len");
						BuildArray(enbt.size(), enbt);
						return;
					}
				}
			}
		}
		if (insert_type)
			nbt_data.push_back(9);
		InsertType(enbt[0].type_id());
		BuildArray(enbt.size(), enbt);
	}


	void RecuirsiveBuilder(ENBT& enbt,bool insert_type = true) {
		switch (enbt.getType())
		{
		case ENBT::Type_ID::Type::none:
			if(insert_type)
				nbt_data.push_back(0);
			break; 
		case ENBT::Type_ID::Type::integer:
			IntegerInsert(enbt, insert_type);
			break;
		case ENBT::Type_ID::Type::floating:
			FloatingInsert(enbt, insert_type);
			break;
		case ENBT::Type_ID::Type::utf8_str:
			if (insert_type)
				nbt_data.push_back(8);
			if(((uint16_t)enbt.size()) != enbt.size())
				throw std::exception("Unsuported string len");
			insertValue((uint16_t)enbt.size());
			insertString(enbt.getPtr(), enbt.size());
			break;
		case ENBT::Type_ID::Type::array:
			if ((int32_t)enbt.size() > 0) 
				BuildArray((int32_t)enbt.size(), enbt, insert_type, true);
			else {
				if (insert_type)
					nbt_data.push_back(9);
				nbt_data.push_back(0);
				insertValue((int32_t)0);
			}
			break;
		case ENBT::Type_ID::Type::compound:
			BuildCompoud(enbt);
			break;
		default:
			throw std::exception("Unsuported tag");
		}
	}
public:
	NBT(ENBT& enbt) {
		RecuirsiveBuilder(enbt);
		nbt_data.push_back(0);
	}
	NBT(const ENBT& enbt) {
		RecuirsiveBuilder(const_cast<ENBT&>(enbt));
		nbt_data.push_back(0);
	}
	operator std::vector<uint8_t>() {
		return nbt_data;
	}
};