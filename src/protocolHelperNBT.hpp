#ifndef SRC_PROTOCOLHELPERNBT
#define SRC_PROTOCOLHELPERNBT

#include "library/enbt.hpp"
class NBT {
    std::vector<uint8_t> nbt_data;

    template <class T>
    void insertValue(T val, size_t max = sizeof(T)) {
        uint8_t* proxy = (uint8_t*)&val;
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back(proxy[i]);
    }

    void insertString(const uint8_t* val, size_t max) {
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back(val[i]);
    }

    void IntegerInsert(ENBT& val, bool typ_ins) {
        switch (val.getTypeLen()) {
        case ENBT::TypeLen::Tiny:
            if (typ_ins)
                nbt_data.push_back(1);
            if (val.getTypeSign())
                nbt_data.push_back((int8_t)val);
            else
                throw std::exception("Unsupported tag");
            break;

        case ENBT::TypeLen::Short:
            if (typ_ins)
                nbt_data.push_back(2);
            if (val.getTypeSign())
                insertValue((int16_t)val);
            else
                throw std::exception("Unsupported tag");
            break;
        case ENBT::TypeLen::Default:
            if (typ_ins)
                nbt_data.push_back(3);
            if (val.getTypeSign())
                insertValue((int32_t)val);
            else
                throw std::exception("Unsupported tag");
            break;
        case ENBT::TypeLen::Long:
            if (typ_ins)
                nbt_data.push_back(4);
            if (val.getTypeSign())
                insertValue((int64_t)val);
            else
                throw std::exception("Unsupported tag");
            break;
        }
    }

    void FloatingInsert(ENBT& val, bool typ_ins) {
        switch (val.getTypeLen()) {
        case ENBT::TypeLen::Default:
            if (typ_ins)
                nbt_data.push_back(5);
            insertValue((float)val);
            break;
        case ENBT::TypeLen::Long:
            if (typ_ins)
                nbt_data.push_back(6);
            insertValue((double)val);
            break;
        }
    }

    void BuildCompound(ENBT& comp) {
        for (const auto& [name, tmp] : comp) {
            if (name.size() > UINT16_MAX)
                throw std::out_of_range("ENBT string too big to fit in NBT");
            InsertType(tmp.type_id());
            insertValue((uint16_t)name.size());
            insertString((uint8_t*)name.data(), name.size());
            RecursiveBuilder(tmp, false);
        }
    }

    void InsertType(ENBT::Type_ID t) {
        switch (t.type) {
        case ENBT::Type::none:
            nbt_data.push_back(0);
            break;
        case ENBT::Type::integer:
            switch (t.length) {
            case ENBT::TypeLen::Tiny:
                nbt_data.push_back(1);
                break;
            case ENBT::TypeLen::Short:
                nbt_data.push_back(2);
                break;
            case ENBT::TypeLen::Default:
                nbt_data.push_back(3);
                break;
            case ENBT::TypeLen::Long:
                nbt_data.push_back(4);
                break;
            }
            break;
        case ENBT::Type::floating:
            switch (t.length) {
            case ENBT::TypeLen::Default:
                nbt_data.push_back(5);
                break;
            case ENBT::TypeLen::Long:
                nbt_data.push_back(6);
                break;
            }
            break;
        case ENBT::Type::sarray:
            nbt_data.push_back(8);
            break;
        case ENBT::Type::array:
            nbt_data.push_back(9);
            break;
        case ENBT::Type::compound:
            nbt_data.push_back(10);
            break;
        default:
            throw std::exception("Unsupported tag");
        }
    }

    void BuildArray(int32_t len, ENBT& arr) {
        insertValue(len);
        for (int32_t i = 0; i < len; i++)
            RecursiveBuilder(arr[i], false);
    }

    void BuildArray(int32_t len, ENBT& enbt, bool insert_type, bool compress) {
        if (enbt[0].getType() == ENBT::Type::integer && compress) {
            if (enbt[0].getTypeLen() != ENBT::TypeLen::Short) {
                if (enbt[0].getTypeLen() == ENBT::TypeLen::Tiny) {
                    if (enbt[0].getTypeSign()) {
                        if (insert_type)
                            nbt_data.push_back(7);
                        if (((int32_t)enbt.size()) != enbt.size())
                            throw std::exception("Unsupported array len");
                        BuildArray(enbt.size(), enbt);
                        return;
                    }
                }
            }
            if (enbt[0].getTypeLen() != ENBT::TypeLen::Default) {
                if (enbt[0].getTypeLen() == ENBT::TypeLen::Tiny) {
                    if (enbt[0].getTypeSign()) {
                        if (insert_type)
                            nbt_data.push_back(11);
                        if (((int32_t)enbt.size()) != enbt.size())
                            throw std::exception("Unsupported array len");
                        BuildArray(enbt.size(), enbt);
                        return;
                    }
                }
            }
            if (enbt[0].getTypeLen() != ENBT::TypeLen::Long) {
                if (enbt[0].getTypeLen() == ENBT::TypeLen::Tiny) {
                    if (enbt[0].getTypeSign()) {
                        if (insert_type)
                            nbt_data.push_back(12);
                        if (((int32_t)enbt.size()) != enbt.size())
                            throw std::exception("Unsupported array len");
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

    void RecursiveBuilder(ENBT& enbt, bool insert_type = true) {
        switch (enbt.getType()) {
        case ENBT::Type::none:
            if (insert_type)
                nbt_data.push_back(0);
            break;
        case ENBT::Type::integer:
            IntegerInsert(enbt, insert_type);
            break;
        case ENBT::Type::floating:
            FloatingInsert(enbt, insert_type);
            break;
        case ENBT::Type::sarray:
            if (insert_type)
                nbt_data.push_back(8);
            if (((uint16_t)enbt.size()) != enbt.size())
                throw std::exception("Unsupported string len");
            insertValue((uint16_t)enbt.size());
            insertString(enbt.getPtr(), enbt.size());
            break;
        case ENBT::Type::array:
            if ((int32_t)enbt.size() > 0)
                BuildArray((int32_t)enbt.size(), enbt, insert_type, true);
            else {
                if (insert_type)
                    nbt_data.push_back(9);
                nbt_data.push_back(0);
                insertValue((int32_t)0);
            }
            break;
        case ENBT::Type::compound:
            BuildCompound(enbt);
            break;
        default:
            throw std::exception("Unsupported tag");
        }
    }

public:
	NBT(ENBT& enbt) {
        RecursiveBuilder(enbt);
        nbt_data.push_back(0);
    }
    NBT(const ENBT& enbt) {
        RecursiveBuilder(const_cast<ENBT&>(enbt));
        nbt_data.push_back(0);
    }
    operator std::vector<uint8_t>() {
		return nbt_data;
	}
};


#endif /* SRC_PROTOCOLHELPERNBT */
