/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <algorithm>
#include <cassert>
#include <src/util/nbt_stream.hpp>

namespace copper_server::util {
    namespace __internal {
        void endian_swap(void* value_ptr, std::size_t len) {
            std::byte* prox = static_cast<std::byte*>(value_ptr);
            std::reverse(prox, prox + len);
        }

        void convert_endian(std::endian value_endian, void* value_ptr, std::size_t len) {
            if (std::endian::native != value_endian)
                endian_swap(value_ptr, len);
        }

        std::string read_string(std::istream& read_stream) {
            std::string res;
            res.resize(read_value<uint16_t>(read_stream));
            read_stream.read(res.data(), res.size());
            return res;
        }
    }

    template <class T>
    static void write_value(std::ostream& write_stream, T value) {
        __internal::convert_endian(std::endian::big, &value, sizeof(T));
        write_stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    static void write_string(std::ostream& write_stream, std::string_view value) {
        if (value.size() > UINT16_MAX)
            throw std::out_of_range("The string size is too big to fit in uint16_t length");
        while (value.ends_with('\n'))
            value.remove_suffix(1);

        write_value<uint16_t>(write_stream, static_cast<uint16_t>(value.size()));
        write_stream.write(value.data(), value.size());
    }

    nbt_read_stream::nbt_read_stream(std::istream& read_stream, nbt_type current_type_id)
        : read_stream(read_stream), current_type_id(current_type_id) {}

    void nbt_read_stream::check_io_state() {
        if (readed)
            throw std::runtime_error("Invalid read state, item has been already readed");
    }

    nbt_read_stream::nbt_read_stream(std::istream& read_stream)
        : read_stream(read_stream) {
        current_type_id = __internal::read_value<nbt_type>(read_stream);
    }

    nbt_read_stream::~nbt_read_stream() {
        if (!readed)
            skip();
    }

    template <class T>
    void _read_number(T& res, nbt_type type, auto& stream) {
        switch (type) {
        case nbt_type::tag_byte:
            res = (T)__internal::read_value<std::int8_t>(stream);
            return;
        case nbt_type::tag_short:
            res = (T)__internal::read_value<std::int16_t>(stream);
            return;
        case nbt_type::tag_int:
            res = (T)__internal::read_value<std::int32_t>(stream);
            return;
        case nbt_type::tag_long:
            res = (T)__internal::read_value<std::int64_t>(stream);
            return;
        case nbt_type::tag_float:
            res = (T)__internal::read_value<float>(stream);
            return;
        case nbt_type::tag_double:
            res = (T)__internal::read_value<float>(stream);
            return;
        default:
            throw enbt::exception("Non castable value to numeric type");
        }
    }

    void _read_number_string(std::string& res, nbt_type type, auto& stream) {
        switch (type) {
        case nbt_type::tag_byte:
            res = std::to_string(__internal::read_value<std::int8_t>(stream));
            return;
        case nbt_type::tag_short:
            res = std::to_string(__internal::read_value<std::int16_t>(stream));
            return;
        case nbt_type::tag_int:
            res = std::to_string(__internal::read_value<std::int32_t>(stream));
            return;
        case nbt_type::tag_long:
            res = std::to_string(__internal::read_value<std::int64_t>(stream));
            return;
        case nbt_type::tag_float:
            res = std::to_string(__internal::read_value<float>(stream));
            return;
        case nbt_type::tag_double:
            res = std::to_string(__internal::read_value<float>(stream));
            return;
        case nbt_type::tag_string:
            res = __internal::read_string(stream);
            return;
        default:
            throw enbt::exception("Non castable value to numeric type");
        }
    }

    nbt_read_stream& nbt_read_stream::read_into(bool& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_byte)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<bool>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(uint8_t& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_byte)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<uint8_t>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(uint16_t& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_short)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<uint16_t>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(uint32_t& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_int)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<uint32_t>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(uint64_t& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_long)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<uint64_t>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(int8_t& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_byte)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<int8_t>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(int16_t& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_short)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<int16_t>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(int32_t& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_int)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<int32_t>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(int64_t& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_long)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<int64_t>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(float& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_float)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<float>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(double& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_double)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_value<double>(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_into(std::string& res) {
        check_io_state();
        if (current_type_id != nbt_type::tag_string)
            throw std::runtime_error("Type mismatch");
        res = __internal::read_string(read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(bool& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(uint8_t& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(uint16_t& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(uint32_t& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(uint64_t& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(int8_t& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(int16_t& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(int32_t& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(int64_t& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(float& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(double& res) {
        check_io_state();
        _read_number(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    nbt_read_stream& nbt_read_stream::read_as(std::string& res) {
        check_io_state();
        _read_number_string(res, current_type_id, read_stream);
        readed = true;
        return *this;
    }

    void nbt_read_stream::skip() {
        check_io_state();
        readed = true;
        switch (current_type_id) {
        case nbt_type::tag_byte:
            __internal::read_value<std::int8_t>(read_stream);
            return;
        case nbt_type::tag_short:
            __internal::read_value<std::int16_t>(read_stream);
            return;
        case nbt_type::tag_int:
            __internal::read_value<std::int32_t>(read_stream);
            return;
        case nbt_type::tag_long:
            __internal::read_value<std::int64_t>(read_stream);
            return;
        case nbt_type::tag_float:
            __internal::read_value<float>(read_stream);
            return;
        case nbt_type::tag_double:
            __internal::read_value<float>(read_stream);
            return;
        case nbt_type::tag_string:
            __internal::read_string(read_stream);
            return;
        default:
            blind_iterate([](auto& name, auto& it) { it.skip(); }, [](auto& it) { it.skip(); });
            return;
        }

    }

    nbt_type nbt_read_stream::get_type() const {
        return current_type_id;
    }

    nbt_read_list_stream::nbt_read_list_stream(std::istream& read_stream) : read_stream(read_stream) {
        items_type = __internal::read_value<nbt_type>(read_stream);
        items = __internal::read_value<int32_t>(read_stream);
    }

    nbt_type nbt_read_list_stream::get_items_type() const {
        return items_type;
    }

    void nbt_read_list_stream::advance() {
        if (items == current_item)
            throw std::out_of_range("Thread tried to read the nbt list outside of bound.");
        ++current_item;
    }

    nbt_read_list_stream::~nbt_read_list_stream() {}

    int32_t nbt_read_list_stream::size() const noexcept {
        return items;
    }

    int32_t nbt_read_list_stream::current_index() const noexcept {
        return current_item;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(bool& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(uint8_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(uint16_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(uint32_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(uint64_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(int8_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(int16_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(int32_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(int64_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(float& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(double& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_into(std::string& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_into(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(bool& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(uint8_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(uint16_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(uint32_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(uint64_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(int8_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(int16_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(int32_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(int64_t& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(float& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(double& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_list_stream& nbt_read_list_stream::read_one_as(std::string& res) {
        advance();
        nbt_read_stream tmp(read_stream, items_type);
        tmp.read_as(res);
        return *this;
    }

    nbt_read_compound_stream::nbt_read_compound_stream(std::istream& read_stream, bool enable_collector_strict_order) : read_stream(read_stream), enable_collector_strict_order(enable_collector_strict_order) {
        current_type_id = __internal::read_value<nbt_type>(read_stream);
        if (current_type_id == nbt_type::tag_end)
            reached_end = true;
    }

    nbt_read_compound_stream::~nbt_read_compound_stream() {
        if (!reached_end)
            iterable([](auto& name, auto& it) {
                it.skip();
            });
    }

    bool nbt_read_compound_stream::is_reached_end() const noexcept {
        return reached_end;
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, bool& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, uint8_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, uint16_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, uint32_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, uint64_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, int8_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, int16_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, int32_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, int64_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, float& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, double& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_into(const std::string& name, std::string& res) {
        return collect(name, [&res](auto& stream) { stream.read_into(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, bool& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, uint8_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, uint16_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, uint32_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, uint64_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, int8_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, int16_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, int32_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, int64_t& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, float& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, double& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::collect_as(const std::string& name, std::string& res) {
        return collect(name, [&res](auto& stream) { stream.read_as(res); });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::make_collect() {
        if (!enable_collector_strict_order)
            return iterable([this](std::string& name, nbt_read_stream& stream) {
                if (auto it = automated_collector.find(name); it != automated_collector.end())
                    it->second(stream);
            });
        else
            return iterable([this, order = size_t(0)](std::string& name, nbt_read_stream& stream) mutable {
                if (auto& excepted = collector_strict_order_data.at(order++); excepted != name)
                    throw std::runtime_error("Invalid order, excepted: " + excepted + ", but got: " + name);
                automated_collector.at(name)(stream);
            });
    }

    nbt_read_compound_stream& nbt_read_compound_stream::force_all_collect() {
        std::unordered_set<std::string> collected_items;
        if (enable_collector_strict_order) {
            collected_items.reserve(automated_collector.size());
            iterable([this, &collected_items](auto& name, auto& stream) {
                if (auto it = automated_collector.find(name); it != automated_collector.end()) {
                    it->second(stream);
                    collected_items.emplace(name);
                } else
                    throw std::runtime_error("Not all elements is collected, skipped item: " + name);
            });
            for (auto& [it, _] : automated_collector)
                if (!collected_items.contains(it))
                    throw std::runtime_error("Not all elements is collected, invalid format");
            return *this;
        } else {
            collected_items.reserve(automated_collector.size());
            iterable([this, &collected_items, order = size_t(0)](auto& name, auto& stream) mutable {
                if (order == collector_strict_order_data.size())
                    throw std::runtime_error("Too much elements");
                if (auto& excepted = collector_strict_order_data.at(order++); excepted != name)
                    throw std::runtime_error("Invalid order, excepted: " + excepted + ", but got: " + name);
                automated_collector.at(name)(stream);
                collected_items.emplace(name);
            });

            for (auto& [it, _] : automated_collector)
                if (!collected_items.contains(it))
                    throw std::runtime_error("Not all elements is collected, invalid format");
        }
        return *this;
    }

    nbt_read_list_stream nbt_read_stream::read_list() {
        if (current_type_id != nbt_type::tag_list)
            throw std::runtime_error("Type mismatch");
        readed = true;
        return nbt_read_list_stream(read_stream);
    }

    nbt_read_compound_stream nbt_read_stream::read_compound(bool enable_collector_strict_order) {
        if (current_type_id != nbt_type::tag_compound)
            throw std::runtime_error("Type mismatch");
        readed = true;
        return nbt_read_compound_stream(read_stream, enable_collector_strict_order);
    }

    nbt_type nbt_write_stream::get_written_type() const {
        return written_type_id;
    }

    nbt_write_list_stream::nbt_write_list_stream(std::ostream& write_stream, uint16_t depth)
        : write_stream(write_stream),
          depth(depth),
          items(0),
          check_type(nbt_type::tag_end),
          type_id_written(false),
          length_fixed(false) {

        type_size_field_pos = write_stream.tellp();
        write_value(write_stream, check_type);
        write_value(write_stream, items);
    }

    nbt_write_list_stream::nbt_write_list_stream(std::ostream& write_stream, uint16_t depth, int32_t items, nbt_type type)
        : write_stream(write_stream),
          depth(depth),
          items(items),
          check_type(type), type_id_written(true), length_fixed(true), type_size_field_pos(0) {
        write_value(write_stream, check_type);
        write_value(write_stream, items);
    }

    nbt_write_list_stream::~nbt_write_list_stream() {
        assert((length_fixed && !items) || (!length_fixed && items));
        if (!length_fixed) {
            if (items) {
                auto curr_pos = write_stream.tellp();
                write_stream.seekp(type_size_field_pos);
                write_value(write_stream, check_type);
                write_value(write_stream, items);
                write_stream.seekp(curr_pos);
            }
        }
    }

    nbt_write_list_stream& nbt_write_list_stream::write(bool res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(uint8_t res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(uint16_t res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(uint32_t res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(uint64_t res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(int8_t res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(int16_t res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(int32_t res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(int64_t res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(float res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(double res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(const std::string& res) {
        return write([&res](auto& s) { s.write(res); });
    }

    nbt_write_list_stream& nbt_write_list_stream::write(std::string_view res) {
        return write([res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream::nbt_write_compound_stream(std::ostream& write_stream, uint16_t depth) : write_stream(write_stream), depth(depth) {}

    nbt_write_compound_stream::~nbt_write_compound_stream() {
        write_value<nbt_type>(write_stream, nbt_type::tag_end);
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, bool res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, uint8_t res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, uint16_t res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, uint32_t res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, uint64_t res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, int8_t res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, int16_t res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, int32_t res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, int64_t res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, float res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, double res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, const std::string& res) {
        return write(filed_name, [&res](auto& s) { s.write(res); });
    }

    nbt_write_compound_stream& nbt_write_compound_stream::write(std::string_view filed_name, std::string_view res) {
        return write(filed_name, [res](auto& s) { s.write(res); });
    }

    void nbt_write_stream::write(bool res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_byte;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(uint8_t res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_byte;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(uint16_t res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_short;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(uint32_t res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_int;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(uint64_t res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_long;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(int8_t res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_byte;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(int16_t res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_short;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(int32_t res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_int;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(int64_t res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_long;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(float res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_float;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(double res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_double;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_value(write_stream, res);
    }

    void nbt_write_stream::write(const std::string& res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_string;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_string(write_stream, res);
    }

    void nbt_write_stream::write(std::string_view res) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_string;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        write_string(write_stream, res);
    }

    nbt_write_compound_stream nbt_write_stream::write_compound() {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_list;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        if (depth == 512)
            throw std::runtime_error("Invalid format, the depth limit is 512");
        return nbt_write_compound_stream(write_stream, depth + 1);
    }

    nbt_write_list_stream nbt_write_stream::write_list() {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_list;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        if (depth == 512)
            throw std::runtime_error("Invalid format, the depth limit is 512");
        return nbt_write_list_stream(write_stream, depth + 1);
    }

    nbt_write_list_stream nbt_write_stream::write_list(int32_t fixed_size, nbt_type tag) {
        if (already_written)
            throw std::runtime_error("Invalid write state, item has been already written");
        already_written = true;
        written_type_id = nbt_type::tag_list;
        write_value(write_stream, written_type_id);
        if (field_required)
            write_string(write_stream, to_write_field_name);
        if (depth == 512)
            throw std::runtime_error("Invalid format, the depth limit is 512");
        return nbt_write_list_stream(write_stream, depth + 1, fixed_size, tag);
    }

    nbt_write_stream::nbt_write_stream(std::ostream& write_stream, std::string_view to_write_field_name, bool field_required, uint16_t depth)
        : write_stream(write_stream), to_write_field_name(to_write_field_name), field_required(field_required), depth(depth) {}
}