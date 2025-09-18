/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_TEMPLATES
#define SRC_UTIL_TEMPLATES
#include <tuple>
#include <variant>

namespace copper_server::util{

    namespace __internal {
        template <template <class...> class Base, class... Ts>
        void test(Base<Ts...>&);
    }

    template <template <class...> class, class, class = void>
    constexpr bool is_template_base_of = false;

    template <template <class...> class Base, class Derived>
    constexpr bool is_template_base_of<Base, Derived, std::void_t<decltype(__internal::test<Base>(std::declval<Derived&>()))>> = true;

    template <class A>
    struct for_each_type {
        static constexpr auto each(auto&& fn) {
            fn.template operator()<A>();
        }
    };

    template <class... Args>
    struct for_each_type<std::variant<Args...>> {
        static constexpr void each(auto&& lambda) {
            (
                [&]() {
                    lambda.template operator()<Args>();
                }(),
                ...
            );
        }
    };

    template <class... Args>
    struct for_each_type<std::tuple<Args...>> {
        static constexpr void each(auto&& lambda) {
            (
                [&]() {
                    lambda.template operator()<Args>();
                }(),
                ...
            );
        }
    };

    template <class T>
    struct function_traits;

    template <class Ret, class... Args>
    struct function_traits<Ret (*)(Args...)> {
        using argument_tuple = std::tuple<Args...>;
    };

    template <class Functor>
    struct function_traits : function_traits<decltype(&std::decay_t<Functor>::operator())> {};

    template <class ClassType, class Ret, class... Args>
    struct function_traits<Ret (ClassType::*)(Args...) const> {
        using argument_tuple = std::tuple<Args...>;
    };

    template <class ClassType, class Ret, class... Args>
    struct function_traits<Ret (ClassType::*)(Args...)> {
        using argument_tuple = std::tuple<Args...>;
    };

    template <template <class...> class T, class Tuple>
    struct apply_tuple_to;

    template <template <class...> class T, class... Args>
    struct apply_tuple_to<T, std::tuple<Args...>> {
        using type = T<Args...>;
    };

    template <typename T, typename Variant>
    concept is_gettable_from_variant = !is_template_base_of<std::variant, T> && requires(Variant v) {
        { std::get<std::decay_t<T>>(v) };
    };

    template <class T>
    concept IsVariant = is_template_base_of<std::variant, T>;

    template <class T, class S>
        requires std::is_same_v<T, S>
    decltype(auto) make_cast(S&& value) {
        return std::forward<S>(value);
    }

    template <class Target, class Source>
        requires(!std::is_same_v<Target, Source>) && IsVariant<std::remove_cvref_t<Target>> && IsVariant<std::remove_cvref_t<Source>>
    auto make_cast(Source&& value) {
        using ResultType = std::conditional_t<
            std::is_rvalue_reference_v<Source>,
            Target&&,
            std::conditional_t<
                std::is_const_v<std::remove_reference_t<Source>>,
                const Target&,
                Target&>>;

        return std::visit([&](auto&& inner_value) -> ResultType {
            return static_cast<ResultType>(std::forward<decltype(inner_value)>(inner_value));
        },
                          static_cast<typename std::remove_cvref_t<Source>::base>(std::forward<Source>(value)));
    }

    template <class Target, class Source>
        requires(!std::is_same_v<Target, Source>) && std::is_convertible_v<Source, Target>
    auto make_cast(Source&& value) -> decltype(static_cast<Target>(std::forward<Source>(value))) {
        return static_cast<Target>(std::forward<Source>(value));
    }

    template <class Target, class... Source>
        requires is_gettable_from_variant<Target, std::variant<Source...>>
    Target&& make_cast(std::variant<Source...>&& value) {
        return std::get<std::decay_t<Target>>(std::move(value));
    }

    template <class Target, class... Source>
        requires is_gettable_from_variant<Target, std::variant<Source...>&>
    Target& make_cast(std::variant<Source...>& value) {
        return std::get<std::decay_t<Target>>(value);
    }

    template <class Target, class... Source>
        requires is_gettable_from_variant<Target, const std::variant<Source...>&>
    const Target& make_cast(const std::variant<Source...>& value) {
        return std::get<std::decay_t<Target>>(value);
    }

    template <class Target, class Source>
    concept is_convertible = requires(Source&& s) {
        make_cast<Target>(std::forward<Source>(s));
    };

    // Allows for creating function proxies where arguments are convertible,
    // including custom conversions for std::variant.
    template <class Ret, class... Args>
    struct convertible_function {
        template <class... OArgs>
            requires((sizeof...(Args) == sizeof...(OArgs)) && (is_convertible<OArgs, Args> && ...))
        struct holder : public std::function<Ret(OArgs...)> {
            using std::function<Ret(OArgs...)>::function;

            std::function<Ret(Args...)> make_proxy() && {
                return [fn = std::move(*this)](Args... args) {
                    return fn(make_cast<OArgs>(std::forward<Args>(args))...);
                };
            }
        };

        template <class Func>
        static auto make_proxy_from_callable(Func&& func) {
            using functor_trait = function_traits<std::decay_t<Func>>;
            using args_tuple = typename functor_trait::argument_tuple;
            using convertible_func = typename apply_tuple_to<holder, args_tuple>::type;

            return convertible_func(std::forward<Func>(func)).make_proxy();
        }
    };
}

#endif /* SRC_UTIL_TEMPLATES */
