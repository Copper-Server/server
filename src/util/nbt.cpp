/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <istream>
#include <src/util/nbt.hpp>

namespace copper_server::util {
    template <class T>
    T read_value(std::istream& read_stream) {
        T res;
        read_stream.read((char*)&res, sizeof(T));
        if constexpr (sizeof(T) != 1)
            return enbt::endian_helpers::convert_endian(std::endian::big, res);
        else
            return res;
    }

#pragma region NBT

    void nbt::clear() {
        if (data) {
            switch (type) {
            case nbt_type::tag_byte_array:
                delete (list_array<uint8_t>*)data;
                break;
            case nbt_type::tag_string:
                delete (std::string*)data;
                break;
            case nbt_type::tag_list:
                delete (list_array<nbt>*)data;
                break;
            case nbt_type::tag_compound:
                delete (std::unordered_map<std::string, nbt>*)data;
                break;
            case nbt_type::tag_int_array:
                delete (list_array<int32_t>*)data;
                break;
            case nbt_type::tag_long_array:
                delete (list_array<int64_t>*)data;
                break;
            default:
                break;
            }
            data = nullptr;
        }
    }

    nbt::nbt() : data(nullptr), type(nbt_type::tag_end) {}

    nbt::nbt(int8_t value) : data((void*)static_cast<ptrdiff_t>(value)), type(nbt_type::tag_byte) {}

    nbt::nbt(int16_t value) : data((void*)static_cast<ptrdiff_t>(value)), type(nbt_type::tag_short) {}

    nbt::nbt(int32_t value) : data((void*)static_cast<ptrdiff_t>(value)), type(nbt_type::tag_int) {}

    nbt::nbt(int64_t value) : data((void*)static_cast<ptrdiff_t>(value)), type(nbt_type::tag_long) {}

    nbt::nbt(float value) : data((void*)static_cast<ptrdiff_t>(value)), type(nbt_type::tag_float) {}

    nbt::nbt(double value) : data((void*)static_cast<ptrdiff_t>(value)), type(nbt_type::tag_double) {}

    nbt::nbt(const list_array<uint8_t>& value) : data(new list_array<uint8_t>(value)), type(nbt_type::tag_byte_array) {}

    nbt::nbt(list_array<uint8_t>&& value) : data(new list_array<uint8_t>(std::move(value))), type(nbt_type::tag_byte_array) {}

    nbt::nbt(const std::string& value) : data(new std::string(value)), type(nbt_type::tag_byte_array) {}

    nbt::nbt(std::string&& value) : data(new std::string(std::move(value))), type(nbt_type::tag_string) {}

    nbt::nbt(const char* value) : data(new std::string(value)), type(nbt_type::tag_string) {}

    nbt::nbt(const char* value, size_t size) : data(new std::string(value, size)), type(nbt_type::tag_string) {}

    nbt::nbt(std::string_view value) : data(new std::string(value)), type(nbt_type::tag_string) {}

    nbt::nbt(const list_array<nbt>& value) : data(new list_array<nbt>(value)), type(nbt_type::tag_list) {}

    nbt::nbt(list_array<nbt>&& value) : data(new list_array<nbt>(std::move(value))), type(nbt_type::tag_list) {}

    nbt::nbt(const std::unordered_map<std::string, nbt>& value) : data(new std::unordered_map<std::string, nbt>(value)), type(nbt_type::tag_compound) {}

    nbt::nbt(std::unordered_map<std::string, nbt>&& value) : data(new std::unordered_map<std::string, nbt>(std::move(value))), type(nbt_type::tag_compound) {}

    nbt::nbt(const list_array<int32_t>& value) : data(new list_array<int32_t>(value)), type(nbt_type::tag_int_array) {}

    nbt::nbt(list_array<int32_t>&& value) : data(new list_array<int32_t>(std::move(value))), type(nbt_type::tag_int_array) {}

    nbt::nbt(const list_array<int64_t>& value) : data(new list_array<int64_t>(value)), type(nbt_type::tag_long_array) {}

    nbt::nbt(list_array<int64_t>&& value) : data(new list_array<int64_t>(std::move(value))), type(nbt_type::tag_long_array) {}

    nbt::nbt(const nbt& copy) {
        data = nullptr;
        operator=(copy);
    }

    nbt::nbt(nbt&& move) {
        type = move.type;
        data = move.data;
        move.type = nbt_type::tag_end;
        move.data = nullptr;
    }

    nbt::~nbt() {
        clear();
    }

    nbt& nbt::operator=(const nbt& copy) {
        if (this == &copy)
            return *this;
        clear();
        type = copy.type;
        switch (type) {
        case nbt_type::tag_end:
        case nbt_type::tag_byte:
        case nbt_type::tag_short:
        case nbt_type::tag_int:
        case nbt_type::tag_long:
        case nbt_type::tag_float:
        case nbt_type::tag_double:
            data = copy.data;
            break;
        case nbt_type::tag_byte_array:
            data = new list_array<uint8_t>(*(list_array<uint8_t>*)copy.data);
            break;
        case nbt_type::tag_string:
            data = new std::string(*(std::string*)copy.data);
            break;
        case nbt_type::tag_list:
            data = new list_array<nbt>(*(list_array<nbt>*)copy.data);
            break;
        case nbt_type::tag_compound:
            data = new std::unordered_map<std::string, nbt>(*(std::unordered_map<std::string, nbt>*)copy.data);
            break;
        case nbt_type::tag_int_array:
            data = new list_array<int32_t>(*(list_array<int32_t>*)copy.data);
            break;
        case nbt_type::tag_long_array:
            data = new list_array<int64_t>(*(list_array<int64_t>*)copy.data);
            break;
        }
        return *this;
    }

    nbt& nbt::operator=(nbt&& move) {
        if (this == &move)
            return *this;
        clear();
        type = move.type;
        data = move.data;
        move.type = nbt_type::tag_end;
        move.data = nullptr;
        return *this;
    }

    bool nbt::operator==(const nbt& other) const {
        if (type != other.type)
            return false;
        switch (type) {
        case nbt_type::tag_end:
        case nbt_type::tag_byte:
        case nbt_type::tag_short:
        case nbt_type::tag_int:
        case nbt_type::tag_long:
        case nbt_type::tag_float:
        case nbt_type::tag_double:
            return data == other.data;
        case nbt_type::tag_byte_array:
            return *(list_array<uint8_t>*)data == *(list_array<uint8_t>*)other.data;
        case nbt_type::tag_string:
            return *(std::string*)data == *(std::string*)other.data;
        case nbt_type::tag_list:
            return *(list_array<nbt>*)data == *(list_array<nbt>*)other.data;
        case nbt_type::tag_compound:
            return *(std::unordered_map<std::string, nbt>*)data == *(std::unordered_map<std::string, nbt>*)other.data;
        case nbt_type::tag_int_array:
            return *(list_array<int32_t>*)data == *(list_array<int32_t>*)other.data;
        case nbt_type::tag_long_array:
            return *(list_array<int64_t>*)data == *(list_array<int64_t>*)other.data;
        default:
            return false;
        }
    }

    bool nbt::operator!=(const nbt& move) const {
        return !(*this == move);
    }

    nbt_type nbt::get_type() const noexcept {
        return type;
    }

    int8_t nbt::get_byte() const {
        if (type != nbt_type::tag_byte)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<const int8_t*>(&data);
    }

    int16_t nbt::get_short() const {
        if (type != nbt_type::tag_short)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<const int16_t*>(&data);
    }

    int32_t nbt::get_int() const {
        if (type != nbt_type::tag_int)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<const int32_t*>(&data);
    }

    int64_t nbt::get_long() const {
        if (type != nbt_type::tag_long)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<const int64_t*>(&data);
    }

    float nbt::get_float() const {
        if (type != nbt_type::tag_float)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<const float*>(&data);
    }

    double nbt::get_double() const {
        if (type != nbt_type::tag_double)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<const double*>(&data);
    }

    const list_array<uint8_t>& nbt::get_byte_array() const {
        if (type != nbt_type::tag_byte_array)
            throw std::runtime_error("Invalid type");
        return *(list_array<uint8_t>*)data;
    }

    const std::string& nbt::get_string() const {
        if (type != nbt_type::tag_string)
            throw std::runtime_error("Invalid type");
        return *(std::string*)data;
    }

    const list_array<nbt>& nbt::get_list() const {
        if (type != nbt_type::tag_list)
            throw std::runtime_error("Invalid type");
        return *(list_array<nbt>*)data;
    }

    const std::unordered_map<std::string, nbt>& nbt::get_compound() const {
        if (type != nbt_type::tag_compound)
            throw std::runtime_error("Invalid type");
        return *(std::unordered_map<std::string, nbt>*)data;
    }

    const list_array<int32_t>& nbt::get_int_array() const {
        if (type != nbt_type::tag_int_array)
            throw std::runtime_error("Invalid type");
        return *(list_array<int32_t>*)data;
    }

    const list_array<int64_t>& nbt::get_long_array() const {
        if (type != nbt_type::tag_long_array)
            throw std::runtime_error("Invalid type");
        return *(list_array<int64_t>*)data;
    }

    int8_t& nbt::get_byte() {
        if (type != nbt_type::tag_byte)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<int8_t*>(&data);
    }

    int16_t& nbt::get_short() {
        if (type != nbt_type::tag_short)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<int16_t*>(&data);
    }

    int32_t& nbt::get_int() {
        if (type != nbt_type::tag_int)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<int32_t*>(&data);
    }

    int64_t& nbt::get_long() {
        if (type != nbt_type::tag_long)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<int64_t*>(&data);
    }

    float& nbt::get_float() {
        if (type != nbt_type::tag_float)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<float*>(&data);
    }

    double& nbt::get_double() {
        if (type != nbt_type::tag_double)
            throw std::runtime_error("Invalid type");
        return *reinterpret_cast<double*>(&data);
    }

    list_array<uint8_t>& nbt::get_byte_array() {
        if (type != nbt_type::tag_byte_array)
            throw std::runtime_error("Invalid type");
        return *(list_array<uint8_t>*)data;
    }

    std::string& nbt::get_string() {
        if (type != nbt_type::tag_string)
            throw std::runtime_error("Invalid type");
        return *(std::string*)data;
    }

    list_array<nbt>& nbt::get_list() {
        if (type != nbt_type::tag_list)
            throw std::runtime_error("Invalid type");
        return *(list_array<nbt>*)data;
    }

    std::unordered_map<std::string, nbt>& nbt::get_compound() {
        if (type != nbt_type::tag_compound)
            throw std::runtime_error("Invalid type");
        return *(std::unordered_map<std::string, nbt>*)data;
    }

    list_array<int32_t>& nbt::get_int_array() {
        if (type != nbt_type::tag_int_array)
            throw std::runtime_error("Invalid type");
        return *(list_array<int32_t>*)data;
    }

    list_array<int64_t>& nbt::get_long_array() {
        if (type != nbt_type::tag_long_array)
            throw std::runtime_error("Invalid type");
        return *(list_array<int64_t>*)data;
    }

    bool nbt::is_byte() const {
        return type == nbt_type::tag_byte;
    }

    bool nbt::is_short() const {
        return type == nbt_type::tag_short;
    }

    bool nbt::is_int() const {
        return type == nbt_type::tag_int;
    }

    bool nbt::is_long() const {
        return type == nbt_type::tag_long;
    }

    bool nbt::is_float() const {
        return type == nbt_type::tag_float;
    }

    bool nbt::is_double() const {
        return type == nbt_type::tag_double;
    }

    bool nbt::is_byte_array() const {
        return type == nbt_type::tag_byte_array;
    }

    bool nbt::is_string() const {
        return type == nbt_type::tag_string;
    }

    bool nbt::is_list() const {
        return type == nbt_type::tag_list;
    }

    bool nbt::is_compound() const {
        return type == nbt_type::tag_compound;
    }

    bool nbt::is_int_array() const {
        return type == nbt_type::tag_int_array;
    }

    bool nbt::is_long_array() const {
        return type == nbt_type::tag_long_array;
    }

    int8_t nbt::as_byte() const {
        switch (type) {
        case nbt_type::tag_byte:
            return get_byte();
        case nbt_type::tag_short:
            return (int8_t)get_short();
        case nbt_type::tag_int:
            return (int8_t)get_int();
        case nbt_type::tag_long:
            return (int8_t)get_long();
        case nbt_type::tag_float:
            return (int8_t)get_float();
        case nbt_type::tag_double:
            return (int8_t)get_double();
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    int16_t nbt::as_short() const {
        switch (type) {
        case nbt_type::tag_byte:
            return (int16_t)get_byte();
        case nbt_type::tag_short:
            return get_short();
        case nbt_type::tag_int:
            return (int16_t)get_int();
        case nbt_type::tag_long:
            return (int16_t)get_long();
        case nbt_type::tag_float:
            return (int16_t)get_float();
        case nbt_type::tag_double:
            return (int16_t)get_double();
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    int32_t nbt::as_int() const {
        switch (type) {
        case nbt_type::tag_byte:
            return (int32_t)get_byte();
        case nbt_type::tag_short:
            return (int32_t)get_short();
        case nbt_type::tag_int:
            return get_int();
        case nbt_type::tag_long:
            return (int32_t)get_long();
        case nbt_type::tag_float:
            return (int32_t)get_float();
        case nbt_type::tag_double:
            return (int32_t)get_double();
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    int64_t nbt::as_long() const {
        switch (type) {
        case nbt_type::tag_byte:
            return (int64_t)get_byte();
        case nbt_type::tag_short:
            return (int64_t)get_short();
        case nbt_type::tag_int:
            return (int64_t)get_int();
        case nbt_type::tag_long:
            return get_long();
        case nbt_type::tag_float:
            return (int64_t)get_float();
        case nbt_type::tag_double:
            return (int64_t)get_double();
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    float nbt::as_float() const {
        switch (type) {
        case nbt_type::tag_byte:
            return (float)get_byte();
        case nbt_type::tag_short:
            return (float)get_short();
        case nbt_type::tag_int:
            return (float)get_int();
        case nbt_type::tag_long:
            return (float)get_long();
        case nbt_type::tag_float:
            return get_float();
        case nbt_type::tag_double:
            return (float)get_double();
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    double nbt::as_double() const {
        switch (type) {
        case nbt_type::tag_byte:
            return (double)get_byte();
        case nbt_type::tag_short:
            return (double)get_short();
        case nbt_type::tag_int:
            return (double)get_int();
        case nbt_type::tag_long:
            return (double)get_long();
        case nbt_type::tag_float:
            return (double)get_float();
        case nbt_type::tag_double:
            return get_double();
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    std::string nbt::as_string() const {
        switch (type) {
        case nbt_type::tag_byte:
            return std::to_string(get_byte());
        case nbt_type::tag_short:
            return std::to_string(get_short());
        case nbt_type::tag_int:
            return std::to_string(get_int());
        case nbt_type::tag_long:
            return std::to_string(get_long());
        case nbt_type::tag_float:
            return std::to_string(get_float());
        case nbt_type::tag_double:
            return std::to_string(get_double());
        case nbt_type::tag_string:
            return get_string();
        default:
            throw std::runtime_error("Invalid type");
        }
    }

#pragma endregion


#pragma region ENBT_TO_NBT

    template <class T>
    void nbt_enbt_convert::insertValue(T val, size_t max) {
        val = enbt::endian_helpers::convert_endian(std::endian::big, val);
        uint8_t* proxy = (uint8_t*)&val;
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back(proxy[i]);
    }

    template <class Target, class T>
    void nbt_enbt_convert::insertValue(T val, size_t max) {
        Target tmp = (Target)val;
        if constexpr (!std::is_same<Target, T>::value) {
            if constexpr (std::is_unsigned_v<Target> == std::is_unsigned_v<T>) {
                if (tmp != val)
                    throw std::runtime_error("Unsupported tag");
            } else if constexpr (std::is_unsigned_v<Target>) {
                if (val < 0)
                    throw std::runtime_error("Unsupported tag");
                if (tmp != (std::make_unsigned_t<T>)val)
                    throw std::runtime_error("Unsupported tag");
            } else if constexpr (std::is_unsigned_v<T>) {
                if (tmp < 0)
                    throw std::runtime_error("Unsupported tag");
                if ((std::make_unsigned_t<Target>)val != (T)val)
                    throw std::runtime_error("Unsupported tag");
            }
        }
        insertValue(tmp, max);
    }

    template <class T>
    T nbt_enbt_convert::uncheckedExtractValue(const uint8_t* data, size_t& i) {
        uint8_t tmp[sizeof(T)];
        for (size_t j = 0; j < sizeof(T); j++)
            tmp[j] = data[i++];
        return enbt::endian_helpers::convert_endian(std::endian::big, *(T*)tmp);
    }

    template <class T>
    T nbt_enbt_convert::extractValue(const uint8_t* data, size_t& i, size_t max_size) {
        if (i + sizeof(T) >= max_size)
            throw std::out_of_range("Out of bounds");
        return uncheckedExtractValue<T>(data, i);
    }

    template <class T>
    enbt::value nbt_enbt_convert::extractArray(const uint8_t* data, size_t& i, size_t max_size) {
        int32_t len = extractValue<int32_t>(data, i, max_size);
        if (i + sizeof(T) * len >= max_size)
            throw std::out_of_range("Out of bounds");
        std::vector<enbt::value> ret;
        ret.reserve(len);
        for (int32_t j = 0; j < len; j++)
            ret.push_back(uncheckedExtractValue<T>(data, i));
        return enbt::value(ret, enbt::type_id(enbt::type::array, enbt::type_len::Default));
    }

    void nbt_enbt_convert::insertString(const char* val, size_t max) {
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back((uint8_t)val[i]);
    }

    void nbt_enbt_convert::IntegerInsert(const enbt::value& val, bool typ_ins) {
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

    void nbt_enbt_convert::FloatingInsert(const enbt::value& val, bool typ_ins) {
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
            throw std::runtime_error("Unsupported tag");
        }
    }

    void nbt_enbt_convert::BuildCompoundItem(std::string_view c_name, const enbt::value& comp, bool compress) {
        InsertType(comp.type_id());
        bool negate_zero = c_name.ends_with('\0');
        insertValue<uint16_t>(c_name.size() - negate_zero);
        insertString(c_name.data(), c_name.size() - negate_zero);
        RecursiveBuilder(comp, false, "", compress, false);
    }

    void nbt_enbt_convert::BuildCompound(std::string_view c_name, const enbt::value& comp, bool compress, bool insert_name) {
        if (insert_name) {
            bool negate_zero = c_name.ends_with('\0');
            insertValue<uint16_t>(c_name.size() - negate_zero);
            insertString(c_name.data(), c_name.size() - negate_zero);
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

    void nbt_enbt_convert::InsertType(enbt::type_id t) {
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
                throw std::runtime_error("Unsupported tag");
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
            throw std::runtime_error("Unsupported tag");
        }
    }

    void nbt_enbt_convert::BuildBaseIntArray(int32_t len, const enbt::value& arr, enbt::type_id base_id) {
        insertValue(len);
        for (int32_t i = 0; i < len; i++) {
            if (arr[i].type_id() != base_id)
                throw std::runtime_error("Array type mismatch");
            IntegerInsert(arr[i], false);
        }
    }

    void nbt_enbt_convert::BuildSimpleIntArray(int32_t len, const enbt::value& arr, enbt::type_id base_id) {
        insertValue(len);
        for (int32_t i = 0; i < len; i++) {
            auto val = arr.get_index(i);
            if (val.type_id() != base_id)
                throw std::runtime_error("Array type mismatch");
            IntegerInsert(val, false);
        }
    }

    void nbt_enbt_convert::BuildArray(int32_t len, const enbt::value& arr, enbt::type_id base_id, bool compress) {
        insertValue(len);
        if (arr.is_sarray()) {
            for (int32_t i = 0; i < len; i++) {
                auto val = arr.get_index(i);
                auto type = val.type_id();
                if (type.type != base_id.type || type.length != base_id.length || type.is_signed != base_id.is_signed)
                    throw std::runtime_error("Array type mismatch");
                RecursiveBuilder(val, false, "", compress, false);
            }
        } else {
            if (arr[0].is_numeric()) {
                for (int32_t i = 0; i < len; i++) {
                    auto type = arr[i].type_id();
                    if (type.type != base_id.type || type.length != base_id.length || type.is_signed != base_id.is_signed)
                        throw std::runtime_error("Array type mismatch");
                    RecursiveBuilder(arr[i], false, "", compress, false);
                }
            } else {
                for (int32_t i = 0; i < len; i++) {
                    if (arr[i].type_id() != base_id)
                        throw std::runtime_error("Array type mismatch");
                    RecursiveBuilder(arr[i], false, "", compress, false);
                }
            }
        }
    }

    void nbt_enbt_convert::BuildArray(const enbt::value& enbt, bool insert_type, bool compress) {
        if (!enbt.size()) {
            if (insert_type)
                nbt_data.push_back(9);
            InsertType(enbt::type::none);
            insertValue(0);
            return;
        }
        auto check_siz = (int32_t)enbt.size();
        if (check_siz < 0)
            throw std::runtime_error("Unsupported array len");
        if ((size_t)check_siz != enbt.size())
            throw std::runtime_error("Unsupported array len");
        auto base_type = enbt.is_sarray() ? enbt.get_index(0).type_id() : enbt[0].type_id();
        if ((base_type.type == enbt::type::integer || base_type.type == enbt::type::var_integer) && compress) {
            switch (base_type.length) {
            case enbt::type_len::Tiny:
                if (base_type.is_signed) {
                    if (insert_type)
                        nbt_data.push_back(7);
                    BuildBaseIntArray((int32_t)enbt.size(), enbt, base_type);
                    return;
                }
                break;
            case enbt::type_len::Short:
                break;
            case enbt::type_len::Default:
                if (enbt[0].get_type_sign()) {
                    if (insert_type)
                        nbt_data.push_back(11);
                    BuildBaseIntArray((int32_t)enbt.size(), enbt, base_type);
                    return;
                }
                break;
            case enbt::type_len::Long:
                if (enbt[0].get_type_sign()) {
                    if (insert_type)
                        nbt_data.push_back(12);
                    BuildBaseIntArray((int32_t)enbt.size(), enbt, base_type);
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
        BuildArray((int32_t)enbt.size(), enbt, base_type, compress);
    }

    void nbt_enbt_convert::RecursiveBuilder(const enbt::value& enbt, bool insert_type, std::string_view name, bool compress, bool insert_name) {
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
            bool negate_zero = str.ends_with('\0');
            insertValue<uint16_t>(str.size() - negate_zero);
            insertString(str.data(), str.size() - negate_zero);
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
            BuildArray(enbt, insert_type, compress);
            break;
        case enbt::type::compound:
            if (insert_type)
                nbt_data.push_back(10);
            BuildCompound(name, enbt, compress, insert_name);
            break;
        default:
            throw std::runtime_error("Unsupported tag");
        }
    }

#pragma endregion
#pragma region NBT_TO_ENBT

    enbt::value nbt_enbt_convert::RecursiveExtractor_1(uint8_t type, const uint8_t* data, size_t& i, size_t max_size) {
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
            if (i + length > max_size)
                throw std::out_of_range("Out of bounds");
            i += length;
            if (length <= 32 && length >= 16) {
                std::string_view check((const char*)data + i - length, length);
                if (check.find_first_not_of("0123456789ABCDEF-") == std::string_view::npos) {
                    try {
                        enbt::raw_uuid res;
                        auto check_view = enbt::raw_uuid::from_uuid_string(res, check);
                        if (check_view.size() == length)
                            return enbt::value(res);
                    } catch (...) {
                    }
                }
            }
            return enbt::value((const char*)data + i - length, length);
        }
        case 9: { //list
            uint8_t list_type = data[i++];
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (length < 0)
                length = 0;
            std::vector<enbt::value> res;
            res.reserve(length);
            for (int32_t iterate = 0; iterate < length; iterate++)
                res.emplace_back(RecursiveExtractor_1(list_type, data, i, max_size));
            return enbt::value(res, enbt::type_id(enbt::type::array, enbt::type_len::Default));
        }
        case 10: { //compound
            std::unordered_map<std::string, enbt::value> compound;
            while (true) {
                uint8_t compound_type = data[i++];
                if (!compound_type)
                    break;
                uint16_t length = extractValue<uint16_t>(data, i, max_size);
                if (i + length >= max_size)
                    throw std::out_of_range("Out of bounds");
                std::string res(data + i, data + i + length);
                i += length;
                compound[res] = RecursiveExtractor_1(compound_type, data, i, max_size);
            }
            return compound;
        }
        case 11: { //int array
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (i + length * 4 >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length * 4;
            return enbt::value((const int32_t*)data, length, std::endian::big, true);
        }
        case 12: { //long array
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (i + length * 8 >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length * 8;
            return enbt::value((const int64_t*)data, length, std::endian::big, true);
        }
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    enbt::value nbt_enbt_convert::RecursiveExtractor(const uint8_t* data, size_t& i, size_t max_size) {
        if (max_size == 0)
            return enbt::value();
        if (data[0] == 10) {
            //skip first base compound name tag
            i++;
            i += extractValue<uint16_t>(data, i, max_size);
            return RecursiveExtractor_1(10, data, i, max_size);
        }
        return RecursiveExtractor_1(data[i++], data, i, max_size);
    }

    enbt::value nbt_enbt_convert::RecursiveExtractorNetwork(const uint8_t* data, size_t& i, size_t max_size) {
        if (max_size == 0)
            return enbt::value();
        return RecursiveExtractor_1(data[i++], data, i, max_size);
    }

#pragma endregion
#pragma region NBT_TO_NBT

    void nbt_enbt_convert::BuildCompoundItem(std::string_view c_name, const nbt& comp) {
        insertValue(comp.get_type());
        bool negate_zero = c_name.ends_with('\0');
        insertValue<uint16_t>(c_name.size() - negate_zero);
        insertString(c_name.data(), c_name.size() - negate_zero);
        RecursiveBuilder(comp, false, "", false);
    }

    void nbt_enbt_convert::RecursiveBuilder(const nbt& comp, bool insert_type, std::string_view name, bool insert_name) {
        switch (comp.get_type()) {
        case nbt_type::tag_end:
            if (insert_type)
                nbt_data.push_back(0);
            break;
        case nbt_type::tag_byte:
            if (insert_type)
                nbt_data.push_back(1);
            nbt_data.push_back(comp.get_byte());
            break;
        case nbt_type::tag_short:
            if (insert_type)
                nbt_data.push_back(2);
            insertValue(comp.get_short());
            break;
        case nbt_type::tag_int:
            if (insert_type)
                nbt_data.push_back(3);
            insertValue(comp.get_int());
            break;
        case nbt_type::tag_long:
            if (insert_type)
                nbt_data.push_back(4);
            insertValue(comp.get_long());
            break;
        case nbt_type::tag_float:
            if (insert_type)
                nbt_data.push_back(5);
            insertValue(comp.get_float());
            break;
        case nbt_type::tag_double:
            if (insert_type)
                nbt_data.push_back(6);
            insertValue(comp.get_double());
            break;
        case nbt_type::tag_byte_array: {
            if (insert_type)
                nbt_data.push_back(7);
            auto& arr = comp.get_byte_array();
            insertValue<int32_t>(arr.size());
            nbt_data.push_back(arr);
            break;
        }
        case nbt_type::tag_string: {
            if (insert_type)
                nbt_data.push_back(8);
            const std::string& str = comp.get_string();
            bool negate_zero = str.ends_with('\0');
            insertValue<uint16_t>(str.size() - negate_zero);
            insertString(str.data(), str.size() - negate_zero);
            break;
        }
        case nbt_type::tag_list:
            if (insert_type)
                nbt_data.push_back(9);
            auto& arr = comp.get_list();
            if (arr.size())
                insertValue(arr[0].get_type());
            else
                insertValue(nbt_type::tag_end);
            insertValue<int32_t>(arr.size());
            for (auto& it : arr)
                RecursiveBuilder(it, false, "", false);
            break;
        case nbt_type::tag_compound:
            if (insert_type)
                nbt_data.push_back(10);
            auto& components = comp.get_compound();
            if (name.size() > UINT16_MAX)
                throw std::out_of_range("nbt compound is too big to fit in actual format");
            if (insert_name) {
                bool negate_zero = name.ends_with('\0');
                insertValue<uint16_t>(name.size() - negate_zero);
                insertString(name.data(), name.size() - negate_zero);
            }

            for (const auto& [name, tmp] : components)
                BuildCompoundItem(name, tmp);
            InsertType(enbt::type::none);
            break;
        case nbt_type::tag_int_array:
            if (insert_type)
                nbt_data.push_back(11);
            auto& arr = comp.get_int_array();
            insertValue<int32_t>(arr.size());
            for (auto& it : arr)
                insertValue(it);
            break;
        case nbt_type::tag_long_array:
            if (insert_type)
                nbt_data.push_back(12);
            auto& arr = comp.get_long_array();
            insertValue<int32_t>(arr.size());
            for (auto& it : arr)
                insertValue(it);
            break;
        default:
            throw std::runtime_error("Unsupported tag");
        }
    }

    template <class T>
    nbt nbt_enbt_convert::extractArray_NBT(const uint8_t* data, size_t& i, size_t max_size) {
        int32_t len = extractValue<int32_t>(data, i, max_size);
        if (i + sizeof(T) * len >= max_size)
            throw std::out_of_range("Out of bounds");
        list_array<T> ret;
        ret.reserve(len);
        for (int32_t j = 0; j < len; j++)
            ret.push_back(uncheckedExtractValue<T>(data, i));
        return nbt(ret);
    }

    nbt nbt_enbt_convert::RecursiveExtractor_1_NBT(uint8_t type, const uint8_t* data, size_t& i, size_t max_size) {
        switch (type) {
        case 0: //end
            return nbt();
        case 1: //byte
            return nbt(extractValue<int8_t>(data, i, max_size));
        case 2: //short
            return nbt(extractValue<int16_t>(data, i, max_size));
        case 3: //int
            return nbt(extractValue<int32_t>(data, i, max_size));
        case 4: //long
            return nbt(extractValue<int64_t>(data, i, max_size));
        case 5: //float
            return nbt(extractValue<float>(data, i, max_size));
        case 6: //double
            return nbt(extractValue<double>(data, i, max_size));
        case 7: //byte array
            return extractArray_NBT<int8_t>(data, i, max_size);
        case 8: { //string
            uint16_t length = extractValue<uint16_t>(data, i, max_size);
            if (i + length > max_size)
                throw std::out_of_range("Out of bounds");
            i += length;
            return nbt((const char*)data + i - length, length);
        }
        case 9: { //list
            uint8_t list_type = data[i++];
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (length < 0)
                length = 0;
            list_array<nbt> res;
            res.reserve(length);
            for (int32_t iterate = 0; iterate < length; iterate++)
                res.emplace_back(RecursiveExtractor_1_NBT(list_type, data, i, max_size));
            return nbt(res);
        }
        case 10: { //compound
            std::unordered_map<std::string, nbt> compound;
            while (true) {
                uint8_t compound_type = data[i++];
                if (!compound_type)
                    break;
                uint16_t length = extractValue<uint16_t>(data, i, max_size);
                if (i + length >= max_size)
                    throw std::out_of_range("Out of bounds");
                std::string res(data + i, data + i + length);
                i += length;
                compound[res] = RecursiveExtractor_1_NBT(compound_type, data, i, max_size);
            }
            return compound;
        }
        case 11: { //int array
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (i + length * 4 >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length * 4;
            return nbt(list_array<int32_t>((const int32_t*)data, length));
        }
        case 12: { //long array
            int32_t length = extractValue<int32_t>(data, i, max_size);
            if (i + length * 8 >= max_size)
                throw std::out_of_range("Out of bounds");
            i += length * 8;
            return nbt(list_array<int64_t>((const int64_t*)data, length));
        }
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    nbt nbt_enbt_convert::RecursiveExtractor_NBT(const uint8_t* data, size_t& i, size_t max_size) {
        if (max_size == 0)
            return nbt();
        if (data[0] == 10) {
            //skip first base compound name tag
            i++;
            i += extractValue<uint16_t>(data, i, max_size);
            return RecursiveExtractor_1_NBT(10, data, i, max_size);
        }
        return RecursiveExtractor_1_NBT(data[i++], data, i, max_size);
    }

    nbt nbt_enbt_convert::RecursiveExtractorNetwork_NBT(const uint8_t* data, size_t& i, size_t max_size) {
        if (max_size == 0)
            return nbt();
        return RecursiveExtractor_1_NBT(data[i++], data, i, max_size);
    }

#pragma endregion

#pragma region STREAM_TO_NBT

    void nbt_enbt_convert::RecursiveBuilder(nbt_type type, std::istream& stream, bool insert_type) {
        switch (type) {
        case nbt_type::tag_end:
            if (insert_type)
                nbt_data.push_back(0);
            break;
        case nbt_type::tag_byte:
            if (insert_type)
                nbt_data.push_back(1);
            nbt_data.push_back(read_value<int8_t>(stream));
            break;
        case nbt_type::tag_short:
            if (insert_type)
                nbt_data.push_back(2);
            insertValue(read_value<int16_t>(stream));
            break;
        case nbt_type::tag_int:
            if (insert_type)
                nbt_data.push_back(3);
            insertValue(read_value<int32_t>(stream));
            break;
        case nbt_type::tag_long:
            if (insert_type)
                nbt_data.push_back(4);
            insertValue(read_value<int64_t>(stream));
            break;
        case nbt_type::tag_float:
            if (insert_type)
                nbt_data.push_back(5);
            insertValue(read_value<float>(stream));
            break;
        case nbt_type::tag_double:
            if (insert_type)
                nbt_data.push_back(6);
            insertValue(read_value<double>(stream));
            break;
        case nbt_type::tag_byte_array: {
            if (insert_type)
                nbt_data.push_back(7);
            int32_t size = read_value<int32_t>(stream);
            insertValue(size);
            list_array<uint8_t> arr;
            arr.resize(size);
            stream.read((char*)arr.data(), size);
            nbt_data.push_back(arr);
            break;
        }
        case nbt_type::tag_string: {
            if (insert_type)
                nbt_data.push_back(8);
            uint16_t size = read_value<uint16_t>(stream);
            insertValue(size);
            list_array<uint8_t> arr;
            arr.resize(size);
            stream.read((char*)arr.data(), size);
            nbt_data.push_back(arr);
            break;
        }
        case nbt_type::tag_list: {
            if (insert_type)
                nbt_data.push_back(9);
            auto type = read_value<nbt_type>(stream);
            insertValue(type);
            int32_t size = read_value<int32_t>(stream);
            insertValue(size);
            for (int32_t i = 0; i < size; i++)
                RecursiveBuilder(type, stream, false);
            break;
        }
        case nbt_type::tag_compound: {
            if (insert_type)
                nbt_data.push_back(10);
            uint16_t size = read_value<uint16_t>(stream);
            insertValue(size);
            list_array<uint8_t> arr;
            arr.resize(size);
            stream.read((char*)arr.data(), size);
            nbt_data.push_back(arr);
            nbt_type curr_type;
            while ((curr_type = read_value<nbt_type>(stream)) != nbt_type::tag_end) {
                insertValue(curr_type);

                uint16_t size = read_value<uint16_t>(stream);
                insertValue(size);
                list_array<uint8_t> arr;
                arr.resize(size);
                stream.read((char*)arr.data(), size);
                nbt_data.push_back(arr);

                RecursiveBuilder(curr_type, stream, false);
            }
            insertValue(curr_type);
            break;
        }
        case nbt_type::tag_int_array: {
            if (insert_type)
                nbt_data.push_back(11);
            int32_t size = read_value<int32_t>(stream);
            list_array<uint8_t> arr;
            arr.resize(size * sizeof(int32_t));
            stream.read((char*)arr.data(), size * sizeof(int32_t));
            nbt_data.push_back(arr);
            break;
        }
        case nbt_type::tag_long_array: {
            if (insert_type)
                nbt_data.push_back(12);
            int32_t size = read_value<int32_t>(stream);
            list_array<uint8_t> arr;
            arr.resize(size * sizeof(int64_t));
            stream.read((char*)arr.data(), size * sizeof(int64_t));
            nbt_data.push_back(arr);
            break;
        }
        default:
            throw std::runtime_error("Unsupported tag");
        }
    }

#pragma endregion
    nbt_enbt_convert::nbt_enbt_convert() {}

    enbt::value nbt_enbt_convert::readNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractor(data, nbt_size, max_size);
    }

    nbt nbt_enbt_convert::readNBT_asNBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractor_NBT(data, nbt_size, max_size);
    }

    nbt_enbt_convert nbt_enbt_convert::readNBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        return build(readNBT_asNBT(data, max_size, nbt_size));
    }

    enbt::value nbt_enbt_convert::readNetworkNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractorNetwork(data, nbt_size, max_size);
    }

    nbt nbt_enbt_convert::readNetworkNBT_asNBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractorNetwork_NBT(data, nbt_size, max_size);
    }

    nbt_enbt_convert nbt_enbt_convert::readNetworkNBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        return build(readNetworkNBT_asNBT(data, max_size, nbt_size));
    }

    nbt_enbt_convert::nbt_enbt_convert(nbt_enbt_convert&& move)
        : nbt_data(std::move(move.nbt_data)) {}

    nbt_enbt_convert::nbt_enbt_convert(const nbt_enbt_convert& copy) : nbt_data(copy.nbt_data) {}

    nbt_enbt_convert& nbt_enbt_convert::operator=(nbt_enbt_convert&& move) {
        nbt_data = std::move(move.nbt_data);
        return *this;
    }

    nbt_enbt_convert& nbt_enbt_convert::operator=(const nbt_enbt_convert& copy) {
        nbt_data = copy.nbt_data;
        return *this;
    }

    nbt_enbt_convert::~nbt_enbt_convert() = default;

    nbt_enbt_convert nbt_enbt_convert::build(const enbt::value& enbt, bool compress, std::string_view entry_name) {
        nbt_enbt_convert ret;
        ret.RecursiveBuilder(enbt, true, entry_name, compress, true);
        return ret;
    }

    nbt_enbt_convert nbt_enbt_convert::build(const list_array<uint8_t>& data) {
        nbt_enbt_convert ret;
        ret.nbt_data = data;
        return ret;
    }

    nbt_enbt_convert nbt_enbt_convert::build(const nbt& comp, std::string_view entry_name) {
        nbt_enbt_convert ret;
        ret.RecursiveBuilder(comp, true, entry_name, true);
        return ret;
    }

    nbt_enbt_convert nbt_enbt_convert::build_network(const list_array<uint8_t>& data) {
        nbt_enbt_convert ret;
        ret.nbt_data = data;
        uint8_t tmp[] = {0, 0};
        if (ret.nbt_data[0] == 10)
            ret.nbt_data.insert(1, tmp, 2); //add length to fix it

        return ret;
    }

    nbt_enbt_convert nbt_enbt_convert::build(nbt_type type, std::istream& stream) {
        nbt_enbt_convert ret;
        ret.RecursiveBuilder(type, stream, true);
        return ret;
    }

    nbt_enbt_convert nbt_enbt_convert::build_network(nbt_type type, std::istream& stream) {
        nbt_enbt_convert ret;
        ret.RecursiveBuilder(type, stream, true);
        uint8_t tmp[] = {0, 0};
        if (ret.nbt_data[0] == 10)
            ret.nbt_data.insert(1, tmp, 2); //add length to fix it
        return ret;
    }

    nbt_enbt_convert::operator list_array<uint8_t>() const {
        return nbt_data;
    }

    list_array<uint8_t> nbt_enbt_convert::get_as_normal() const {
        return nbt_data;
    }

    list_array<uint8_t> nbt_enbt_convert::get_as_network() const {
        if (nbt_data.size())
            if (nbt_data[0] == 10) {
                list_array<uint8_t> ret = nbt_data;
                ret.erase(1, 3);
                return ret;
            }
        return nbt_data;
    }

    enbt::value nbt_enbt_convert::get_as_enbt() const {
        size_t i = 0;
        return RecursiveExtractor(nbt_data.data(), i, nbt_data.size());
    }

    nbt nbt_enbt_convert::get_as_nbt() const {
        size_t i = 0;
        return RecursiveExtractor_NBT(nbt_data.data(), i, nbt_data.size());
    }

    std::string nbt_enbt_convert::get_entry_name() const {
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

    enbt::value nbt_enbt_convert::extract_from_array_enbt(const uint8_t* arr, size_t& result, size_t max_size) {
        result = 0;
        return RecursiveExtractor(arr, result, max_size);
    }

    nbt nbt_enbt_convert::extract_from_array_nbt(const uint8_t* arr, size_t& result, size_t max_size) {
        result = 0;
        return RecursiveExtractor_NBT(arr, result, max_size);
    }

    list_array<uint8_t> nbt_enbt_convert::take_data() {
        return nbt_data.take();
    }
}
