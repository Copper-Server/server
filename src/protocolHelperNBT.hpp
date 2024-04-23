#ifndef SRC_PROTOCOLHELPERNBT
#define SRC_PROTOCOLHELPERNBT

#include "library/enbt.hpp"
#include "library/list_array.hpp"

//bridge class between ENBT and NBT formats
class NBT {
    list_array<uint8_t> nbt_data;

#pragma region ENBT_TO_NBT
    template <class T>
    void insertValue(T val, size_t max = sizeof(T)) {
        val = ENBT::ConvertEndian(std::endian::big, val);
        uint8_t* proxy = (uint8_t*)&val;
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back(proxy[i]);
    }

    void insertString(const char* val, size_t max) {
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back((uint8_t)val[i]);
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

    void BuildCompound(const std::string& c_name, ENBT& comp) {
        insertValue((uint16_t)c_name.size());
        insertString(c_name.data(), c_name.size());
        for (const auto& [name, tmp] : comp) {
            if (name.size() > UINT16_MAX)
                throw std::out_of_range("ENBT string too big to fit in NBT");
            InsertType(tmp.type_id());
            if (tmp.getType() == ENBT::Type::compound) {
                RecursiveBuilder(tmp, false, name);
            } else {
                insertValue((uint16_t)name.size());
                insertString(name.data(), name.size());
                RecursiveBuilder(tmp, false, "");
            }
        }
        InsertType(ENBT::Type::none);
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
            default:
                throw std::exception("Unsupported tag");
            }
            break;
        case ENBT::Type::string:
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
            RecursiveBuilder(arr[i], false, "");
    }

    void BuildArray(int32_t len, ENBT& enbt, bool insert_type, bool compress) {
        if (!enbt.size()) {
            if (insert_type)
                nbt_data.push_back(9);
            InsertType(ENBT::Type::none);
            insertValue(0);
            return;
        }
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

    void RecursiveBuilder(ENBT& enbt, bool insert_type, const std::string& name) {
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
        case ENBT::Type::string: {
            if (insert_type)
                nbt_data.push_back(8);
            std::string& str = enbt;
            if (((uint16_t)str.size()) != str.size())
                throw std::exception("Unsupported string len");
            insertValue((uint16_t)str.size());
            insertString(str.data(), str.size());
            break;
        }
        case ENBT::Type::array:
            BuildArray((int32_t)enbt.size(), enbt, insert_type, true);
            break;
        case ENBT::Type::compound:
            if (insert_type)
                nbt_data.push_back(10);
            BuildCompound(name, enbt);
            break;
        default:
            throw std::exception("Unsupported tag");
        }
    }

#pragma endregion
#pragma region NBT_TO_ENBT

    template <class T>
    static T uncheckedExtractValue(const uint8_t* data, size_t& i) {
        uint8_t tmp[sizeof(T)];
        for (size_t j = 0; j < sizeof(T); j++)
            tmp[j] = data[i++];
        return ENBT::ConvertEndian(std::endian::big, *(T*)tmp);
    }

    template <class T>
    static T extractValue(const uint8_t* data, size_t& i, size_t max_size) {
        if (i + sizeof(T) >= max_size)
            throw std::out_of_range("Out of bounds");
        return uncheckedExtractValue<T>(data, i);
    }

    template <class T>
    static ENBT extractArray(const uint8_t* data, size_t& i, size_t max_size) {
        int32_t len = extractValue<int32_t>(data, i, max_size);
        if (i + sizeof(T) * len >= max_size)
            throw std::out_of_range("Out of bounds");
        std::vector<ENBT> ret;
        ret.reserve(len);
        for (int32_t j = 0; j < len; j++)
            ret.push_back(uncheckedExtractValue<T>(data, i));
        return ENBT(ret, ENBT::Type_ID(ENBT::Type::array, ENBT::TypeLen::Default));
    }

    static ENBT RecursiveExtractor_1(uint8_t type, const uint8_t* data, size_t& i, size_t max_size) {
        switch (type) {
        case 0: //end
            return ENBT();
        case 1: //byte
            return ENBT(extractValue<int8_t>(data, i, max_size));
        case 2: //short
            return ENBT(extractValue<int16_t>(data, i, max_size));
        case 3: //int
            return ENBT(extractValue<int32_t>(data, i, max_size));
        case 4: //long
            return ENBT(extractValue<int64_t>(data, i, max_size));
        case 5: //float
            return ENBT(extractValue<float>(data, i, max_size));
        case 6: //double
            return ENBT(extractValue<double>(data, i, max_size));
        case 7: //byte array
            return extractArray<int8_t>(data, i, max_size);
        case 8: { //string
            uint16_t length = extractValue<uint16_t>(data, i, max_size);
            if (i + length >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length;
            return ENBT((const char*)data, length);
        }
        case 9: { //list
            uint8_t type = data[i++];
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (length < 0)
                length = 0;
            std::vector<ENBT> res;
            res.reserve(length);
            for (int32_t iterate = 0; iterate < length; iterate++)
                res.push_back(RecursiveExtractor_1(type, data, i, max_size));
            return ENBT(res, ENBT::Type_ID(ENBT::Type::array, ENBT::TypeLen::Default));
        }
        case 10: { //compound
            std::unordered_map<std::string, ENBT> compound;
            while (*data) {
                uint8_t type = data[i++];
                uint16_t length = extractValue<uint16_t>(data, i, max_size);
                if (i + length >= max_size)
                    throw std::out_of_range("Out of bounds");
                std::string res(data, data + length);
                i += length;
                compound[res] = RecursiveExtractor_1(type, data, i, max_size);
            }
            return compound;
        }
        case 11: { //int array
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (i + length * 4 >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length * 4;
            return ENBT((int32_t*)data, length, std::endian::big, true);
        }
        case 12: { //long array
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (i + length * 8 >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length * 8;
            return ENBT((int64_t*)data, length, std::endian::big, true);
        }
        default:
            throw std::exception("Invalid type");
        }
    }

    static ENBT RecursiveExtractor(const uint8_t* data, size_t& i, size_t max_size) {
        if (max_size == 0)
            return ENBT();
        if (data[0] == 10) {
            //skip first base compound name tag
            i++;
            i += extractValue<uint16_t>(data, i, max_size);
            return RecursiveExtractor_1(10, data, i, max_size);
        }
        return RecursiveExtractor_1(data[i++], data, i, max_size);
    }


public:
    NBT(const ENBT& enbt, const std::string& entry_name = "", bool align_entry = false) {
        RecursiveBuilder(const_cast<ENBT&>(enbt), true, entry_name);
    }

    NBT(const list_array<uint8_t>& nbt) {
        nbt_data = nbt;
    }

    operator list_array<uint8_t>() {
        return nbt_data;
    }

    list_array<uint8_t> get_as_normal() {
        return nbt_data;
    }

    list_array<uint8_t> get_as_network() {
        list_array<uint8_t> ret = nbt_data;
        ret.remove(1, 3);
        return ret;
    }

    ENBT get_as_enbt() {
        size_t i = 0;
        return RecursiveExtractor(nbt_data.data(), i, nbt_data.size());
    }

    std::string get_entry_name() {
        size_t i = 0;
        uint8_t* data = nbt_data.data();
        if (data[0] == 10) {
            //skip first base compound name tag
            i++;
            uint16_t length = extractValue<uint16_t>(data, i, nbt_data.size());
            return std::string(data, data + length);
        }
    }

    static ENBT extract_from_array(const uint8_t* arr, size_t& result, size_t max_size) {
        result = 0;
        return RecursiveExtractor(arr, result, max_size);
    }
};


#endif /* SRC_PROTOCOLHELPERNBT */
