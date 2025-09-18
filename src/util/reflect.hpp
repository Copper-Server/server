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
#include <charconv>
#include <string>

namespace copper_server::reflect {
    template <class T>
    consteval std::string_view type_name();

    template <class T>
    consteval size_t fields_count() {
        return 0;
    }

    template <class T>
    struct enum_data {};

    template <class T>
    struct for_each_type_s {};

    template <class T>
    struct for_each_type_with_name_s {};

    template <class T>
    struct visit_field_s {};

    template <class T>
    struct visit_field_with_name_s {};

    template <class T, class FN>
    constexpr void for_each_type(FN&& fn) {
        for_each_type_s<T>::each(std::move(fn));
    }

    template <class T, class FN>
    constexpr void for_each_type_with_name(FN&& fn) {
        for_each_type_with_name_s<T>::each(std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field(std::string_view name, FN&& fn) {
        visit_field_s<T>::visit(name, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field_with_name(std::string_view name, FN&& fn) {
        visit_field_with_name_s<T>::visit(name, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field(size_t index, FN&& fn) {
        visit_field_s<T>::visit(index, std::move(fn));
    }

    template <class T, class FN>
    constexpr void visit_field_with_name(size_t index, FN&& fn) {
        visit_field_with_name_s<T>::visit(index, std::move(fn));
    }

    template <class T>
    consteval std::string_view type_name() {
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
    consteval std::string_view get_pretty_type_name() {
        constexpr std::string_view name = type_name<T>();
        if (name == "enbt::value")
            return "NBT";
        if (name == "enbt::raw_uuid")
            return "UUID";
        if (auto it = name.rfind("::"); it != name.npos) {
            return name.substr(it + 2);
        } else
            return name;
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
