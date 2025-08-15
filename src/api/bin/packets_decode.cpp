/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/network/tcp.hpp>
#include <src/api/packets.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/log.hpp>
#include <src/util/readers.hpp>
#include <src/util/reflect.hpp>
#include <src/util/reflect/calculations.hpp>
#include <src/util/reflect/component.hpp>
#include <src/util/reflect/dye_color.hpp>
#include <src/util/reflect/packets.hpp>
#include <src/util/reflect/packets_help.hpp>
#include <src/util/reflect/parsers.hpp>

namespace copper_server::api::packets {
    extern bool debugging_enabled;
    using namespace base_objects;

    namespace __internal {
        bool visit_packet_viewer(client_bound_packet& packet, SharedClientData& context);
        bool visit_packet_viewer(server_bound_packet& packet, SharedClientData& context);
    }


    template <class type>
    concept is_packet = requires(type& d) {
        type::packet_id::value;
    };


    template <template <auto...> class Base, auto... Ts>
    void value_test(Base<Ts...>&);

    template <template <class, auto...> class Base, class T, auto... Ts>
    void tvalue_test(Base<T, Ts...>&);


    template <template <auto...> class, class, class = void>
    constexpr bool is_value_template_base_of = false;

    template <template <auto...> class Base, class Derived>
    constexpr bool is_value_template_base_of<Base, Derived, std::void_t<decltype(value_test<Base>(std::declval<Derived&>()))>> = true;


    template <template <class, auto...> class, class, class = void>
    constexpr bool is_tvalue_template_base_of = false;

    template <template <class, auto...> class Base, class Derived>
    constexpr bool is_tvalue_template_base_of<Base, Derived, std::void_t<decltype(tvalue_test<Base>(std::declval<Derived&>()))>> = true;

    // Helper to detect instantiations of flags_list_from
    template <class T>
    struct is_flags_list_from_helper : std::false_type {};

    template <class S, class ST, ST S::* M, class... Ts>
    struct is_flags_list_from_helper<flags_list_from<S, ST, M, Ts...>> : std::true_type {};

    template <class type>
    concept is_flags_list_from = is_flags_list_from_helper<std::decay_t<type>>::value;

    template <class type>
    concept is_string_sized = is_value_template_base_of<string_sized, type>;
    template <class type>
    concept is_bitset_fixed = is_value_template_base_of<bitset_fixed, type>;

    template <class type>
    concept is_list_array_sized = is_tvalue_template_base_of<list_array_sized, type> || is_tvalue_template_base_of<list_array_sized_siz_from_packet, type> || is_tvalue_template_base_of<list_array_sized_no_size, type>;

    template <class type>
    concept is_list_array_fixed = is_tvalue_template_base_of<list_array_fixed, type>;

    template <class type>
    concept is_std_array = is_tvalue_template_base_of<std::array, type>;

    template <class type>
    concept is_limited_num = is_tvalue_template_base_of<limited_num, type>;

    template <class type>
    concept requires_check = is_limited_num<type> || is_string_sized<type> || is_list_array_sized<type> || is_list_array_fixed<type> || is_value_template_base_of<no_size, type>;


    struct processor_handle_data {
        uint8_t mode;
        size_t id;
    };

    struct processors_manager {
        uint64_t id_counter = 0;
        std::unordered_map<uint64_t, processor_handle_data> hd;
        std::unordered_map<size_t, std::function<void(server_bound_packet&&, base_objects::SharedClientData&)>> handles[5];

        base_objects::events::event_register_id register_h(uint8_t mode, size_t id, std::function<void(server_bound_packet&&, base_objects::SharedClientData&)>&& fn) {
            auto& it = handles[mode][id];
            if (it)
                throw std::runtime_error("This packet already registered.");
            it = std::move(fn);
            do {
                ++id_counter;
            } while (hd.contains(id_counter));
            hd[id_counter] = {mode, id};
            return {id_counter};
        }

        void handle(uint8_t mode, size_t id, server_bound_packet&& packet, base_objects::SharedClientData& context) {
            auto& it = handles[mode][id];
            if (!it) {
                switch (mode) {
                case 0:
                    throw std::runtime_error("Handler for packet with id handshake:" + std::to_string(id) + " is not registered.");
                case 1:
                    throw std::runtime_error("Handler for packet with id status:" + std::to_string(id) + " is not registered.");
                case 2:
                    throw std::runtime_error("Handler for packet with id login:" + std::to_string(id) + " is not registered.");
                case 3:
                    throw std::runtime_error("Handler for packet with id configuration:" + std::to_string(id) + " is not registered.");
                case 4:
                    throw std::runtime_error("Handler for packet with id play:" + std::to_string(id) + " is not registered.");
                default:
                    std::unreachable();
                }
            }
            it(std::move(packet), context);
        }

        void unregister_h(base_objects::events::event_register_id id) {
            if (auto it = hd.find(id.id); it != hd.end()) {
                auto& item = it->second;
                handles[item.mode].erase(item.id);
                hd.erase(it);
            }
        }
    } handle_server_processor_manager;

    
    namespace __internal {
        base_objects::events::event_register_id register_server_processor(uint8_t mode, size_t id, std::function<void(server_bound_packet&&, base_objects::SharedClientData&)>&& fn) {
            return handle_server_processor_manager.register_h(mode, id, std::move(fn));
        }

        void unregister_server_processor(base_objects::events::event_register_id id) {
            return handle_server_processor_manager.unregister_h(id);
        }
    }

    template <class T, class Prev_T>
    void decode_entry(SharedClientData& context, ArrayStream& stream, T& value, Prev_T* prev) {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_move_constructible_v<T>);
        static_assert(std::is_copy_assignable_v<T>);
        static_assert(std::is_move_assignable_v<T>);
        using Type = std::decay_t<T>;
        if constexpr (is_convertible_to_packet_form<Type>) {
            convertible_to_packet_type<Type> res;
            decode_entry(context, stream, res, prev);
            value = Type::from_packet(std::move(res));
        } else if constexpr (std::is_same_v<identifier, Type>)
            value.value = stream.read_identifier();
        else if constexpr (is_std_array<Type>)
            for (auto& it : value)
                decode_entry(context, stream, it, prev);
        else if constexpr (is_string_sized<Type>)
            value.value = stream.read_string(Type::max_size);
        else if constexpr (std::is_same_v<json_text_component, Type>)
            value.value = stream.read_json_component();
        else if constexpr (std::is_same_v<var_int32, Type>)
            value.value = stream.read_var<int32_t>();
        else if constexpr (std::is_same_v<var_int64, Type>)
            value.value = stream.read_var<int64_t>();
        else if constexpr (std::is_same_v<optional_var_int32, Type>) {
            auto res = stream.read_var<int32_t>();
            if (res)
                value = res - 1;
        } else if constexpr (std::is_same_v<optional_var_int64, Type>) {
            auto res = stream.read_var<int64_t>();
            if (res)
                value = res - 1;
        } else if constexpr (std::is_same_v<position, Type>)
            value.set(stream.read_value<decltype(value.get())>());
        else if constexpr (std::is_arithmetic_v<Type>)
            value = stream.read_value<Type>();
        else if constexpr (std::is_same_v<std::string, Type>)
            value = stream.read_string();
        else if constexpr (std::is_same_v<enbt::raw_uuid, Type>)
            value = stream.read_uuid();
        else if constexpr (std::is_same_v<Chat, Type>)
            value = Chat::fromEnbt(ReadNetworkNBT_enbt(stream));
        else if constexpr (
            std::is_same_v<enbt::value, Type>
            || std::is_same_v<enbt::compound, Type>
            || std::is_same_v<enbt::dynamic_array, Type>
            || std::is_same_v<enbt::fixed_array, Type>
            || std::is_same_v<enbt::uuid, Type>
            || std::is_same_v<enbt::simple_array_i8, Type>
            || std::is_same_v<enbt::simple_array_i16, Type>
            || std::is_same_v<enbt::simple_array_i32, Type>
            || std::is_same_v<enbt::simple_array_i64, Type>
            || std::is_same_v<enbt::simple_array_ui8, Type>
            || std::is_same_v<enbt::simple_array_ui16, Type>
            || std::is_same_v<enbt::simple_array_ui32, Type>
            || std::is_same_v<enbt::simple_array_ui64, Type>
        )
            value = ReadNetworkNBT_enbt(stream);
        else if constexpr (std::is_base_of_v<base_objects::pallete_container, Type>) {
            auto bits_per_entry = stream.read_value<uint8_t>();
            static constexpr auto max_indirect
                = std::is_same_v<base_objects::pallete_container_biome, Type>
                      ? base_objects::pallete_container::max_indirect_biomes
                      : base_objects::pallete_container::max_indirect_blocks;
            static constexpr auto entries_count
                = std::is_same_v<base_objects::pallete_container_biome, Type>
                      ? 64
                      : 4096;
            if (bits_per_entry == 0) {
                base_objects::pallete_container_single res;
                stream.read_value<uint8_t>(); //always zero
                res.id_of_palette = stream.read_var<int32_t>();
                value.decompile(std::move(res));
            } else if (bits_per_entry <= max_indirect) {
                base_objects::pallete_container_indirect res(bits_per_entry);
                uint32_t pallete = stream.read_var<uint32_t>();
                res.palette.reserve(pallete);
                for (uint32_t i = 0; i < pallete; i++)
                    res.palette.push_back(stream.read_var<uint32_t>());
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                auto range = stream.range_read(size);
                res.data.bits_per_entry = bits_per_entry;
                res.data.data.data() = list_array<uint8_t>(range.data_read(), range.size_read());
                value.decompile(std::move(res));
            } else {
                base_objects::pallete_data res(bits_per_entry);
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                auto range = stream.range_read(size);
                res.data.data() = list_array<uint8_t>(range.data_read(), range.size_read());
                value.decompile(std::move(res));
            }
        } else if constexpr (std::is_same_v<base_objects::pallete_data_height_map, Type>) {
            value.bits_per_entry = base_objects::pallete_data::bits_for_max(get_size_source_value(context, size_source::get_world_blocks_height));
            auto size = value.bits_per_entry * 256;
            size += size % 8;
            value.data.data() = stream.read_array<uint64_t>(int32_t(size / 8));
        } else if constexpr (is_template_base_of<list_array_depend, Type>) {
            bool has_next = false;
            do {
                typename Type::value_type next;
                decode_entry(context, stream, next, prev);
                has_next = (bool)next.has_next_item;
                value.push_back(std::move(next));
            } while (has_next);
        } else if constexpr (is_template_base_of<_list_array_impl::list_array, Type>) {
            if constexpr (!is_value_template_base_of<no_size, Type> && !std::is_base_of_v<size_from_packet, Type>) {
                value.resize(stream.read_var<int32_t>(), typename Type::value_type{});
            } else if constexpr (is_value_template_base_of<no_size, Type>) {
                value.resize(Type::get_depended_size(context, *prev), typename Type::value_type{});
            } else
                value.resize(stream.size_read() / sizeof(typename Type::value_type), typename Type::value_type{});
            for (auto&& it : value)
                decode_entry(context, stream, it, prev);
        } else if constexpr (is_template_base_of<ignored, Type>) {
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            value = std::nullopt;
            if (stream.read_value<bool>()) {
                value.emplace();
                decode_entry(context, stream, *value, prev);
            }
        } else if constexpr (is_template_base_of<enum_as, Type> || is_template_base_of<enum_as_flag, Type>) {
            typename Type::encode_t val;
            decode_entry(context, stream, val, prev);
            value.value = (std::decay_t<decltype(value.value)>)val;
        } else if constexpr (is_template_base_of<or_, Type>) {
            auto res = stream.read_var<int32_t>();
            if (res)
                value = typename Type::var_0(res - 1);
            else {
                typename Type::var_1 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            }
        } else if constexpr (is_template_base_of<bool_or, Type>) {
            auto res = stream.read_value<bool>();
            if (res) {
                typename Type::var_0 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            } else {
                typename Type::var_1 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            }
        } else if constexpr (is_template_base_of<enum_switch, Type>) {
            typename Type::encode_type id_check;
            decode_entry(context, stream, id_check, prev);
            Type::get_enum(id_check, [&]<class enum_T>() {
                static_assert(std::is_copy_constructible_v<enum_T>);
                static_assert(std::is_move_constructible_v<enum_T>);
                static_assert(std::is_copy_assignable_v<enum_T>);
                static_assert(std::is_move_assignable_v<enum_T>);
                enum_T make_res{};
                decode_entry(context, stream, make_res, prev);
                value = std::move(make_res);
            });
        } else if constexpr (is_template_base_of<partial_enum_switch, Type>) {
            typename Type::encode_type id_check;
            decode_entry(context, stream, id_check, prev);
            Type::get_enum(id_check, [&]<class enum_T>() {
                if constexpr (std::is_same_v<enum_T, typename Type::encode_type>) {
                    value = std::move(id_check);
                } else {
                    enum_T make_res;
                    decode_entry(context, stream, make_res, prev);
                    value = std::move(make_res);
                }
            });
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            value = std::make_shared<typename Type::value_type>();
            decode_entry(context, stream, *value, prev);
        } else if constexpr (is_template_base_of<base_objects::depends_next, Type>) {
            decode_entry(context, stream, value.value, prev);
        } else if constexpr (is_template_base_of<any_of, Type> || is_template_base_of<packet_compress, Type>) {
            decode_entry(context, stream, value.value, prev);
        } else if constexpr (is_template_base_of<flags_list, Type>) {
            decode_entry(context, stream, value.flag, prev);
            Type res;
            value.for_each_set_flag_in_order([&]<class flag_T>() {
                flag_T make_res;
                decode_entry(context, stream, make_res, prev);
                res.set(std::move(make_res));
            });
            value = std::move(res);
        } else if constexpr (is_flags_list_from<Type>) {
            Type res;
            auto& it = (*prev).*Type::preprocess_source_name::value;
            value.for_each_set_flag_in_order(it, [&]<class flag_T>() {
                flag_T make_res{};
                decode_entry(context, stream, make_res, prev);
                res.set(std::move(make_res));
            });
            value = std::move(res);
        } else if constexpr (is_template_base_of<id_set, Type>) {
            var_int32 size = 0;
            decode_entry(context, stream, size, prev);
            if (!size)
                value = (identifier)stream.read_identifier();
            else {
                int32_t arr_size = size - 1;
                list_array<typename Type::id_type> res;
                res.resize(arr_size);
                for (int32_t i = 0; i < arr_size; i++)
                    decode_entry(context, stream, res[i], prev);
                value = std::move(res);
            }
        } else if constexpr (is_template_base_of<value_optional, Type>) {
            decode_entry(context, stream, value.v, prev);
            if (value.v) {
                std::decay_t<decltype(*value.rest)> tmp{};
                decode_entry(context, stream, tmp, prev);
                value.rest = std::move(tmp);
            }
        } else if constexpr (is_template_base_of<sized_entry, Type>) {
            typename Type::size_type size;
            decode_entry(context, stream, size, prev);
            ArrayStream inner = stream.range_read(size);
            decode_entry(context, inner, value.value, prev);
        } else if constexpr (is_limited_num<Type>) {
            decode_entry(context, stream, value.value, prev);
        } else if constexpr (is_bitset_fixed<Type>) {
            bit_list_array<uint8_t> res;
            res.resize(Type::max_size::value);
            size_t r = res.data().size();
            for (size_t i = 0; i < r; i++)
                res.data()[i] = stream.read_value<uint8_t>();
            value.value = std::move(res);
        } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, Type>) {
            value.data() = stream.read_array<uint64_t>();
        } else if constexpr (is_id_source<Type>) {
            decode_entry(context, stream, value.value, prev);
        } else {
            bool process_next = true;
            reflect::for_each_field(value, [&](auto& item) {
                if (process_next) {
                    if constexpr (is_item_depend<T>) {
                        typename T::base_depend tmp = item;
                        decode_entry(context, stream, tmp, &value);
                        value.*T::body_depend::value = {bool(tmp & T::depend_value::value)};
                        tmp = tmp | ~T::depend_value::value;
                    } else
                        decode_entry(context, stream, item, &value);
                    if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                        process_next = (bool)item.value;
                }
            });
        }
    }

    template <class T>
    server_bound_packet decode_server_packet(SharedClientData& context, ArrayStream& stream) {
        T res = T();
        decode_entry(context, stream, res, &res);
        

        if constexpr (std::is_constructible_v<server_bound::handshake_packet, T>) {
            return server_bound_packet(server_bound::handshake_packet(std::move(res)));
        } else if constexpr (std::is_constructible_v<server_bound::status_packet, T>) {
            return server_bound_packet(server_bound::status_packet(std::move(res)));
        } else if constexpr (std::is_constructible_v<server_bound::login_packet, T>) {
            return server_bound_packet(server_bound::login_packet(std::move(res)));
        } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, T>) {
            return server_bound_packet(server_bound::configuration_packet(std::move(res)));
        } else
            return server_bound_packet(server_bound::play_packet(std::move(res)));
    }

    template <class T>
    bool decode_server_packet_handle(SharedClientData& context, ArrayStream& stream) {
        auto packet = decode_server_packet<T>(context, stream);
        uint8_t mode;
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_move_constructible_v<T>);
        static_assert(std::is_copy_assignable_v<T>);
        static_assert(std::is_move_assignable_v<T>);
        if constexpr (std::is_constructible_v<server_bound::handshake_packet, T>) {
            mode = 0;
        } else if constexpr (std::is_constructible_v<server_bound::status_packet, T>) {
            mode = 1;
        } else if constexpr (std::is_constructible_v<server_bound::login_packet, T>) {
            mode = 2;
        } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, T>) {
            mode = 3;
        } else
            mode = 4;
        if (debugging_enabled) {
            auto id = context.get_session() ? context.get_session()->id : -1;
            log::debug("protocol", "server_bound:client_id: " + std::to_string(id) + "\n" + stringize_packet(packet));
        }

        if (__internal::visit_packet_viewer(packet, context))
            return false;
        std::visit(
            [&](auto& mode) {
                return std::visit(
                    [&]<class P>(P& _) {
                        if constexpr (std::is_base_of_v<switches_to::status, P>)
                            context << switches_to::status{};
                        else if constexpr (std::is_base_of_v<switches_to::login, P>)
                            context << switches_to::login{};
                        else if constexpr (std::is_base_of_v<switches_to::configuration, P>)
                            context << switches_to::configuration{};
                        else if constexpr (std::is_base_of_v<switches_to::play, P>)
                            context << switches_to::play{};
                    },
                    mode
                );
            },
            packet
        );

        handle_server_processor_manager.handle(mode, T::packet_id::value, std::move(packet), context);
        return true;
    }

    bool make_process(base_objects::SharedClientData& context, server_bound_packet&& packet) {
        if (__internal::visit_packet_viewer(packet, context))
            return false;

        uint8_t mode;
        size_t id = 0;
        std::visit(
            [&]<class T>(const T& it) {
                if constexpr (std::is_same_v<server_bound::handshake_packet, T>) {
                    mode = 0;
                } else if constexpr (std::is_same_v<server_bound::status_packet, T>) {
                    mode = 1;
                } else if constexpr (std::is_same_v<server_bound::login_packet, T>) {
                    mode = 2;
                } else if constexpr (std::is_same_v<server_bound::configuration_packet, T>) {
                    mode = 3;
                } else
                    mode = 4;
                std::visit(
                    [&]<class U>(const U& _) {
                        id = (size_t)U::packet_id::value;
                        if constexpr (requires { U::switches_to::value; })
                            context << U::switches_to::value;
                    },
                    it
                );
            },
            packet
        );

        handle_server_processor_manager.handle(mode, id, std::move(packet), context);
        return true;
    }

    template <class T>
    client_bound_packet decode_client_packet(SharedClientData& context, ArrayStream& stream) {
        T res;
        decode_entry(context, stream, res, &res);
        if constexpr (std::is_constructible_v<client_bound::status_packet, T>) {
            return client_bound::status_packet(std::move(res));
        } else if constexpr (std::is_constructible_v<client_bound::login_packet, T>) {
            return client_bound::login_packet(std::move(res));
        } else if constexpr (std::is_constructible_v<client_bound::configuration_packet, T>) {
            return client_bound::configuration_packet(std::move(res));
        } else
            return client_bound_packet(client_bound::play_packet(std::move(res)));
    }

    template <class A>
    struct decoders_handle {
        static constexpr auto make() {
        }
    };

    template <class... Args>
    struct decoders_handle<std::variant<Args...>> {
        static constexpr std::array<bool (*)(SharedClientData&, ArrayStream&), sizeof...(Args)> make() {
            std::array<bool (*)(SharedClientData&, ArrayStream&), sizeof...(Args)> flat;

            (
                [&]() {
                    flat.at(Args::packet_id::value) = decode_server_packet_handle<Args>;
                }(),
                ...
            );
            return flat;
        }
    };

    template <class A>
    struct decoders_client {
        static constexpr auto make() {
        }
    };

    template <class... Args>
    struct decoders_client<std::variant<Args...>> {
        static constexpr std::array<client_bound_packet (*)(SharedClientData&, ArrayStream&), sizeof...(Args)> make() {
            std::array<client_bound_packet (*)(SharedClientData&, ArrayStream&), sizeof...(Args)> flat;

            (
                [&]() {
                    if constexpr (is_packet<Args>)
                        flat.at(Args::packet_id::value) = decode_client_packet<Args>;
                }(),
                ...
            );
            return flat;
        }
    };

    template <class A>
    struct decoders_server {
        static constexpr auto make() {
        }
    };

    template <class... Args>
    struct decoders_server<std::variant<Args...>> {
        static constexpr std::array<server_bound_packet (*)(SharedClientData&, ArrayStream&), sizeof...(Args)> make() {
            std::array<server_bound_packet (*)(SharedClientData&, ArrayStream&), sizeof...(Args)> flat;

            (
                [&]() {
                    flat.at(Args::packet_id::value) = decode_server_packet<Args>;
                }(),
                ...
            );
            return flat;
        }
    };

    static const auto server_handle_handshake_dec = decoders_handle<server_bound::handshake_packet::base>::make();
    static const auto server_handle_status_dec = decoders_handle<server_bound::status_packet::base>::make();
    static const auto server_handle_login_dec = decoders_handle<server_bound::login_packet::base>::make();
    static const auto server_handle_configuration_dec = decoders_handle<server_bound::configuration_packet::base>::make();
    static const auto server_handle_play_dec = decoders_handle<server_bound::play_packet::base>::make();

    static const auto client_status_dec = decoders_client<client_bound::status_packet::base>::make();
    static const auto client_login_dec = decoders_client<client_bound::login_packet::base>::make();
    static const auto client_configuration_dec = decoders_client<client_bound::configuration_packet::base>::make();
    static const auto client_play_dec = decoders_client<client_bound::play_packet::base>::make();

    static const auto server_handshake_dec = decoders_server<server_bound::handshake_packet::base>::make();
    static const auto server_status_dec = decoders_server<server_bound::status_packet::base>::make();
    static const auto server_login_dec = decoders_server<server_bound::login_packet::base>::make();
    static const auto server_configuration_dec = decoders_server<server_bound::configuration_packet::base>::make();
    static const auto server_play_dec = decoders_server<server_bound::play_packet::base>::make();

    bool decode(SharedClientData& context, ArrayStream& stream) {
        auto packet_id = stream.read_var<int32_t>();
        switch (context.packets_state.state) {
        case SharedClientData::packets_state_t::protocol_state::handshake:
            return !server_handle_handshake_dec.at(packet_id)(context, stream);
        case SharedClientData::packets_state_t::protocol_state::status:
            return !server_handle_status_dec.at(packet_id)(context, stream);
        case SharedClientData::packets_state_t::protocol_state::login:
            return !server_handle_login_dec.at(packet_id)(context, stream);
        case SharedClientData::packets_state_t::protocol_state::configuration:
            return !server_handle_configuration_dec.at(packet_id)(context, stream);
        case SharedClientData::packets_state_t::protocol_state::play:
            return !server_handle_play_dec.at(packet_id)(context, stream);
        default:
            return false;
        }
    }

    client_bound::status_packet decode_client_status(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::status;
        return std::move(std::get<client_bound::status_packet>(client_status_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    client_bound::login_packet decode_client_login(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::login;
        return std::move(std::get<client_bound::login_packet>(client_login_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    client_bound::configuration_packet decode_client_configuration(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::configuration;
        return std::move(std::get<client_bound::configuration_packet>(client_configuration_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    client_bound::play_packet decode_client_play(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::play;
        return std::move(std::get<client_bound::play_packet>(client_play_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    server_bound::handshake_packet decode_server_handshake(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::handshake;
        return std::move(std::get<server_bound::handshake_packet>(server_handshake_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    server_bound::status_packet decode_server_status(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::status;
        return std::move(std::get<server_bound::status_packet>(server_status_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    server_bound::login_packet decode_server_login(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::login;
        return std::move(std::get<server_bound::login_packet>(server_login_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    server_bound::configuration_packet decode_server_configuration(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::configuration;
        return std::move(std::get<server_bound::configuration_packet>(server_configuration_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    server_bound::play_packet decode_server_play(ArrayStream& stream) {
        SharedClientData client;
        client.packets_state.state = SharedClientData::packets_state_t::protocol_state::play;
        return std::move(std::get<server_bound::play_packet>(server_play_dec.at(stream.read_var<int32_t>())(client, stream)));
    }

    client_bound::status_packet decode_client_status(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::status)
            throw std::invalid_argument("The client is not in status protocol stage");
        return std::move(std::get<client_bound::status_packet>(client_login_dec.at(stream.read_var<int32_t>())(context, stream)));
    }

    client_bound::login_packet decode_client_login(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::login)
            throw std::invalid_argument("The client is not in login protocol stage");
        return std::move(std::get<client_bound::login_packet>(client_login_dec.at(stream.read_var<int32_t>())(context, stream)));
    }

    client_bound::configuration_packet decode_client_configuration(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::configuration)
            throw std::invalid_argument("The client is not in configuration protocol stage");
        return std::move(std::get<client_bound::configuration_packet>(client_configuration_dec.at(stream.read_var<int32_t>())(context, stream)));
    }

    client_bound::play_packet decode_client_play(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::play)
            throw std::invalid_argument("The client is not in play protocol stage");
        return std::move(std::get<client_bound::play_packet>(client_play_dec.at(stream.read_var<int32_t>())(context, stream)));
    }

    server_bound::handshake_packet decode_server_handshake(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::handshake)
            throw std::invalid_argument("The client is not in handshake protocol stage");
        return std::move(std::get<server_bound::handshake_packet>(server_handshake_dec.at(stream.read_var<int32_t>())(context, stream)));
    }

    server_bound::status_packet decode_server_status(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::status)
            throw std::invalid_argument("The client is not in status protocol stage");
        return std::move(std::get<server_bound::status_packet>(server_status_dec.at(stream.read_var<int32_t>())(context, stream)));
    }

    server_bound::login_packet decode_server_login(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::login)
            throw std::invalid_argument("The client is not in login protocol stage");
        return std::move(std::get<server_bound::login_packet>(server_login_dec.at(stream.read_var<int32_t>())(context, stream)));
    }

    server_bound::configuration_packet decode_server_configuration(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::configuration)
            throw std::invalid_argument("The client is not in configuration protocol stage");
        return std::move(std::get<server_bound::configuration_packet>(server_configuration_dec.at(stream.read_var<int32_t>())(context, stream)));
    }

    server_bound::play_packet decode_server_play(base_objects::SharedClientData& context, ArrayStream& stream) {
        if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::play)
            throw std::invalid_argument("The client is not in play protocol stage");
        return std::move(std::get<server_bound::play_packet>(server_play_dec.at(stream.read_var<int32_t>())(context, stream)));
    }
}