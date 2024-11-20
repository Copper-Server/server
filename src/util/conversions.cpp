
#include <boost/json.hpp>
#include <cstddef>
#include <cstdint>
#include <library/enbt/enbt.hpp>
#include <stdexcept>
#include <string>
#include <utf8.h>
#include <utility>
#include <vector>

namespace copper_server {
    namespace util {
        namespace conversions {

            namespace base64 {
                // clang-format off
                namespace detail{
                    //this code is from boost/beast/core/detail/base64.ipp and boost/beast/core/detail/base64.hpp (version 1_85_0)
                    //
                    // Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
                    //
                    // Distributed under the Boost Software License, Version 1.0. (See accompanying
                    // file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
                    //
                    // Official repository: https://github.com/boostorg/beast
                    //
                    char const*
                    get_alphabet()
                    {
                        static char constexpr tab[] = {
                            "ABCDEFGHIJKLMNOP"
                            "QRSTUVWXYZabcdef"
                            "ghijklmnopqrstuv"
                            "wxyz0123456789+/"
                        };
                        return &tab[0];
                    }

                    signed char const*
                    get_inverse()
                    {
                        static signed char constexpr tab[] = {
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //   0-15
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  16-31
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, //  32-47
                            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, //  48-63
                            -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, //  64-79
                            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, //  80-95
                            -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, //  96-111
                            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, // 112-127
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128-143
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 144-159
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 160-175
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 176-191
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 192-207
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 208-223
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 224-239
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 240-255
                        };
                        return &tab[0];
                    }
                    
                    std::size_t constexpr
                    encoded_size(std::size_t n)
                    {
                        return 4 * ((n + 2) / 3);
                    }

                    /// Returns max bytes needed to decode a base64 string
                    inline
                    std::size_t constexpr
                    decoded_size(std::size_t n)
                    {
                        return n / 4 * 3; // requires n&3==0, smaller
                    }

                    /** Encode a series of octets as a padded, base64 string.

                        The resulting string will not be null terminated.

                        @par Requires

                        The memory pointed to by `out` points to valid memory
                        of at least `encoded_size(len)` bytes.

                        @return The number of characters written to `out`. This
                        will exclude any null termination.
                    */
                    std::size_t
                    encode(void* dest, void const* src, std::size_t len)
                    {
                        char*      out = static_cast<char*>(dest);
                        char const* in = static_cast<char const*>(src);
                        auto const tab = get_alphabet();

                        for(auto n = len / 3; n--;)
                        {
                            *out++ = tab[ (in[0] & 0xfc) >> 2];
                            *out++ = tab[((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4)];
                            *out++ = tab[((in[2] & 0xc0) >> 6) + ((in[1] & 0x0f) << 2)];
                            *out++ = tab[  in[2] & 0x3f];
                            in += 3;
                        }

                        switch(len % 3)
                        {
                        case 2:
                            *out++ = tab[ (in[0] & 0xfc) >> 2];
                            *out++ = tab[((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4)];
                            *out++ = tab[                         (in[1] & 0x0f) << 2];
                            *out++ = '=';
                            break;

                        case 1:
                            *out++ = tab[ (in[0] & 0xfc) >> 2];
                            *out++ = tab[((in[0] & 0x03) << 4)];
                            *out++ = '=';
                            *out++ = '=';
                            break;

                        case 0:
                            break;
                        }

                        return out - static_cast<char*>(dest);
                    }

                    /** Decode a padded base64 string into a series of octets.

                        @par Requires

                        The memory pointed to by `out` points to valid memory
                        of at least `decoded_size(len)` bytes.

                        @return The number of octets written to `out`, and
                        the number of characters read from the input string,
                        expressed as a pair.
                    */
                    std::pair<std::size_t, std::size_t>
                    decode(void* dest, char const* src, std::size_t len)
                    {
                        char* out = static_cast<char*>(dest);
                        auto in = reinterpret_cast<unsigned char const*>(src);
                        unsigned char c3[3], c4[4] = {0,0,0,0};
                        int i = 0;
                        int j = 0;

                        auto const inverse = get_inverse();

                        while(len-- && *in != '=')
                        {
                            auto const v = inverse[*in];
                            if(v == -1)
                                break;
                            ++in;
                            c4[i] = v;
                            if(++i == 4)
                            {
                                c3[0] =  (c4[0]        << 2) + ((c4[1] & 0x30) >> 4);
                                c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
                                c3[2] = ((c4[2] & 0x3) << 6) +   c4[3];

                                for(i = 0; i < 3; i++)
                                    *out++ = c3[i];
                                i = 0;
                            }
                        }

                        if(i)
                        {
                            c3[0] = ( c4[0]        << 2) + ((c4[1] & 0x30) >> 4);
                            c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
                            c3[2] = ((c4[2] & 0x3) << 6) +   c4[3];

                            for(j = 0; j < i - 1; j++)
                                *out++ = c3[j];
                        }

                        return {out - static_cast<char*>(dest),
                            in - reinterpret_cast<unsigned char const*>(src)};
                    }
                }

                // clang-format on

                std::string encode(const void* data, std::size_t size) {
                    std::string result;
                    result.resize(detail::encoded_size(size));
                    result.resize(detail::encode(result.data(), data, size));
                    return result;
                }

                std::string encode(const std::vector<std::uint8_t>& data) {
                    return encode(data.data(), data.size());
                }

                std::vector<std::uint8_t> decode(std::string_view data) {
                    std::vector<std::uint8_t> result;
                    result.resize(detail::decoded_size(data.size()));
                    result.resize(detail::decode(result.data(), data.data(), data.size()).first);
                    return result;
                }
            }

            namespace hex {
                std::string encode(const void* data, std::size_t size) {
                    static const char hex[] = "0123456789abcdef";
                    const auto* bytes = static_cast<const std::uint8_t*>(data);
                    std::string result;
                    result.reserve(size * 2);
                    for (std::size_t i = 0; i < size; i++) {
                        result.push_back(hex[bytes[i] >> 4]);
                        result.push_back(hex[bytes[i] & 0xF]);
                    }
                    return result;
                }

                std::string encode(const std::vector<std::uint8_t>& data) {
                    return encode(data.data(), data.size());
                }

                std::vector<std::uint8_t> decode(std::string_view data) {
                    struct construct_ {
                        uint8_t hex_map_index[256]{0};

                        construct_() {
                            for (int i = 0; i < 10; i++)
                                hex_map_index['0' + i] = i;
                            for (int i = 0; i < 6; i++)
                                hex_map_index['a' + i] = 10 + i;
                            for (int i = 0; i < 6; i++)
                                hex_map_index['A' + i] = 10 + i;
                        }
                    } static const map;

                    size_t data_size = data.size();
                    if (data_size % 2 != 0)
                        throw std::invalid_argument("Invalid hex string");
                    size_t to_add = data_size / 2;
                    size_t result_i = 0;


                    std::vector<std::uint8_t> result;
                    result.resize(to_add);
                    for (size_t i = 0; i < data_size; i += 2)
                        result[result_i++] = (map.hex_map_index[data[i]] << 4) | map.hex_map_index[data[i + 1]];
                    return result;
                }
            }

            namespace uuid {
                namespace __internal__ {
                    static uint8_t unHex_0(char ch0) {
                        switch (ch0) {
                        case '0':
                            return 0;
                        case '1':
                            return 1;
                        case '2':
                            return 2;
                        case '3':
                            return 3;
                        case '4':
                            return 4;
                        case '5':
                            return 5;
                        case '6':
                            return 6;
                        case '7':
                            return 7;
                        case '8':
                            return 8;
                        case '9':
                            return 9;
                        case 'a':
                        case 'A':
                            return 0xa;
                        case 'b':
                        case 'B':
                            return 0xb;
                        case 'c':
                        case 'C':
                            return 0xc;
                        case 'd':
                        case 'D':
                            return 0xd;
                        case 'e':
                        case 'E':
                            return 0xe;
                        case 'f':
                        case 'F':
                            return 0xf;
                        default:
                            throw std::invalid_argument("This function accepts only hex symbols");
                        }
                    }

                    static uint8_t unHex(char ch0, char ch1) {
                        return unHex_0(ch0) & (unHex_0(ch1) << 4);
                    }

                    static void addHex(std::string& str, uint8_t i) {
                        static constexpr char map[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
                        str += map[i & 0xF];
                        str += map[(i >> 4) & 0xF0];
                    }
                }

                std::string to(enbt::raw_uuid id) {
                    std::string res;
                    __internal__::addHex(res, id.data[0]);
                    __internal__::addHex(res, id.data[1]);
                    __internal__::addHex(res, id.data[2]);
                    __internal__::addHex(res, id.data[3]);
                    res += '-';
                    __internal__::addHex(res, id.data[4]);
                    __internal__::addHex(res, id.data[5]);
                    res += '-';
                    __internal__::addHex(res, id.data[6]);
                    __internal__::addHex(res, id.data[7]);
                    res += '-';
                    __internal__::addHex(res, id.data[8]);
                    __internal__::addHex(res, id.data[9]);
                    res += '-';
                    __internal__::addHex(res, id.data[10]);
                    __internal__::addHex(res, id.data[11]);
                    __internal__::addHex(res, id.data[12]);
                    __internal__::addHex(res, id.data[13]);
                    __internal__::addHex(res, id.data[14]);
                    __internal__::addHex(res, id.data[15]);
                    return res;
                }

                enbt::raw_uuid from(std::string_view id) {
                    enbt::raw_uuid res;
                    uint8_t index = 0;
                    char cache;
                    bool cached = false;
                    for (char ch : id) {
                        if (ch == '-')
                            continue;
                        if (!cached) {
                            cache = ch;
                            cached = true;
                        } else {
                            res.data[index++] = __internal__::unHex(cached, ch);
                            cached = false;
                        }
                        if (index == 16)
                            break;
                    }
                    return res;
                }
            }

            namespace string {
                std::string to_direct(std::string_view string) {
                    //handle in string \n \t  and other \* commands
                    std::string result;
                    std::string utf_code_pint;
                    bool got_slash = false;
                    bool got_utf_code_point = false;
                    bool got_big_utf_code_point = false;
                    for (char c : string) {
                        if (got_utf_code_point) {
                            utf_code_pint += c;
                            if (utf_code_pint.size() == 4) {
                                utf8::utfchar16_t code_point = std::stoi(utf_code_pint, nullptr, 16);
                                char utf8_code_point[4];
                                result += std::string(utf8_code_point, utf8::utf16to8(&code_point, &code_point + 1, utf8_code_point));
                                got_utf_code_point = false;
                                utf_code_pint.clear();
                            }
                        } else if (got_big_utf_code_point) {
                            utf_code_pint += c;
                            if (utf_code_pint.size() == 8) {
                                utf8::utfchar32_t code_point = std::stoi(utf_code_pint, nullptr, 16);
                                char utf8_code_point[4];
                                result += std::string(utf8_code_point, utf8::utf32to8(&code_point, &code_point + 1, utf8_code_point));
                                got_big_utf_code_point = false;
                                utf_code_pint.clear();
                            }
                        } else if (got_slash) {
                            switch (c) {
                            case 'n':
                                result += '\n';
                                break;
                            case 't':
                                result += '\t';
                                break;
                            case 'r':
                                result += '\r';
                                break;
                            case 'f':
                                result += '\f';
                                break;
                            case 'b':
                                result += '\b';
                                break;
                            case '\\':
                                result += '\\';
                                break;
                            case '\'':
                                result += '\'';
                                break;
                            case '\"':
                                result += '\"';
                                break;
                            case 'v':
                                result += '\v';
                                break;
                            case 'u':
                                got_utf_code_point = true;
                                break;
                            case 'U':
                                got_big_utf_code_point = true;
                                break;
                            default:
                                result += '\\';
                                result += c;
                                break;
                            }
                            got_slash = false;
                        } else if (c == '\\') {
                            got_slash = true;
                        } else
                            result += c;
                    }
                    return result;
                }

                size_t direct_find(const std::string& string, char item) {
                    bool got_slash = false;
                    size_t index = 0;
                    for (char c : string) {
                        if (got_slash) {
                            switch (c) {
                            case 'n':
                                if ('\n' == item)
                                    return index;
                                break;
                            case 't':
                                if ('\t' == item)
                                    return index;
                                break;
                            case 'r':
                                if ('\r' == item)
                                    return index;
                                break;
                            case 'f':
                                if ('\f' == item)
                                    return index;
                                break;
                            case 'b':
                                if ('\b' == item)
                                    return index;
                                break;
                            case '\\':
                                if ('\\' == item)
                                    return index;
                                break;
                            case '\'':
                                if ('\'' == item)
                                    return index;
                                break;
                            case '\"':
                                if ('\"' == item)
                                    return index;
                                break;
                            case 'v':
                                if ('\v' == item)
                                    return index;
                                break;
                            default:
                                if (c == item)
                                    return index;
                                break;
                            }
                            got_slash = false;
                        } else if (c == '\\') {
                            got_slash = true;
                        } else {
                            if (c == item)
                                return index;
                        }
                        ++index;
                    }
                    return std::string::npos;
                }

                std::string to_transport(std::string_view string) {
                    std::string result;
                    std::string utf_code_pint;
                    bool got_slash = false;
                    for (char c : string) {
                        if (got_slash) {
                            switch (c) {
                            case '\n':
                                result += "\\n";
                                break;
                            case '\t':
                                result += "\\t";
                                break;
                            case '\r':
                                result += "\\r";
                                break;
                            case '\f':
                                result += "\\f";
                                break;
                            case '\b':
                                result += "\\b";
                                break;
                            case '\\':
                                result += "\\\\";
                                break;
                            case '\'':
                                result += "\\'";
                                break;
                            case '"':
                                result += "\\\"";
                                break;
                            case '\v':
                                result += "\\v";
                                break;
                            default:
                                result += '\\';
                                result += c;
                                break;
                            }
                            got_slash = false;
                        } else if (c == '\\') {
                            got_slash = true;
                        } else
                            result += c;
                    }
                    return result;
                }
            }

            namespace json {
                boost::json::value to_json(const enbt::value& enbt) {
                    auto type_id = enbt.type_id();
                    switch (type_id.type) {
                        using enum enbt::type;
                    case none:
                        return boost::json::value();
                    case integer:
                        switch (type_id.length) {
                            using enum enbt::type_len;
                        case Tiny:
                            if (type_id.is_signed)
                                return boost::json::value((int8_t)enbt);
                            else
                                return boost::json::value((uint8_t)enbt);
                        case Short:
                            if (type_id.is_signed)
                                return boost::json::value((int16_t)enbt);
                            else
                                return boost::json::value((uint16_t)enbt);
                        case Default:
                            if (type_id.is_signed)
                                return boost::json::value((int32_t)enbt);
                            else
                                return boost::json::value((uint32_t)enbt);
                        case Long:
                            if (type_id.is_signed)
                                return boost::json::value((int64_t)enbt);
                            else
                                return boost::json::value((uint64_t)enbt);
                        default:
                            throw std::runtime_error("Unknown type");
                        }
                        break;
                    case floating:
                        switch (type_id.length) {
                            using enum enbt::type_len;
                        case Tiny:
                        case Short:
                        case Default:
                            return boost::json::value((float)enbt);
                        case Long:
                            return boost::json::value((double)enbt);
                        default:
                            throw std::runtime_error("Unknown type");
                        }
                    case var_integer:
                        switch (type_id.length) {
                            using enum enbt::type_len;
                        case Tiny:
                            if (type_id.is_signed)
                                return boost::json::value((int8_t)enbt);
                            else
                                return boost::json::value((uint8_t)enbt);
                        case Short:
                            if (type_id.is_signed)
                                return boost::json::value((int16_t)enbt);
                            else
                                return boost::json::value((uint16_t)enbt);
                        case Default:
                            if (type_id.is_signed)
                                return boost::json::value((int32_t)enbt);
                            else
                                return boost::json::value((uint32_t)enbt);
                        case Long:
                            if (type_id.is_signed)
                                return boost::json::value((int64_t)enbt);
                            else
                                return boost::json::value((uint64_t)enbt);
                        default:
                            throw std::runtime_error("Unknown type");
                        }
                    case uuid:
                        return boost::json::value(uuid::to((enbt::raw_uuid)enbt));
                    case sarray: {
                        boost::json::array result;
                        switch (type_id.length) {
                            using enum enbt::type_len;
                        case Tiny:
                            if (type_id.is_signed)
                                for (auto& item : enbt.as_i8_array())
                                    result.push_back(item);
                            else
                                for (auto& item : enbt.as_ui8_array())
                                    result.push_back(item);
                            break;
                        case Short:
                            if (type_id.is_signed)
                                for (auto& item : enbt.as_i16_array())
                                    result.push_back(item);
                            else
                                for (auto& item : enbt.as_ui16_array())
                                    result.push_back(item);
                            break;
                        case Default:
                            if (type_id.is_signed)
                                for (auto& item : enbt.as_i32_array())
                                    result.push_back(item);
                            else
                                for (auto& item : enbt.as_ui32_array())
                                    result.push_back(item);

                            break;
                        case Long:
                            if (type_id.is_signed)
                                for (auto& item : enbt.as_i32_array())
                                    result.push_back(item);
                            else
                                for (auto& item : enbt.as_ui32_array())
                                    result.push_back(item);
                            break;
                        }
                        return result;
                    }
                    case compound: {
                        boost::json::object result;
                        for (auto&& [key, value] : enbt.as_compound())
                            result[key] = to_json(value);
                        return result;
                    }
                    case darray: {
                        boost::json::array result;
                        for (auto& item : enbt.as_dyn_array())
                            result.push_back(to_json(item));
                        return result;
                    }
                    case array: {
                        boost::json::array result;
                        for (auto& item : enbt.as_fixed_array())
                            result.push_back(to_json(item));
                        return result;
                    }
                    case optional: {
                        if (enbt.contains())
                            return to_json(*enbt.get_optional());
                        else
                            return boost::json::value();
                    }
                    case bit:
                        return boost::json::value((bool)enbt);
                    case string:
                        return boost::json::value(string::to_transport((std::string)enbt));
                    case log_item:
                        return boost::json::value(to_json(enbt::from_log_item(enbt)));
                    default:
                        throw std::runtime_error("Unknown type");
                    }
                }

                enbt::value from_json(const boost::json::value& json) {
                    switch (json.kind()) {
                    case boost::json::kind::null:
                        return enbt::value();
                    case boost::json::kind::bool_:
                        return enbt::value(json.as_bool());
                    case boost::json::kind::int64:
                        return enbt::value(json.as_int64());
                    case boost::json::kind::uint64:
                        return enbt::value(json.as_uint64());
                    case boost::json::kind::double_:
                        return enbt::value(json.as_double());
                    case boost::json::kind::string:
                        return enbt::value(string::to_direct(json.as_string()));
                    case boost::json::kind::array: {
                        auto arr = json.as_array();
                        std::vector<enbt::value> result;
                        result.reserve(arr.size());
                        for (auto& item : arr)
                            result.push_back(from_json(item));
                        return enbt::dynamic_array(std::move(result));
                    }
                    case boost::json::kind::object: {
                        auto obj = json.as_object();
                        enbt::compound result;
                        for (auto& [key, value] : obj)
                            result[key] = from_json(value);
                        return result;
                    }
                    default:
                        throw std::runtime_error("Unknown type");
                    }
                }

                enbt::value from_json(const boost::json::object& obj) {
                    enbt::compound result;
                    for (auto& [key, value] : obj)
                        result[key] = from_json(value);
                    return result;
                }

                enbt::value from_json(const boost::json::array& arr) {
                    std::vector<enbt::value> result;
                    result.reserve(arr.size());
                    for (auto& item : arr)
                        result.push_back(from_json(item));
                    return enbt::dynamic_array(std::move(result));
                }
            }
        }
    }
}
