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

namespace copper_server::util {
    //bridge class between ENBT and NBT formats
    class NBT {
        list_array<uint8_t> nbt_data;

#pragma region ENBT_TO_NBT

        template <class T>
        void insertValue(T val, size_t max = sizeof(T));

        template <class Target, class T>
        void insertValue(T val, size_t max = sizeof(T));
        void insertString(const char* val, size_t max);
        void IntegerInsert(enbt::value& val, bool typ_ins);
        void FloatingInsert(enbt::value& val, bool typ_ins);
        void BuildCompoundItem(const std::string& c_name, enbt::value& comp, bool compress);
        void BuildCompound(const std::string& c_name, enbt::value& comp, bool compress, bool in_array);
        void InsertType(enbt::type_id t);
        void BuildBaseIntArray(int32_t len, enbt::value& arr, enbt::type_id base_id);
        void BuildSimpleIntArray(int32_t len, enbt::value& arr, enbt::type_id base_id);
        void BuildArray(int32_t len, enbt::value& arr, enbt::type_id base_id, bool compress);
        void BuildArray(enbt::value& enbt, bool insert_type, bool compress);
        void RecursiveBuilder(enbt::value& enbt, bool insert_type, const std::string& name, bool compress, bool in_array);
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

        NBT();

    public:
        static enbt::value readNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size);
        static NBT readNBT(const uint8_t* data, size_t max_size, size_t& nbt_size, bool compress = true, const std::string& entry_name = "");

        static enbt::value readNetworkNBT_asENBT(const uint8_t* data, size_t max_size, size_t& nbt_size);
        static NBT readNetworkNBT(const uint8_t* data, size_t max_size, size_t& nbt_size, bool compress = true, const std::string& entry_name = "");

        NBT(NBT&& move);
        ~NBT();

        static NBT build(const enbt::value& enbt, bool compress = true, const std::string& entry_name = "");
        static NBT build(const list_array<uint8_t>& data);
        static NBT build_network(const list_array<uint8_t>& data);
        operator list_array<uint8_t>() const;
        list_array<uint8_t> get_as_normal() const;
        list_array<uint8_t> get_as_network() const;
        enbt::value get_as_enbt() const;
        std::string get_entry_name() const;
        static enbt::value extract_from_array(const uint8_t* arr, size_t& result, size_t max_size);
    };
}

#endif /* SRC_UTIL_NBT */
