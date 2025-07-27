#include <library/enbt/senbt.hpp>
#include <src/api/configuration.hpp>
#include <src/api/new_packets.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/registers.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/readers.hpp>

#define REFLECT_CPP_C_ARRAYS_OR_INHERITANCE
#include <rfl/enums.hpp>
#include <rfl/to_view.hpp>


namespace copper_server {
    namespace api::new_packets {
        using namespace base_objects;

        namespace reflect {
            template <class EnumT>
            struct enum_limits {
                static constexpr std::underlying_type_t<EnumT> min = std::numeric_limits<std::underlying_type_t<EnumT>>::min();
                static constexpr std::underlying_type_t<EnumT> max = std::numeric_limits<std::underlying_type_t<EnumT>>::max();
            };

            template<>
            struct enum_limits<copper_server::api::new_packets::client_bound::play::level_event::event_id>{
                static constexpr uint16_t min = 1000;
                static constexpr uint16_t max = 3022;
            };
        }
    }
}
namespace copper_server {
    namespace api::new_packets {
        using namespace base_objects;
        namespace reflect {

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
            };

            template <class EnumT, size_t... Is>
            constexpr std::array<std::string_view, sizeof...(Is)> make_enum_flag_names_impl(std::index_sequence<Is...>) {
                using U = std::underlying_type_t<EnumT>;
                return {reflect::get_enum_by_value<static_cast<EnumT>(U(1) << Is)>::by_num()...};
            }

            template <class EnumT>
            constexpr auto make_enum_names() {
                using U = std::underlying_type_t<EnumT>;
                return make_enum_flag_names_impl<EnumT>(std::make_index_sequence<sizeof(U) * 8>{});
            }

            template <class EnumT>
            struct enum_flags_extractor {
                static constexpr inline auto res = make_enum_names<EnumT>();

                static std::string_view get_enum_value(size_t bit) {
                    return res[bit];
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
        }

        namespace client_bound {
            namespace play {
                chunks_biomes chunks_biomes::create(const storage::chunk_data& chunk) {
                    chunks_biomes result;
                    result.x = (int32_t)chunk.chunk_x;
                    result.z = (int32_t)chunk.chunk_z;
                    for (auto& section : chunk.sub_chunks) {
                        base_objects::pallete_container_biome biomes(registers::biomes.size());
                        for (auto& x : section.biomes)
                            for (auto& y : x)
                                for (auto& z : y)
                                    biomes.add(z);
                        result.sections_of_biomes.value.push_back(std::move(biomes));
                    }
                    return result;
                }

                level_chunk_with_light level_chunk_with_light::create(const storage::chunk_data& chunk, const storage::world_data& world) {
                    level_chunk_with_light result;
                    static auto build_height_map = [](uint8_t type, const uint64_t (&hei_map)[16][16], size_t world_height) {
                        base_objects::pallete_data_height_map data(base_objects::pallete_data::bits_for_max(world_height));
                        for (uint_fast8_t x = 0; x < 16; x++)
                            for (uint_fast8_t z = 0; z < 16; z++)
                                data.add(hei_map[x][z]);
                        return height_map{
                            .type = height_map::type_e(type),
                            .pallete_data = std::move(data)
                        };
                    };
                    size_t world_height = chunk.sub_chunks.size() * 16;
                    result.height_maps = {
                        build_height_map(1, chunk.height_maps.surface, world_height),
                        build_height_map(3, chunk.height_maps.ocean_floor, world_height),
                        build_height_map(4, chunk.height_maps.motion_blocking, world_height),
                        build_height_map(5, chunk.height_maps.motion_blocking_no_leaves, world_height)
                    };


                    result.sections.value.reserve(chunk.sub_chunks.size());
                    for (auto& section_ : chunk.sub_chunks) {
                        uint16_t block_count = 0;
                        base_objects::pallete_container_block blocks(base_objects::block::block_states_size());
                        base_objects::pallete_container_biome biomes(registers::biomes.size());
                        for (auto& x : section_.blocks)
                            for (auto& y : x)
                                for (auto z : y) {
                                    block_count += !z.is_air();
                                    blocks.add(z.id);
                                }
                        for (auto& x : section_.biomes)
                            for (auto& y : x)
                                for (auto& z : y)
                                    biomes.add(z);
                        result.sections.value.push_back(section{block_count, std::move(blocks), std::move(biomes)});
                    }
                    if (api::configuration::get().protocol.send_nbt_data_in_chunk) {
                        auto sub_chunk = world.get_world_y_chunk_offset();
                        for (auto& section : chunk.sub_chunks) {
                            auto sub_chunk_pos = sub_chunk * 16;
                            section.for_each_block_entity(
                                [&result, sub_chunk_pos](uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, const enbt::value& entity_data) {
                                    result.block_entities.push_back(
                                        block_entity{
                                            .xz = uint8_t((local_x << 4) | local_z),
                                            .y = int16_t(sub_chunk_pos + local_y),
                                            .type = block.block_entity_id(),
                                            .data = entity_data
                                        }
                                    );
                                }
                            );
                            ++sub_chunk;
                        }
                    }

                    auto [x, z, sky_light_mask, block_light_mask, empty_sky_light_mask, empty_block_light_mask, sky_light, block_light] = light_update::create(chunk);
                    result.x = x;
                    result.z = z;
                    result.sky_light_mask = std::move(sky_light_mask);
                    result.block_light_mask = std::move(block_light_mask);
                    result.empty_sky_light_mask = std::move(empty_sky_light_mask);
                    result.empty_block_light_mask = std::move(empty_block_light_mask);
                    result.sky_light = std::move(sky_light);
                    result.block_light = std::move(block_light);
                    return result;
                }

                light_update light_update::create(const storage::chunk_data& chunk) {
                    bit_list_array<uint64_t> sky_light_mask;
                    bit_list_array<uint64_t> block_light_mask;
                    bit_list_array<uint64_t> empty_sky_light_mask;
                    bit_list_array<uint64_t> empty_block_light_mask;
                    list_array<list_array<uint8_t>> sky_light;
                    list_array<list_array<uint8_t>> block_light;
                    {
                        //light below world is unset
                        sky_light_mask.push_back(false);
                        block_light_mask.push_back(false);
                        empty_sky_light_mask.push_back(true);
                        empty_block_light_mask.push_back(true);
                        for (auto& section : chunk.sub_chunks) {
                            sky_light_mask.push_back(section.sky_lighted);
                            block_light_mask.push_back(section.block_lighted);
                            empty_sky_light_mask.push_back(section.sky_lighted);
                            empty_block_light_mask.push_back(section.block_lighted);

                            if (section.sky_lighted) {
                                bit_list_array<> section_sky_light;
                                for (auto& x : section.sky_light.light_map)
                                    for (auto& y : x)
                                        for (auto z : y) {
                                            section_sky_light.push_back(z.light_point & 1);
                                            section_sky_light.push_back(z.light_point & 2);
                                            section_sky_light.push_back(z.light_point & 4);
                                            section_sky_light.push_back(z.light_point & 8);
                                        }
                                sky_light.push_back(section_sky_light.take());
                            }
                            if (section.block_lighted) {
                                bit_list_array<> section_block_light;
                                for (auto& x : section.block_light.light_map)
                                    for (auto& y : x)
                                        for (auto z : y) {
                                            section_block_light.push_back(z.light_point & 1);
                                            section_block_light.push_back(z.light_point & 2);
                                            section_block_light.push_back(z.light_point & 4);
                                            section_block_light.push_back(z.light_point & 8);
                                        }
                                block_light.push_back(section_block_light.take());
                            }
                        }
                        //light above world is unset
                        sky_light_mask.push_back(false);
                        block_light_mask.push_back(false);
                        empty_sky_light_mask.push_back(true);
                        empty_block_light_mask.push_back(true);
                    }

                    static auto convert_light = [](list_array<list_array<uint8_t>>& arr) {
                        return arr
                            .convert_fn(
                                [](const list_array<uint8_t>& it) {
                                    return it.to_container<vector_fixed<uint8_t, 2048>>();
                                }
                            )
                            .to_container<std::vector<vector_fixed<uint8_t, 2048>>>();
                    };
                    static auto convert_light_mask = [](bit_list_array<uint64_t>& arr) {
                        return arr.data().to_container<std::vector<uint64_t>>();
                    };

                    light_update update;
                    update.x = (int32_t)chunk.chunk_x;
                    update.z = (int32_t)chunk.chunk_z;
                    update.sky_light_mask = convert_light_mask(sky_light_mask);
                    update.block_light_mask = convert_light_mask(block_light_mask);
                    update.empty_sky_light_mask = convert_light_mask(empty_sky_light_mask);
                    update.empty_block_light_mask = convert_light_mask(empty_block_light_mask);
                    update.sky_light = convert_light(sky_light);
                    update.block_light = convert_light(block_light);
                    return update;
                }
            }
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
        concept is_vector_sized = is_tvalue_template_base_of<vector_sized, type> || is_tvalue_template_base_of<vector_sized_siz_from_packet, type> || is_tvalue_template_base_of<vector_sized_no_size, type>;

        template <class type>
        concept is_vector_fixed = is_tvalue_template_base_of<vector_fixed, type>;

        template <class type>
        concept is_std_array = is_tvalue_template_base_of<std::array, type>;

        template <class type>
        concept is_limited_num = is_tvalue_template_base_of<limited_num, type>;

        template <class type>
        concept requires_check = is_limited_num<type> || is_string_sized<type> || is_vector_sized<type> || is_vector_fixed<type> || is_value_template_base_of<no_size, type>;


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
                                } else
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
            } else if constexpr (std::is_same_v<position, Type>)
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
            else if constexpr (std::is_same_v<base_objects::pallete_container, Type>) {
                std::visit(
                    [&](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        if constexpr (std::is_same_v<base_objects::pallete_container_indirect, T>) {
                            res.write_value(it.bits_per_entry);
                            res.write_var32_check(it.palette.size());
                            for (auto& i : it.palette)
                                res.write_var32(i);
                            auto data = it.data.get();
                            uint8_t padding = it.size() % 8;
                            res.write_direct(std::move(data));
                            while (padding--)
                                res.write_value((uint8_t)0);
                        } else if constexpr (std::is_same_v<base_objects::pallete_container_single, T>) {
                            res.write_value((uint8_t)0);
                            res.write_var32(it.id_of_palette);
                        } else if constexpr (std::is_same_v<base_objects::pallete_data, T>) {
                            res.write_value((uint8_t)it.bits_per_entry);
                            auto data = it.get();
                            uint8_t padding = data.size() % 8;
                            res.write_direct(std::move(data));
                            while (padding--)
                                res.write_value((uint8_t)0);
                        }
                    },
                    value.compile()
                );
            } else if constexpr (std::is_same_v<base_objects::pallete_data_height_map, Type>) {
                auto data = value.get();
                uint8_t padding = data.size() % 8;
                res.write_direct(std::move(data));
                while (padding--)
                    res.write_value((uint8_t)0);
            } else if constexpr (is_template_base_of<std::vector, Type>) {
                if constexpr (!is_value_template_base_of<no_size, Type> && !std::is_base_of_v<size_from_packet, Type>)
                    res.write_var32_check(value.size());
                for (auto&& it : value)
                    serialize_entry(res, context, it);
            } else if constexpr (is_template_base_of<ignored, Type>) {
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
            } else if constexpr (is_template_base_of<std::unique_ptr, Type>) {
                serialize_entry(res, context, *value);
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
            } else if constexpr (is_template_base_of<sized_entry, Type>) {
                base_objects::network::response::item inner;
                serialize_entry(inner, context, value.value);
                typename Type::size_type size;
                if constexpr (sizeof(typename Type::size_type) >= 8)
                    size = inner.data.size();
                else if constexpr (sizeof(typename Type::size_type) >= 4)
                    size = (int32_t)inner.data.size();
                else
                    size = (int16_t)inner.data.size();
                serialize_entry(res, context, size);
                res.write_in(inner);
            } else if constexpr (is_limited_num<Type>) {
                serialize_entry(res, context, value.value);
            } else if constexpr (is_bitset_fixed<Type>) {
                res.write_direct(value.value.data());
            } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, Type>) {
                res.write_var32_check(value.data().size());
                res.write_direct(value.data());
            } else {
                bool process_next = true;
                reflect::for_each_field(value, [&res, &context, &process_next](auto& item) {
                    if (process_next)
                        serialize_entry(res, context, item);
                    if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                        process_next = (bool)item.value;
                });
            }
        }

        template <class T, class T_prev>
        void preprocess_structure(SharedClientData& context, T& value, T_prev& prev) {
            if constexpr (is_value_template_base_of<no_size, T>) {
                if constexpr (is_template_base_of<std::vector, T>)
                    if (value.size() != T::get_depended_size(context, prev))
                        throw std::overflow_error("The size of vector did not equals to depended values.");
            }
            if constexpr (is_bitset_fixed<T>) {
                if (value.value.size() != T::max_size::value)
                    throw std::overflow_error("The bitset size not equal required one.");
            } else if constexpr (is_vector_fixed<T>) {
                if (value.size() != T::required_size)
                    throw std::overflow_error("The vector size not equal required one.");
                if constexpr (need_preprocess<typename T::value_type>)
                    for (auto& it : value)
                        preprocess_structure(context, it, prev);
            } else if constexpr (is_template_base_of<std::vector, T> || is_std_array<T>) {
                if constexpr (need_preprocess<typename T::value_type>)
                    for (auto& it : value)
                        preprocess_structure(context, it, prev);
            } else if constexpr (is_string_sized<T>) {
                if (value.value.size() > T::max_size)
                    throw std::overflow_error("The string size is over the limit.");
            } else if constexpr (is_vector_sized<T>) {
                if (value.size() > T::max_size)
                    throw std::overflow_error("The vector size is over the limit.");
                if constexpr (need_preprocess<typename T::value_type>)
                    for (auto& it : value.value)
                        preprocess_structure(context, it, prev);
            } else if constexpr (is_template_base_of<std::optional, T>) {
                if constexpr (need_preprocess<typename T::value_type>)
                    if (value)
                        preprocess_structure(context, *value, prev);
            } else if constexpr (is_limited_num<T>) {
                if (value.value > T::check_max)
                    throw std::overflow_error("The value is too big");
                if (value.value < T::check_min)
                    throw std::underflow_error("The value is too low");
            } else if constexpr (is_template_base_of<value_optional, T>) {
                if constexpr (need_preprocess<typename std::decay_t<decltype(*value.rest)>::value_type>)
                    if (value)
                        preprocess_structure(context, *value.rest, prev);
            } else if constexpr (is_flags_list_from<T> || is_template_base_of<flags_list, T>) {
                value.for_each([&](auto& it) {
                    if constexpr (need_preprocess<typename std::decay_t<decltype(it)>::value_type>)
                        preprocess_structure(context, it, prev);
                });
            } else if constexpr (is_template_base_of<enum_switch, T>) {
                std::visit(
                    [&](auto& it) {
                        if constexpr (need_preprocess<typename std::decay_t<decltype(it)>::value_type>)
                            preprocess_structure(context, it, prev);
                    },
                    value
                );
            } else if constexpr (is_template_base_of<std::unique_ptr, T>) {
                preprocess_structure(context, *value, prev);
            } else {
                bool process_next = true;
                reflect::for_each_field(value, [&value, &context, &process_next](auto& item) {
                    if (!process_next)
                        return;
                    using I = std::decay_t<decltype(item)>;
                    if constexpr (need_preprocess<I>)
                        preprocess_structure(context, item, value);
                    if constexpr (could_be_preprocessed<I, T>)
                        item.preprocess(value);

                    if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                        process_next = (bool)item.value;
                });
            }
        }

        template <class T>
        void serialize_packet(base_objects::network::response& res, base_objects::SharedClientData& context, T& value) {
            using Type = std::decay_t<T>;
            if constexpr (std::is_base_of_v<packet_preprocess, Type>)
                preprocess_structure(context, value, value);

            if constexpr (is_packet<Type>) {
                base_objects::network::response::item it;
                it.write_id(Type::packet_id::value);
                serialize_entry(it, context, value);
                res += it;
            } else if constexpr (std::is_base_of_v<compound_packet, Type>) {
                reflect::for_each_field(value, [&res, &value, &context](auto& item) {
                    using I = std::decay_t<decltype(item)>;
                    if constexpr (is_packet<I>) {
                        serialize_packet(res, context, item);
                    } else if constexpr (
                        std::is_same_v<I, client_bound::login_packet>
                        || std::is_same_v<I, client_bound::configuration_packet>
                        || std::is_same_v<I, client_bound::play_packet>
                        || std::is_same_v<I, server_bound::login_packet>
                        || std::is_same_v<I, server_bound::configuration_packet>
                        || std::is_same_v<I, server_bound::play_packet>
                    ) {
                        std::visit([&](auto& it) { serialize_packet(res, context, it); }, item);
                    } else if (is_template_base_of<std::vector, I>) {
                        for (auto& it : item)
                            serialize_packet(res, context, it);
                    }
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
            else if constexpr (std::is_same_v<enbt::value, Type>)
                value = ReadNetworkNBT_enbt(stream);
            else if constexpr (std::is_same_v<base_objects::slot, Type>)
                ; //TODO
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
                auto range = stream.range_read(size);
                value.data.data() = list_array<uint8_t>(range.data_read(), range.size_read());
            } else if constexpr (is_template_base_of<std::vector, Type>) {
                if constexpr (!is_value_template_base_of<no_size, Type> && !std::is_base_of_v<size_from_packet, Type>) {
                    value.resize(stream.read_var<int32_t>());
                } else if constexpr (is_value_template_base_of<no_size, Type>) {
                    value.resize(Type::get_depended_size(context, *prev));
                } else
                    value.resize(stream.size_read() / sizeof(typename Type::value_type));
                for (auto&& it : value)
                    decode_entry(context, stream, it, prev);
            } else if constexpr (is_template_base_of<ignored, Type>) {
            } else if constexpr (is_template_base_of<std::optional, Type>) {
                value = std::nullopt;
                if (stream.read_value<bool>()) {
                    value = typename Type::value_type{};
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
            } else if constexpr (is_template_base_of<enum_switch, Type>) {
                typename Type::encode_type id_check;
                decode_entry(context, stream, id_check, prev);
                Type::get_enum(id_check, [&]<class enum_T>() {
                    enum_T make_res;
                    decode_entry(context, stream, make_res, prev);
                    value = std::move(make_res);
                });
            } else if constexpr (is_template_base_of<std::unique_ptr, Type>) {
                value = std::make_unique<typename T::element_type>();
                decode_entry(context, stream, *value, prev);
            } else if constexpr (is_template_base_of<any_of, Type>) {
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
            } else if constexpr (is_template_base_of<unordered_id, Type>) {
                decode_entry(context, stream, value.value, prev); //TODO
            } else if constexpr (is_template_base_of<ordered_id, Type>) {
                decode_entry(context, stream, value.value, prev); //TODO
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
            } else {
                bool process_next = true;
                reflect::for_each_field(value, [&](auto& item) {
                    if (process_next)
                        decode_entry(context, stream, item, &value);
                    if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                        process_next = (bool)item.value;
                });
            }
        }

        template <class T>
        server_bound_packet decode_server_packet(SharedClientData& context, ArrayStream& stream) {
            T res{};
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

        client_bound::login_packet decode_client_login(base_objects::SharedClientData& context, ArrayStream& stream) {
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::initialization)
                throw std::invalid_argument("The client is not in login protocol stage");
            return std::get<client_bound::login_packet>(client_login_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        client_bound::configuration_packet decode_client_configuration(base_objects::SharedClientData& context, ArrayStream& stream) {
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::configuration)
                throw std::invalid_argument("The client is not in configuration protocol stage");
            return std::get<client_bound::configuration_packet>(client_configuration_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        client_bound::play_packet decode_client_play(base_objects::SharedClientData& context, ArrayStream& stream) {
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::play)
                throw std::invalid_argument("The client is not in play protocol stage");
            return std::get<client_bound::play_packet>(client_play_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        server_bound::login_packet decode_server_login(base_objects::SharedClientData& context, ArrayStream& stream) {
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::initialization)
                throw std::invalid_argument("The client is not in login protocol stage");
            return std::get<server_bound::login_packet>(server_login_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        server_bound::configuration_packet decode_server_configuration(base_objects::SharedClientData& context, ArrayStream& stream) {
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::configuration)
                throw std::invalid_argument("The client is not in configuration protocol stage");
            return std::get<server_bound::configuration_packet>(server_configuration_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        server_bound::play_packet decode_server_play(base_objects::SharedClientData& context, ArrayStream& stream) {
            if (context.packets_state.state != SharedClientData::packets_state_t::protocol_state::play)
                throw std::invalid_argument("The client is not in play protocol stage");
            return std::get<server_bound::play_packet>(server_play_dec.at(stream.read_var<int32_t>())(context, stream));
        }

        namespace sp {
            template <class T>
            void serialize_entry(std::string& res, size_t spacing, const T& value);

            template <class T>
            void serialize_array(std::string& res, size_t spacing, const T& value) {
                std::vector<std::string> res_tmp;
                res.resize(value.size());
                size_t i = 0;
                for (auto&& it : value)
                    serialize_entry(res_tmp[i++], spacing + 4, it);
                bool one_line = true;
                for (auto& it : res_tmp)
                    if (it.size() > 10)
                        one_line = false;
                if (one_line) {
                    res += "[";
                    for (auto& it : res_tmp)
                        res += std::move(it);
                    res += "]";
                } else {
                    res += "[\n";
                    for (auto& it : res_tmp)
                        res += std::move(it);
                    res += "\n" + std::string(spacing, ' ') + "]";
                }
            }

            template <class T>
            void serialize_entry(std::string& res, size_t spacing, const T& value) {
                using Type = std::decay_t<T>;
                if constexpr (std::is_same_v<identifier, Type>)
                    res += "\"" + value.value + "\"";
                else if constexpr (is_string_sized<Type>)
                    res += "\"" + value.value + "\"";
                else if constexpr (std::is_same_v<json_text_component, Type>)
                    res += "\"" + value.value + "\"";
                else if constexpr (std::is_same_v<var_int32, Type>)
                    res += std::to_string(value.value);
                else if constexpr (std::is_same_v<var_int64, Type>)
                    res += std::to_string(value.value);
                else if constexpr (std::is_same_v<optional_var_int32, Type>) {
                    if (value)
                        res += std::to_string(*value);
                    else
                        res += "null";
                } else if constexpr (std::is_same_v<optional_var_int64, Type>) {
                    if (value)
                        res += std::to_string(*value);
                    else
                        res += "null";
                } else if constexpr (std::is_same_v<position, Type>)
                    res += "{.x = " + std::to_string(value.x) + ".z = " + std::to_string(value.z) + ".y = " + std::to_string(value.y) + "}";
                else if constexpr (std::is_arithmetic_v<Type>)
                    res += std::to_string(value);
                else if constexpr (std::is_same_v<std::string, Type>)
                    res += "\"" + value + "\"";
                else if constexpr (std::is_same_v<enbt::raw_uuid, Type>)
                    res += "\"" + value.to_string() + "\"";
                else if constexpr (std::is_same_v<Chat, Type>) {
                    std::string alignment(spacing + 1, ' ');
                    alignment[0] = '\n';
                    res += list_array<char>(senbt::serialize(value.ToENBT(), false, true))
                               .replace('\n', alignment.data(), alignment.size())
                               .to_container<std::string>();
                } else if constexpr (std::is_same_v<enbt::value, Type>) {
                    std::string alignment(spacing + 1, ' ');
                    alignment[0] = '\n';
                    res += list_array<char>(senbt::serialize(value, false, true))
                               .replace('\n', alignment.data(), alignment.size())
                               .to_container<std::string>();
                } else if constexpr (std::is_same_v<base_objects::slot, Type>) {
                    if (value) {
                        std::string alignment(spacing + 1, ' ');
                        alignment[0] = '\n';
                        res += list_array<char>(senbt::serialize(value->to_enbt(), false, true))
                                   .replace('\n', alignment.data(), alignment.size())
                                   .to_container<std::string>();
                    } else
                        res += "null";
                } else if constexpr (std::is_same_v<base_objects::pallete_container, Type>) {
                    res += "pallete_data";
                } else if constexpr (std::is_same_v<base_objects::pallete_data_height_map, Type>) {
                    res += "pallete_data";
                } else if constexpr (is_template_base_of<std::vector, Type> || is_std_array<Type>) {
                    serialize_array(res, spacing, value);
                } else if constexpr (is_template_base_of<ignored, Type>) {
                } else if constexpr (is_template_base_of<std::optional, Type>) {
                    if (value)
                        serialize_entry(res, spacing, *value);
                    else
                        res += "null";
                } else if constexpr (is_template_base_of<enum_as, Type>) {
                    res += reflect::get_enum_value(value.value);
                } else if constexpr (is_template_base_of<enum_as_flag, Type>) {
                    res += reflect::get_enum_flag_value(value.value);
                } else if constexpr (is_template_base_of<or_, Type>) {
                    std::visit([&](auto& it) { serialize_entry(res, spacing, it); }, value);
                } else if constexpr (is_template_base_of<enum_switch, Type>) {
                    std::visit(
                        [&](auto& it) {
                            using it_T = std::decay_t<decltype(it)>;
                            res += "{ ";
                            serialize_entry(res, spacing, Type::encode_type(it_T::item_id::value));
                            res += ": ";
                            serialize_entry(res, spacing, it);
                            res += "}";
                        },
                        value
                    );
                } else if constexpr (is_template_base_of<std::unique_ptr, Type>) {
                    serialize_entry(res, spacing, *value);
                } else if constexpr (is_template_base_of<any_of, Type>) {
                    serialize_entry(res, spacing, value.value);
                } else if constexpr (is_template_base_of<flags_list, Type>) {
                    res += "{ ";
                    serialize_entry(res, spacing, value.flag);
                    res += ": {";
                    value.for_each_in_order([&](auto& it) {
                        res += "\n" + std::string(spacing, ' ');
                        serialize_entry(res, spacing + 4, it);
                    });
                    res += "}}";
                } else if constexpr (is_flags_list_from<Type>) {
                    res += "{";
                    value.for_each_in_order([&](auto& it) {
                        res += "\n" + std::string(spacing, ' ');
                        serialize_entry(res, spacing + 4, it);
                    });
                    res += "}";
                } else if constexpr (is_template_base_of<unordered_id, Type>) {
                    serialize_entry(res, spacing, value.value); //TODO
                } else if constexpr (is_template_base_of<ordered_id, Type>) {
                    serialize_entry(res, spacing, value.value); //TODO
                } else if constexpr (is_template_base_of<value_optional, Type>) {
                    res += "{";
                    if (value.rest && value.v) {
                        serialize_entry(res, spacing + 4, value.v);
                        res += ":";
                        serialize_entry(res, spacing + 4, *value.rest);
                    } else {
                        decltype(value.v) tmp{0};
                        serialize_entry(res, spacing + 4, tmp);
                    }
                    res += "}";
                } else if constexpr (is_template_base_of<sized_entry, Type>) {
                    serialize_entry(res, spacing, value.value);
                } else if constexpr (is_limited_num<Type>) {
                    serialize_entry(res, spacing, value.value);
                } else if constexpr (is_bitset_fixed<Type>) {
                    serialize_array(res, spacing, value.value.data());
                } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, Type>) {
                    serialize_array(res, spacing, value.data());
                } else {
                    bool process_next = true;
                    bool processed = false;
                    reflect::for_each_field_with_name(value, [&res, spacing, &process_next, &processed](auto& item, auto name) {
                        if (process_next) {
                            if (processed)
                                res += ",";
                            res += "\n" + std::string(spacing + 4, ' ') + std::string(name) + ": ";
                            serialize_entry(res, spacing + 4, item);
                            processed = true;
                        }
                        if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                            process_next = (bool)item.value;
                    });
                }
            }

            template <class T>
            void serialize_packet(std::string& res, size_t spacing, T& value) {
                using Type = std::decay_t<T>;
                if constexpr (is_packet<Type>) {
                    res += std::string(reflect::get_pretty_type_name<Type>()) + "<" + std::to_string(Type::packet_id::value) + "> {\n " + std::string(spacing, ' ');
                    serialize_entry(res, spacing, value);
                    res += std::string(spacing, ' ') + "}";
                } else if constexpr (std::is_base_of_v<compound_packet, Type>) {
                    res += std::string(reflect::get_pretty_type_name<Type>()) + "<compound> {";
                    bool processed = false;
                    reflect::for_each_field_with_name(value, [&res, spacing, &processed](auto& item, auto name) {
                        using I = std::decay_t<decltype(item)>;
                        if (processed)
                            res += ",";
                        res += "\n" + std::string(spacing + 4, ' ') + std::string(name) + ": ";
                        if constexpr (is_packet<I>) {
                            serialize_packet(res, spacing + 4, item);
                        } else if constexpr (
                            std::is_same_v<I, client_bound::login_packet>
                            || std::is_same_v<I, client_bound::configuration_packet>
                            || std::is_same_v<I, client_bound::play_packet>
                            || std::is_same_v<I, server_bound::login_packet>
                            || std::is_same_v<I, server_bound::configuration_packet>
                            || std::is_same_v<I, server_bound::play_packet>
                        ) {
                            std::visit([&](auto& it) { serialize_packet(res, spacing + 4, it); }, item);
                        } else if (is_template_base_of<std::vector, I>) {
                            res += "[";
                            bool arr_processed = false;
                            for (auto&& it : item) {
                                if (arr_processed)
                                    res += ",\n";
                                else
                                    res += "\n";
                                res += std::string(spacing, ' ');
                                serialize_packet(res, spacing + 4, it);
                                arr_processed = true;
                            }
                            if (arr_processed)
                                res += "\n";
                            res += std::string(spacing, ' ') + "]";
                        }
                        processed = true;
                    });
                    if (processed)
                        res += "\n" + std::string(spacing, ' ');

                    res += "}";
                }
            }
        }

        std::string stringize_packet(const client_bound::login_packet& packet) {
            return std::visit(
                [](auto& it) -> std::string {
                    std::string res;
                    sp::serialize_packet(res, 0, it);
                    return res;
                },
                packet
            );
        }

        std::string stringize_packet(const client_bound::configuration_packet& packet) {
            return std::visit(
                [](auto& it) -> std::string {
                    std::string res;
                    sp::serialize_packet(res, 0, it);
                    return res;
                },
                packet
            );
        }

        std::string stringize_packet(const client_bound::play_packet& packet) {
            return std::visit(
                [](auto& it) -> std::string {
                    std::string res;
                    sp::serialize_packet(res, 0, it);
                    return res;
                },
                packet
            );
        }

        std::string stringize_packet(const server_bound::login_packet& packet) {
            return std::visit(
                [](auto& it) -> std::string {
                    std::string res;
                    sp::serialize_packet(res, 0, it);
                    return res;
                },
                packet
            );
        }

        std::string stringize_packet(const server_bound::configuration_packet& packet) {
            return std::visit(
                [](auto& it) -> std::string {
                    std::string res;
                    sp::serialize_packet(res, 0, it);
                    return res;
                },
                packet
            );
        }

        std::string stringize_packet(const server_bound::play_packet& packet) {
            return std::visit(
                [](auto& it) -> std::string {
                    std::string res;
                    sp::serialize_packet(res, 0, it);
                    return res;
                },
                packet
            );
        }

        std::string stringize_packet(const client_bound_packet& packet) {
            return std::visit([](auto& it) { return stringize_packet(it); }, packet);
        }

        std::string stringize_packet(const server_bound_packet& packet) {
            return std::visit([](auto& it) { return stringize_packet(it); }, packet);
        }
    }
}
