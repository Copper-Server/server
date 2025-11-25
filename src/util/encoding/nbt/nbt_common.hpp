#ifndef SRC_UTIL_ENCODING_NBT_NBT_COMMON
#define SRC_UTIL_ENCODING_NBT_NBT_COMMON
#include <src/util/nbt_stream.hpp>

namespace copper_server::util::encoding::nbt {

    template <class T>
    consteval nbt_type get_nbt_type() {
        using Type = std::decay_t<T>;
        if constexpr (std::is_same_v<std::string, Type> || std::is_same_v<std::string_view, Type> || std::is_same_v<base_objects::uuid_hex, Type> || std::is_same_v<base_objects::uuid_flat_hex, Type>)
            return nbt_type::tag_string;
        else if constexpr (std::is_same_v<bool, Type> || std::is_same_v<int8_t, Type> || std::is_same_v<uint8_t, Type>)
            return nbt_type::tag_byte;
        else if constexpr (std::is_same_v<int16_t, Type> || std::is_same_v<uint16_t, Type>)
            return nbt_type::tag_short;
        else if constexpr (std::is_same_v<int32_t, Type> || std::is_same_v<uint32_t, Type>)
            return nbt_type::tag_int;
        else if constexpr (std::is_same_v<int64_t, Type> || std::is_same_v<uint64_t, Type>)
            return nbt_type::tag_long;
        else if constexpr (std::is_same_v<float, Type>)
            return nbt_type::tag_float;
        else if constexpr (std::is_same_v<double, Type>)
            return nbt_type::tag_double;
        else if constexpr (std::is_same_v<::enbt::raw_uuid, Type> || std::is_same_v<base_objects::uuid, Type>)
            return nbt_type::tag_int_array;
        else if constexpr (std::is_same_v<::enbt::compound, Type>)
            return nbt_type::tag_compound;
        else if constexpr (
            std::is_same_v<::enbt::dynamic_array, Type>
            || std::is_same_v<::enbt::fixed_array, Type>
            || std::is_same_v<::enbt::simple_array_i16, Type>
            || std::is_same_v<::enbt::simple_array_ui16, Type>
        )
            return nbt_type::tag_list;
        else if constexpr (std::is_same_v<::enbt::uuid, Type>)
            return nbt_type::tag_string;
        else if constexpr (std::is_same_v<::enbt::simple_array_i8, Type> || std::is_same_v<::enbt::simple_array_ui8, Type>)
            return nbt_type::tag_byte_array;
        else if constexpr (std::is_same_v<::enbt::simple_array_i32, Type> || std::is_same_v<::enbt::simple_array_ui32, Type>)
            return nbt_type::tag_int_array;
        else if constexpr (std::is_same_v<::enbt::simple_array_i64, Type> || std::is_same_v<::enbt::simple_array_ui64, Type>)
            return nbt_type::tag_long_array;
        else if constexpr (is_std_array<Type> || is_template_base_of<_list_array_impl::list_array, Type>)
            return nbt_type::tag_list;
        else if constexpr (
            std::is_same_v<api::packets::identifier, Type>
            || is_string_sized<Type>
            || std::is_same_v<api::packets::json_text_component, Type>
            || std::is_same_v<api::packets::var_int32, Type>
            || std::is_same_v<api::packets::var_int64, Type>
        )
            return get_nbt_type<typename Type::underlying_type>();
        else if constexpr (std::is_same_v<base_objects::velocity, Type>)
            return nbt_type::tag_compound;
        else if constexpr (std::is_same_v<base_objects::chat, Type>)
            return nbt_type::tag_compound; //Variant in variant is not handled
        else if constexpr (std::is_same_v<api::packets::optional_var_int32, Type>)
            return nbt_type::tag_int;
        else if constexpr (std::is_same_v<api::packets::optional_var_int64, Type>)
            return nbt_type::tag_long;
        else if constexpr (std::is_same_v<base_objects::position, Type>)
            return nbt_type::tag_compound;
        else if constexpr (is_template_base_of<api::packets::ignored, Type>)
            return nbt_type::tag_compound;
        else if constexpr (is_template_base_of<std::optional, Type>)
            return get_nbt_type<typename Type::value_type>();
        else if constexpr (is_template_base_of<api::packets::enum_as, Type>)
            return nbt_type::tag_string;
        else if constexpr (is_template_base_of<api::packets::enum_as_flag, Type>)
            return nbt_type::tag_string;
        else if constexpr (is_template_base_of<api::packets::or_, Type> || is_template_base_of<api::packets::bool_or, Type>)
            return nbt_type::tag_compound; //Variant in variant is not handled
        else if constexpr (is_template_base_of<api::packets::enum_switch, Type>)
            return nbt_type::tag_compound; //Variant in variant is not handled
        else if constexpr (is_template_base_of<base_objects::box, Type>)
            return get_nbt_type<typename Type::value_type>();
        else if constexpr (is_template_base_of<api::packets::any_of, Type>)
            return get_nbt_type<typename Type::base_type>();
        else if constexpr (is_template_base_of<api::packets::flags_list, Type>)
            return nbt_type::tag_compound;
        else if constexpr (is_flags_list_from<Type>)
            return nbt_type::tag_list;
        else if constexpr (is_ordered_id<Type>)
            return get_nbt_type<typename Type::value_type>();
        else if constexpr (is_template_base_of<api::packets::value_optional, Type>)
            return nbt_type::tag_compound;
        else if constexpr (is_template_base_of<api::packets::sized_entry, Type>)
            return get_nbt_type<typename Type::value_type>();
        else if constexpr (is_limited_num<Type>)
            return get_nbt_type<typename Type::underlying_type>();
        else if constexpr (is_convertible_to_nbt_form<Type>)
            return nbt_type::tag_compound;
        else if constexpr (api::packets::is_convertible_to_packet_form<Type>)
            return get_nbt_type<api::packets::convertible_to_packet_type<Type>>();
        else if constexpr (api::id::is_source<Type>)
            return nbt_type::tag_string;
        else if constexpr (is_template_base_of<base_objects::pool, Type>)
            return nbt_type::tag_list;
        else if constexpr (nbt_is_inline<Type> && reflect::fields_count<Type> == 1) {
            nbt_type res;
            reflect::visit_field<Type>(0, []<class T>() {
                res = get_nbt_type<T>();
            });
            return res;
        } else
            return nbt_type::tag_compound;
    }

    template <class T>
    consteval bool enum_switch_is_inline_eligible(){
        using Type = std::decay_t<T>;
        if constexpr(is_template_base_of<api::packets::enum_switch, Type>) {
            std::vector<nbt_type> types;
            Type::for_each([]<class T>(){
                types.push_back(get_nbt_type<T>());
            });
            for (auto type : types){
                size_t count = 0;
                for(auto other : types)
                    count += other == type;
                if(count != 1)
                    return false;
            }
            return true;
        }
        return false;
    }
}

#endif /* SRC_UTIL_ENCODING_NBT_NBT_COMMON */
