/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_OPS
#define SRC_API_PACKETS_OPS
#include <functional>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/network/response.hpp>
#include <variant>

namespace copper_server {
    struct ArrayStream;

    namespace base_objects {
        struct shared_client_data;
    }
}

namespace copper_server::api::packets {
    namespace __internal {
        enum class current_state {
            handshake,
            status,
            login,
            configuration,
            play
        };

        current_state get_state(base_objects::shared_client_data&);
        size_t get_packet_id(ArrayStream&);

        template <class Ret, class Arg0, class... Rest>
        Arg0 first_argument_helper(Ret (*)(Arg0, Rest...));

        template <class Ret, class Fn, class Arg0, class... Rest>
        Arg0 first_argument_helper(Ret (Fn::*)(Arg0, Rest...));

        template <class Ret, class Fn, class Arg0, class... Rest>
        Arg0 first_argument_helper(Ret (Fn::*)(Arg0, Rest...) const);

        template <class Fn>
        decltype(first_argument_helper(&Fn::operator())) first_argument_helper(Fn);

        template <class T>
        using first_argument_type = std::decay_t<decltype(first_argument_helper(std::declval<T>()))>;
    }

    template <class packet>
    struct packet_ops {
        using packet_type = packet;
        static base_objects::events::sync_event<packet&, base_objects::shared_client_data&>& send_viewer();
        static base_objects::events::sync_event_no_cancel<packet&, base_objects::shared_client_data&>& post_send_viewer();
        static base_objects::events::sync_event<packet&, base_objects::shared_client_data&>& receive_viewer();
        static base_objects::events::sync_event_single<packet&&, base_objects::shared_client_data&>& processor();


        static bool send(base_objects::shared_client_data& client, packet&&);
        static base_objects::network::response client_encode(base_objects::shared_client_data& context, packet&& packet);
        static base_objects::network::response encode(packet&& packet);

        static bool make_process(base_objects::shared_client_data& client, packet&&);

        static packet decode(ArrayStream&);
        static packet client_decode(base_objects::shared_client_data& context, ArrayStream&);
        static std::string stringize(const packet&);

        template <class plugin>
        static void send_viewer(plugin& self, auto&& fn) {
            self.register_event(send_viewer(), std::move(fn));
        }

        template <class plugin>
        static void post_send_viewer(plugin& self, auto&& fn) {
            self.register_event(post_send_viewer(), std::move(fn));
        }

        template <class plugin>
        static void receive_viewer(plugin& self, auto&& fn) {
            self.register_event(receive_viewer(), std::move(fn));
        }

        template <class plugin>
        static void processor(plugin& self, auto&& fn) {
            self.register_event(processor(), std::move(fn));
        }
    };

    template <class... packets>
    struct state_ops {
        using packet_variants = std::variant<packets...>;
        using packet_ref_variants = std::variant<std::reference_wrapper<packets>...>;

        static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&>& send_viewer() {
            static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    packet_ops<packets>::send_viewer().join([](auto& packet, base_objects::shared_client_data& client) {
                        return result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static base_objects::events::sync_event_no_cancel<packet_ref_variants, base_objects::shared_client_data&>& post_send_viewer() {
            static base_objects::events::sync_event_no_cancel<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    packet_ops<packets>::post_send_viewer().join([](auto& packet, base_objects::shared_client_data& client) {
                        result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            (void)once_init;
            return result;
        }

        static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&>& receive_viewer() {
            static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    packet_ops<packets>::receive_viewer().join([](auto& packet, base_objects::shared_client_data& client) {
                        return result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static base_objects::events::sync_event_single<packet_variants, base_objects::shared_client_data&>& processor() {
            static base_objects::events::sync_event<packet_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    packet_ops<packets>::processor().join([](auto& packet, base_objects::shared_client_data& client) {
                        result.notify(std::move(packet), client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static packet_variants client_decode(base_objects::shared_client_data& context, ArrayStream& stream) {
            packet_variants result;
            client_decode(context, stream, [&result](auto& client, auto&& packet) {
                result = std::move(packet);
            });
            return result;
        }

        template <class FN>
        static decltype(auto) client_decode(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) {
            using fn_t = void (*)(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn);
            static constexpr fn_t selector[]{
                ([](base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) -> decltype(auto) {
                    return fn(context, packet_ops<packets>::client_decode(context, stream));
                })...
            };
            return selector[__internal::get_packet_id(stream)](context, stream, std::forward<FN>(fn));
        }

        static packet_variants decode(ArrayStream& stream) {
            packet_variants result;
            decode(stream, [&result](auto&& packet) {
                result = std::move(packet);
            });
            return result;
        }

        template <class FN>
        static decltype(auto) decode(ArrayStream& stream, FN&& fn) {
            using fn_t = void (*)(ArrayStream& stream, FN&& fn);
            static constexpr fn_t selector[]{
                ([](ArrayStream& stream, FN&& fn) -> decltype(auto) {
                    return fn(packet_ops<packets>::decode(stream));
                })...
            };
            return selector[__internal::get_packet_id(stream)](stream, std::forward<FN>(fn));
        }

        template <class plugin>
        static void send_viewer(plugin& self, auto&& fn) {
            self.register_event(send_viewer(), [_fn = std::move(fn)](packet_ref_variants state, base_objects::shared_client_data& client) {
                return std::visit(
                    [&_fn, &client](auto& packet) {
                        return _fn(packet.get(), client);
                    },
                    state
                );
            });
        }

        template <class plugin>
        static void post_send_viewer(plugin& self, auto&& fn) {
            self.register_event(post_send_viewer(), [_fn = std::move(fn)](packet_ref_variants state, base_objects::shared_client_data& client) {
                std::visit(
                    [&_fn, &client](auto& packet) {
                        _fn(packet.get(), client);
                    },
                    state
                );
            });
        }

        template <class plugin>
        static void receive_viewer(plugin& self, auto&& fn) {
            self.register_event(receive_viewer(), [_fn = std::move(fn)](packet_ref_variants state, base_objects::shared_client_data& client) {
                return std::visit(
                    [&_fn, &client](auto& packet) {
                        return _fn(packet.get(), client);
                    },
                    state
                );
            });
        }

        template <class plugin>
        static void processor(plugin& self, auto&& fn) {
            self.register_event(processor(), [_fn = std::move(fn)](packet_variants state, base_objects::shared_client_data& client) {
                std::visit(
                    [&_fn, &client](auto& packet) {
                        _fn(std::move(packet), client);
                    },
                    state
                );
            });
        }
    
        template<class packet>
        static constexpr bool contains() {
            return std::disjunction_v<std::is_same<packet, typename packets::packet_type>...>;
        }
    };

    //must be declared by order handshake, status, login, configuration, play
    template <class... states>
    struct direction_ops {
        using packet_variants = std::variant<typename states::packet_variants...>;
        using packet_ref_variants = std::variant<typename states::packet_ref_variants...>;

        static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&>& send_viewer() {
            static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    states::send_viewer().join([](auto&& packet, base_objects::shared_client_data& client) {
                        return result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static base_objects::events::sync_event_no_cancel<packet_ref_variants, base_objects::shared_client_data&>& post_send_viewer() {
            static base_objects::events::sync_event_no_cancel<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    states::post_send_viewer().join([](auto&& packet, base_objects::shared_client_data& client) {
                        result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            (void)once_init;
            return result;
        }

        static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&>& receive_viewer() {
            static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    states::receive_viewer().join([](auto&& packet, base_objects::shared_client_data& client) {
                        return result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static base_objects::events::sync_event_single<packet_variants, base_objects::shared_client_data&>& processor() {
            static base_objects::events::sync_event<packet_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    states::processor().join([](auto&& packet, base_objects::shared_client_data& client) {
                        result.notify(std::move(packet), client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static packet_variants client_decode(base_objects::shared_client_data& context, ArrayStream& stream) {
            packet_variants result;
            client_decode(context, stream, [&result](auto& client, auto&& packet) {
                result = std::move(packet);
            });
            return result;
        }

        template <class FN>
        static decltype(auto) client_decode(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) {
            using fn_t = void (*)(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn);
            static constexpr fn_t selector[]{
                ([](base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) -> decltype(auto) {
                    return fn(context, states::client_decode(context, stream));
                })...
            };
            return selector[static_cast<int>(__internal::get_state(context))](context, stream, std::forward<FN>(fn));
        }

        template <class FN>
        static decltype(auto) client_decode_direct(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) {
            using fn_t = void (*)(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn);
            static constexpr fn_t selector[]{
                ([](base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) -> decltype(auto) {
                    return states::client_decode(context, stream, std::forward<FN>(fn));
                })...
            };
            return selector[static_cast<int>(__internal::get_state(context))](context, stream, std::forward<FN>(fn));
        }

        template <class plugin>
        static void send_viewer(plugin& self, auto&& fn) {
            self.register_event(send_viewer(), [_fn = std::move(fn)](packet_ref_variants vars, base_objects::shared_client_data& client) {
                return std::visit(
                    [&_fn, &client](auto& state) {
                        return std::visit(
                            [&_fn, &client](auto& packet) {
                                return _fn(packet.get(), client);
                            },
                            state
                        );
                    },
                    vars
                );
            });
        }

        template <class plugin>
        static void post_send_viewer(plugin& self, auto&& fn) {
            self.register_event(post_send_viewer(), [_fn = std::move(fn)](packet_ref_variants vars, base_objects::shared_client_data& client) {
                std::visit(
                    [&_fn, &client](auto& state) {
                        std::visit(
                            [&_fn, &client](auto& packet) {
                                _fn(packet.get(), client);
                            },
                            state
                        );
                    },
                    vars
                );
            });
        }

        template <class plugin>
        static void receive_viewer(plugin& self, auto&& fn) {
            self.register_event(receive_viewer(), [_fn = std::move(fn)](packet_ref_variants vars, base_objects::shared_client_data& client) {
                return std::visit(
                    [&_fn, &client](auto& state) {
                        return std::visit(
                            [&_fn, &client](auto& packet) {
                                return _fn(packet.get(), client);
                            },
                            state
                        );
                    },
                    vars
                );
            });
        }

        template <class plugin>
        static void processor(plugin& self, auto&& fn) {
            self.register_event(processor(), [_fn = std::move(fn)](packet_variants vars, base_objects::shared_client_data& client) {
                std::visit(
                    [&_fn, &client](auto& state) {
                        std::visit(
                            [&_fn, &client](auto& packet) {
                                _fn(std::move(packet), client);
                            },
                            state
                        );
                    },
                    vars
                );
            });
        }

        template <class packet>
        static constexpr bool contains() {
            return ((states::template contains<packet>()) || ... || false);
        }
    };

    //first should be server_bound
    template <class... directions>
    struct global_packets_ops {
        using packet_variants = std::variant<typename directions::packet_variants...>;
        using packet_ref_variants = std::variant<typename directions::packet_ref_variants...>;

        static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&>& send_viewer() {
            static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    directions::send_viewer().join([](auto&& packet, base_objects::shared_client_data& client) {
                        return result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static base_objects::events::sync_event_no_cancel<packet_ref_variants, base_objects::shared_client_data&>& post_send_viewer() {
            static base_objects::events::sync_event_no_cancel<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    directions::post_send_viewer().join([](auto&& packet, base_objects::shared_client_data& client) {
                        result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            (void)once_init;
            return result;
        }

        static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&>& receive_viewer() {
            static base_objects::events::sync_event<packet_ref_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    directions::receive_viewer().join([](auto&& packet, base_objects::shared_client_data& client) {
                        return result.notify(packet, client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static base_objects::events::sync_event_single<packet_variants, base_objects::shared_client_data&>& processor() {
            static base_objects::events::sync_event<packet_variants, base_objects::shared_client_data&> result;

            static auto once_init = []() {
                (
                    directions::processor().join([](auto&& packet, base_objects::shared_client_data& client) {
                        result.notify(std::move(packet), client);
                    }),
                    ...
                );
                return true;
            }();
            return result;
        }

        static packet_variants client_decode(base_objects::shared_client_data& context, ArrayStream& stream) {
            packet_variants result;
            client_decode(context, stream, [&result](auto& client, auto&& packet) {
                result = std::move(packet);
            });
            return result;
        }

        template <class FN>
        static decltype(auto) client_decode(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) {
            using fn_t = void (*)(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn);
            static constexpr fn_t selector[]{
                ([](base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) -> decltype(auto) {
                    return fn(context, directions::client_decode(context, stream));
                })...
            };
            return selector[0](context, stream, std::forward<FN>(fn));
        }

        template <class FN>
        static decltype(auto) client_decode_direct(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) {
            using fn_t = void (*)(base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn);
            static constexpr fn_t selector[]{
                ([](base_objects::shared_client_data& context, ArrayStream& stream, FN&& fn) -> decltype(auto) {
                    return directions::client_decode_direct(context, stream, std::forward<FN>(fn));
                })...
            };
            return selector[0](context, stream, std::forward<FN>(fn));
        }

        template <class plugin>
        static void send_viewer(plugin& self, auto&& fn) {
            self.register_event(send_viewer(), [_fn = std::move(fn)](packet_ref_variants direction, base_objects::shared_client_data& client) {
                return std::visit(
                    [&_fn, &client](auto& vars) {
                        return std::visit(
                            [&_fn, &client](auto& state) {
                                return std::visit(
                                    [&_fn, &client](auto& packet) {
                                        return _fn(packet.get(), client);
                                    },
                                    state
                                );
                            },
                            vars
                        );
                    },
                    direction
                );
            });
        }

        template <class plugin>
        static void post_send_viewer(plugin& self, auto&& fn) {
            self.register_event(post_send_viewer(), [_fn = std::move(fn)](packet_ref_variants direction, base_objects::shared_client_data& client) {
                std::visit(
                    [&_fn, &client](auto& vars) {
                        std::visit(
                            [&_fn, &client](auto& state) {
                                std::visit(
                                    [&_fn, &client](auto& packet) {
                                        _fn(packet.get(), client);
                                    },
                                    state
                                );
                            },
                            vars
                        );
                    },
                    direction
                );
            });
        }

        template <class plugin>
        static void receive_viewer(plugin& self, auto&& fn) {
            self.register_event(receive_viewer(), [_fn = std::move(fn)](packet_ref_variants direction, base_objects::shared_client_data& client) {
                std::visit(
                    [&_fn, &client](auto& vars) {
                        return std::visit(
                            [&_fn, &client](auto& state) {
                                return std::visit(
                                    [&_fn, &client](auto& packet) {
                                        return _fn(packet.get(), client);
                                    },
                                    state
                                );
                            },
                            vars
                        );
                    },
                    direction
                );
            });
        }

        template <class plugin>
        static void processor(plugin& self, auto&& fn) {
            self.register_event(processor(), [_fn = std::move(fn)](packet_variants direction, base_objects::shared_client_data& client) {
                std::visit(
                    [&_fn, &client](auto& vars) {
                        std::visit(
                            [&_fn, &client](auto& state) {
                                std::visit(
                                    [&_fn, &client](auto& packet) {
                                        _fn(std::move(packet), client);
                                    },
                                    state
                                );
                            },
                            vars
                        );
                    },
                    direction
                );
            });
        }
   
        template<class packet>
        static constexpr bool contains() {
            return ((directions::template contains<packet>()) || ... || false);
        }
    };

    template <class packet>

    bool send(base_objects::shared_client_data& client, packet&& p) {
        return packet_ops<packet>::send(client, std::move(p));
    }

    template <class packet>
    base_objects::network::response client_encode(base_objects::shared_client_data& client, packet&& p) {
        return packet_ops<packet>::client_encode(client, std::move(p));
    }

    template <class packet>
    base_objects::network::response encode(packet&& p) {
        return packet_ops<packet>::encode(std::move(p));
    }

    template <class packet>
    std::string stringize(const packet& p) {
        return packet_ops<packet>::stringize(p);
    }

    template <class packet>
    void make_process(base_objects::shared_client_data& client, packet&& p) {
        return packet_ops<packet>::make_process(client, std::move(p));
    }

    template <class plugin>
    inline void send_viewer(plugin& self, auto&& fn) {
        packet_ops<__internal::first_argument_type<decltype(fn)>>::template send_viewer(self, std::move(fn));
    }

    template <class plugin>
    inline void post_send_viewer(plugin& self, auto&& fn) {
        packet_ops<__internal::first_argument_type<decltype(fn)>>::template post_send_viewer(self, std::move(fn));
    }

    template <class plugin>
    inline void receive_viewer(plugin& self, auto&& fn) {
        packet_ops<__internal::first_argument_type<decltype(fn)>>::template receive_viewer(self, std::move(fn));
    }

    template <class plugin>
    inline void processor(plugin& self, auto&& fn) {
        packet_ops<__internal::first_argument_type<decltype(fn)>>::template processor(self, std::move(fn));
    }
}

template <class packet>
inline copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, packet&& p) {
    copper_server::api::packets::packet_ops<packet>::send(client, std::move(p));
    return client;
}

#endif /* SRC_API_PACKETS_OPS */
