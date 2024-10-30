#ifndef SRC_UTIL_READERS
#define SRC_UTIL_READERS
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include "../protocolHelperNBT.hpp"
#include <bit>
#include <cstdint>
#include <exception>
#include <string>

namespace crafted_craft {
    struct ArrayStream {
        uint8_t* arrau;
        size_t mi;

        ArrayStream(uint8_t* arr, size_t size) {
            arrau = arr;
            mi = size;
            read_only = false;
        }

        ArrayStream(const uint8_t* arr, size_t size) {
            arrau = const_cast<uint8_t*>(arr);
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
            arrau[w++] = value;
        }

        uint8_t read() {
            if (mi <= r)
                throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try read in: " + std::to_string(r));
            return arrau[r++];
        }

        uint8_t peek() {
            if (mi <= r)
                throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try read in: " + std::to_string(r));
            return arrau[r];
        }

        bool empty() {
            return r >= mi;
        }

        ArrayStream range(size_t start, size_t end) {
            if (start > end)
                throw std::out_of_range("start > end");
            if (end > mi)
                throw std::out_of_range("end > max_index");
            return ArrayStream(arrau + start, end - start);
        }

        ArrayStream range_read(size_t size) {
            if (r + size > mi)
                throw std::out_of_range("r + size > max_index");

            ArrayStream tmp(arrau + r, size);
            r += size;
            return tmp;
        }

        ArrayStream read_left() {
            size_t left = mi - r;
            ArrayStream tmp(arrau + r, left);
            r = mi;
            return tmp;
        }

        size_t size_read() {
            return mi - r;
        }

        uint8_t* data_read() {
            return arrau + r;
        }

        size_t size_write() {
            return mi - w;
        }

        uint8_t* data_write() {
            return arrau + w;
        }

        bool can_read(size_t size) {
            return r + size <= mi;
        }

        list_array<uint8_t> to_vector() {
            return list_array<uint8_t>(arrau + r, arrau + mi + r);
        }


    private:
        bool read_only;
    };

    template <class Res>
    static Res ReadVar(ArrayStream& data) {
        size_t len = sizeof(Res);
        Res res = enbt::value::fromVar<Res>(data.arrau + data.r, len);
        data.r += len;
        enbt::endian_helpers::convert_endian(std::endian::little, res);
        return res;
    }

    template <class ResultT, class T>
    static void WriteVar(T val, list_array<uint8_t>& data) {
        if (!std::is_same<ResultT, T>::value)
            if ((ResultT)val != val)
                throw std::out_of_range("Value out of range");

        constexpr size_t buf_len = sizeof(ResultT) + (sizeof(ResultT) / 7) + 1;
        uint8_t buf[buf_len];
        size_t len = enbt::value::toVar(buf, buf_len, enbt::endian_helpers::convert_endian<ResultT>(std::endian::little, (ResultT)val));
        for (size_t i = 0; i < len; i++)
            data.push_back(buf[i]);
    }

    template <class ResultT, class T>
    static void WriteVar(T val, ArrayStream& data) {
        if (!std::is_same<ResultT, T>::value)
            if ((ResultT)val != val)
                throw std::out_of_range("Value out of range");

        constexpr size_t buf_len = sizeof(T) + (sizeof(T) / 7) + 1;
        uint8_t buf[buf_len];
        size_t len = enbt::value::toVar(buf, buf_len, enbt::endian_helpers::convert_endian<ResultT>(std::endian::little, (ResultT)val));
        for (size_t i = 0; i < len; i++)
            data.write(buf[i]);
    }

    static void WriteUUID(const enbt::raw_uuid& val, list_array<uint8_t>& data) {
        enbt::raw_uuid temp = enbt::endian_helpers::convert_endian(std::endian::big, val);
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < 16; i++)
            data.push_back(tmp[i]);
    }

    static enbt::raw_uuid ReadUUID(ArrayStream& data) {
        enbt::raw_uuid temp;
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < 16; i++)
            tmp[i] = data.read();
        return enbt::endian_helpers::convert_endian(std::endian::big, temp);
    }

    template <class T>
    static void WriteValue(const T& val, list_array<uint8_t>& data) {
        T temp = enbt::endian_helpers::convert_endian(std::endian::big, val);
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < sizeof(T); i++)
            data.push_back(tmp[i]);
    }

    template <class T>
    static T ReadValue(ArrayStream& data) {
        uint8_t tmp[sizeof(T)];
        for (size_t i = 0; i < sizeof(T); i++)
            tmp[i] = data.read();
        return enbt::endian_helpers::convert_endian(std::endian::big, *(T*)tmp);
    }

    static std::string ReadString(ArrayStream& data, int32_t max_string_len) {
        std::string res = "";
        int32_t actual_len = ReadVar<int32_t>(data);
        if (actual_len > max_string_len)
            throw std::out_of_range("actual string len out of range");
        if (actual_len < 0)
            throw std::out_of_range("actual string len out of range");
        for (int32_t i = 0; i < actual_len;) {
            char tmp = (char)data.read();
            i += (tmp & 0xc0) != 0x80;
            res += tmp;
        }
        return res;
    }

    static std::string ReadIdentifier(ArrayStream& data) {
        return ReadString(data, 32767);
    }

    static void WriteString(list_array<uint8_t>& data, const std::string& str, int32_t max_string_len = INT32_MAX) {
        int32_t actual_len = str.size();
        if (actual_len != str.size())
            throw std::out_of_range("actual string len out of range");
        if (actual_len > max_string_len)
            throw std::out_of_range("actual string len out of range");
        WriteVar<int32_t>(actual_len, data);
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

    template <class T>
    static list_array<T> ReadArray(ArrayStream& data) {
        int32_t len = ReadVar<int32_t>(data);
        if (len < 0)
            throw std::out_of_range("array len out of range");
        list_array<T> res;
        res.reserve(len);
        for (int32_t i = 0; i < len; i++)
            res.push_back(ReadValue<T>(data));
        return res;
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

    static list_array<uint8_t> ReadListArray(ArrayStream& data) {
        int32_t len = ReadVar<int32_t>(data);
        if (len < 0)
            throw std::out_of_range("list array len out of range");
        list_array<uint8_t> res;
        res.reserve(len);
        for (int32_t i = 0; i < len; i++)
            res.push_back(data.read());
        return res;
    }

    static NBT ReadNBT(ArrayStream& data) {
        size_t readed = 0;
        NBT res(NBT::readNBT(data.data_read(), data.size_read(), readed));
        data.range_read(readed);
        return res;
    }
}

#endif /* SRC_UTIL_READERS */
