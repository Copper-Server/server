/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_READERS
#define SRC_UTIL_READERS
#include <bit>
#include <cstdint>
#include <exception>
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <src/util/nbt.hpp>
#include <string>

namespace copper_server {
    namespace util {
        template <class T>
        static T fromVar(uint8_t* ch, size_t& len) {
            constexpr int max_offset = (sizeof(T) / 5 * 5 + ((sizeof(T) % 5) > 0)) * 8;
            T decodedInt = 0;
            T bitOffset = 0;
            char currentByte = 0;
            size_t i = 0;
            do {
                if (i >= len)
                    throw std::overflow_error("VarInt is too big");
                if (bitOffset == max_offset)
                    throw std::overflow_error("VarInt is too big");
                currentByte = ch[i++];
                decodedInt |= (currentByte & 0b01111111) << bitOffset;
                bitOffset += 7;
            } while ((currentByte & 0b10000000) != 0);
            len = i;
            return decodedInt;
        }

        template <class T>
        static size_t toVar(uint8_t* buf, size_t buf_len, T val) {
            size_t i = 0;
            do {
                if (i >= buf_len)
                    throw std::overflow_error("VarInt is too big");
                buf[i] = (uint8_t)(val & 0b01111111);
                val >>= 7;
                if (val != 0)
                    buf[i] |= 0b10000000;
                i++;
            } while (val != 0);
            return i;
        }
    }
    struct ArrayStream {
        uint8_t* arr_;
        size_t mi;

        ArrayStream(uint8_t* arr, size_t size) {
            arr_ = arr;
            mi = size;
            read_only = false;
        }

        ArrayStream(const uint8_t* arr, size_t size) {
            arr_ = const_cast<uint8_t*>(arr);
            mi = size;
            read_only = true;
        }

        size_t r = 0;
        size_t w = 0;

        void write(uint8_t value) {
            if (mi <= w)
                throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try write in: " + std::to_string(w));
            if (read_only)
                throw std::exception("Readonly Mode");
            arr_[w++] = value;
        }

        uint8_t read() {
            if (mi <= r)
                throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try read in: " + std::to_string(r));
            return arr_[r++];
        }

        uint8_t peek() {
            if (mi <= r)
                throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try read in: " + std::to_string(r));
            return arr_[r];
        }

        bool empty() {
            return r >= mi;
        }

        ArrayStream range(size_t start, size_t end) {
            if (start > end)
                throw std::out_of_range("start > end");
            if (end > mi)
                throw std::out_of_range("end > max_index");
            return ArrayStream(arr_ + start, end - start);
        }

        ArrayStream range_read(size_t size) {
            if (r + size > mi)
                throw std::out_of_range("r + size > max_index");

            ArrayStream tmp(arr_ + r, size);
            r += size;
            return tmp;
        }

        ArrayStream read_left(size_t max_size = SIZE_MAX) {
            size_t left = mi - r;
            if (left > max_size)
                throw std::out_of_range("array len out of range");

            ArrayStream tmp(arr_ + r, left);
            r = mi;
            return tmp;
        }

        size_t size_read() {
            return mi - r;
        }

        uint8_t* data_read() {
            return arr_ + r;
        }

        size_t size_write() {
            return mi - w;
        }

        uint8_t* data_write() {
            return arr_ + w;
        }

        bool can_read(size_t size) {
            return r + size <= mi;
        }

        void sync_to_read() {
            r = w;
        }

        void sync_to_write() {
            w = r;
        }

        list_array<uint8_t> to_vector() {
            return list_array<uint8_t>(arr_ + r, arr_ + mi + r);
        }

        template <class Res>
        Res read_var() {
            size_t len = sizeof(Res);
            Res res = util::fromVar<Res>(arr_ + r, len);
            r += len;
            enbt::endian_helpers::convert_endian(std::endian::little, res);
            return res;
        }

        enbt::raw_uuid read_uuid() {
            enbt::raw_uuid temp;
            uint8_t* tmp = (uint8_t*)&temp;
            for (size_t i = 0; i < 16; i++)
                tmp[i] = read();
            return enbt::endian_helpers::convert_endian(std::endian::big, temp);
        }

        template <class T>
        T read_value() {
            uint8_t tmp[sizeof(T)];
            for (size_t i = 0; i < sizeof(T); i++)
                tmp[i] = read();
            return enbt::endian_helpers::convert_endian(std::endian::big, *(T*)tmp);
        }

        std::string read_string(int32_t max_string_len = INT32_MAX) {
            std::string res = "";
            int32_t actual_len = read_var<int32_t>();
            if (actual_len > max_string_len)
                throw std::out_of_range("actual string len out of range");
            if (actual_len < 0)
                throw std::out_of_range("actual string len out of range");
            for (int32_t i = 0; i < actual_len;) {
                char tmp = (char)read();
                i += (tmp & 0xc0) != 0x80;
                res += tmp;
            }
            return res;
        }

        std::string read_identifier() {
            return read_string(32767);
        }

        std::string read_json_component() {
            return read_string(262144);
        }


        template <class T>
        list_array<T> read_array(int32_t max_len = INT32_MAX) {
            int32_t len = read_var<int32_t>();
            if (len < 0)
                throw std::out_of_range("array len out of range");
            if (len < max_len)
                throw std::out_of_range("array len out of range");
            list_array<T> res;
            res.reserve(len);
            for (int32_t i = 0; i < len; i++)
                res.push_back(read_value<T>());
            return res;
        }

        list_array<uint8_t> read_list_array(int32_t max_len = INT32_MAX) {
            int32_t len = read_var<int32_t>();
            if (len < 0)
                throw std::out_of_range("list array len out of range");
            if (len < max_len)
                throw std::out_of_range("list array len out of range");
            list_array<uint8_t> res;
            res.reserve(len);
            for (int32_t i = 0; i < len; i++)
                res.push_back(read());
            return res;
        }

    private:
        bool read_only;
    };

    template <class ResultT, class T>
    static void WriteVar(T val, list_array<uint8_t>& data) {
        if constexpr (!std::is_same<ResultT, T>::value) {
            if constexpr (std::is_unsigned_v<ResultT> == std::is_unsigned_v<T>) {
                if ((ResultT)val != val)
                    throw std::out_of_range("Value out of range");
            } else if constexpr (std::is_unsigned_v<ResultT>) {
                if (val < 0)
                    throw std::out_of_range("Value out of range");
                if ((ResultT)val != (std::make_unsigned_t<T>)val)
                    throw std::out_of_range("Value out of range");
            } else if constexpr (std::is_unsigned_v<T>) {
                auto chk = (ResultT)val;
                if (chk < 0)
                    throw std::out_of_range("Value out of range");
                if ((std::make_unsigned_t<ResultT>)val != (T)val)
                    throw std::out_of_range("Value out of range");
            }
        }

        constexpr size_t buf_len = sizeof(ResultT) + (sizeof(ResultT) / 7) + 1;
        uint8_t buf[buf_len];
        size_t len = util::toVar(buf, buf_len, enbt::endian_helpers::convert_endian<ResultT>(std::endian::little, (ResultT)val));
        for (size_t i = 0; i < len; i++)
            data.push_back(buf[i]);
    }

    template <class ResultT, class T>
    static void WriteVar(T val, ArrayStream& data) {
        if constexpr (!std::is_same<ResultT, T>::value) {
            if constexpr (std::is_unsigned_v<ResultT> == std::is_unsigned_v<T>) {
                if ((ResultT)val != val)
                    throw std::out_of_range("Value out of range");
            } else if constexpr (std::is_unsigned_v<ResultT>) {
                if (val < 0)
                    throw std::out_of_range("Value out of range");
                if ((ResultT)val != (std::make_unsigned_t<T>)val)
                    throw std::out_of_range("Value out of range");
            } else if constexpr (std::is_unsigned_v<T>) {
                auto chk = (ResultT)val;
                if (chk < 0)
                    throw std::out_of_range("Value out of range");
                if ((std::make_unsigned_t<ResultT>)val != (T)val)
                    throw std::out_of_range("Value out of range");
            }
        }

        constexpr size_t buf_len = sizeof(T) + (sizeof(T) / 7) + 1;
        uint8_t buf[buf_len];
        size_t len = util::toVar(buf, buf_len, enbt::endian_helpers::convert_endian<ResultT>(std::endian::little, (ResultT)val));
        for (size_t i = 0; i < len; i++)
            data.write(buf[i]);
    }

    static void WriteUUID(const enbt::raw_uuid& val, list_array<uint8_t>& data) {
        enbt::raw_uuid temp = enbt::endian_helpers::convert_endian(std::endian::big, val);
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < 16; i++)
            data.push_back(tmp[i]);
    }

    template <class T>
    static void WriteValue(const T& val, list_array<uint8_t>& data) {
        T temp = enbt::endian_helpers::convert_endian(std::endian::big, val);
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < sizeof(T); i++)
            data.push_back(tmp[i]);
    }

    static void WriteString(list_array<uint8_t>& data, const std::string& str, int32_t max_string_len = INT32_MAX) {
        if (str.size() > (size_t)max_string_len)
            throw std::out_of_range("actual string len out of range");
        WriteVar<int32_t>(str.size(), data);
        data.push_back((uint8_t*)str.data(), str.size());
    }

    static void WriteIdentifier(list_array<uint8_t>& data, const std::string& str) {
        WriteString(data, str, 32767);
    }

    static void WriteJSONComponent(list_array<uint8_t>& data, const std::string& str) {
        WriteString(data, str, 262144);
    }

    template <class ArrayT>
    static void WriteArray(list_array<uint8_t>& data, const ArrayT& arr) {
        WriteVar<int32_t>(arr.size(), data);
        if constexpr (std::is_same_v<ArrayT, list_array<uint8_t>>)
            data.push_back(arr);
        else
            for (auto& it : arr)
                WriteValue<ArrayT>(it, data);
    }

    static std::string UUID2String(const enbt::raw_uuid& uuid) {
        char buf[36];
        size_t index = 0;
        for (size_t i = 0; i < 16; i++) {
            if (i == 4 || i == 6 || i == 8 || i == 10)
                buf[index++] = '-';
            uint8_t tmp = uuid.data[i];
            buf[index++] = "0123456789abcdef"[tmp >> 4];
            buf[index++] = "0123456789abcdef"[tmp & 0x0F];
        }
        return std::string(buf, 36);
    }

    static util::NBT ReadNBT(ArrayStream& data) {
        size_t readed = 0;
        util::NBT res(util::NBT::readNBT(data.data_read(), data.size_read(), readed));
        data.range_read(readed);
        return res;
    }

    static enbt::value ReadNBT_enbt(ArrayStream& data) {
        size_t readed = 0;
        auto res = util::NBT::readNBT_asENBT(data.data_read(), data.size_read(), readed);
        data.range_read(readed);
        return res;
    }

    static util::NBT ReadNetworkNBT(ArrayStream& data) {
        size_t readed = 0;
        util::NBT res(util::NBT::readNetworkNBT(data.data_read(), data.size_read(), readed));
        data.range_read(readed);
        return res;
    }

    static enbt::value ReadNetworkNBT_enbt(ArrayStream& data) {
        size_t readed = 0;
        auto res = util::NBT::readNetworkNBT_asENBT(data.data_read(), data.size_read(), readed);
        data.range_read(readed);
        return res;
    }
}
#endif /* SRC_UTIL_READERS */
