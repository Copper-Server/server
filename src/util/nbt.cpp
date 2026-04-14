/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <bit>
#include <ctre.hpp>
#include <istream>
#include <regex>
#include <src/base_objects/uuid.hpp>
#include <src/util/endian.hpp>
#include <src/util/nbt.hpp>
#include <src/util/snbt_stream.hpp>

namespace copper_server::util {
    template <class T>
    T read_value(std::istream& read_stream) {
        T res;
        read_stream.read((char*)&res, sizeof(T));
        if constexpr (sizeof(T) != 1)
            return util::convert_endian(std::endian::big, res);
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

    nbt::nbt(float value) : data((void*)static_cast<ptrdiff_t>(std::bit_cast<int32_t>(value))), type(nbt_type::tag_float) {}

    nbt::nbt(double value) : data((void*)std::bit_cast<ptrdiff_t>(value)), type(nbt_type::tag_double) {}

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

    nbt::nbt(const base_objects::uuid& u) : nbt(list_array<int32_t>((int32_t*)u.data, 4)) {}

    nbt::nbt(const base_objects::uuid_hex& u) : nbt(u.to_string()) {}

    nbt::nbt(const base_objects::uuid_flat_hex& u) : nbt(u.to_string_flat()) {}

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

    bool nbt::is_end() const {
        return type == nbt_type::tag_end;
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
            return to_chars_inl(get_byte());
        case nbt_type::tag_short:
            return to_chars_inl(get_short());
        case nbt_type::tag_int:
            return to_chars_inl(get_int());
        case nbt_type::tag_long:
            return to_chars_inl(get_long());
        case nbt_type::tag_float:
            return to_chars_inl(get_float());
        case nbt_type::tag_double:
            return to_chars_inl(get_double());
        case nbt_type::tag_string:
            return get_string();
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    base_objects::uuid nbt::as_uuid() const {
        switch (type) {
        case nbt_type::tag_byte_array: {
            if (get_int_array().size() != 16)
                throw std::runtime_error("Invalid array size");
            return *std::launder<base_objects::uuid>((base_objects::uuid*)get_int_array().data());
        }
        case nbt_type::tag_int_array: {
            if (get_int_array().size() != 4)
                throw std::runtime_error("Invalid array size");
            return *std::launder<base_objects::uuid>((base_objects::uuid*)get_int_array().data());
        }
        case nbt_type::tag_long_array: {
            if (get_int_array().size() != 2)
                throw std::runtime_error("Invalid array size");
            return *std::launder<base_objects::uuid>((base_objects::uuid*)get_int_array().data());
        }
        case nbt_type::tag_string: {
            base_objects::uuid res;
            base_objects::uuid::from_uuid_string(res, get_string(), true);
            return res;
        }
        default:
            throw std::runtime_error("Invalid type");
        }
    }

    nbt& nbt::operator[](const std::string& key) {
        return get_compound()[key];
    }

    nbt& nbt::at(const std::string& key) {
        return get_compound().at(key);
    }

    const nbt& nbt::at(const std::string& key) const {
        return get_compound().at(key);
    }

    bool nbt::contains(const std::string& key) const {
        return get_compound().contains(key);
    }

    void nbt::remove(const std::string& key) {
        get_compound().erase(key);
    }

    nbt& nbt::operator[](size_t index) {
        return get_list()[index];
    }

    nbt& nbt::at(size_t index) {
        return get_list().at(index);
    }

    const nbt& nbt::at(size_t index) const {
        return get_list().at(index);
    }

    std::string snbt_enclose_string(std::string_view str) {
        if (!ctre::starts_with<"[0-9\\-\\.\\+]">(str) && ctre::match<"[a-zA-Z0-9_\\-\\.\\+]">(str).size() == str.size())
            return std::string(str);
        else {
            std::string escaped = "\"";
            for (char c : str) {
                switch (c) {
                case '\n':
                    escaped += "\\n";
                    break;
                case '\r':
                    escaped += "\\r";
                    break;
                case '\t':
                    escaped += "\\t";
                    break;
                case '\"':
                    escaped += "\\\"";
                    break;
                case '\\':
                    escaped += "\\\\";
                    break;
                default:
                    escaped += c;
                    break;
                }
            }
            escaped += "\"";
            return escaped;
        }
    }

    template <class T>
    std::string to_chars_inl(T value) {
        char buffer[64];
        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
        if (ec == std::errc())
            return std::string(buffer, ptr);
        else
            throw std::runtime_error("Conversion to chars failed");
    }

    std::string nbt::as_snbt() const {
        switch (type) {
        case nbt_type::tag_end:
            return "";
        case nbt_type::tag_byte:
            return to_chars_inl(get_byte()) + "b";
        case nbt_type::tag_short:
            return to_chars_inl(get_short()) + "s";
        case nbt_type::tag_int:
            return to_chars_inl(get_int());
        case nbt_type::tag_long:
            return to_chars_inl(get_long()) + "l";
        case nbt_type::tag_float:
            return to_chars_inl(get_float()) + "f";
        case nbt_type::tag_double:
            return to_chars_inl(get_double()) + "d";
        case nbt_type::tag_byte_array: {
            const auto& arr = get_byte_array();
            std::string result = "[B;";
            for (size_t i = 0; i < arr.size(); i++) {
                if (i > 0)
                    result += ",";
                result += to_chars_inl(arr[i]) + "b";
            }
            result += "]";
            return result;
        }
        case nbt_type::tag_string:
            return snbt_enclose_string(get_string());
        case nbt_type::tag_list: {
            const auto& list = get_list();
            std::string result = "[";
            for (size_t i = 0; i < list.size(); i++) {
                if (i > 0)
                    result += ",";
                result += list[i].as_snbt();
            }
            result += "]";
            return result;
        }
        case nbt_type::tag_compound: {
            const auto& comp = get_compound();
            std::string result = "{";
            bool first = true;
            for (const auto& pair : comp) {
                if (!first)
                    result += ",";
                first = false;
                result += snbt_enclose_string(pair.first) + ":" + pair.second.as_snbt();
            }
            result += "}";
            return result;
        }
        case nbt_type::tag_int_array: {
            const auto& arr = get_int_array();
            std::string result = "[I;";
            for (size_t i = 0; i < arr.size(); i++) {
                if (i > 0)
                    result += ",";
                result += to_chars_inl(arr[i]);
            }
            result += "]";
            return result;
        }
        case nbt_type::tag_long_array: {
            const auto& arr = get_long_array();
            std::string result = "[L;";
            for (size_t i = 0; i < arr.size(); i++) {
                if (i > 0)
                    result += ",";
                result += to_chars_inl(arr[i]) + "l";
            }
            result += "]";
            return result;
        }
        default:
            break;
        }
    }

    nbt nbt::from_snbt(const std::string& snbt) {
        return parse_snbt(snbt);
    }

#pragma endregion


#pragma region NBT_COMPOUND

    nbt_compound::nbt_compound() = default;

    nbt_compound::nbt_compound(const std::unordered_map<std::string, nbt>& data) : compound_data(data) {}

    nbt_compound::nbt_compound(std::unordered_map<std::string, nbt>&& data) : compound_data(std::move(data)) {}

    nbt_compound::nbt_compound(const nbt_compound& copy) : compound_data(copy.compound_data) {}

    nbt_compound::nbt_compound(nbt_compound&& move) : compound_data(std::move(move.compound_data)) {}

    nbt_compound::nbt_compound(const nbt& nbt_val) : compound_data(nbt_val.get_compound()) {}

    nbt_compound::nbt_compound(nbt&& nbt_val) : compound_data(std::move(nbt_val.get_compound())) {}

    nbt_compound& nbt_compound::operator=(const nbt_compound& copy) {
        compound_data = copy.compound_data;
        return *this;
    }

    nbt_compound& nbt_compound::operator=(nbt_compound&& move) {
        compound_data = std::move(move.compound_data);
        return *this;
    }

    nbt_compound& nbt_compound::operator=(const nbt& nbt_val) {
        compound_data = nbt_val.get_compound();
        return *this;
    }

    nbt_compound& nbt_compound::operator=(nbt&& nbt_val) {
        compound_data = std::move(nbt_val.get_compound());
        return *this;
    }

    nbt& nbt_compound::operator[](const std::string& key) {
        return compound_data[key];
    }

    const nbt& nbt_compound::operator[](const std::string& key) const {
        return compound_data.at(key);
    }

    nbt& nbt_compound::at(const std::string& key) {
        return compound_data.at(key);
    }

    const nbt& nbt_compound::at(const std::string& key) const {
        return compound_data.at(key);
    }

    void nbt_compound::set(const std::string& key, const nbt& value) {
        compound_data[key] = value;
    }

    void nbt_compound::set(const std::string& key, nbt&& value) {
        compound_data[key] = std::move(value);
    }

    bool nbt_compound::contains(const std::string& key) const {
        return compound_data.contains(key);
    }

    void nbt_compound::remove(const std::string& key) {
        compound_data.erase(key);
    }

    size_t nbt_compound::size() const {
        return compound_data.size();
    }

    void nbt_compound::clear() {
        compound_data.clear();
    }

    bool nbt_compound::empty() const {
        return compound_data.empty();
    }

    void nbt_compound::reserve(size_t max_count) {
        return compound_data.reserve(max_count);
    }

    std::unordered_map<std::string, nbt>& nbt_compound::get_map() {
        return compound_data;
    }

    const std::unordered_map<std::string, nbt>& nbt_compound::get_map() const {
        return compound_data;
    }

    nbt nbt_compound::take_map() & {
        return std::move(compound_data);
    }

    nbt nbt_compound::take_map() && {
        return std::move(compound_data);
    }

    nbt_compound::operator std::unordered_map<std::string, nbt>() const {
        return compound_data;
    }

#pragma endregion

    template <class T>
    void nbt_convert::insertValue(T val, size_t max) {
        val = convert_endian(std::endian::big, val);
        uint8_t* proxy = (uint8_t*)&val;
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back(proxy[i]);
    }

    template <class Target, class T>
    void nbt_convert::insertValue(T val, size_t max) {
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
    T nbt_convert::uncheckedExtractValue(const uint8_t* data, size_t& i) {
        uint8_t tmp[sizeof(T)];
        for (size_t j = 0; j < sizeof(T); j++)
            tmp[j] = data[i++];
        return util::convert_endian(std::endian::big, *(T*)tmp);
    }

    template <class T>
    T nbt_convert::extractValue(const uint8_t* data, size_t& i, size_t max_size) {
        if (i + sizeof(T) >= max_size)
            throw std::out_of_range("Out of bounds");
        return uncheckedExtractValue<T>(data, i);
    }

    void nbt_convert::insertString(const char* val, size_t max) {
        for (size_t i = 0; i < max; i++)
            nbt_data.push_back((uint8_t)val[i]);
    }

#pragma region NBT_TO_NBT

    void nbt_convert::BuildCompoundItem(std::string_view c_name, const nbt& comp) {
        insertValue(comp.get_type());
        bool negate_zero = c_name.ends_with('\0');
        insertValue<uint16_t>(c_name.size() - negate_zero);
        insertString(c_name.data(), c_name.size() - negate_zero);
        RecursiveBuilder(comp, false, "", false);
    }

    void nbt_convert::RecursiveBuilder(const nbt& comp, bool insert_type, std::string_view name, bool insert_name) {
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
            insertValue(nbt_type::tag_end);
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
    nbt nbt_convert::extractArray_NBT(const uint8_t* data, size_t& i, size_t max_size) {
        int32_t len = extractValue<int32_t>(data, i, max_size);
        if (i + sizeof(T) * len >= max_size)
            throw std::out_of_range("Out of bounds");
        list_array<T> ret;
        ret.reserve(len);
        for (int32_t j = 0; j < len; j++)
            ret.push_back(uncheckedExtractValue<T>(data, i));
        return nbt(ret);
    }

    nbt nbt_convert::RecursiveExtractor_1_NBT(uint8_t type, const uint8_t* data, size_t& i, size_t max_size) {
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

    nbt nbt_convert::RecursiveExtractor_NBT(const uint8_t* data, size_t& i, size_t max_size) {
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

    nbt nbt_convert::RecursiveExtractorNetwork_NBT(const uint8_t* data, size_t& i, size_t max_size) {
        if (max_size == 0)
            return nbt();
        return RecursiveExtractor_1_NBT(data[i++], data, i, max_size);
    }

#pragma endregion

#pragma region STREAM_TO_NBT

    void nbt_convert::RecursiveBuilder(nbt_type type, std::istream& stream, bool insert_type) {
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

    nbt_convert::nbt_convert() {}

    nbt nbt_convert::readNBT_asNBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractor_NBT(data, nbt_size, max_size);
    }

    nbt_convert nbt_convert::readNBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        return build(readNBT_asNBT(data, max_size, nbt_size));
    }

    nbt nbt_convert::readNetworkNBT_asNBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        nbt_size = 0;
        return RecursiveExtractorNetwork_NBT(data, nbt_size, max_size);
    }

    nbt_convert nbt_convert::readNetworkNBT(const uint8_t* data, size_t max_size, size_t& nbt_size) {
        return build(readNetworkNBT_asNBT(data, max_size, nbt_size));
    }

    nbt_convert::nbt_convert(nbt_convert&& move)
        : nbt_data(std::move(move.nbt_data)) {}

    nbt_convert::nbt_convert(const nbt_convert& copy) : nbt_data(copy.nbt_data) {}

    nbt_convert& nbt_convert::operator=(nbt_convert&& move) {
        nbt_data = std::move(move.nbt_data);
        return *this;
    }

    nbt_convert& nbt_convert::operator=(const nbt_convert& copy) {
        nbt_data = copy.nbt_data;
        return *this;
    }

    nbt_convert::~nbt_convert() = default;

    nbt_convert nbt_convert::build(const list_array<uint8_t>& data) {
        nbt_convert ret;
        ret.nbt_data = data;
        return ret;
    }

    nbt_convert nbt_convert::build(const nbt& comp, std::string_view entry_name) {
        nbt_convert ret;
        ret.RecursiveBuilder(comp, true, entry_name, true);
        return ret;
    }

    nbt_convert nbt_convert::build_network(const list_array<uint8_t>& data) {
        nbt_convert ret;
        ret.nbt_data = data;
        uint8_t tmp[] = {0, 0};
        if (ret.nbt_data[0] == 10)
            ret.nbt_data.insert(1, tmp, 2); //add length to fix it

        return ret;
    }

    nbt_convert nbt_convert::build(nbt_type type, std::istream& stream) {
        nbt_convert ret;
        ret.RecursiveBuilder(type, stream, true);
        return ret;
    }

    nbt_convert nbt_convert::build_network(nbt_type type, std::istream& stream) {
        nbt_convert ret;
        ret.RecursiveBuilder(type, stream, true);
        uint8_t tmp[] = {0, 0};
        if (ret.nbt_data[0] == 10)
            ret.nbt_data.insert(1, tmp, 2); //add length to fix it
        return ret;
    }

    nbt_convert::operator list_array<uint8_t>() const {
        return nbt_data;
    }

    list_array<uint8_t> nbt_convert::get_as_normal() const {
        return nbt_data;
    }

    list_array<uint8_t> nbt_convert::get_as_network() const {
        if (nbt_data.size())
            if (nbt_data[0] == 10) {
                list_array<uint8_t> ret = nbt_data;
                ret.erase(1, 3);
                return ret;
            }
        return nbt_data;
    }

    nbt nbt_convert::get_as_nbt() const {
        size_t i = 0;
        return RecursiveExtractor_NBT(nbt_data.data(), i, nbt_data.size());
    }

    std::string nbt_convert::get_entry_name() const {
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

    nbt nbt_convert::extract_from_array_nbt(const uint8_t* arr, size_t& result, size_t max_size) {
        result = 0;
        return RecursiveExtractor_NBT(arr, result, max_size);
    }

    list_array<uint8_t> nbt_convert::take_data() {
        return nbt_data.take();
    }
}
