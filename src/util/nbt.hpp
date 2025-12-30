/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_NBT
#define SRC_UTIL_NBT

#include <iosfwd>
#include <library/list_array.hpp>
#include <unordered_map>

namespace copper_server::util {
    enum class nbt_type : uint8_t {
        tag_end = 0,
        tag_byte = 1,
        tag_short = 2,
        tag_int = 3,
        tag_long = 4,
        tag_float = 5,
        tag_double = 6,
        tag_byte_array = 7,
        tag_string = 8,
        tag_list = 9,
        tag_compound = 10,
        tag_int_array = 11,
        tag_long_array = 12
    };

    class nbt {
        void* data;
        nbt_type type;

        void clear();

    public:
        nbt();

        nbt(int8_t);
        nbt(int16_t);
        nbt(int32_t);
        nbt(int64_t);
        nbt(float);
        nbt(double);
        nbt(const list_array<uint8_t>&);
        nbt(list_array<uint8_t>&&);
        nbt(const std::string&);
        nbt(std::string&&);
        nbt(const char*);
        nbt(const char*, size_t);
        nbt(std::string_view);
        nbt(const list_array<nbt>&);
        nbt(list_array<nbt>&&);
        nbt(const std::unordered_map<std::string, nbt>&);
        nbt(std::unordered_map<std::string, nbt>&&);
        nbt(const list_array<int32_t>&);
        nbt(list_array<int32_t>&&);
        nbt(const list_array<int64_t>&);
        nbt(list_array<int64_t>&&);

        nbt(const nbt& copy);
        nbt(nbt&& move);

        ~nbt();

        nbt& operator=(const nbt& copy);
        nbt& operator=(nbt&& move);

        bool operator==(const nbt& move) const;
        bool operator!=(const nbt& move) const;

        nbt_type get_type() const noexcept;


        int8_t get_byte() const;
        int16_t get_short() const;
        int32_t get_int() const;
        int64_t get_long() const;
        float get_float() const;
        double get_double() const;
        const list_array<uint8_t>& get_byte_array() const;
        const std::string& get_string() const;
        const list_array<nbt>& get_list() const;
        const std::unordered_map<std::string, nbt>& get_compound() const;
        const list_array<int32_t>& get_int_array() const;
        const list_array<int64_t>& get_long_array() const;

        int8_t& get_byte();
        int16_t& get_short();
        int32_t& get_int();
        int64_t& get_long();
        float& get_float();
        double& get_double();
        list_array<uint8_t>& get_byte_array();
        std::string& get_string();
        list_array<nbt>& get_list();
        std::unordered_map<std::string, nbt>& get_compound();
        list_array<int32_t>& get_int_array();
        list_array<int64_t>& get_long_array();

        bool is_end() const;
        bool is_byte() const;
        bool is_short() const;
        bool is_int() const;
        bool is_long() const;
        bool is_float() const;
        bool is_double() const;
        bool is_byte_array() const;
        bool is_string() const;
        bool is_list() const;
        bool is_compound() const;
        bool is_int_array() const;
        bool is_long_array() const;

        int8_t as_byte() const;
        int16_t as_short() const;
        int32_t as_int() const;
        int64_t as_long() const;
        float as_float() const;
        double as_double() const;
        std::string as_string() const;


        nbt& at(const std::string& key);
        const nbt& at(const std::string& key) const;

        nbt& at(size_t index);
        const nbt& at(size_t index) const;

        std::string as_snbt() const;
        static nbt from_snbt(const std::string& snbt);
    };

    class nbt_compound {
        std::unordered_map<std::string, nbt> compound_data;

    public:
        nbt_compound();
        nbt_compound(const std::unordered_map<std::string, nbt>& data);
        nbt_compound(std::unordered_map<std::string, nbt>&& data);
        nbt_compound(const nbt_compound& copy);
        nbt_compound(nbt_compound&& move);
        nbt_compound(const nbt& nbt_val);
        nbt_compound(nbt&& nbt_val);

        nbt_compound& operator=(const nbt_compound& copy);
        nbt_compound& operator=(nbt_compound&& move);
        nbt_compound& operator=(const nbt& nbt_val);
        nbt_compound& operator=(nbt&& nbt_val);

        nbt& operator[](const std::string& key);
        const nbt& operator[](const std::string& key) const;

        nbt& at(const std::string& key);
        const nbt& at(const std::string& key) const;

        void set(const std::string& key, const nbt& value);
        void set(const std::string& key, nbt&& value);

        bool contains(const std::string& key) const;
        void remove(const std::string& key);
        size_t size() const;
        void clear();
        bool empty() const;

        std::unordered_map<std::string, nbt>& get_map();
        const std::unordered_map<std::string, nbt>& get_map() const;

        nbt take_map() &;
        nbt take_map() &&;

        operator std::unordered_map<std::string, nbt>() const;
    };

    class nbt_convert {
        list_array<uint8_t> nbt_data;


        template <class T>
        void insertValue(T val, size_t max = sizeof(T));
        template <class Target, class T>
        void insertValue(T val, size_t max = sizeof(T));
        void insertString(const char* val, size_t max);
        template <class T>
        static T uncheckedExtractValue(const uint8_t* data, size_t& i);
        template <class T>
        static T extractValue(const uint8_t* data, size_t& i, size_t max_size);

#pragma region NBT_TO_NBT
        void BuildCompoundItem(std::string_view c_name, const nbt& comp);
        void RecursiveBuilder(const nbt& nbt, bool insert_type, std::string_view name, bool in_array);

        template <class T>
        static nbt extractArray_NBT(const uint8_t* data, size_t& i, size_t max_size);

        static nbt RecursiveExtractor_1_NBT(uint8_t type, const uint8_t* data, size_t& i, size_t max_size);
        static nbt RecursiveExtractor_NBT(const uint8_t* data, size_t& i, size_t max_size);
        static nbt RecursiveExtractorNetwork_NBT(const uint8_t* data, size_t& i, size_t max_size);

#pragma endregion

#pragma region STREAM_TO_NBT
        void RecursiveBuilder(nbt_type type, std::istream& stream, bool insert_type);
#pragma endregion


    public:
        static nbt readNBT_asNBT(const uint8_t* data, size_t max_size, size_t& nbt_size);
        static nbt_convert readNBT(const uint8_t* data, size_t max_size, size_t& nbt_size);

        static nbt readNetworkNBT_asNBT(const uint8_t* data, size_t max_size, size_t& nbt_size);
        static nbt_convert readNetworkNBT(const uint8_t* data, size_t max_size, size_t& nbt_size);

        nbt_convert();

        nbt_convert(nbt_convert&& move);
        nbt_convert(const nbt_convert& copy);
        nbt_convert& operator=(nbt_convert&& move);
        nbt_convert& operator=(const nbt_convert& copy);
        ~nbt_convert();

        static nbt_convert build(const nbt& nbt, std::string_view entry_name = "");
        static nbt_convert build(const list_array<uint8_t>& data);
        static nbt_convert build_network(const list_array<uint8_t>& data);
        static nbt_convert build_snbt(const std::string& snbt);

        static nbt_convert build(nbt_type type, std::istream& stream);
        static nbt_convert build_network(nbt_type type, std::istream& stream);
        operator list_array<uint8_t>() const;
        list_array<uint8_t> get_as_normal() const;
        list_array<uint8_t> get_as_network() const;
        nbt get_as_nbt() const;
        std::string get_entry_name() const;
        static nbt extract_from_array_nbt(const uint8_t* arr, size_t& result, size_t max_size);

        list_array<uint8_t> take_data();


        std::string to_snbt() const;
    };
}

#endif /* SRC_UTIL_NBT */
