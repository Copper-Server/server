#include <src/util/nbt.hpp>

namespace copper_server::util {
#pragma region ENBT_TO_NBT

    template <class T>
    void NBT::insertValue(T val, size_t max) {
        val = enbt::endian_helpers::convert_endian(std::endian::big, val);
        uint8_t* proxy = (uint8_t*)&val;
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back(proxy[i]);
    }

    template <class Target, class T>
    void NBT::insertValue(T val, size_t max) {
        Target tmp = (Target)val;
        if (tmp != val)
            throw std::exception("Unsupported tag");
        insertValue(tmp, max);
    }

    template <class T>
    T NBT::uncheckedExtractValue(const uint8_t* data, size_t& i) {
        uint8_t tmp[sizeof(T)];
        for (size_t j = 0; j < sizeof(T); j++)
            tmp[j] = data[i++];
        return enbt::endian_helpers::convert_endian(std::endian::big, *(T*)tmp);
    }

    template <class T>
    T NBT::extractValue(const uint8_t* data, size_t& i, size_t max_size) {
        if (i + sizeof(T) >= max_size)
            throw std::out_of_range("Out of bounds");
        return uncheckedExtractValue<T>(data, i);
    }

    template <class T>
    enbt::value NBT::extractArray(const uint8_t* data, size_t& i, size_t max_size) {
        int32_t len = extractValue<int32_t>(data, i, max_size);
        if (i + sizeof(T) * len >= max_size)
            throw std::out_of_range("Out of bounds");
        std::vector<enbt::value> ret;
        ret.reserve(len);
        for (int32_t j = 0; j < len; j++)
            ret.push_back(uncheckedExtractValue<T>(data, i));
        return enbt::value(ret, enbt::type_id(enbt::type::array, enbt::type_len::Default));
    }

    void NBT::insertString(const char* val, size_t max) {
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back((uint8_t)val[i]);
    }

    void NBT::IntegerInsert(enbt::value& val, bool typ_ins) {
        switch (val.get_type_len()) {
        case enbt::type_len::Tiny:
            if (typ_ins)
                nbt_data.push_back(1);
            if (val.get_type_sign())
                nbt_data.push_back((int8_t)val);
            else
                insertValue<int8_t>((uint8_t)val);
            break;

        case enbt::type_len::Short:
            if (typ_ins)
                nbt_data.push_back(2);
            if (val.get_type_sign())
                insertValue((int16_t)val);
            else
                insertValue<int16_t>((uint16_t)val);
            break;
        case enbt::type_len::Default:
            if (typ_ins)
                nbt_data.push_back(3);
            if (val.get_type_sign())
                insertValue((int32_t)val);
            else
                insertValue<int32_t>((uint32_t)val);
            break;
        case enbt::type_len::Long:
            if (typ_ins)
                nbt_data.push_back(4);
            if (val.get_type_sign())
                insertValue((int64_t)val);
            else
                insertValue<int64_t>((uint64_t)val);
            break;
        }
    }

    void NBT::FloatingInsert(enbt::value& val, bool typ_ins) {
        switch (val.get_type_len()) {
        case enbt::type_len::Default:
            if (typ_ins)
                nbt_data.push_back(5);
            insertValue((float)val);
            break;
        case enbt::type_len::Long:
            if (typ_ins)
                nbt_data.push_back(6);
            insertValue((double)val);
            break;
        default:
            throw std::exception("Unsupported tag");
        }
    }

    void NBT::BuildCompoundItem(const std::string& c_name, enbt::value& comp, bool compress) {
        if (c_name.size() > UINT16_MAX)
            throw std::out_of_range("enbt::value string too big to fit in NBT");
        InsertType(comp.type_id());
        insertValue((uint16_t)c_name.size());
        insertString(c_name.data(), c_name.size());
        RecursiveBuilder(comp, false, "", compress, false);
    }

    void NBT::BuildCompound(const std::string& c_name, enbt::value& comp, bool compress, bool insert_name) {
        if (insert_name) {
            insertValue((uint16_t)c_name.size());
            insertString(c_name.data(), c_name.size());
        }

        for (const auto& [name, tmp] : comp) {
            if (tmp.is_optional()) {
                if (!tmp.contains())
                    continue;
                else
                    BuildCompoundItem(name, *tmp.get_optional(), compress);
            } else
                BuildCompoundItem(name, tmp, compress);
        }

        InsertType(enbt::type::none);
    }

    void NBT::InsertType(enbt::type_id t) {
        switch (t.type) {
        case enbt::type::none:
            nbt_data.push_back(0);
            break;
        case enbt::type::bit:
            nbt_data.push_back(1);
            break;
        case enbt::type::integer:
        case enbt::type::var_integer:
        case enbt::type::comp_integer:
            switch (t.length) {
            case enbt::type_len::Tiny:
                nbt_data.push_back(1);
                break;
            case enbt::type_len::Short:
                nbt_data.push_back(2);
                break;
            case enbt::type_len::Default:
                nbt_data.push_back(3);
                break;
            case enbt::type_len::Long:
                nbt_data.push_back(4);
                break;
            }
            break;
        case enbt::type::floating:
            switch (t.length) {
            case enbt::type_len::Default:
                nbt_data.push_back(5);
                break;
            case enbt::type_len::Long:
                nbt_data.push_back(6);
                break;
            default:
                throw std::exception("Unsupported tag");
            }
            break;
        case enbt::type::uuid:
        case enbt::type::string:
            nbt_data.push_back(8);
            break;
        case enbt::type::array:
        case enbt::type::darray:
            nbt_data.push_back(9);
            break;
        case enbt::type::compound:
            nbt_data.push_back(10);
            break;
        default:
            throw std::exception("Unsupported tag");
        }
    }

    void NBT::BuildBaseIntArray(int32_t len, enbt::value& arr, enbt::type_id base_id) {
        insertValue(len);
        for (int32_t i = 0; i < len; i++) {
            if (arr[i].type_id() != base_id)
                throw std::exception("Array type mismatch");
            IntegerInsert(arr[i], false);
        }
    }

    void NBT::BuildSimpleIntArray(int32_t len, enbt::value& arr, enbt::type_id base_id) {
        insertValue(len);
        for (int32_t i = 0; i < len; i++) {
            auto val = arr.get_index(i);
            if (val.type_id() != base_id)
                throw std::exception("Array type mismatch");
            IntegerInsert(val, false);
        }
    }

    void NBT::BuildArray(int32_t len, enbt::value& arr, enbt::type_id base_id, bool compress) {
        insertValue(len);
        if (arr.is_sarray()) {
            for (int32_t i = 0; i < len; i++) {
                auto val = arr.get_index(i);
                auto type = val.type_id();
                if (type.type != base_id.type || type.length != base_id.length || type.is_signed != base_id.is_signed)
                    throw std::exception("Array type mismatch");
                RecursiveBuilder(val, false, "", compress, false);
            }
        } else {
            if (arr[0].is_numeric()) {
                for (int32_t i = 0; i < len; i++) {
                    auto type = arr[i].type_id();
                    if (type.type != base_id.type || type.length != base_id.length || type.is_signed != base_id.is_signed)
                        throw std::exception("Array type mismatch");
                    RecursiveBuilder(arr[i], false, "", compress, false);
                }
            } else {
                for (int32_t i = 0; i < len; i++) {
                    if (arr[i].type_id() != base_id)
                        throw std::exception("Array type mismatch");
                    RecursiveBuilder(arr[i], false, "", compress, false);
                }
            }
        }
    }

    void NBT::BuildArray(int32_t len, enbt::value& enbt, bool insert_type, bool compress) {
        if (!enbt.size()) {
            if (insert_type)
                nbt_data.push_back(9);
            InsertType(enbt::type::none);
            insertValue(0);
            return;
        }
        auto base_type = enbt.is_sarray() ? enbt.get_index(0).type_id() : enbt[0].type_id();
        if ((base_type.type == enbt::type::integer || base_type.type == enbt::type::var_integer) && compress) {
            switch (base_type.length) {
            case enbt::type_len::Tiny:
                if (base_type.is_signed) {
                    if (insert_type)
                        nbt_data.push_back(7);
                    if (((int32_t)enbt.size()) != enbt.size())
                        throw std::exception("Unsupported array len");
                    BuildBaseIntArray(enbt.size(), enbt, base_type);
                    return;
                }
                break;
            case enbt::type_len::Short:
                break;
            case enbt::type_len::Default:
                if (enbt[0].get_type_sign()) {
                    if (insert_type)
                        nbt_data.push_back(11);
                    if (((int32_t)enbt.size()) != enbt.size())
                        throw std::exception("Unsupported array len");
                    BuildBaseIntArray(enbt.size(), enbt, base_type);
                    return;
                }
                break;
            case enbt::type_len::Long:
                if (enbt[0].get_type_sign()) {
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

    void NBT::RecursiveBuilder(enbt::value& enbt, bool insert_type, const std::string& name, bool compress, bool insert_name) {
        switch (enbt.get_type()) {
        case enbt::type::none:
            if (insert_type)
                nbt_data.push_back(0);
            break;
        case enbt::type::bit:
            if (insert_type)
                nbt_data.push_back(1);
            nbt_data.push_back((bool)enbt);
            break;
        case enbt::type::integer:
        case enbt::type::var_integer:
        case enbt::type::comp_integer:
            IntegerInsert(enbt, insert_type);
            break;
        case enbt::type::floating:
            FloatingInsert(enbt, insert_type);
            break;
        case enbt::type::string: {
            if (insert_type)
                nbt_data.push_back(8);
            const std::string& str = (const std::string&)enbt;
            if (((uint16_t)str.size()) != str.size())
                throw std::exception("Unsupported string len");
            insertValue((uint16_t)str.size());
            insertString(str.data(), str.size());
            break;
        }
        case enbt::type::uuid: {
            enbt::value uid = ((enbt::raw_uuid)enbt).to_string();
            RecursiveBuilder(uid, insert_type, name, compress, insert_name);
            break;
        }
        case enbt::type::sarray:
        case enbt::type::array:
        case enbt::type::darray:
            BuildArray((int32_t)enbt.size(), enbt, insert_type, compress);
            break;
        case enbt::type::compound:
            if (insert_type)
                nbt_data.push_back(10);
            BuildCompound(name, enbt, compress, insert_name);
            break;
        default:
            throw std::exception("Unsupported tag");
        }
    }

#pragma endregion
#pragma region NBT_TO_ENBT

    enbt::value NBT::RecursiveExtractor_1(uint8_t type, const uint8_t* data, size_t& i, size_t max_size, bool in_compound) {
        switch (type) {
        case 0: //end
            return enbt::value();
        case 1: //byte
            return enbt::value(extractValue<int8_t>(data, i, max_size));
        case 2: //short
            return enbt::value(extractValue<int16_t>(data, i, max_size));
        case 3: //int
            return enbt::value(extractValue<int32_t>(data, i, max_size));
        case 4: //long
            return enbt::value(extractValue<int64_t>(data, i, max_size));
        case 5: //float
            return enbt::value(extractValue<float>(data, i, max_size));
        case 6: //double
            return enbt::value(extractValue<double>(data, i, max_size));
        case 7: //byte array
            return extractArray<int8_t>(data, i, max_size);
        case 8: { //string
            uint16_t length = extractValue<uint16_t>(data, i, max_size);
            if (i + length >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length;
            if (length <= 32 && length >= 16) {
                try {
                    enbt::raw_uuid res;
                    auto check_view = enbt::raw_uuid::from_uuid_string(res, std::string_view((const char*)data + i - length, length));
                    if (check_view.size() == length)
                        return enbt::value(res);
                } catch (...) {
                }
            }
            return enbt::value((const char*)data + i - length, length);
        }
        case 9: { //list
            uint8_t type = data[i++];
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (length < 0)
                length = 0;
            std::vector<enbt::value> res;
            res.reserve(length);
            for (int32_t iterate = 0; iterate < length; iterate++)
                res.push_back(RecursiveExtractor_1(type, data, i, max_size, false));
            return enbt::value(res, enbt::type_id(enbt::type::array, enbt::type_len::Default));
        }
        case 10: { //compound
            std::unordered_map<std::string, enbt::value> compound;
            while (true) {
                uint8_t type = data[i++];
                if (!type)
                    break;
                uint16_t length = extractValue<uint16_t>(data, i, max_size);
                if (i + length >= max_size)
                    throw std::out_of_range("Out of bounds");
                std::string res(data + i, data + i + length);
                i += length;
                compound[res] = RecursiveExtractor_1(type, data, i, max_size, true);
            }
            return compound;
        }
        case 11: { //int array
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (i + length * 4 >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length * 4;
            return enbt::value((int32_t*)data, length, std::endian::big, true);
        }
        case 12: { //long array
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (i + length * 8 >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length * 8;
            return enbt::value((int64_t*)data, length, std::endian::big, true);
        }
        default:
            throw std::exception("Invalid type");
        }
    }

    enbt::value NBT::RecursiveExtractor(const uint8_t* data, size_t& i, size_t max_size) {
        if (max_size == 0)
            return enbt::value();
        if (data[0] == 10) {
            //skip first base compound name tag
            i++;
            i += extractValue<uint16_t>(data, i, max_size);
            return RecursiveExtractor_1(10, data, i, max_size, true);
        }
        return RecursiveExtractor_1(data[i++], data, i, max_size, false);
    }

    enbt::value NBT::RecursiveExtractorNetwork(const uint8_t* data, size_t& i, size_t max_size) {
        if (max_size == 0)
            return enbt::value();
        return RecursiveExtractor_1(data[i++], data, i, max_size, data[0] == 10);
    }

#pragma endregion

    NBT::NBT() {}

    enbt::value NBT::readNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractor(data, nbt_size, max_size);
    }

    NBT NBT::readNBT(const uint8_t* data, size_t max_size, size_t& nbt_size, bool compress, const std::string& entry_name) {
        return build(readNBT_asENBT(data, max_size, nbt_size), compress, entry_name);
    }

    enbt::value NBT::readNetworkNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractorNetwork(data, nbt_size, max_size);
    }

    NBT NBT::readNetworkNBT(const uint8_t* data, size_t max_size, size_t& nbt_size, bool compress, const std::string& entry_name) {
        return build(readNetworkNBT_asENBT(data, max_size, nbt_size), compress, entry_name);
    }

    NBT::NBT(NBT&& move)
        : nbt_data(std::move(nbt_data)) {}

    NBT::~NBT() = default;

    NBT NBT::build(const enbt::value& enbt, bool compress, const std::string& entry_name) {
        NBT ret;
        ret.RecursiveBuilder(const_cast<enbt::value&>(enbt), true, entry_name, compress, true);
        return ret;
    }

    NBT NBT::build(const list_array<uint8_t>& data) {
        NBT ret;
        ret.nbt_data = data;
        return ret;
    }

    NBT NBT::build_network(const list_array<uint8_t>& data) {
        NBT ret;
        ret.nbt_data = data;
        uint8_t tmp[] = {0, 0};
        if (ret.nbt_data[0] == 10)
            ret.nbt_data.insert(1, tmp, 2); //add length to fix it

        return ret;
    }

    NBT::operator list_array<uint8_t>() const {
        return nbt_data;
    }

    list_array<uint8_t> NBT::get_as_normal() const {
        return nbt_data;
    }

    list_array<uint8_t> NBT::get_as_network() const {
        if (nbt_data.size())
            if (nbt_data[0] == 10) {
                list_array<uint8_t> ret = nbt_data;
                ret.erase(1, 3);
                return ret;
            }
        return nbt_data;
    }

    enbt::value NBT::get_as_enbt() const {
        size_t i = 0;
        return RecursiveExtractor(nbt_data.data(), i, nbt_data.size());
    }

    std::string NBT::get_entry_name() const {
        size_t i = 0;
        const uint8_t* data = nbt_data.data();
        if (data[0] == 10) {
            //skip first base compound name tag
            i++;
            uint16_t length = extractValue<uint16_t>(data, i, nbt_data.size());
            return std::string(data, data + length);
        } else
            return "";
    }

    enbt::value NBT::extract_from_array(const uint8_t* arr, size_t& result, size_t max_size) {
        result = 0;
        return RecursiveExtractor(arr, result, max_size);
    }
}
