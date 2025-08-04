#ifndef SRC_UTIL_REFLECT
#define SRC_UTIL_REFLECT
#define REFLECT_CPP_C_ARRAYS_OR_INHERITANCE
#include <rfl/to_view.hpp>

namespace copper_server::reflect {
    template <class EnumT>
    struct enum_limits {
        static constexpr std::underlying_type_t<EnumT> min = std::numeric_limits<std::underlying_type_t<EnumT>>::min();
        static constexpr std::underlying_type_t<EnumT> max = std::numeric_limits<std::underlying_type_t<EnumT>>::max();
    };

    template <class T, class F>
    constexpr void for_each_field(T&& obj, F&& func) {
        rfl::apply(
            [f = std::forward<F>(func)](auto&&... elements) mutable {
                (f(*elements), ...);
            },
            rfl::to_view(obj).values()
        );
    }

    template <class T>
    struct names_extractor {};

    template <class... Literals>
    struct names_extractor<rfl::Tuple<Literals...>> {
        static constexpr inline std::array<std::string_view, sizeof...(Literals)> literals{Literals::name_.string_view()...};
    };

    template <class Literals, class F, class... Types, int... _is>
    auto apply_w_names_unfold(F&& _f, const rfl::Tuple<Types...>& _tup, std::integer_sequence<int, _is...>) {
        (_f(*rfl::get<_is>(_tup), Literals::literals[_is]), ...);
    }

    template <class Literals, class F, class... Types, int... _is>
    auto apply_w_names_unfold(F&& _f, rfl::Tuple<Types...>& _tup, std::integer_sequence<int, _is...>) {
        (_f(*rfl::get<_is>(_tup), Literals::literals[_is]), ...);
    }

    template <class Literals, class F, class... Types, int... _is>
    auto apply_w_names_unfold(F&& _f, rfl::Tuple<Types...>&& _tup, std::integer_sequence<int, _is...>) {
        (_f(std::move(*rfl::get<_is>(_tup)), Literals::literals[_is]), ...);
    }

    template <class Literals, class F, class... Types>
    void apply_w_names(F&& _f, const rfl::Tuple<Types...>& _tup) {
        apply_w_names_unfold<Literals>(
            _f,
            _tup,
            std::make_integer_sequence<int, sizeof...(Types)>()
        );
    }

    template <class Literals, class F, class... Types>
    void apply_w_names(F&& _f, rfl::Tuple<Types...>& _tup) {
        apply_w_names_unfold<Literals>(
            _f,
            _tup,
            std::make_integer_sequence<int, sizeof...(Types)>()
        );
    }

    template <class Literals, class F, class... Types>
    void apply_w_names(F&& _f, rfl::Tuple<Types...>&& _tup) {
        apply_w_names_unfold<Literals>(
            _f,
            std::move(_tup),
            std::make_integer_sequence<int, sizeof...(Types)>()
        );
    }

    template <class T, class F>
    constexpr void for_each_field_with_name(T&& obj, F&& func) {
        auto res = rfl::to_view(obj);
        apply_w_names<names_extractor<typename decltype(res)::Fields>>(func, res.values());
    }

    template <class T>
    constexpr std::string_view get_type_name() {
#if defined(__clang__) || defined(__GNUC__)
        constexpr std::string_view func = __PRETTY_FUNCTION__;
        constexpr std::string_view prefix = "T = ";
        auto start = func.find(prefix) + prefix.size();
        auto end = func.find(']', start);
        return func.substr(start, end - start);
#elif defined(_MSC_VER)
        constexpr std::string_view func = __FUNCSIG__;
        constexpr std::string_view prefix = "get_type_name<";
        auto start = func.find(prefix) + prefix.size();
        auto end = func.rfind('>');
        return func.substr(start, end - start);
#else
        return "unknown";
#endif
    }

    template <class T>
    consteval std::string_view get_pretty_type_name() {
        constexpr std::string_view name = get_type_name<T>();
        if (name == "enbt::value")
            return "NBT";
        if (name == "enbt::raw_uuid")
            return "UUID";
        if (auto it = name.rfind("::"); it != name.npos) {
            return name.substr(it + 2);
        } else
            return name;
    }

    template <auto T>
    consteval std::string_view get_value_name() {
#if defined(__clang__) || defined(__GNUC__)
        constexpr std::string_view func = __PRETTY_FUNCTION__;
        constexpr std::string_view prefix = "T = ";
        auto start = func.find(prefix) + prefix.size();
        auto end = func.find(']', start);
        return func.substr(start, end - start);
#elif defined(_MSC_VER)
        constexpr std::string_view func = __FUNCSIG__;
        constexpr std::string_view prefix = "get_value_name<";
        auto start = func.find(prefix) + prefix.size();
        auto end = func.rfind('>');
        return func.substr(start, end - start);
#else
        return "unknown";
#endif
    }

    template <auto T>
    consteval std::string_view get_pretty_value_name() {
        constexpr std::string_view name = get_value_name<T>();
        if (auto it = name.rfind("::"); it != name.npos)
            return name.substr(it + 2);
        else
            return name;
    }

    template <auto E>
    struct get_enum_by_value {
        static consteval std::string_view __get_enum_value_by_num() {
            constexpr std::string_view res = get_pretty_value_name<E>();
            if (res.ends_with(')'))
                return res.substr(0, res.size() - 1);
            else
                return res;
        }

        static consteval std::string_view _get_enum_value_by_num() {
            constexpr std::string_view res = __get_enum_value_by_num();
            if (auto it = res.rfind(")"); it != res.npos)
                return res.substr(it + 1);
            else
                return res;
        }

        static consteval std::string_view by_num() {
            constexpr std::string_view res = _get_enum_value_by_num();
            if (auto it = res.find("("); it != res.npos)
                return res.substr(it + 1);
            return res;
        }

        static consteval bool is_exists() {
            return by_num().find_first_not_of("-0123456789") != std::string_view::npos;
        }
    };

    template <class EnumT, int64_t... Is>
    constexpr std::array<std::string_view, sizeof...(Is)> make_enum_value_names_impl(std::integer_sequence<int64_t, Is...>) {
        return {reflect::get_enum_by_value<static_cast<EnumT>(Is)>::by_num()...};
    }

    template <class EnumT, int64_t min, int64_t max>
    static constexpr std::array<std::string_view, max - min> make_enum_names() {
        return make_enum_value_names_impl<EnumT>(std::make_integer_sequence<int64_t, max - min>{});
    }

    template <class EnumT, int64_t min, int64_t max>
    struct enum_value_extractor {
        static constexpr int min_v = min;
        static constexpr int max_v = max;
        static constexpr inline std::array<std::string_view, max_v - min_v> res = make_enum_names<EnumT, min_v, max_v>();

        static std::string_view get_enum_value(EnumT val) {
            int64_t i = static_cast<int64_t>(val);
            if (i < min_v || i >= max_v)
                return "";
            return res[i - min_v];
        }

        static EnumT from_string(std::string_view str) {
            size_t i = 0;
            for (auto& it : res) {
                if (it == str)
                    return static_cast<EnumT>(i + min);
                i++;
            }
            return static_cast<EnumT>(min);
        }
    };

    template <class EnumT, size_t... Is>
    constexpr std::array<std::string_view, sizeof...(Is)> make_enum_flag_names_impl(std::index_sequence<Is...>) {
        using U = std::underlying_type_t<EnumT>;
        return {reflect::get_enum_by_value<static_cast<EnumT>(U(1) << Is)>::by_num()...};
    }

    template <class EnumT>
    constexpr auto make_enum_flag_names() {
        using U = std::underlying_type_t<EnumT>;
        return make_enum_flag_names_impl<EnumT>(std::make_index_sequence<sizeof(U) * 8>{});
    }

    template <class EnumT>
    struct enum_flags_extractor {
        static constexpr inline auto res = make_enum_flag_names<EnumT>();

        static std::string_view get_enum_value(size_t bit) {
            return res[bit];
        }

        static EnumT from_string(std::string_view str) {
            size_t i = 0;
            for (auto& it : res) {
                if (it == str)
                    return static_cast<EnumT>(1 << i);
                i++;
            }
            return EnumT{};
        }

    };

    template <class EnumT>
    std::string get_enum_value(EnumT value) {
        using U = std::underlying_type_t<EnumT>;
        return std::string(enum_value_extractor<EnumT, enum_limits<EnumT>::min, enum_limits<EnumT>::max>::get_enum_value(value));
    }

    template <class EnumT>
    std::string get_enum_flag_value(EnumT value) {
        using U = std::underlying_type_t<EnumT>;
        U check = 0x1;
        std::string res;
        using extract = enum_flags_extractor<EnumT>;
        size_t index = 0;
        for (size_t shifts_left = sizeof(U) * 8; shifts_left; shifts_left--) {
            if (value == static_cast<EnumT>(check))
                if (res.size())
                    res += "|" + std::string(extract::get_enum_value(index));
                else
                    res += std::string(extract::get_enum_value(index));
            index++;
        }
        return res;
    }

    template <class EnumT>
    EnumT get_enum_form_string(std::string_view value) {
        using U = std::underlying_type_t<EnumT>;
        return std::string(enum_value_extractor<EnumT, enum_limits<EnumT>::min, enum_limits<EnumT>::max>::from_string(value));
    }

    template <class EnumT>
    EnumT get_enum_flag_form_string(std::string_view value) {
        using extract = enum_flags_extractor<EnumT>;
        EnumT r{};
        for (; value.size();) {
            auto i = value.find('|');
            if (i != std::string_view::npos) {
                auto tmp = value.substr(0, i);
                value = value.substr(i + 1);
                r = r | extract::from_string(tmp);
            } else {
                r = r | extract::from_string(value);
                break;
            }
        }
        return r;
    }
}

#endif /* SRC_UTIL_REFLECT */
