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

    void BuildCompound(const std::string& c_name, ENBT& comp, bool compress) {
        insertValue((uint16_t)c_name.size());
        insertString(c_name.data(), c_name.size());
        for (const auto& [name, tmp] : comp) {
            if (name.size() > UINT16_MAX)
                throw std::out_of_range("ENBT string too big to fit in NBT");
            InsertType(tmp.type_id());
            if (tmp.getType() == ENBT::Type::compound) {
                RecursiveBuilder(tmp, false, name, compress);
            } else {
                insertValue((uint16_t)name.size());
                insertString(name.data(), name.size());
                RecursiveBuilder(tmp, false, "", compress);
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
        case ENBT::Type::darray:
            nbt_data.push_back(9);
            break;
        case ENBT::Type::compound:
            nbt_data.push_back(10);
            break;
        default:
            throw std::exception("Unsupported tag");
        }
    }

    void BuildBaseIntArray(int32_t len, ENBT& arr, ENBT::Type_ID base_id) {
        insertValue(len);
        for (int32_t i = 0; i < len; i++) {
            if (arr[i].type_id() != base_id)
                throw std::exception("Array type mismatch");
            IntegerInsert(arr[i], false);
        }
    }

    void BuildSimpleIntArray(int32_t len, ENBT& arr, ENBT::Type_ID base_id) {
        insertValue(len);
        for (int32_t i = 0; i < len; i++) {
            auto val = arr.getIndex(i);
            if (val.type_id() != base_id)
                throw std::exception("Array type mismatch");
            IntegerInsert(val, false);
        }
    }

    void BuildArray(int32_t len, ENBT& arr, ENBT::Type_ID base_id, bool compress) {
        insertValue(len);
        if (arr.is_sarray()) {
            for (int32_t i = 0; i < len; i++) {
                auto val = arr.getIndex(i);
                auto type = val.type_id();
                if (type.type != base_id.type || type.length != base_id.length || type.is_signed != base_id.is_signed)
                    throw std::exception("Array type mismatch");
                RecursiveBuilder(val, false, "", compress);
            }
        } else {
            if (arr[0].is_numeric()) {
                for (int32_t i = 0; i < len; i++) {
                    auto type = arr[i].type_id();
                    if (type.type != base_id.type || type.length != base_id.length || type.is_signed != base_id.is_signed)
                        throw std::exception("Array type mismatch");
                    RecursiveBuilder(arr[i], false, "", compress);
                }
            } else {
                for (int32_t i = 0; i < len; i++) {
                    if (arr[i].type_id() != base_id)
                        throw std::exception("Array type mismatch");
                    RecursiveBuilder(arr[i], false, "", compress);
                }
            }
        }
    }

    void BuildArray(int32_t len, ENBT& enbt, bool insert_type, bool compress) {
        if (!enbt.size()) {
            if (insert_type)
                nbt_data.push_back(9);
            InsertType(ENBT::Type::none);
            insertValue(0);
            return;
        }
        auto base_type = enbt.is_sarray() ? enbt.getIndex(0).type_id() : enbt[0].type_id();
        if ((base_type.type == ENBT::Type::integer || base_type.type == ENBT::Type::var_integer) && compress) {
            switch (base_type.length) {
            case ENBT::TypeLen::Tiny:
                if (base_type.is_signed) {
                    if (insert_type)
                        nbt_data.push_back(7);
                    if (((int32_t)enbt.size()) != enbt.size())
                        throw std::exception("Unsupported array len");
                    BuildBaseIntArray(enbt.size(), enbt, base_type);
                    return;
                }
                break;
            case ENBT::TypeLen::Short:
                break;
            case ENBT::TypeLen::Default:
                if (enbt[0].getTypeSign()) {
                    if (insert_type)
                        nbt_data.push_back(11);
                    if (((int32_t)enbt.size()) != enbt.size())
                        throw std::exception("Unsupported array len");
                    BuildBaseIntArray(enbt.size(), enbt, base_type);
                    return;
                }
                break;
            case ENBT::TypeLen::Long:
                if (enbt[0].getTypeSign()) {
                    if (insert_type)
                        nbt_data.push_back(12);
                    if (((int32_t)enbt.size()) != enbt.size())
                        throw std::exception("Unsupported array len");
                    BuildBaseIntArray(enbt.size(), enbt, base_type);
                    return;
                }
                break;
                default:
                    break;
            }
        }
        if (insert_type)
            nbt_data.push_back(9);
        InsertType(base_type);
        BuildArray(enbt.size(), enbt, base_type, compress);
    }

    void RecursiveBuilder(ENBT& enbt, bool insert_type, const std::string& name, bool compress) {
        switch (enbt.getType()) {
        case ENBT::Type::none:
            if (insert_type)
                nbt_data.push_back(0);
            break;
        case ENBT::Type::integer:
        case ENBT::Type::var_integer:
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
        case ENBT::Type::sarray:
        case ENBT::Type::array:
        case ENBT::Type::darray:
            BuildArray((int32_t)enbt.size(), enbt, insert_type, compress);
            break;
        case ENBT::Type::compound:
            if (insert_type)
                nbt_data.push_back(10);
            BuildCompound(name, enbt, compress);
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
            return ENBT((const char*)data + i - length, length);
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
                if (!type)
                    break;
                uint16_t length = extractValue<uint16_t>(data, i, max_size);
                if (i + length >= max_size)
                    throw std::out_of_range("Out of bounds");
                std::string res(data + i, data + i + length);
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

#pragma endregion

    NBT() {}

public:
    static ENBT readNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractor(data, nbt_size, max_size);
    }

    static NBT readNBT(const uint8_t* data, size_t max_size, size_t& nbt_size, bool compress = true, const std::string& entry_name = "") {
        return build(readNBT_asENBT(data, max_size, nbt_size), compress, entry_name);
    }

    ~NBT() = default;

    static NBT build(const ENBT& enbt, bool compress = true, const std::string& entry_name = "") {
        NBT ret;
        ret.RecursiveBuilder(const_cast<ENBT&>(enbt), true, entry_name, compress);
        return ret;
    }

    static NBT build(const list_array<uint8_t>& data) {
        NBT ret;
        ret.nbt_data = data;
        return ret;
    }


    operator list_array<uint8_t>() {
        return nbt_data;
    }

    list_array<uint8_t> get_as_normal() {
        return nbt_data;
    }

    list_array<uint8_t> get_as_network() {
        list_array<uint8_t> ret = nbt_data;
        ret.erase(1, 3);
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
