/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_REFLECT
#define SRC_UTIL_REFLECT
#include <array>
#include <charconv>
#include <string>

namespace copper_server::reflect {
    template <class T>
    struct enum_data {};

    template <class T>
    struct meta_for_type_s {};

    template <class T>
    consteval size_t fields_count() {
        if constexpr (requires { meta_for_type_s<T>::fields_count; })
            return meta_for_type_s<T>::fields_count;
        else
            return 0;
    }

    template <class T, class FN>
    constexpr void for_each_field(T& val, FN&& fn) {
        meta_for_type_s<T>::for_each_field(val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void for_each_field(const T& val, FN&& fn) {
        meta_for_type_s<T>::for_each_field(val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void for_each_type(FN&& fn) {
        meta_for_type_s<T>::for_each_type(std::move(fn));
    }

    template <class T, class FN>
    constexpr void for_each_field_with_name(T& val, FN&& fn) {
        meta_for_type_s<T>::for_each_field_with_name(val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void for_each_field_with_name(const T& val, FN&& fn) {
        meta_for_type_s<T>::for_each_field_with_name(val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void for_each_type_with_name(FN&& fn) {
        meta_for_type_s<T>::for_each_type_with_name(std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field(std::string_view name, T& val, FN&& fn) {
        meta_for_type_s<T>::visit_field(name, val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field(std::string_view name, const T& val, FN&& fn) {
        meta_for_type_s<T>::visit_field(name, val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field(std::string_view name, FN&& fn) {
        meta_for_type_s<T>::visit_field(name, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field_with_name(std::string_view name, T& val, FN&& fn) {
        meta_for_type_s<T>::visit_field_with_name(name, val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field_with_name(std::string_view name, const T& val, FN&& fn) {
        meta_for_type_s<T>::visit_field_with_name(name, val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field_with_name(std::string_view name, FN&& fn) {
        meta_for_type_s<T>::visit_field_with_name(name, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field(size_t index, T& val, FN&& fn) {
        meta_for_type_s<T>::visit_field(index, val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field(size_t index, const T& val, FN&& fn) {
        meta_for_type_s<T>::visit_field(index, val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field(size_t index, FN&& fn) {
        meta_for_type_s<T>::visit_field(index, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field_with_name(size_t index, T& val, FN&& fn) {
        meta_for_type_s<T>::visit_field_with_name(index, val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field_with_name(size_t index, const T& val, FN&& fn) {
        meta_for_type_s<T>::visit_field_with_name(index, val, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field_with_name(size_t index, FN&& fn) {
        meta_for_type_s<T>::visit_field_with_name(index, std::move(fn));
    }

    template <class T>
    consteval std::string_view type_name_compile_time() {
#if defined(__clang__) || defined(__GNUC__)
        constexpr std::string_view func = __PRETTY_FUNCTION__;
        constexpr std::string_view prefix = "T = ";
        auto start = func.find(prefix) + prefix.size();
        auto end = func.find(']', start);
        return func.substr(start, end - start);
#elif defined(_MSC_VER)
        constexpr std::string_view func = __FUNCSIG__;
        constexpr std::string_view prefix = "type_name<";
        auto start = func.find(prefix) + prefix.size();
        auto end = func.rfind('>');
        return func.substr(start, end - start);
#else
        return "unknown";
#endif
    }

    template <class T>
    consteval std::string_view type_name() {
        if constexpr (requires { meta_for_type_s<T>::type_name(); }) {
            return meta_for_type_s<T>::type_name();
        } else {
            return type_name_compile_time<T>();
        }
    }

    template <class T>
    concept has_name_override_cts = requires { T::name_override::data; };

    template <class T>
    consteval std::string_view get_pretty_type_name() {
        if constexpr (has_name_override_cts<T>) {
            return T::name_override::data;
        } else {
            constexpr std::string_view name = type_name<T>();
            if (name == "nbt_compound")
                return "nbt";
            if (auto it = name.rfind("::"); it != name.npos) {
                return name.substr(it + 2);
            } else
                return name;
        }
    }

    template <class T>
    concept declared_enum = requires { enum_data<T>::values; };

    template <class EnumT>
    constexpr std::string get_enum_value(EnumT value) {
        for (auto& it : enum_data<EnumT>::values)
            if (it.second == value)
                return std::string(it.first);

        char res[20];
        auto success = std::to_chars(res, res + sizeof(res), static_cast<std::underlying_type_t<EnumT>>(value));
        if (success.ec == std::errc())
            return std::string(res, success.ptr);
        else
            return "<unknown_enum_value>";
    }

    template <class EnumT>
    constexpr EnumT get_enum_value(std::string_view value) {
        for (auto& it : enum_data<EnumT>::values)
            if (it.first == value)
                return it.second;

        std::underlying_type_t<EnumT> res;
        auto success = std::from_chars(value.data(), value.data() + value.size(), res);
        if (success.ec == std::errc())
            return static_cast<EnumT>(res);
        else
            return EnumT(0);
    }

    template <class EnumT>
    constexpr std::string get_enum_flag_value(EnumT value) {
        using U = std::underlying_type_t<EnumT>;
        U check = 0x1;
        std::string res;
        for (size_t shifts_left = sizeof(U) * 8; shifts_left; shifts_left--) {
            if (bool(static_cast<U>(value) & check))
                res += (res.size() ? "|" : "") + get_enum_value(static_cast<EnumT>(check));
            check <<= 1;
        }
        return res;
    }

    template <class EnumT>
    constexpr EnumT get_enum_flag_value(std::string_view value) {
        EnumT res;
        for (; value.size();) {
            auto i = value.find('|');
            if (i != std::string_view::npos) {
                auto tmp = value.substr(0, i);
                value = value.substr(i + 1);
                res = EnumT(static_cast<std::underlying_type_t<EnumT>>(res) | static_cast<std::underlying_type_t<EnumT>>(get_enum_value<EnumT>(tmp)));
            } else {
                res = EnumT(static_cast<std::underlying_type_t<EnumT>>(res) | static_cast<std::underlying_type_t<EnumT>>(get_enum_value<EnumT>(value)));
                break;
            }
        }
        return res;
    }

    //template <class EnumT>
    //EnumT get_enum_flag_form_string(std::string_view value) {
    //    EnumT r{};
    //    for (; value.size();) {
    //        auto i = value.find('|');
    //        if (i != std::string_view::npos) {
    //            auto tmp = value.substr(0, i);
    //            value = value.substr(i + 1);
    //            r = r | from_string(tmp);
    //        } else {
    //            r = r | from_string(value);
    //            break;
    //        }
    //    }
    //    return r;
    //}
}

#endif /* SRC_UTIL_REFLECT */
