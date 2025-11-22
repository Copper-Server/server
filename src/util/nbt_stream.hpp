/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_NBT_STREAM
#define SRC_UTIL_NBT_STREAM
#include <bit>
#include <functional>
#include <istream>
#include <src/base_objects/uuid.hpp>
#include <src/util/nbt.hpp>
#include <sstream>
#include <type_traits>
#include <unordered_set>

namespace copper_server::util {
    namespace __internal {

        void endian_swap(void* value_ptr, std::size_t len);

        void convert_endian(std::endian value_endian, void* value_ptr, std::size_t len);

        template <class T>
        T convert_endian(std::endian value_endian, T val) {
            if (std::endian::native != value_endian)
                endian_swap(&val, sizeof(T));
            return val;
        }

        template <class T>
        void convert_endian_arr(std::endian value_endian, T* val, std::size_t size) {
            if (std::endian::native != value_endian)
                for (std::size_t i = 0; i < size; i++)
                    endian_swap(&val[i], sizeof(T));
        }

        template <class T>
        void convert_endian_arr(std::endian value_endian, std::vector<T>& val) {
            if (std::endian::native != value_endian)
                for (auto& it : val)
                    endian_swap(&it, sizeof(T));
        }

        template <class T>
        T read_value(std::istream& read_stream) {
            T res;
            read_stream.read((char*)&res, sizeof(T));
            if constexpr (sizeof(T) != 1)
                return convert_endian(std::endian::big, res);
            else
                return res;
        }

        std::string read_string(std::istream& read_stream);
    }

    class nbt_read_list_stream;
    class nbt_read_compound_stream;

    class nbt_read_stream {
        std::istream& read_stream;

        nbt_type current_type_id;
        bool readed = false;

        nbt_read_stream(std::istream& read_stream, nbt_type current_type_id);

        void check_io_state();
        friend class nbt_read_list_stream;
        friend class nbt_read_compound_stream;

    public:
        nbt_read_stream(std::istream& read_stream, bool is_root = false);
        ~nbt_read_stream();
        nbt_read_stream& read_into(bool& res);
        nbt_read_stream& read_into(uint8_t& res);
        nbt_read_stream& read_into(uint16_t& res);
        nbt_read_stream& read_into(uint32_t& res);
        nbt_read_stream& read_into(uint64_t& res);
        nbt_read_stream& read_into(int8_t& res);
        nbt_read_stream& read_into(int16_t& res);
        nbt_read_stream& read_into(int32_t& res);
        nbt_read_stream& read_into(int64_t& res);
        nbt_read_stream& read_into(float& res);
        nbt_read_stream& read_into(double& res);
        nbt_read_stream& read_into(std::string& res);
        nbt_read_stream& read_into(nbt_enbt_convert& res);
        nbt_read_stream& read_into(nbt& res);
        nbt_read_stream& read_into(base_objects::uuid& res);
        nbt_read_stream& read_into(base_objects::uuid_hex& res);
        nbt_read_stream& read_into(base_objects::uuid_flat_hex& res);

        nbt_read_stream& read_as(bool& res);
        nbt_read_stream& read_as(uint8_t& res);
        nbt_read_stream& read_as(uint16_t& res);
        nbt_read_stream& read_as(uint32_t& res);
        nbt_read_stream& read_as(uint64_t& res);
        nbt_read_stream& read_as(int8_t& res);
        nbt_read_stream& read_as(int16_t& res);
        nbt_read_stream& read_as(int32_t& res);
        nbt_read_stream& read_as(int64_t& res);
        nbt_read_stream& read_as(float& res);
        nbt_read_stream& read_as(double& res);
        nbt_read_stream& read_as(std::string& res);
        nbt_read_stream& read_as(nbt_enbt_convert& res);
        nbt_read_stream& read_as(nbt& res);
        nbt_read_stream& read_as(base_objects::uuid& res);
        nbt_read_stream& read_as(base_objects::uuid_hex& res);
        nbt_read_stream& read_as(base_objects::uuid_flat_hex& res);


        void skip();

        nbt_type get_type() const;

        nbt_read_list_stream read_list();
        nbt_read_compound_stream read_compound(bool enable_collector_strict_order = false);

        template <class FN>
        void iterate(FN&& callback)
            requires(std::is_invocable_v<FN, std::string_view, nbt_read_stream&> || std::is_invocable_v<FN, const std::string&, nbt_read_stream&>)
        {
            iterate([](int32_t) {}, std::move(callback));
        }

        template <class FN>
        void iterate(FN&& callback)
            requires(std::is_invocable_v<FN, nbt_read_stream&>)
        {
            iterate([](int32_t) {}, std::move(callback));
        }

        template <class T>
        void iterate_into(T* arr, int32_t size) {
            check_io_state();
            if (current_type_id == nbt_type::tag_byte_array) {
                if constexpr (sizeof(T) != sizeof(int8_t))
                    throw std::runtime_error("Type mismatch");
                else {
                    int32_t len = __internal::read_value<int32_t>(read_stream);
                    if (len != size)
                        throw std::out_of_range("Invalid array size");
                    read_stream.read((char*)arr, size * sizeof(T));
                    readed = true;
                }
            } else if (current_type_id == nbt_type::tag_int_array) {
                if constexpr (sizeof(T) != sizeof(int32_t))
                    throw std::runtime_error("Type mismatch");
                else {
                    int32_t len = __internal::read_value<int32_t>(read_stream);
                    if (len != size)
                        throw std::out_of_range("Invalid array size");
                    read_stream.read((char*)arr, size * sizeof(T));
                    readed = true;
                    __internal::convert_endian_arr(std::endian::big, arr, size);
                }
            } else if (current_type_id == nbt_type::tag_long_array) {
                if constexpr (sizeof(T) != sizeof(int64_t))
                    throw std::runtime_error("Type mismatch");
                else {
                    int32_t len = __internal::read_value<int32_t>(read_stream);
                    if (len != size)
                        throw std::out_of_range("Invalid array size");
                    read_stream.read((char*)arr, size * sizeof(T));
                    readed = true;
                    __internal::convert_endian_arr(std::endian::big, arr, size);
                }
            } else {
                size_t index = 0;
                iterate(
                    [&](size_t len) {
                        if (len != size)
                            throw std::runtime_error("Invalid array size");
                    },
                    [&](nbt_read_stream& self) {
                        self.read_into(arr[index++]);
                    }
                );
            }
        }

        template <class T>
        void iterate_into(std::vector<T>& arr) {
            check_io_state();
            if (current_type_id == nbt_type::tag_byte_array) {
                if constexpr (sizeof(T) != sizeof(int8_t))
                    throw std::runtime_error("Type mismatch");
                else {
                    int32_t len = __internal::read_value<int32_t>(read_stream);
                    if (len < 0)
                        throw std::runtime_error("Invalid array size");
                    arr.resize(size_t(len));
                    read_stream.read((char*)arr.data(), len * sizeof(T));
                    readed = true;
                }
            } else if (current_type_id == nbt_type::tag_int_array) {
                if constexpr (sizeof(T) != sizeof(int32_t))
                    throw std::runtime_error("Type mismatch");
                else {
                    int32_t len = __internal::read_value<int32_t>(read_stream);
                    if (len < 0)
                        throw std::runtime_error("Invalid array size");
                    arr.resize(size_t(len));
                    read_stream.read((char*)arr.data(), len * sizeof(T));
                    readed = true;
                    __internal::convert_endian_arr(std::endian::big, arr);
                }
            } else if (current_type_id == nbt_type::tag_long_array) {
                if constexpr (sizeof(T) != sizeof(int64_t))
                    throw std::runtime_error("Type mismatch");
                else {
                    int32_t len = __internal::read_value<int32_t>(read_stream);
                    if (len < 0)
                        throw std::runtime_error("Invalid array size");
                    arr.resize(size_t(len));
                    read_stream.read((char*)arr.data(), len * sizeof(T));
                    readed = true;
                    __internal::convert_endian_arr(std::endian::big, arr);
                }
            } else {
                iterate(
                    [&](size_t len) {
                        arr.reserve(len);
                    },
                    [&](nbt_read_stream& self) {
                        T value;
                        self.read_into(value);
                        arr.emplace_back(std::move(value));
                    }
                );
            }
        }

        template <class T>
        std::vector<T> iterate_into() {
            std::vector<T> res;
            iterate_into(res);
            return res;
        }

        template <class SIZE_FN, class FN>
        void iterate(SIZE_FN&& size_callback, FN&& callback)
            requires((std::is_invocable_v<FN, std::string_view, nbt_read_stream&> || std::is_invocable_v<FN, const std::string&, nbt_read_stream&>) && std::is_invocable_v<SIZE_FN, uint64_t>)
        {
            check_io_state();
            if (current_type_id == nbt_type::tag_compound) {
                for (;;) {
                    auto tag = __internal::read_value<nbt_type>(read_stream);
                    if (tag == nbt_type::tag_end)
                        break;
                    auto name = __internal::read_string(read_stream);
                    nbt_read_stream stream(read_stream, tag);
                    callback(name, stream);
                }
                readed = true;
            } else
                throw std::invalid_argument("not compound type");
        }

        template <class SIZE_FN, class FN>
        void iterate(SIZE_FN&& size_callback, FN&& callback)
            requires(std::is_invocable_v<FN, nbt_read_stream&> && std::is_invocable_v<SIZE_FN, uint64_t>)
        {
            check_io_state();
            if (current_type_id == nbt_type::tag_list) {
                auto tag = __internal::read_value<nbt_type>(read_stream);
                int32_t len = __internal::read_value<int32_t>(read_stream);
                size_callback(len);
                if (len) {
                    for (int32_t i = 0; i < len; i++) {
                        nbt_read_stream stream(read_stream, tag);
                        callback(stream);
                    }
                }
                readed = true;
            } else if (current_type_id == nbt_type::tag_byte_array) {
                int32_t len = __internal::read_value<int32_t>(read_stream);
                size_callback(len);
                if (len) {
                    for (int32_t i = 0; i < len; i++) {
                        nbt_read_stream stream(read_stream, nbt_type::tag_byte);
                        callback(stream);
                    }
                }
                readed = true;
            } else if (current_type_id == nbt_type::tag_int_array) {
                int32_t len = __internal::read_value<int32_t>(read_stream);
                size_callback(len);
                if (len) {
                    for (int32_t i = 0; i < len; i++) {
                        nbt_read_stream stream(read_stream, nbt_type::tag_int);
                        callback(stream);
                    }
                }
                readed = true;
            } else if (current_type_id == nbt_type::tag_long_array) {
                int32_t len = __internal::read_value<int32_t>(read_stream);
                size_callback(len);
                if (len) {
                    for (int32_t i = 0; i < len; i++) {
                        nbt_read_stream stream(read_stream, nbt_type::tag_long);
                        callback(stream);
                    }
                }
                readed = true;
            } else
                throw std::invalid_argument("not array type");
        }

        template <class COMPOUND_FN, class ARRAY_FN>
        void blind_iterate(
            COMPOUND_FN&& compound,
            ARRAY_FN&& array_or_log_item
        )
            requires(std::is_invocable_v<ARRAY_FN, nbt_read_stream&> && (std::is_invocable_v<COMPOUND_FN, std::string_view, nbt_read_stream&> || std::is_invocable_v<COMPOUND_FN, const std::string&, nbt_read_stream&>))
        {
            blind_iterate([](int32_t) {}, std::move(compound), std::move(array_or_log_item));
        }

        template <class SIZE_FN, class COMPOUND_FN, class ARRAY_FN>
        void blind_iterate(
            SIZE_FN&& size_callback, COMPOUND_FN&& compound, ARRAY_FN&& array
        )
            requires(std::is_invocable_v<ARRAY_FN, nbt_read_stream&> && (std::is_invocable_v<COMPOUND_FN, std::string_view, nbt_read_stream&> || std::is_invocable_v<COMPOUND_FN, const std::string&, nbt_read_stream&>) && std::is_invocable_v<SIZE_FN, uint64_t>)
        {
            check_io_state();
            readed = true;
            if (current_type_id == nbt_type::tag_compound) {
                for (;;) {
                    auto tag = __internal::read_value<nbt_type>(read_stream);
                    if (tag == nbt_type::tag_end)
                        break;
                    auto name = __internal::read_string(read_stream);
                    nbt_read_stream stream(read_stream, tag);
                    compound(name, stream);
                }
                readed = true;
            } else if (current_type_id == nbt_type::tag_list) {
                auto tag = __internal::read_value<nbt_type>(read_stream);
                int32_t len = __internal::read_value<int32_t>(read_stream);
                size_callback(len);
                if (len) {
                    for (int32_t i = 0; i < len; i++) {
                        nbt_read_stream stream(read_stream, tag);
                        array(stream);
                    }
                }
                readed = true;
            } else if (current_type_id == nbt_type::tag_byte_array) {
                int32_t len = __internal::read_value<int32_t>(read_stream);
                size_callback(len);
                if (len) {
                    for (int32_t i = 0; i < len; i++) {
                        nbt_read_stream stream(read_stream, nbt_type::tag_byte);
                        array(stream);
                    }
                }
                readed = true;
            } else if (current_type_id == nbt_type::tag_int_array) {
                int32_t len = __internal::read_value<int32_t>(read_stream);
                size_callback(len);
                if (len) {
                    for (int32_t i = 0; i < len; i++) {
                        nbt_read_stream stream(read_stream, nbt_type::tag_int);
                        array(stream);
                    }
                }
                readed = true;
            } else if (current_type_id == nbt_type::tag_long_array) {
                int32_t len = __internal::read_value<int32_t>(read_stream);
                size_callback(len);
                if (len) {
                    for (int32_t i = 0; i < len; i++) {
                        nbt_read_stream stream(read_stream, nbt_type::tag_long);
                        array(stream);
                    }
                }
                readed = true;
            } else
                throw std::invalid_argument("non iterable type");
        }
    };

    class nbt_read_list_stream {
        std::istream& read_stream;
        int32_t current_item = 0;
        int32_t items = 0;
        nbt_type items_type;

        void advance();

    public:
        nbt_read_list_stream(std::istream& read_stream);

        nbt_type get_items_type() const;

        ~nbt_read_list_stream();

        int32_t size() const noexcept;
        int32_t current_index() const noexcept;
        nbt_read_list_stream& read_one_into(bool& res);
        nbt_read_list_stream& read_one_into(uint8_t& res);
        nbt_read_list_stream& read_one_into(uint16_t& res);
        nbt_read_list_stream& read_one_into(uint32_t& res);
        nbt_read_list_stream& read_one_into(uint64_t& res);
        nbt_read_list_stream& read_one_into(int8_t& res);
        nbt_read_list_stream& read_one_into(int16_t& res);
        nbt_read_list_stream& read_one_into(int32_t& res);
        nbt_read_list_stream& read_one_into(int64_t& res);
        nbt_read_list_stream& read_one_into(float& res);
        nbt_read_list_stream& read_one_into(double& res);
        nbt_read_list_stream& read_one_into(std::string& res);
        nbt_read_list_stream& read_one_into(nbt_enbt_convert& res);
        nbt_read_list_stream& read_one_into(nbt& res);
        nbt_read_list_stream& read_one_into(base_objects::uuid& res);
        nbt_read_list_stream& read_one_into(base_objects::uuid_hex& res);
        nbt_read_list_stream& read_one_into(base_objects::uuid_flat_hex& res);

        nbt_read_list_stream& read_one_as(bool& res);
        nbt_read_list_stream& read_one_as(uint8_t& res);
        nbt_read_list_stream& read_one_as(uint16_t& res);
        nbt_read_list_stream& read_one_as(uint32_t& res);
        nbt_read_list_stream& read_one_as(uint64_t& res);
        nbt_read_list_stream& read_one_as(int8_t& res);
        nbt_read_list_stream& read_one_as(int16_t& res);
        nbt_read_list_stream& read_one_as(int32_t& res);
        nbt_read_list_stream& read_one_as(int64_t& res);
        nbt_read_list_stream& read_one_as(float& res);
        nbt_read_list_stream& read_one_as(double& res);
        nbt_read_list_stream& read_one_as(std::string& res);
        nbt_read_list_stream& read_one_as(nbt_enbt_convert& res);
        nbt_read_list_stream& read_one_as(nbt& res);
        nbt_read_list_stream& read_one_as(base_objects::uuid& res);
        nbt_read_list_stream& read_one_as(base_objects::uuid_hex& res);
        nbt_read_list_stream& read_one_as(base_objects::uuid_flat_hex& res);

        template <class FN>
        nbt_read_list_stream& read_one(FN&& fn)
            requires(std::is_invocable_v<FN, nbt_read_stream&>)
        {
            advance();
            nbt_read_stream inner(read_stream);
            fn(inner);
            return *this;
        }

        template <class FN>
        nbt_read_list_stream& iterable(FN&& fn)
            requires(std::is_invocable_v<FN, nbt_read_stream&>)
        {
            while (current_item != items)
                read_one(fn);
            return *this;
        }
    };

    class nbt_read_compound_stream {
        std::istream& read_stream;
        std::size_t current_item = 0;
        nbt_type current_type_id;
        bool enable_collector_strict_order = false;
        bool reached_end = false;

        std::unordered_map<std::string, std::function<void(nbt_read_stream&)>> automated_collector;
        std::vector<std::string> collector_strict_order_data;

    public:
        nbt_read_compound_stream(std::istream& read_stream, bool enable_collector_strict_order);
        ~nbt_read_compound_stream();

        bool is_reached_end() const noexcept;

        template <class FN>
        nbt_read_compound_stream& read(FN&& fn)
            requires(std::is_invocable_v<FN, std::string&, nbt_read_stream&>)
        {
            if (reached_end)
                throw std::out_of_range("Tried to read value out of compounds range.");
            auto str = __internal::read_string(read_stream);
            nbt_read_stream inner(read_stream, current_type_id);
            fn(str, inner);
            current_type_id = __internal::read_value<nbt_type>(read_stream);
            if (current_type_id == nbt_type::tag_end)
                reached_end = true;
            return *this;
        }

        template <class FN>
        nbt_read_compound_stream& iterable(FN&& fn)
            requires(std::is_invocable_v<FN, std::string&, nbt_read_stream&>)
        {
            while (reached_end == false)
                read(fn);
            return *this;
        }

        template <class FN>
        nbt_read_compound_stream& collect(const std::string& name, FN&& fn)
            requires(std::is_invocable_v<FN, nbt_read_stream&>)
        {
            automated_collector[name] = std::forward<FN>(fn);
            if (enable_collector_strict_order)
                collector_strict_order_data.push_back(name);
            return *this;
        }

        nbt_read_compound_stream& collect_into(const std::string& name, bool& res);
        nbt_read_compound_stream& collect_into(const std::string& name, uint8_t& res);
        nbt_read_compound_stream& collect_into(const std::string& name, uint16_t& res);
        nbt_read_compound_stream& collect_into(const std::string& name, uint32_t& res);
        nbt_read_compound_stream& collect_into(const std::string& name, uint64_t& res);
        nbt_read_compound_stream& collect_into(const std::string& name, int8_t& res);
        nbt_read_compound_stream& collect_into(const std::string& name, int16_t& res);
        nbt_read_compound_stream& collect_into(const std::string& name, int32_t& res);
        nbt_read_compound_stream& collect_into(const std::string& name, int64_t& res);
        nbt_read_compound_stream& collect_into(const std::string& name, float& res);
        nbt_read_compound_stream& collect_into(const std::string& name, double& res);
        nbt_read_compound_stream& collect_into(const std::string& name, std::string& res);
        nbt_read_compound_stream& collect_into(const std::string& name, nbt_enbt_convert& res);
        nbt_read_compound_stream& collect_into(const std::string& name, nbt& res);
        nbt_read_compound_stream& collect_into(const std::string& name,base_objects::uuid& res);
        nbt_read_compound_stream& collect_into(const std::string& name,base_objects::uuid_hex& res);
        nbt_read_compound_stream& collect_into(const std::string& name,base_objects::uuid_flat_hex& res);
        nbt_read_compound_stream& collect_as(const std::string& name, bool& res);
        nbt_read_compound_stream& collect_as(const std::string& name, uint8_t& res);
        nbt_read_compound_stream& collect_as(const std::string& name, uint16_t& res);
        nbt_read_compound_stream& collect_as(const std::string& name, uint32_t& res);
        nbt_read_compound_stream& collect_as(const std::string& name, uint64_t& res);
        nbt_read_compound_stream& collect_as(const std::string& name, int8_t& res);
        nbt_read_compound_stream& collect_as(const std::string& name, int16_t& res);
        nbt_read_compound_stream& collect_as(const std::string& name, int32_t& res);
        nbt_read_compound_stream& collect_as(const std::string& name, int64_t& res);
        nbt_read_compound_stream& collect_as(const std::string& name, float& res);
        nbt_read_compound_stream& collect_as(const std::string& name, double& res);
        nbt_read_compound_stream& collect_as(const std::string& name, std::string& res);
        nbt_read_compound_stream& collect_as(const std::string& name, nbt_enbt_convert& res);
        nbt_read_compound_stream& collect_as(const std::string& name, nbt& res);
        nbt_read_compound_stream& collect_as(const std::string& name, base_objects::uuid& res);
        nbt_read_compound_stream& collect_as(const std::string& name, base_objects::uuid_hex& res);
        nbt_read_compound_stream& collect_as(const std::string& name, base_objects::uuid_flat_hex& res);

        template <class FN>
        nbt_read_compound_stream& collect_iterate(const std::string& name, FN&& fn)
            requires(
                std::is_invocable_v<FN, nbt_read_stream&>
                || std::is_invocable_v<FN, std::string_view, nbt_read_stream&>
                || std::is_invocable_v<FN, const std::string&, nbt_read_stream&>
            )
        {
            automated_collector[name] = [fn](nbt_read_stream& stream) {
                stream.iterate(fn);
            };
            if (enable_collector_strict_order)
                collector_strict_order_data.push_back(name);
            return *this;
        }

        template <class FN>
        nbt_read_compound_stream& make_collect(FN&& on_uncollected)
            requires(std::is_invocable_v<FN, std::string&, nbt_read_stream&>)
        {
            if (!enable_collector_strict_order)
                return iterable([this, &on_uncollected](std::string& name, nbt_read_stream& stream) {
                    if (auto it = automated_collector.find(name); it != automated_collector.end())
                        it->second(stream);
                    else
                        on_uncollected(name, stream);
                });
            else
                return iterable([this, order = size_t(0)](std::string& name, nbt_read_stream& stream) mutable {
                    if (auto& excepted = collector_strict_order_data.at(order++); excepted != name)
                        throw std::runtime_error("Invalid order, excepted: " + excepted + ", but got: " + name);
                    automated_collector.at(name)(stream);
                });
        }

        nbt_read_compound_stream& make_collect();
        nbt_read_compound_stream& force_all_collect();
    };


    class nbt_write_list_stream;
    class nbt_write_compound_stream;

    class nbt_write_stream {
        std::ostream& write_stream;
        std::string_view to_write_field_name;
        nbt_type written_type_id;
        bool field_required;
        bool already_written = false;
        uint16_t depth = 0;

    public:
        nbt_type get_written_type() const;

        void write(bool res);
        void write(uint8_t res);
        void write(uint16_t res);
        void write(uint32_t res);
        void write(uint64_t res);
        void write(int8_t res);
        void write(int16_t res);
        void write(int32_t res);
        void write(int64_t res);
        void write(float res);
        void write(double res);
        void write(const std::string& res);
        void write(std::string_view res);
        void write(const nbt_enbt_convert&);
        void write(nbt_enbt_convert&&);
        void write(const nbt&);
        void write(base_objects::uuid res);
        void write(base_objects::uuid_hex res);
        void write(base_objects::uuid_flat_hex res);

        nbt_write_compound_stream write_compound();
        nbt_write_list_stream write_list();
        nbt_write_list_stream write_list(int32_t fixed_size, nbt_type tag);


        void write(const int8_t* arr, uint32_t size);
        void write(const uint8_t* arr, uint32_t size);
        void write(const int32_t* arr, uint32_t size);
        void write(const int64_t* arr, uint32_t size);

        nbt_write_stream(std::ostream& write_stream, std::string_view to_write_field_name = "", bool field_required = false, uint16_t depth = 0);
    };

    class nbt_write_list_stream {
        std::ostream& write_stream;
        std::ostream::pos_type type_size_field_pos;
        int32_t items;
        bool type_id_written;
        bool length_fixed;
        uint16_t depth;
        nbt_type check_type;

    public:
        nbt_write_list_stream(std::ostream& write_stream, uint16_t depth);
        nbt_write_list_stream(std::ostream& write_stream, uint16_t depth, int32_t items, nbt_type);
        ~nbt_write_list_stream();

        nbt_write_list_stream& write(bool res);
        nbt_write_list_stream& write(uint8_t res);
        nbt_write_list_stream& write(uint16_t res);
        nbt_write_list_stream& write(uint32_t res);
        nbt_write_list_stream& write(uint64_t res);
        nbt_write_list_stream& write(int8_t res);
        nbt_write_list_stream& write(int16_t res);
        nbt_write_list_stream& write(int32_t res);
        nbt_write_list_stream& write(int64_t res);
        nbt_write_list_stream& write(float res);
        nbt_write_list_stream& write(double res);
        nbt_write_list_stream& write(const std::string& res);
        nbt_write_list_stream& write(std::string_view res);
        nbt_write_list_stream& write(const nbt_enbt_convert&);
        nbt_write_list_stream& write(nbt_enbt_convert&&);
        nbt_write_list_stream& write(const nbt&);
        nbt_write_list_stream& write(base_objects::uuid res);
        nbt_write_list_stream& write(base_objects::uuid_hex res);
        nbt_write_list_stream& write(base_objects::uuid_flat_hex res);

        template <class FN>
        nbt_write_list_stream& write(FN&& fn)
            requires(std::is_invocable_v<FN, nbt_write_stream&>)
        {
            nbt_write_stream inner(write_stream, "", false, depth + 1);
            fn(inner);
            items++;
            return *this;
        }

        //fn(item, inner)
        template <class Iterable, class FN>
        nbt_write_list_stream& iterable(const Iterable& iter, FN&& fn) {
            for (const auto& item : iter)
                write([&](nbt_write_stream& inner) {
                    fn(item, inner);
                });
            return *this;
        }

        template <class Iterable>
        nbt_write_list_stream& iterable(const Iterable& iter) {
            for (const auto& item : iter)
                write([&](nbt_write_stream& inner) {
                    inner.write(item);
                });
            return *this;
        }
    };

    class nbt_write_compound_stream {
        std::ostream& write_stream;
        uint16_t depth = 0;

    public:
        nbt_write_compound_stream(std::ostream& write_stream, uint16_t depth = 0);
        ~nbt_write_compound_stream();

        nbt_write_compound_stream& write(std::string_view filed_name, bool res);
        nbt_write_compound_stream& write(std::string_view filed_name, uint8_t res);
        nbt_write_compound_stream& write(std::string_view filed_name, uint16_t res);
        nbt_write_compound_stream& write(std::string_view filed_name, uint32_t res);
        nbt_write_compound_stream& write(std::string_view filed_name, uint64_t res);
        nbt_write_compound_stream& write(std::string_view filed_name, int8_t res);
        nbt_write_compound_stream& write(std::string_view filed_name, int16_t res);
        nbt_write_compound_stream& write(std::string_view filed_name, int32_t res);
        nbt_write_compound_stream& write(std::string_view filed_name, int64_t res);
        nbt_write_compound_stream& write(std::string_view filed_name, float res);
        nbt_write_compound_stream& write(std::string_view filed_name, double res);
        nbt_write_compound_stream& write(std::string_view filed_name, const std::string& res);
        nbt_write_compound_stream& write(std::string_view filed_name, std::string_view res);
        nbt_write_compound_stream& write(std::string_view filed_name, const nbt_enbt_convert&);
        nbt_write_compound_stream& write(std::string_view filed_name, nbt_enbt_convert&&);
        nbt_write_compound_stream& write(std::string_view filed_name, const nbt&);
        nbt_write_compound_stream& write(std::string_view filed_name, base_objects::uuid res);
        nbt_write_compound_stream& write(std::string_view filed_name, base_objects::uuid_hex res);
        nbt_write_compound_stream& write(std::string_view filed_name, base_objects::uuid_flat_hex res);

        template <class FN>
        nbt_write_compound_stream& write(std::string_view filed_name, FN&& fn)
            requires(std::is_invocable_v<FN, nbt_write_stream&>)
        {
            nbt_write_stream inner(write_stream, filed_name, true, depth + 1);
            fn(inner);
            return *this;
        }

        //fn(name, item, inner)
        template <class Iterable, class FN>
        nbt_write_compound_stream& iterable(const Iterable& iter, FN&& fn) {
            for (const auto& [name, item] : iter)
                write(name, [&](nbt_write_stream& inner) {
                    fn(name, item, inner);
                });
            return *this;
        }

        template <class Iterable, class FN>
        nbt_write_compound_stream& iterable(const Iterable& iter) {
            for (const auto& [name, item] : iter)
                write(name, [&](nbt_write_stream& inner) {
                    inner.write(item);
                });
            return *this;
        }
    };

    namespace nbt_collection {
        class compound_relaxed {
            std::unordered_map<std::string, std::function<void(nbt_read_stream&)>> automated_collector;

        public:
            compound_relaxed() = default;
            ~compound_relaxed() = default;

            template <class FN>
            compound_relaxed& collect(const std::string& name, FN&& fn)
                requires(std::is_invocable_v<FN, nbt_read_stream&>)
            {
                automated_collector[name] = std::forward<FN>(fn);
                return *this;
            }

            compound_relaxed& collect_into(const std::string& name, bool& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, uint8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, uint16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, uint32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, uint64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, int8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, int16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, int32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, int64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, float& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, double& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, std::string& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name,nbt_enbt_convert& res){
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            
            compound_relaxed& collect_into(const std::string& name, nbt& res){
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_into(const std::string& name, base_objects::uuid& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            compound_relaxed& collect_into(const std::string& name, base_objects::uuid_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            
            compound_relaxed& collect_into(const std::string& name, base_objects::uuid_flat_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_relaxed& collect_as(const std::string& name, bool& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, uint8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, uint16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, uint32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, uint64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, int8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, int16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, int32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, int64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, float& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, double& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, std::string& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, nbt_enbt_convert& res){
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            
            compound_relaxed& collect_as(const std::string& name, nbt& res){
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_relaxed& collect_as(const std::string& name, base_objects::uuid& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            compound_relaxed& collect_as(const std::string& name, base_objects::uuid_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            
            compound_relaxed& collect_as(const std::string& name, base_objects::uuid_flat_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            template <class FN>
            compound_relaxed& collect_iterate(const std::string& name, FN&& fn)
                requires(
                    std::is_invocable_v<FN, nbt_read_stream&>
                    || std::is_invocable_v<FN, std::string_view, nbt_read_stream&>
                    || std::is_invocable_v<FN, const std::string&, nbt_read_stream&>
                )
            {
                automated_collector[name] = [fn](nbt_read_stream& stream) {
                    stream.iterate(fn);
                };
                return *this;
            }

            template <class FN>
            compound_relaxed& make_collect(nbt_read_stream& stream, FN&& on_uncollected)
                requires(std::is_invocable_v<FN, const std::string&, nbt_read_stream&>)
            {
                stream.iterate([this, &on_uncollected](const std::string& name, nbt_read_stream& stream) {
                    if (auto it = automated_collector.find(name); it != automated_collector.end())
                        it->second(stream);
                    else
                        on_uncollected(name, stream);
                });
                return *this;
            }

            compound_relaxed& make_collect(nbt_read_stream& stream) {
                return make_collect(stream, [](auto&, auto&) {});
            }

            compound_relaxed& force_all_collect(nbt_read_stream& stream) {
                std::unordered_set<std::string> collected_items;
                collected_items.reserve(automated_collector.size());
                stream.iterate([this, &collected_items](auto& name, auto& stream) {
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
            }
        };

        class compound_strict {
            std::unordered_map<std::string, std::function<void(nbt_read_stream&)>> automated_collector;
            std::vector<std::string> collector_strict_order_data;

        public:
            compound_strict() = default;
            ~compound_strict() = default;

            template <class FN>
            compound_strict& collect(const std::string& name, FN&& fn)
                requires(std::is_invocable_v<FN, nbt_read_stream&>)
            {
                automated_collector[name] = std::forward<FN>(fn);
                collector_strict_order_data.push_back(name);
                return *this;
            }

            compound_strict& collect_into(const std::string& name, bool& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, uint8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, uint16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, uint32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, uint64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, int8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, int16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, int32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, int64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, float& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, double& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, std::string& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, nbt_enbt_convert& res){
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            
            compound_strict& collect_into(const std::string& name, nbt& res){
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_into(const std::string& name, base_objects::uuid& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            compound_strict& collect_into(const std::string& name, base_objects::uuid_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            
            compound_strict& collect_into(const std::string& name, base_objects::uuid_flat_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_strict& collect_as(const std::string& name, bool& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, uint8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, uint16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, uint32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, uint64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, int8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, int16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, int32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, int64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, float& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, double& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, std::string& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            compound_strict& collect_as(const std::string& name, nbt_enbt_convert& res){
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            
            compound_strict& collect_as(const std::string& name, nbt& res){
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_strict& collect_as(const std::string& name, base_objects::uuid& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            compound_strict& collect_as(const std::string& name, base_objects::uuid_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            
            compound_strict& collect_as(const std::string& name, base_objects::uuid_flat_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }


            template <class FN>
            compound_strict& collect_iterate(const std::string& name, FN&& fn)
                requires(
                    std::is_invocable_v<FN, nbt_read_stream&>
                    || std::is_invocable_v<FN, std::string_view, nbt_read_stream&>
                    || std::is_invocable_v<FN, const std::string&, nbt_read_stream&>
                )
            {
                automated_collector[name] = [fn](nbt_read_stream& stream) {
                    stream.iterate(fn);
                };
                collector_strict_order_data.push_back(name);
                return *this;
            }

            template <class SIZE_FN, class FN>
            compound_strict& collect_iterate(const std::string& name, SIZE_FN&& fn_size, FN&& fn)
                requires(
                    (
                        std::is_invocable_v<FN, nbt_read_stream&>
                        || std::is_invocable_v<FN, std::string_view, nbt_read_stream&>
                        || std::is_invocable_v<FN, const std::string&, nbt_read_stream&>
                    )
                    && std::is_invocable_v<SIZE_FN, uint64_t>
                )
            {
                automated_collector[name] = [fn, fn_size](nbt_read_stream& stream) {
                    stream.iterate(fn_size, fn);
                };
                collector_strict_order_data.push_back(name);
                return *this;
            }

            compound_strict& make_collect(nbt_read_stream& stream) {
                stream.iterate([this, order = size_t(0)](const std::string& name, nbt_read_stream& stream) mutable {
                    if (order == collector_strict_order_data.size())
                        throw std::runtime_error("Too much elements");

                    if (auto& excepted = collector_strict_order_data.at(order++); excepted != name)
                        throw std::runtime_error("Invalid order, excepted: " + excepted + ", but got: " + name);
                    automated_collector.at(name)(stream);
                });
                return *this;
            }

            compound_strict& force_all_collect(nbt_read_stream& stream) {
                std::unordered_set<std::string> collected_items;
                collected_items.reserve(automated_collector.size());
                stream.iterate([this, &collected_items, order = size_t(0)](auto& name, auto& stream) mutable {
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
                return *this;
            }
        };

        class compound_flex {
            std::unordered_map<std::string, std::function<void(nbt_read_stream&)>> automated_collector;
            std::unordered_map<std::string, std::function<void(nbt_read_stream&)>> automated_collector_required;

        public:
            compound_flex() = default;
            ~compound_flex() = default;

            template <class FN>
            compound_flex& collect(const std::string& name, FN&& fn)
                requires(std::is_invocable_v<FN, nbt_read_stream&>)
            {
                automated_collector[name] = std::forward<FN>(fn);
                return *this;
            }

            template <class FN>
            compound_flex& collect_required(const std::string& name, FN&& fn)
                requires(std::is_invocable_v<FN, nbt_read_stream&>)
            {
                automated_collector_required[name] = std::forward<FN>(fn);
                return *this;
            }

            compound_flex& collect_into(const std::string& name, bool& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, uint8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, uint16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, uint32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, uint64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, int8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, int16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, int32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, int64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, float& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, double& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, std::string& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, nbt_enbt_convert& res){
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            
            compound_flex& collect_into(const std::string& name, nbt& res){
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into(const std::string& name, base_objects::uuid& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            compound_flex& collect_into(const std::string& name, base_objects::uuid_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }
            
            compound_flex& collect_into(const std::string& name, base_objects::uuid_flat_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_as(const std::string& name, bool& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, uint8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, uint16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, uint32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, uint64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, int8_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, int16_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, int32_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, int64_t& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, float& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, double& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, std::string& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, nbt_enbt_convert& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, nbt& res){
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as(const std::string& name, base_objects::uuid& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            compound_flex& collect_as(const std::string& name, base_objects::uuid_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }
            
            compound_flex& collect_as(const std::string& name, base_objects::uuid_flat_hex& res) {
                return collect(name, [&res](auto& stream) { stream.read_as(res); });
            }

            template <class FN>
            compound_flex& collect_iterate(const std::string& name, FN&& fn)
                requires(
                    std::is_invocable_v<FN, nbt_read_stream&>
                    || std::is_invocable_v<FN, std::string_view, nbt_read_stream&>
                    || std::is_invocable_v<FN, const std::string&, nbt_read_stream&>
                )
            {
                automated_collector[name] = [fn](nbt_read_stream& stream) {
                    stream.iterate(fn);
                };
                return *this;
            }

            compound_flex& collect_into_required(const std::string& name, bool& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, uint8_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, uint16_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, uint32_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, uint64_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, int8_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, int16_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, int32_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, int64_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, float& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, double& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, std::string& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, nbt_enbt_convert& res){
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }
            
            compound_flex& collect_into_required(const std::string& name, nbt& res){
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_into_required(const std::string& name, base_objects::uuid& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }
            compound_flex& collect_into_required(const std::string& name, base_objects::uuid_hex& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }
            
            compound_flex& collect_into_required(const std::string& name, base_objects::uuid_flat_hex& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_into(res); });
            }

            compound_flex& collect_as_required(const std::string& name, bool& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, uint8_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, uint16_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, uint32_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, uint64_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, int8_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, int16_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, int32_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, int64_t& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, float& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, double& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, std::string& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, nbt_enbt_convert& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, nbt& res){
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            compound_flex& collect_as_required(const std::string& name, base_objects::uuid& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }
            compound_flex& collect_as_required(const std::string& name, base_objects::uuid_hex& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }
            
            compound_flex& collect_as_required(const std::string& name, base_objects::uuid_flat_hex& res) {
                return collect_required(name, [&res](auto& stream) { stream.read_as(res); });
            }

            template <class FN>
            compound_flex& collect_iterate_required(const std::string& name, FN&& fn)
                requires(
                    std::is_invocable_v<FN, nbt_read_stream&>
                    || std::is_invocable_v<FN, std::string_view, nbt_read_stream&>
                    || std::is_invocable_v<FN, const std::string&, nbt_read_stream&>
                )
            {
                automated_collector_required[name] = [fn](nbt_read_stream& stream) {
                    stream.iterate(fn);
                };
                return *this;
            }

            template <class FN>
            compound_flex& make_collect(nbt_read_stream& stream, FN&& on_uncollected)
                requires(std::is_invocable_v<FN, const std::string&, nbt_read_stream&>)
            {
                std::unordered_set<std::string> collected_required;
                collected_required.reserve(automated_collector_required.size());
                stream.iterate([this, &on_uncollected, &collected_required](const std::string& name, nbt_read_stream& stream) {
                    if (auto it = automated_collector_required.find(name); it != automated_collector_required.end()) {
                        it->second(stream);
                        collected_required.emplace(name);
                    } else if (auto normal_it = automated_collector.find(name); normal_it != automated_collector.end())
                        normal_it->second(stream);
                    else
                        on_uncollected(name, stream);
                });
                for (auto& [it, _] : automated_collector_required)
                    if (!collected_required.contains(it))
                        throw std::runtime_error("Not all required elements is collected, invalid format");
                return *this;
            }

            compound_flex& make_collect(nbt_read_stream& stream) {
                return make_collect(stream, [](auto&, auto&) {});
            }

            compound_flex& force_all_collect(nbt_read_stream& stream) {
                std::unordered_set<std::string> collected_required;
                collected_required.reserve(automated_collector_required.size());
                std::unordered_set<std::string> collected_items;
                collected_items.reserve(automated_collector.size());
                stream.iterate([this, &collected_items, &collected_required](auto& name, auto& stream) {
                    if (auto it = automated_collector_required.find(name); it != automated_collector_required.end()) {
                        it->second(stream);
                        collected_required.emplace(name);
                    } else if (auto normal_it = automated_collector.find(name); normal_it != automated_collector.end()) {
                        normal_it->second(stream);
                        collected_items.emplace(name);
                    } else
                        throw std::runtime_error("Not all elements is collected, skipped item: " + name);
                });
                for (auto& [it, _] : automated_collector)
                    if (!collected_items.contains(it))
                        throw std::runtime_error("Not all elements is collected, invalid format");

                for (auto& [it, _] : automated_collector_required)
                    if (!collected_required.contains(it))
                        throw std::runtime_error("Not all required elements is collected, invalid format");
                return *this;
            }
        };
    };
}

#endif /* SRC_UTIL_NBT_STREAM */
