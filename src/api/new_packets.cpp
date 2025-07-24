#include <src/api/new_packets.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/util/readers.hpp>


#define REFLECT_CPP_C_ARRAYS_OR_INHERITANCE
#include <rfl/to_view.hpp>

namespace copper_server {
    namespace api::new_packets {
        using namespace base_objects;

        namespace reflect {
            template <class T, class F>
            constexpr void for_each_field(T&& obj, F&& func) {
                rfl::to_view(obj).apply(
                    [f = std::forward<F>(func)](auto&&... elements) mutable {
                        f((*elements.value())...);
                    }
                );
            }
        }

        template <class type>
        concept is_packet = requires(type& d) {
            type::packet_id::value;
        };


        template <template <class...> class Base, class... Ts>
        void test(Base<Ts...>&);

        template <template <auto...> class Base, auto... Ts>
        void value_test(Base<Ts...>&);

        template <template <class, auto...> class Base, class T, auto... Ts>
        void tvalue_test(Base<T, Ts...>&);


        template <template <class...> class, class, class = void>
        constexpr bool is_template_base_of = false;

        template <template <class...> class Base, class Derived>
        constexpr bool is_template_base_of<Base, Derived, std::void_t<decltype(test<Base>(std::declval<Derived&>()))>> = true;

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
        concept is_vector_sized = is_tvalue_template_base_of<vector_sized, type> || is_tvalue_template_base_of<vector_sized_siz_from_packet, type> || is_tvalue_template_base_of<vector_sized_no_size, type>;

        template <class type>
        concept is_std_array = is_tvalue_template_base_of<std::array, type>;

        template <class type>
        concept is_limited_num = is_tvalue_template_base_of<limited_num, type>;

        template <class type>
        concept requires_check = is_limited_num<type> || is_string_sized<type> || is_vector_sized<type>;


        template <class type>
        concept need_preprocess = requires_check<type> || std::is_base_of_v<packet_preprocess, type>;

        struct processor_handle_data {
            uint8_t mode;
            size_t id;
        };

        struct processors_manager {
            uint64_t id_counter = 0;
            std::unordered_map<uint64_t, processor_handle_data> hd;
            std::unordered_map<size_t, std::function<void(server_bound_packet&)>> handles[3];

            base_objects::events::event_register_id register_h(uint8_t mode, size_t id, std::function<void(server_bound_packet&)>&& fn) {
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

            void handle(uint8_t mode, size_t id, server_bound_packet& packet) {
                auto& it = handles[mode][id];
                if (!it) {
                    switch (mode) {
                    case 0:
                        throw std::runtime_error("Handler for packet with id login:" + std::to_string(id) + " is not registered.");
                    case 1:
                        throw std::runtime_error("Handler for packet with id configuration:" + std::to_string(id) + " is not registered.");
                    case 2:
                        throw std::runtime_error("Handler for packet with id play:" + std::to_string(id) + " is not registered.");
                    default:
                        std::unreachable();
                    }
                }
                it(packet);
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
            std::unordered_map<size_t, events::sync_event<client_bound_packet&>> client_viewers[3];
            std::unordered_map<size_t, events::sync_event<server_bound_packet&>> server_viewers[3];

            base_objects::events::event_register_id register_client_viewer(uint8_t mode, size_t id, events::sync_event<client_bound_packet&>::function&& fn) {
                return client_viewers[mode][id].join(std::move(fn));
            }

            base_objects::events::event_register_id register_server_viewer(uint8_t mode, size_t id, events::sync_event<server_bound_packet&>::function&& fn) {
                return server_viewers[mode][id].join(std::move(fn));
            }

            void unregister_client_viewer(uint8_t mode, size_t id, base_objects::events::event_register_id reg_id) {
                client_viewers[mode][id].leave(reg_id);
            }

            void unregister_server_viewer(uint8_t mode, size_t id, base_objects::events::event_register_id reg_id) {
                server_viewers[mode][id].leave(reg_id);
            }

            base_objects::events::event_register_id register_server_processor(uint8_t mode, size_t id, std::function<void(server_bound_packet&)>&& fn) {
                return handle_server_processor_manager.register_h(mode, id, std::move(fn));
            }

            void unregister_server_processor(base_objects::events::event_register_id id) {
                return handle_server_processor_manager.unregister_h(id);
            }

            bool visit_packet_viewer(client_bound_packet& packet) {
                return std::visit(
                    [&](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        return std::visit(
                            [&](auto& pack) {
                                using pack_T = std::decay_t<decltype(pack)>;
                                if constexpr (is_packet<pack_T>) {
                                    if constexpr (std::is_same_v<T, client_bound::login_packet>) {
                                        return client_viewers[0][pack_T::packet_id::value].notify(packet);
                                    } else if constexpr (std::is_same_v<T, client_bound::configuration_packet>) {
                                        return client_viewers[1][pack_T::packet_id::value].notify(packet);
                                    } else
                                        return client_viewers[2][pack_T::packet_id::value].notify(packet);
                                }
                                else
                                    return false;
                            },
                            it
                        );
                    },
                    packet
                );
            }

            bool visit_packet_viewer(server_bound_packet& packet) {
                return std::visit(
                    [&](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        return std::visit(
                            [&](auto& pack) {
                                using pack_T = std::decay_t<decltype(pack)>;
                                if constexpr (is_packet<pack_T>) {
                                    if constexpr (std::is_same_v<T, server_bound::login_packet>) {
                                        return server_viewers[0][pack_T::packet_id::value].notify(packet);
                                    } else if constexpr (std::is_same_v<T, server_bound::configuration_packet>) {
                                        return server_viewers[1][pack_T::packet_id::value].notify(packet);
                                    } else
                                        return server_viewers[2][pack_T::packet_id::value].notify(packet);
                                } else
                                    return false;
                            },
                            it
                        );
                    },
                    packet
                );
            }
        }

        template <class T>
        void serialize_entry(base_objects::network::response::item& res, SharedClientData& context, const T& value) {
            using Type = std::decay_t<T>;
            if constexpr (std::is_same_v<identifier, Type>)
                res.write_identifier(value.value);
            else if constexpr (is_std_array<Type>)
                for (auto& it : value)
                    serialize_entry(res, context, it);
            else if constexpr (is_string_sized<Type>)
                res.write_string(value.value, Type::max_size);
            else if constexpr (std::is_same_v<json_text_component, Type>)
                res.write_json_component(value.value);
            else if constexpr (std::is_same_v<var_int32, Type>)
                res.write_var32(value.value);
            else if constexpr (std::is_same_v<var_int64, Type>)
                res.write_var64(value.value);
            else if constexpr (std::is_same_v<optional_var_int32, Type>) {
                if (value)
                    res.write_var32_check(int64_t(*value) + 1);
                else
                    res.write_var32(0);
            } else if constexpr (std::is_same_v<optional_var_int64, Type>) {
                if (value)
                    res.write_var64(*value + 1);
                else
                    res.write_var64(0);
            } else if constexpr (std::is_same_v<Angle, Type>)
                res.write_value(value.value);
            else if constexpr (std::is_same_v<position, Type>)
                res.write_value(value.get());
            else if constexpr (std::is_arithmetic_v<Type>)
                res.write_value(value);
            else if constexpr (std::is_same_v<std::string, Type>)
                res.write_string(value);
            else if constexpr (std::is_same_v<enbt::raw_uuid, Type>)
                res.write_value(value);
            else if constexpr (std::is_same_v<Chat, Type>)
                res.write_direct(util::NBT::build(value.ToENBT()).get_as_network());
            else if constexpr (std::is_same_v<enbt::value, Type>)
                res.write_direct(util::NBT::build(value).get_as_network());
            else if constexpr (std::is_same_v<base_objects::slot, Type>)
                ; //TODO
            else if constexpr (is_template_base_of<std::vector, Type>) {
                if constexpr (!std::is_base_of_v<no_size, Type> && !std::is_base_of_v<size_from_packet, Type>)
                    res.write_var32_check(value.size());
                for (auto& it : value)
                    serialize_entry(res, context, it);
            } else if constexpr (is_template_base_of<std::optional, Type>) {
                res.write_value(bool(value));
                if (value)
                    serialize_entry(res, context, *value);
            } else if constexpr (is_template_base_of<enum_as, Type> || is_template_base_of<enum_as_flag, Type>) {
                serialize_entry(res, context, value.get());
            } else if constexpr (is_template_base_of<or_, Type>) {
                std::visit(
                    [&](auto& it) {
                        if constexpr (std::is_same_v<typename Type::var_0, std::decay_t<decltype(it)>>) {
                            if constexpr (is_template_base_of<enum_as, typename Type::var_0>)
                                res.write_var32_check(int64_t(it.value) + 1);
                            else
                                res.write_var32_check(int64_t(it) + 1);
                        } else {
                            res.write_var32(0);
                            serialize_entry(res, context, it);
                        }
                    },
                    value
                );
            } else if constexpr (is_template_base_of<enum_switch, Type>) {
                std::visit(
                    [&](auto& it) {
                        using it_T = std::decay_t<decltype(it)>;
                        serialize_entry(res, context, Type::encode_type(it_T::item_id::value));
                        serialize_entry(res, context, it);
                    },
                    value
                );
            } else if constexpr (is_template_base_of<any_of, Type>) {
                serialize_entry(res, context, value.value);
            } else if constexpr (is_template_base_of<flags_list, Type>) {
                serialize_entry(res, context, value.flag);
                value.for_each_in_order([&](auto& it) {
                    serialize_entry(res, context, it);
                });
            } else if constexpr (is_flags_list_from<Type>) {
                value.for_each_in_order([&](auto& it) {
                    serialize_entry(res, context, it);
                });
            } else if constexpr (is_template_base_of<unordered_id, Type>) {
                serialize_entry(res, context, value.value); //TODO
            } else if constexpr (is_template_base_of<ordered_id, Type>) {
                serialize_entry(res, context, value.value); //TODO
            } else if constexpr (is_template_base_of<value_optional, Type>) {
                if (value.rest && value.v) {
                    serialize_entry(res, context, value.v);
                    serialize_entry(res, context, *value.rest);
                } else {
                    decltype(value.v) tmp{0};
                    serialize_entry(res, context, tmp);
                }
            } else if constexpr (is_vector_sized<Type>) {
                if constexpr (!std::is_base_of_v<no_size, Type> && !std::is_base_of_v<size_from_packet, Type>)
                    res.write_var32_check(value.value.size());
                for (auto& it : value.value)
                    serialize_entry(res, context, it);
            } else if constexpr (is_limited_num<Type>) {
                serialize_entry(res, context, value.value);
            } else if constexpr (is_bitset_fixed<Type>) {
                res.write_direct(value.data());
            } else
                reflect::for_each_field(value, [&res, &context](auto& item) {
                    serialize_entry(res, context, item);
                });
        }

        template <class T>
        void preprocess_structure(T& value) {
            if constexpr (is_template_base_of<std::vector, T> || is_std_array<T>) {
                if constexpr (need_preprocess<typename T::value_type>)
                    for (auto& it : value)
                        preprocess_structure(it);
            } else if constexpr (is_string_sized<T>) {
                if (value.value.size() > T::max_size)
                    throw std::overflow_error("The string size is over the limit.");
            } else if constexpr (is_vector_sized<T>) {
                if (value.value.size() > T::max_size)
                    throw std::overflow_error("The vector size is over the limit.");
                if constexpr (need_preprocess<typename T::value_type>) //TOTO check compilation error
                    for (auto& it : value.value)
                        preprocess_structure(it);
            } else if constexpr (is_template_base_of<std::optional, T>) {
                if constexpr (need_preprocess<typename T::value_type>)
                    if (value)
                        preprocess_structure(value);
            } else if constexpr (is_limited_num<T>) {
                if (value.value > T::check_max)
                    throw std::overflow_error("The value is too big");
                if (value.value < T::check_min)
                    throw std::underflow_error("The value is too low");
            } else if constexpr (is_template_base_of<value_optional, T>) {
                if constexpr (need_preprocess<typename std::decay_t<decltype(*value.rest)>::value_type>)
                    if (value)
                        preprocess_structure(*value.rest);
            } else if constexpr (is_flags_list_from<T> || is_template_base_of<flags_list, T>) {
                value.for_each([&](auto& it) {
                    if constexpr (need_preprocess<typename std::decay_t<decltype(it)>::value_type>)
                        preprocess_structure(it);
                });
            } else if constexpr (is_template_base_of<enum_switch,T>) {
                std::visit(
                    [&](auto& it) {
                        if constexpr (need_preprocess<typename std::decay_t<decltype(it)>::value_type>)
                            preprocess_structure(it);
                    },
                    value
                );
            } else {
                reflect::for_each_field(value, [&value](auto& item) {
                    using I = std::decay_t<decltype(item)>;
                    if constexpr (need_preprocess<I>)
                        preprocess_structure(item);
                    if constexpr (could_be_preprocessed<I, T>)
                        item.preprocess(value);
                });
            }
        }

        template <class T>
        void serialize_packet(base_objects::network::response& res, base_objects::SharedClientData& context, T& value) {
            using Type = std::decay_t<T>;
            if constexpr (std::is_base_of_v<packet_preprocess, Type>)
                preprocess_structure(value);
                
            if constexpr (is_packet<Type>) {
                base_objects::network::response::item it;
                it.write_id(Type::packet_id::value);
                serialize_entry(it, context, value);
                res += it;
            } else if constexpr (std::is_base_of_v<compound_packet, Type>) {
                reflect::for_each_field(value, [&res, &value, &context](auto& item) {
                    using I = std::decay_t<decltype(item)>;
                    if constexpr (
                        std::is_same_v<I, client_bound::login_packet>
                        || std::is_same_v<I, client_bound::configuration_packet>
                        || std::is_same_v<I, client_bound::play_packet>
                    )
                        std::visit([&](auto& it) { serialize_packet(res, context, it); }, item);
                    else
                        serialize_packet(res, context, item);
                });
            }
        }

        bool send(SharedClientData& client, client_bound_packet&& packet) {
            if (__internal::visit_packet_viewer(packet))
                return false;

            client.sendPacket(
                std::visit(
                    [&client](auto& mode) -> base_objects::network::response {
                        return std::visit(
                            [&client](auto& it) -> base_objects::network::response {
                                base_objects::network::response res;
                                serialize_packet(res, client, it);
                                return res;
                            },
                            mode
                        );
                    },
                    packet
                )
            );
            return true;
        }

        base_objects::network::response encode(client_bound_packet&& packet) {
            return std::visit(
                [](auto& mode) -> base_objects::network::response {
                    return std::visit(
                        [](auto& it) -> base_objects::network::response {
                            SharedClientData client;
                            base_objects::network::response res;
                            serialize_packet(res, client, it);
                            return res;
                        },
                        mode
                    );
                },
                packet
            );
        }

        template <class T, class Prev_T>
        void decode_entry(SharedClientData& context, ArrayStream& stream, T& value, Prev_T* prev) {
            using Type = std::decay_t<T>;
            if constexpr (std::is_same_v<identifier, Type>)
                value.value = stream.read_identifier();
            else if constexpr (is_std_array<Type>)
                for (auto& it : value)
                    decode_entry(context, stream, it, &value);
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
            } else if constexpr (std::is_same_v<Angle, Type>)
                value.value = stream.read_value<decltype(value.value)>();
            else if constexpr (std::is_same_v<position, Type>)
                value.set(stream.read_value<decltype(value.get())>());
            else if constexpr (std::is_arithmetic_v<Type>)
                value = stream.read_value<Type>();
            else if constexpr (std::is_same_v<std::string, Type>)
                value = stream.read_string();
            else if constexpr (std::is_same_v<enbt::raw_uuid, Type>)
                value = stream.read_uuid();
            else if constexpr (std::is_same_v<Chat, Type>)
                value = Chat::fromEnbt(ReadNetworkNBT_enbt(stream));
            else if constexpr (std::is_same_v<base_objects::slot, Type>)
                ; //TODO
            else if constexpr (std::is_same_v<enbt::value, Type>)
                value = ReadNetworkNBT_enbt(stream);
            else if constexpr (is_template_base_of<std::vector, Type>) {
                if constexpr (!std::is_base_of_v<no_size, Type> && !std::is_base_of_v<size_from_packet, Type>) {
                    auto size = stream.read_var<int32_t>();
                    value.resize(size);
                } else {
                    auto size = stream.size_read() / sizeof(typename Type::value_type);
                    value.resize(size);
                }
                for (auto& it : value)
                    decode_entry(context, stream, it, &value);
            } else if constexpr (is_template_base_of<std::optional, Type>) {
                value = std::nullopt;
                if (stream.read_value<bool>()) {
                    value = typename Type::value_type{};
                    decode_entry(context, stream, *value, &value);
                }
            } else if constexpr (is_template_base_of<enum_as, Type> || is_template_base_of<enum_as_flag, Type>) {
                typename Type::encode_t val;
                decode_entry(context, stream, val, &value);
                value.value = (std::decay_t<decltype(value.value)>)val;
            } else if constexpr (is_template_base_of<or_, Type>) {
                auto res = stream.read_var<int32_t>();
                if (res)
                    value = Type::var_0(res - 1);
                else {
                    typename Type::var_1 in_res;
                    decode_entry(context, stream, in_res, &value);
                    value = std::move(in_res);
                }
            } else if constexpr (is_template_base_of<enum_switch, Type>) {
                typename Type::encode_type id_check;
                decode_entry(context, stream, id_check, &value);
                Type::get_enum(id_check, [&]<class enum_T>() {
                    enum_T make_res;
                    decode_entry(context, stream, make_res, &value);
                    value = std::move(make_res);
                });
            } else if constexpr (is_template_base_of<any_of, Type>) {
                decode_entry(context, stream, value.value, &value);
            } else if constexpr (is_template_base_of<flags_list, Type>) {
                decode_entry(context, stream, value.flag, &value);
                Type res;
                value.for_each_set_flag_in_order([&]<class flag_T>() {
                    flag_T make_res;
                    decode_entry(context, stream, make_res, &value);
                    res.set(std::move(make_res));
                });
                value = std::move(res);
            } else if constexpr (is_flags_list_from<Type>) {
                Type res;
                auto& it = (*prev).*Type::preprocess_source_name::value;
                value.for_each_set_flag_in_order(it, [&]<class flag_T>() {
                    flag_T make_res;
                    decode_entry(context, stream, make_res, &value);
                    res.set(std::move(make_res));
                });
                value = std::move(res);
            } else if constexpr (is_template_base_of<unordered_id, Type>) {
                decode_entry(context, stream, value.value, &value); //TODO
            } else if constexpr (is_template_base_of<ordered_id, Type>) {
                decode_entry(context, stream, value.value, &value); //TODO
            } else if constexpr (is_template_base_of<value_optional, Type>) {
                decode_entry(context, stream, value.v, &value);
                if (value.v) {
                    std::decay_t<decltype(*value.rest)> tmp{};
                    decode_entry(context, stream, tmp, &value);
                    value.rest = std::move(tmp);
                }
            } else if constexpr (is_vector_sized<Type>) {
                if constexpr (!std::is_base_of_v<no_size, Type> && !std::is_base_of_v<size_from_packet, Type>) {
                    auto size = stream.read_var<int32_t>();
                    value.value.resize(size);
                } else {
                    auto size = stream.size_read() / sizeof(typename decltype(value.value)::value_type);
                    value.value.resize(size);
                }
                for (auto& it : value.value)
                    decode_entry(context, stream, it, &value);
            } else if constexpr (is_limited_num<Type>) {
                decode_entry(context, stream, value.value, &value);
            } else if constexpr (is_bitset_fixed<Type>) {
                bit_list_array<uint8_t> res;
                res.resize(Type::max_size);
                size_t r = res.data().size();
                for (size_t i = 0; i < r; i++)
                    res.data()[i] = stream.read_value<uint8_t>();
                value.value = std::move(res);
            } else
                reflect::for_each_field(value, [&](auto& item) {
                    decode_entry(context, stream, item, &value);
                });
        }

        template <class T>
        server_bound_packet decode_server_packet(SharedClientData& context, ArrayStream& stream) {
            T res;
            decode_entry(context, stream, res, &res);
            if constexpr (std::is_constructible_v<server_bound::login_packet, T>) {
                return server_bound::login_packet(std::move(res));
            } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, T>) {
                return server_bound::configuration_packet(std::move(res));
            } else
                return server_bound::play_packet(std::move(res));
        }

        template <class T>
        bool decode_server_packet_handle(SharedClientData& context, ArrayStream& stream) {
            auto packet = decode_server_packet<T>(context, stream);
            uint8_t mode;
            if constexpr (std::is_constructible_v<server_bound::login_packet, T>) {
                mode = 0;
            } else if constexpr (std::is_constructible_v<server_bound::configuration_packet, T>) {
                mode = 1;
            } else
                mode = 2;

            if (__internal::visit_packet_viewer(packet))
                return false;

            handle_server_processor_manager.handle(mode, T::packet_id::value, packet);
            return true;
        }

        template <class T>
        client_bound_packet decode_client_packet(SharedClientData& context, ArrayStream& stream) {
            T res;
            decode_entry(context, stream, res, &res);
            if constexpr (std::is_constructible_v<client_bound::login_packet, T>) {
                return client_bound::login_packet(std::move(res));
            } else if constexpr (std::is_constructible_v<client_bound::configuration_packet, T>) {
                return client_bound::configuration_packet(std::move(res));
            } else
                return client_bound::play_packet(std::move(res));
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

        static const auto server_handle_login_dec = decoders_handle<server_bound::login_packet::base>::make();
        static const auto server_handle_configuration_dec = decoders_handle<server_bound::configuration_packet::base>::make();
        static const auto server_handle_play_dec = decoders_handle<server_bound::play_packet::base>::make();

        static const auto client_login_dec = decoders_client<client_bound::login_packet::base>::make();
        static const auto client_configuration_dec = decoders_client<client_bound::configuration_packet::base>::make();
        static const auto client_play_dec = decoders_client<client_bound::play_packet::base>::make();

        static const auto server_login_dec = decoders_server<server_bound::login_packet::base>::make();
        static const auto server_configuration_dec = decoders_server<server_bound::configuration_packet::base>::make();
        static const auto server_play_dec = decoders_server<server_bound::play_packet::base>::make();

        bool decode(SharedClientData& context, ArrayStream& stream) {
            auto packet_id = stream.read_var<int32_t>();
            switch (context.packets_state.state) {
            case SharedClientData::packets_state_t::protocol_state::initialization:
                return !server_handle_login_dec.at(packet_id)(context, stream);
            case SharedClientData::packets_state_t::protocol_state::configuration:
                return !server_handle_configuration_dec.at(packet_id)(context, stream);
            case SharedClientData::packets_state_t::protocol_state::play:
                return !server_handle_play_dec.at(packet_id)(context, stream);
            default:
                return false;
            }
        }

        client_bound::login_packet decode_client_login(ArrayStream& stream) {
            SharedClientData client;
            client.packets_state.state = SharedClientData::packets_state_t::protocol_state::initialization;
            return std::get<client_bound::login_packet>(client_login_dec.at(stream.read_var<int32_t>())(client, stream));
        }

        client_bound::configuration_packet decode_client_configuration(ArrayStream& stream) {
            SharedClientData client;
            client.packets_state.state = SharedClientData::packets_state_t::protocol_state::configuration;
            return std::get<client_bound::configuration_packet>(client_configuration_dec.at(stream.read_var<int32_t>())(client, stream));
        }

        client_bound::play_packet decode_client_play(ArrayStream& stream) {
            SharedClientData client;
            client.packets_state.state = SharedClientData::packets_state_t::protocol_state::play;
            return std::get<client_bound::play_packet>(client_play_dec.at(stream.read_var<int32_t>())(client, stream));
        }

        server_bound::login_packet decode_server_login(ArrayStream& stream) {
            SharedClientData client;
            client.packets_state.state = SharedClientData::packets_state_t::protocol_state::initialization;
            return std::get<server_bound::login_packet>(server_login_dec.at(stream.read_var<int32_t>())(client, stream));
        }

        server_bound::configuration_packet decode_server_configuration(ArrayStream& stream) {
            SharedClientData client;
            client.packets_state.state = SharedClientData::packets_state_t::protocol_state::configuration;
            return std::get<server_bound::configuration_packet>(server_configuration_dec.at(stream.read_var<int32_t>())(client, stream));
        }

        server_bound::play_packet decode_server_play(ArrayStream& stream) {
            SharedClientData client;
            client.packets_state.state = SharedClientData::packets_state_t::protocol_state::play;
            return std::get<server_bound::play_packet>(server_play_dec.at(stream.read_var<int32_t>())(client, stream));
        }

        client_bound::login_packet decode_client_login(base_objects::SharedClientData& context, ArrayStream&stream){
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::initialization)
                throw std::invalid_argument("The client is not in login protocol stage");
            return std::get<client_bound::login_packet>(client_login_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        client_bound::configuration_packet decode_client_configuration(base_objects::SharedClientData& context, ArrayStream&stream){
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::configuration)
                throw std::invalid_argument("The client is not in configuration protocol stage");
            return std::get<client_bound::configuration_packet>(client_configuration_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        client_bound::play_packet decode_client_play(base_objects::SharedClientData& context, ArrayStream& stream) {
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::play)
                throw std::invalid_argument("The client is not in play protocol stage");
            return std::get<client_bound::play_packet>(client_play_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        server_bound::login_packet decode_server_login(base_objects::SharedClientData& context, ArrayStream& stream){
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::initialization)
                throw std::invalid_argument("The client is not in login protocol stage");
            return std::get<server_bound::login_packet>(server_login_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        server_bound::configuration_packet decode_server_configuration(base_objects::SharedClientData& context, ArrayStream& stream){
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::configuration)
                throw std::invalid_argument("The client is not in configuration protocol stage");
            return std::get<server_bound::configuration_packet>(server_configuration_dec.at(stream.read_var<int32_t>())(context, stream));
        }
        server_bound::play_packet decode_server_play(base_objects::SharedClientData& context, ArrayStream& stream){
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::play)
                throw std::invalid_argument("The client is not in play protocol stage");
            return std::get<server_bound::play_packet>(server_play_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        std::string stringize_packet(const client_bound_packet&);
        std::string stringize_packet(const server_bound_packet&);
    }
}