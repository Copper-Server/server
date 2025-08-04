#ifndef SRC_BASE_OBJECTS_PACKETS_HELP
#define SRC_BASE_OBJECTS_PACKETS_HELP
#include <algorithm>
#include <cstdint>
#include <library/list_array.hpp>
#include <src/base_objects/id_registry.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace copper_server::base_objects {
    namespace internal {
        template <template <class...> class Base, class... Ts>
        void test(Base<Ts...>&);
    }

    template <class type>
    concept is_packet = requires(type& d) {
        type::packet_id::value;
    };

    struct SharedClientData;
    template <auto value>
    using ic = std::integral_constant<decltype(value), value>;

    template <class T>
    concept enum_concept = std::is_enum_v<T>;

    template <template <class...> class, class, class = void>
    constexpr bool is_template_base_of = false;

    template <template <class...> class Base, class Derived>
    constexpr bool is_template_base_of<Base, Derived, std::void_t<decltype(internal::test<Base>(std::declval<Derived&>()))>> = true;

    template <class type>
    concept is_convertible_to_packet_form = requires(type& d) {
        d.to_packet();
        type::from_packet(decltype(d.to_packet()){});
    };

    template <is_convertible_to_packet_form type>
    using convertible_to_packet_type = decltype(std::declval<type>().to_packet());

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

    template <size_t id>
    struct alignas(2) packet {
        using packet_id = ic<id>;
    };

    struct alignas(2) disconnect_after {}; //used only with packet<>

    struct alignas(2) compound_packet {};

    //flag to preprocess packet, used for some types that affects other type by current state. Like flags_list_from
    struct alignas(2) packet_preprocess {};

    namespace switches_to {
        struct status {};

        struct login {};

        struct configuration {};

        struct play {};
    }

    template <class T, class R>
    static constexpr bool could_be_preprocessed = requires(R& it) { T::preprocess(it); };

    struct identifier {
        std::string value;

        explicit constexpr identifier() {}

        constexpr identifier(std::string&& value) : value(std::move(value)) {}

        constexpr identifier(const std::string& value) : value(value) {}

        constexpr identifier(identifier&& value) : value(std::move(value.value)) {}

        constexpr identifier(const identifier& value) : value(value.value) {}

        constexpr identifier(std::string_view value) : value(value) {}

        template <size_t siz>
        constexpr identifier(const char (&value)[siz]) : value(value) {}

        constexpr identifier& operator=(identifier&& other) {
            value = std::move(other.value);
            return *this;
        }

        constexpr identifier& operator=(const identifier& other) {
            value = other.value;
            return *this;
        }

        constexpr operator std::string&() {
            return value;
        }

        constexpr operator std::string&&() {
            return std::move(value);
        }

        constexpr operator const std::string&() const {
            return value;
        }

        auto operator<=>(const identifier& other) const = default;
    };

    template <size_t size>
    struct string_sized {
        std::string value;
        static constexpr inline size_t max_size = size;

        explicit constexpr string_sized() {}

        constexpr string_sized(std::string&& value) : value(std::move(value)) {}

        constexpr string_sized(const std::string& value) : value(value) {}

        constexpr string_sized(string_sized<size>&& value) : value(std::move(value.value)) {}

        constexpr string_sized(const string_sized<size>& value) : value(value.value) {}

        template <size_t other_size>
        constexpr string_sized(string_sized<other_size>&& value) : value(std::move(value.value)) {}

        template <size_t other_size>
        constexpr string_sized(const string_sized<other_size>& value) : value(value.value) {}

        template <size_t siz>
        constexpr string_sized(const char (&value)[siz]) : value(value) {}

        constexpr string_sized& operator=(std::string&& other) {
            value = std::move(other);
            return *this;
        }

        constexpr string_sized& operator=(const std::string& other) {
            value = other;
            return *this;
        }

        constexpr string_sized& operator=(string_sized<size>&& other) {
            value = std::move(other.value);
            return *this;
        }

        constexpr string_sized& operator=(const string_sized<size>& other) {
            value = other.value;
            return *this;
        }

        template <size_t other_size>
        constexpr string_sized& operator=(string_sized<other_size>&& other) {
            other = std::move(value.value);
            return *this;
        }

        template <size_t other_size>
        constexpr string_sized& operator=(const string_sized<other_size>& other) {
            other = value.value;
            return *this;
        }

        constexpr operator std::string&() {
            return value;
        }

        constexpr operator std::string&&() {
            return std::move(value);
        }

        constexpr operator const std::string&() const {
            return value;
        }

        auto operator<=>(const string_sized& other) const = default;
    };

    struct json_text_component {
        std::string value;

        explicit constexpr json_text_component() {}

        constexpr json_text_component(std::string&& value) : value(std::move(value)) {}

        constexpr json_text_component(const std::string& value) : value(value) {}

        constexpr json_text_component(json_text_component&& value) : value(std::move(value.value)) {}

        constexpr json_text_component(const json_text_component& value) : value(value.value) {}

        constexpr operator std::string&() {
            return value;
        }

        constexpr operator std::string&&() {
            return std::move(value);
        }

        constexpr operator const std::string&() const {
            return value;
        }

        auto operator<=>(const json_text_component& other) const = default;
    };

    template <class T, T min, T max>
    struct limited_num {
        static constexpr inline T check_min = min;
        static constexpr inline T check_max = max;
        T value;
    };

    struct var_int32 {
        using underlying_type = int32_t;
        using banner_pattern = id_source<var_int32, registry_source::banner_pattern>;
        using cat_variant = id_source<var_int32, registry_source::cat_variant>;
        using chat_type = id_source<var_int32, registry_source::chat_type>;
        using chicken_variant = id_source<var_int32, registry_source::chicken_variant>;
        using cow_variant = id_source<var_int32, registry_source::cow_variant>;
        using damage_type = id_source<var_int32, registry_source::damage_type>;
        using dialog = id_source<var_int32, registry_source::dialog>;
        using dimension_type = id_source<var_int32, registry_source::dimension_type>;
        using enchantment = id_source<var_int32, registry_source::enchantment>;
        using enchantment_provider = id_source<var_int32, registry_source::enchantment_provider>;
        using frog_variant = id_source<var_int32, registry_source::frog_variant>;
        using instrument = id_source<var_int32, registry_source::instrument>;
        using jukebox_song = id_source<var_int32, registry_source::jukebox_song>;
        using loot_table = id_source<var_int32, registry_source::loot_table>;
        using painting_variant = id_source<var_int32, registry_source::painting_variant>;
        using pig_variant = id_source<var_int32, registry_source::pig_variant>;
        using recipe = id_source<var_int32, registry_source::recipe>;
        using test_environment = id_source<var_int32, registry_source::test_environment>;
        using test_instance = id_source<var_int32, registry_source::test_instance>;
        using trim_material = id_source<var_int32, registry_source::trim_material>;
        using trim_pattern = id_source<var_int32, registry_source::trim_pattern>;
        using wolf_sound_variant = id_source<var_int32, registry_source::wolf_sound_variant>;
        using wolf_variant = id_source<var_int32, registry_source::wolf_variant>;
        using worldgen__biome = id_source<var_int32, registry_source::worldgen__biome>;

        using attribute = id_source<var_int32, registry_source::attribute>;
        using block_state = id_source<var_int32, registry_source::block_state>;
        using block_type = id_source<var_int32, registry_source::block_type>;
        using block_entity_type = id_source<var_int32, registry_source::block_entity_type>;
        using entity_type = id_source<var_int32, registry_source::entity_type>;
        using item = id_source<var_int32, registry_source::item>;
        using menu = id_source<var_int32, registry_source::menu>;
        using mob_effect = id_source<var_int32, registry_source::mob_effect>;
        using sound_event = id_source<var_int32, registry_source::sound_event>;
        using command_argument_type = id_source<var_int32, registry_source::command_argument_type>;
        using particle_type = id_source<var_int32, registry_source::particle_type>;
        using potion = id_source<var_int32, registry_source::potion>;
        using data_component_type = id_source<var_int32, registry_source::data_component_type>;
        using villager_variant = id_source<var_int32, registry_source::villager_variant>;
        using fox_variant = id_source<var_int32, registry_source::fox_variant>;
        using parrot_variant = id_source<var_int32, registry_source::parrot_variant>;
        using tropical_fish_pattern = id_source<var_int32, registry_source::tropical_fish_pattern>;
        using mooshroom_variant = id_source<var_int32, registry_source::mooshroom_variant>;
        using rabbit_variant = id_source<var_int32, registry_source::rabbit_variant>;
        using horse_variant = id_source<var_int32, registry_source::horse_variant>;
        using llama_variant = id_source<var_int32, registry_source::llama_variant>;
        using axolotl_variant = id_source<var_int32, registry_source::axolotl_variant>;

        int32_t value;

        explicit constexpr var_int32() {}

        template <enum_concept T>
        constexpr var_int32(T value) : value((int32_t)value) {}

        constexpr var_int32(int32_t value) : value(value) {}

        constexpr var_int32(const var_int32& value) : value(value.value) {}

        constexpr operator int32_t&() {
            return value;
        }

        constexpr operator const int32_t&() const {
            return value;
        }

        template <enum_concept T>
        constexpr operator T() const {
            return (T)value;
        }

        auto operator<=>(const var_int32& other) const = default;
    };

    struct var_int64 {
        using underlying_type = int64_t;
        int64_t value;

        explicit constexpr var_int64() {}

        template <enum_concept T>
        constexpr var_int64(T value) : value((int64_t)value) {}

        constexpr var_int64(int64_t value) : value(value) {}

        constexpr var_int64(const var_int64& value) : value(value.value) {}

        constexpr operator int64_t&() {
            return value;
        }

        constexpr operator const int64_t&() const {
            return value;
        }

        template <enum_concept T>
        constexpr operator T() const {
            return (T)value;
        }

        auto operator<=>(const var_int64& other) const = default;
    };

    struct optional_var_int32 : public std::optional<int32_t> { //encoded same as var_int32 but if set the value incremented and checked for overflow, if not set encoded as 0
        using std::optional<int32_t>::optional;
    };

    struct optional_var_int64 : public std::optional<int64_t> { //encoded same as var_int64 but if set the value incremented and checked for overflow, if not set encoded as 0
        using std::optional<int64_t>::optional;
    };


    template <class Value, class T>
    struct value_optional {
        Value v;
        std::optional<T> rest;
        auto operator<=>(const value_optional& other) const = default;
    };

    //if value would be zero, next fields ignored
    template <class Value>
    struct depends_next {
        using underlying_type = Value;
        Value value;

        explicit constexpr depends_next() {}

        template <class T>
        constexpr depends_next(T value)
            requires(std::is_convertible_v<T, Value>)
            : value((Value)value) {}

        constexpr depends_next(Value value) : value(value) {}

        constexpr depends_next(const depends_next& value) : value(value.value) {}

        constexpr depends_next(depends_next&& value) : value(std::move(value.value)) {}

        constexpr depends_next& operator=(const depends_next& other) {
            value = other.value;
            return *this;
        }

        constexpr depends_next& operator=(depends_next&& other) {
            value = std::move(other.value);
            return *this;
        }

        constexpr operator Value&() {
            return value;
        }

        constexpr operator const Value&() const {
            return value;
        }

        template <class T>
        constexpr operator T() const
            requires(std::is_convertible_v<Value, T>)
        {
            return (T)value;
        }

        auto operator<=>(const depends_next& other) const = default;
    };


    enum class size_source {
        get_world_chunks_height,
        get_world_blocks_height,
    };

    size_t get_size_source_value(SharedClientData&, size_source);

    //this type provides way to get size of array while decoding, the values would also be checked to be equal to size of the container
    template <auto... DependedValues>
    struct no_size {
        template <class T>
        static size_t get_depended_size(SharedClientData& context, const T& val) {
            static auto get_value = [](SharedClientData& context, const T& val, auto&& it) -> size_t {
                if constexpr (std::is_same_v<std::decay_t<decltype(it)>, size_source>)
                    return get_size_source_value(context, it);
                else if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(val.*it)>>) {
                    if constexpr (
                        std::is_same_v<std::decay_t<decltype((val.*it).value)>, var_int64>
                        || std::is_same_v<std::decay_t<decltype((val.*it).value)>, var_int32>
                    )
                        return (val.*it).value.value;
                    else
                        return (val.*it).value;
                } else if constexpr (
                    std::is_same_v<std::decay_t<decltype(val.*it)>, var_int64>
                    || std::is_same_v<std::decay_t<decltype(val.*it)>, var_int32>
                )
                    return (val.*it).value;
                else
                    return val.*it;
            };
            return (0 + ... + get_value(context, val, DependedValues));
        }
    };

    struct size_from_packet {};

    template <class T, size_t size>
    struct vector_sized : public std::vector<T> {
        using std::vector<T>::vector;
        static constexpr inline size_t max_size = size;

        vector_sized(const std::vector<T>& copy) : std::vector<T>(copy) {}

        vector_sized(std::vector<T>&& mov) : std::vector<T>(std::move(mov)) {}
    };

    template <class T, size_t size, auto... DependedValues>
    struct vector_sized_no_size : public no_size<DependedValues...>, vector_sized<T, size> {
        using vector_sized<T, size>::vector_sized;
        static constexpr inline size_t max_size = size;

        vector_sized_no_size(const std::vector<T>& copy) : vector_sized<T, size>(copy) {}

        vector_sized_no_size(std::vector<T>&& mov) : vector_sized<T, size>(std::move(mov)) {}
    };

    template <class T, auto... DependedValues>
    struct vector_no_size : public no_size<DependedValues...>, std::vector<T> {
        using std::vector<T>::vector;

        vector_no_size(const std::vector<T>& copy) : std::vector<T>(copy) {}

        vector_no_size(std::vector<T>&& mov) : std::vector<T>(std::move(mov)) {}
    };

    template <class T, size_t size>
    struct vector_sized_siz_from_packet : public size_from_packet, vector_sized<T, size> {
        using vector_sized<T, size>::vector_sized;

        vector_sized_siz_from_packet(const std::vector<T>& copy) : vector_sized<T, size>(copy) {}

        vector_sized_siz_from_packet(std::vector<T>&& mov) : vector_sized<T, size>(std::move(mov)) {}
    };

    template <class T, class T_size>
    struct sized_entry {
        using size_type = T_size;
        T value;
        auto operator<=>(const sized_entry& other) const = default;
    };

    template <class T, size_t size>
    struct vector_fixed : public std::vector<T> {
        static constexpr inline size_t required_size = size;
        using std::vector<T>::vector;
    };

    template <class T>
    struct vector_siz_from_packet : public size_from_packet, std::vector<T> {
        using std::vector<T>::vector;
    };

    template <size_t size>
    struct bitset_fixed {
        using max_size = ic<size>;
        bit_list_array<uint8_t> value;

        template <class R>
        void preprocess(R&) {
            value.resize(size);
        }

        auto operator<=>(const bitset_fixed& other) const = default;
    };

    template <class T>
    struct unordered_id {
        T value;
        auto operator<=>(const unordered_id& other) const = default;
    };

    template <class T>
    struct ordered_id {
        T value;
        auto operator<=>(const ordered_id& other) const = default;
    };

    template <class Variant0, class Variant1>
    struct or_ : public std::variant<Variant0, Variant1> {
        using var_0 = Variant0;
        using var_1 = Variant1;
        using std::variant<Variant0, Variant1>::variant;
    };

    template <class T>
    struct id_set : public std::variant<identifier, std::vector<T>> {
        using std::variant<identifier, std::vector<T>>::variant;
        using id_type = T;
    };

    struct Angle {
        using pi = ic<3.14159265358979323846>;
        uint8_t value;

        Angle(double val) : value(uint8_t((val * pi::value * 2) / 360)) {}

        Angle() : value(0) {}

        Angle(const Angle& value) : value(value.value) {}

        operator double() {
            return (value * 360) / (pi::value * 2);
        }

        auto operator<=>(const Angle& other) const = default;
    };

    template <class Enum, class T>
    struct enum_as {
        using encode_t = T;
        Enum value;

        explicit enum_as() : value() {}

        enum_as(Enum e) : value(e) {}

        enum_as(T e) : value((Enum)e) {}

        T get() const {
            if constexpr (std::is_same_v<var_int32, T> || std::is_same_v<var_int64, T>)
                return (T)(typename T::underlying_type)value;
            else
                return (T)value;
        }

        auto operator<=>(const enum_as& other) const = default;
    };

    template <size_t id>
    struct alignas(2) enum_item {
        using item_id = ic<id>;
    };

    template <class T>
    static constexpr bool is_enum_item = requires(T& t) {
        []<template <auto> class Base, auto v>(Base<v>&) {}.template operator()<enum_item>(t);
    };

    template <class ValueType, class... Ty>
    struct enum_switch : public std::variant<Ty...> {
        using std::variant<Ty...>::variant;
        using encode_type = ValueType;
        using base = std::variant<Ty...>;

        template <class FN>
        static void get_enum(size_t id, FN&& fn) {
            for_each_type<base>::each(
                [&]<class T>() {
                    if (T::item_id::value == id)
                        fn.template operator()<T>();
                }
            );
        }
    };

    //to exclude from packet use negative order, the order used as id for flags
    template <size_t value, size_t mask, ptrdiff_t order>
    struct alignas(2) flags_item {
        using flag_value = ic<value>;
        using flag_mask = ic<mask>;
        using flag_order = ic<order>;
    };

    template <class T>
    static constexpr bool is_flag_item = requires() {
        []<template <auto, auto, auto> class Base, auto v0, auto v1, auto v2>(Base<v0, v1, v2>&) {}.template operator()<flags_item>(std::declval<T&>());
    };

    template <class flag_type, class... Ty>
    struct flags_list {
        using max_orders = ic<std::max<ptrdiff_t>({Ty::flag_order::value...})>;
        using base = std::variant<Ty...>;
        flag_type flag;
        std::unordered_map<ptrdiff_t, std::variant<Ty...>> values; //order->value

        static flags_list make(std::initializer_list<base> flags) {
            flags_list res;
            for (auto& it : flags)
                res.set(std::move(it));
            return res;
        }

        template <class T>
        bool is_set() const {
            return (values.find(T::flag_order::value) != values.end());
        }

        template <class FN>
        void for_each(FN&& fn) {
            for (auto& [order, it] : values)
                std::visit(fn, it);
        }

        template <class FN>
        void for_each_in_order(FN&& fn) {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                auto it = values.find(order);
                if (it != values.end())
                    std::visit(fn, it->second);
            }
        }

        template <class FN>
        void for_each(FN&& fn) const {
            for (auto& [order, it] : values)
                std::visit(fn, it);
        }

        template <class FN>
        void for_each_in_order(FN&& fn) const {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                auto it = values.find(order);
                if (it != values.end())
                    std::visit(fn, it->second);
            }
        }

        template <class FN>
        static void for_each_flag_in_order(FN&& fn) {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                for_each_type<base>::each(
                    [&]<class T>() {
                        if (T::flag_order::value == order)
                            fn.template operator()<T>();
                    }
                );
            }
        }

        template <class FN>
        void for_each_set_flag_in_order(FN&& fn) {
            for_each_flag_in_order([&]<class T>() {
                if ((flag & T::flag_mask::value) == T::flag_value::value)
                    fn.template operator()<T>();
            });
        }

        template <class FN>
        void for_each_set_flag_in_order(FN&& fn) const {
            for_each_flag_in_order([&]<class T>() {
                if ((flag & T::flag_mask::value) == T::flag_value::value)
                    fn.template operator()<T>();
            });
        }

        template <class T>
        void set(T&& item) {
            values[T::flag_order::value] = std::move(item);
            update_flag();
        }

        template <class T>
        void set(const T&& item) = delete;

        template <class T>
        void set() {
            values[T::flag_order::value] = T();
            update_flag();
        }

        auto operator<=>(const flags_list& other) const = default;

    private:
        void update_flag() {
            flag = 0;
            for (auto& [id, value] : values) {
                std::visit(
                    [this](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        flag |= (T::flag_value::value & T::flag_mask::value);
                    },
                    value
                );
            }
        }
    };

    template <class Source, class SourceType, SourceType Source::* source_name, class... Ty>
    struct flags_list_from {
        using max_orders = ic<std::max<ptrdiff_t>({Ty::flag_order::value...})>;
        using preprocess_source_name = ic<source_name>;
        using base = std::variant<Ty...>;
        using source_type = SourceType;
        std::unordered_map<ptrdiff_t, std::variant<Ty...>> values; //flag_order->value
        source_type pre_process_result;

        static flags_list_from make(std::initializer_list<base> flags) {
            flags_list_from res;
            for (auto& it : flags)
                res.set(std::move(it));
            return res;
        }

        template <class T>
        bool is_set() const {
            return (values.find(T::flag_order::value) != values.end());
        }

        void preprocess(Source& source) {
            source.*source_name = 0;
            for (auto& [id, value] : values)
                source.*source_name |= (decltype(value)::flag_value::value & decltype(value)::flag_mask::value);
            pre_process_result = source.*source_name;
        }

        template <class FN>
        void for_each(FN&& fn) {
            for (auto& [order, it] : values)
                std::visit(fn, it);
        }

        template <class FN>
        void for_each_in_order(FN&& fn) {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                auto it = values.find(order);
                if (it != values.end())
                    std::visit(fn, it->second);
            }
        }

        template <class FN>
        void for_each(FN&& fn) const {
            for (auto& [order, it] : values)
                std::visit(fn, it);
        }

        template <class FN>
        void for_each_in_order(FN&& fn) const {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                auto it = values.find(order);
                if (it != values.end())
                    std::visit(fn, it->second);
            }
        }

        template <class FN>
        static void for_each_flag_in_order(FN&& fn) {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                for_each_type<base>::each(
                    [&]<class T>() {
                        if (T::flag_order::value == order)
                            fn.template operator()<T>();
                    }
                );
            }
        }

        template <class FN>
        static void for_each_set_flag_in_order(auto flag, FN&& fn) {
            for_each_flag_in_order([&]<class T>() {
                if ((flag & T::flag_mask::value) == T::flag_value::value)
                    fn.template operator()<T>();
            });
        }

        template <class T>
        void set(T&& item) {
            values[T::flag_order::value] = base(std::move(item));
        }

        template <class T>
        void set(const T&& item) = delete;

        template <class T>
        void set() {
            values[T::flag_order::value] = base(T());
        }

        auto operator<=>(const flags_list_from& other) const = default;
    };

    template <class Enum, class T>
    struct enum_as_flag {
        using encode_t = T;
        Enum value;

        explicit enum_as_flag() : value() {}

        enum_as_flag(Enum e) : value(e) {}

        enum_as_flag(T e) : value((Enum)e) {}

        T get() const {
            return (T)value;
        }

        auto operator<=>(const enum_as_flag& other) const = default;
    };

    template <class base_type, class... Ty>
    struct any_of {
        base_type value;

        template <class T>
        T& cast() {
            return reinterpret_cast<T&>(value);
        }

        template <class T>
        const T& cast() const {
            return reinterpret_cast<const T&>(value);
        }

        template <class T>
        any_of& operator=(T&& val) {
            value = reinterpret_cast<base_type&&>(val);
            return *this;
        }

        template <class T>
        any_of& operator=(const T& val) {
            value = reinterpret_cast<const base_type&>(val);
            return *this;
        }

        auto operator<=>(const any_of& other) const = default;
    };

    template <class T>
    struct ignored {
        T value;

        operator T&() {
            return value;
        }

        operator const T&() const {
            return value;
        }

        auto operator<=>(const ignored& other) const = default;
    };
}

#endif /* SRC_BASE_OBJECTS_PACKETS_HELP */
