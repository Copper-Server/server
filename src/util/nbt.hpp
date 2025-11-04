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

#include <library/enbt/enbt.hpp>
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

        nbt_compound& operator=(const nbt_compound& copy);
        nbt_compound& operator=(nbt_compound&& move);
        nbt_compound& operator=(const nbt& nbt_val);

        nbt& operator[](const std::string& key);
        const nbt& operator[](const std::string& key) const;

        void set(const std::string& key, const nbt& value);
        void set(const std::string& key, nbt&& value);

        bool contains(const std::string& key) const;
        void remove(const std::string& key);
        size_t size() const;
        void clear();
        bool empty() const;

        std::unordered_map<std::string, nbt>& get_map();
        const std::unordered_map<std::string, nbt>& get_map() const;

        operator nbt() const;
        operator std::unordered_map<std::string, nbt>() const;
    };

    //bridge class between ENBT and nbt formats
    class nbt_enbt_convert {
        list_array<uint8_t> nbt_data;

#pragma region ENBT_TO_NBT

        template <class T>
        void insertValue(T val, size_t max = sizeof(T));

        template <class Target, class T>
        void insertValue(T val, size_t max = sizeof(T));
        void insertString(const char* val, size_t max);
        void IntegerInsert(const enbt::value& val, bool typ_ins);
        void FloatingInsert(const enbt::value& val, bool typ_ins);
        void BuildCompoundItem(const std::string& c_name, const enbt::value& comp, bool compress);
        void BuildCompound(const std::string& c_name, const enbt::value& comp, bool compress, bool in_array);
        void InsertType(enbt::type_id t);
        void BuildBaseIntArray(int32_t len, const enbt::value& arr, enbt::type_id base_id);
        void BuildSimpleIntArray(int32_t len, const enbt::value& arr, enbt::type_id base_id);
        void BuildArray(int32_t len, const enbt::value& arr, enbt::type_id base_id, bool compress);
        void BuildArray(const enbt::value& enbt, bool insert_type, bool compress);
        void RecursiveBuilder(const enbt::value& enbt, bool insert_type, const std::string& name, bool compress, bool in_array);
#pragma endregion
#pragma region NBT_TO_ENBT

        template <class T>
        static T uncheckedExtractValue(const uint8_t* data, size_t& i);
        template <class T>
        static T extractValue(const uint8_t* data, size_t& i, size_t max_size);
        template <class T>
        static enbt::value extractArray(const uint8_t* data, size_t& i, size_t max_size);

        static enbt::value RecursiveExtractor_1(uint8_t type, const uint8_t* data, size_t& i, size_t max_size);
        static enbt::value RecursiveExtractor(const uint8_t* data, size_t& i, size_t max_size);
        static enbt::value RecursiveExtractorNetwork(const uint8_t* data, size_t& i, size_t max_size);

#pragma endregion

        nbt_enbt_convert();

    public:
        static enbt::value readNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size);
        static nbt_enbt_convert readNBT(const uint8_t* data, size_t max_size, size_t& nbt_size, bool compress = true, const std::string& entry_name = "");

        static enbt::value readNetworkNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size);
        static nbt_enbt_convert readNetworkNBT(const uint8_t* data, size_t max_size, size_t& nbt_size, bool compress = true, const std::string& entry_name = "");

        nbt_enbt_convert(nbt_enbt_convert&& move);
        ~nbt_enbt_convert();

        static nbt_enbt_convert build(const enbt::value& enbt, bool compress = true, const std::string& entry_name = "");
        static nbt_enbt_convert build(const list_array<uint8_t>& data);
        static nbt_enbt_convert build_network(const list_array<uint8_t>& data);
        operator list_array<uint8_t>() const;
        list_array<uint8_t> get_as_normal() const;
        list_array<uint8_t> get_as_network() const;
        enbt::value get_as_enbt() const;
        std::string get_entry_name() const;
        static enbt::value extract_from_array(const uint8_t* arr, size_t& result, size_t max_size);
    };
}

#endif /* SRC_UTIL_NBT */
