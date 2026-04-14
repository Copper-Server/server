/*
 * Copyright 2026-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/util/snbt_stream.hpp>
#include <src/util/nbt.hpp>
#include <src/base_objects/uuid.hpp>
#include <charconv>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <unordered_set>
#include <algorithm>

namespace copper_server::util {
    namespace snbt_detail {
        // === TOKENIZER IMPLEMENTATION ===

        snbt_tokenizer::snbt_tokenizer(std::string_view input)
            : input(input), pos(0) {}

        char snbt_tokenizer::current() const {
            return is_at_end() ? '\0' : input[pos];
        }

        char snbt_tokenizer::peek(size_t offset) const {
            size_t peek_pos = pos + offset;
            return peek_pos >= input.size() ? '\0' : input[peek_pos];
        }

        bool snbt_tokenizer::is_at_end() const {
            return pos >= input.size();
        }

        void snbt_tokenizer::advance() {
            if (!is_at_end())
                pos++;
        }

        void snbt_tokenizer::skip_whitespace() {
            while (!is_at_end() && std::isspace(current())) {
                advance();
            }
        }

        token snbt_tokenizer::next_token() {
            if (cached_token) {
                auto result = *cached_token;
                cached_token.reset();
                return result;
            }
            return read_token();
        }

        token snbt_tokenizer::peek_token() {
            if (!cached_token) {
                cached_token = read_token();
            }
            return *cached_token;
        }

        token snbt_tokenizer::read_string(char quote) {
            size_t start = pos;
            advance(); // skip opening quote

            std::string result;
            while (!is_at_end() && current() != quote) {
                if (current() == '\\') {
                    advance();
                    if (is_at_end())
                        throw std::runtime_error("Unterminated escape sequence in string");
                    result += current();
                    advance();
                } else {
                    result += current();
                    advance();
                }
            }

            if (is_at_end())
                throw std::runtime_error("Unterminated string literal");

            advance(); // skip closing quote
            return token{token_type::string, result, start};
        }

        token snbt_tokenizer::read_unquoted_string() {
            size_t start = pos;
            std::string result;

            while (!is_at_end() && (std::isalnum(current()) || current() == '_' || current() == '-' || current() == '.')) {
                result += current();
                advance();
            }

            return token{token_type::key, result, start};
        }

        token snbt_tokenizer::read_number() {
            size_t start = pos;
            std::string result;

            // Handle sign
            if (current() == '-' || current() == '+') {
                result += current();
                advance();
            }

            // Handle hex or binary prefix
            if (current() == '0' && !is_at_end()) {
                if (peek() == 'x' || peek() == 'X') {
                    result += current();
                    advance();
                    result += current();
                    advance();
                    // Read hex digits (with underscores)
                    while (!is_at_end() && (std::isxdigit(current()) || current() == '_')) {
                        result += current();
                        advance();
                    }
                } else if (peek() == 'b' || peek() == 'B') {
                    result += current();
                    advance();
                    result += current();
                    advance();
                    // Read binary digits (with underscores)
                    while (!is_at_end() && (current() == '0' || current() == '1' || current() == '_')) {
                        result += current();
                        advance();
                    }
                } else {
                    // Regular decimal
                    while (!is_at_end() && (std::isdigit(current()) || current() == '_')) {
                        result += current();
                        advance();
                    }
                }
            } else {
                // Regular decimal or floating point
                while (!is_at_end() && (std::isdigit(current()) || current() == '_')) {
                    result += current();
                    advance();
                }
            }

            // Handle decimal point
            if (!is_at_end() && current() == '.' && (std::isdigit(peek()) || peek() == '.')) {
                result += current();
                advance();
                while (!is_at_end() && (std::isdigit(current()) || current() == '_')) {
                    result += current();
                    advance();
                }
            }

            // Handle exponent (E notation)
            if (!is_at_end() && (current() == 'e' || current() == 'E')) {
                result += current();
                advance();
                if (!is_at_end() && (current() == '+' || current() == '-')) {
                    result += current();
                    advance();
                }
                while (!is_at_end() && (std::isdigit(current()) || current() == '_')) {
                    result += current();
                    advance();
                }
            }

            // Handle signedness and type suffixes
            if (!is_at_end() && (current() == 'u' || current() == 'U' || current() == 's' || current() == 'S')) {
                result += current();
                advance();
            }

            // Handle type suffix
            if (!is_at_end() && (current() == 'b' || current() == 'B' || current() == 's' || current() == 'S' ||
                                  current() == 'i' || current() == 'I' || current() == 'l' || current() == 'L' ||
                                  current() == 'f' || current() == 'F' || current() == 'd' || current() == 'D')) {
                result += current();
                advance();
            }

            return token{token_type::number, result, start};
        }

        token snbt_tokenizer::read_token() {
            skip_whitespace();

            if (is_at_end())
                return token{token_type::end_of_input, "", pos};

            char c = current();

            // Check for special characters
            if (c == '{') {
                advance();
                return token{token_type::open_brace, "{", pos - 1};
            }
            if (c == '}') {
                advance();
                return token{token_type::close_brace, "}", pos - 1};
            }
            if (c == '[') {
                advance();
                return token{token_type::open_bracket, "[", pos - 1};
            }
            if (c == ']') {
                advance();
                return token{token_type::close_bracket, "]", pos - 1};
            }
            if (c == ',') {
                advance();
                return token{token_type::comma, ",", pos - 1};
            }
            if (c == ':') {
                advance();
                return token{token_type::colon, ":", pos - 1};
            }
            if (c == ';') {
                advance();
                return token{token_type::semicolon, ";", pos - 1};
            }

            // Quoted strings
            if (c == '"' || c == '\'') {
                return read_string(c);
            }

            // Numbers
            if (std::isdigit(c) || c == '-' || c == '+' || (c == '.' && std::isdigit(peek()))) {
                return read_number();
            }

            // Unquoted strings / keys
            if (std::isalpha(c) || c == '_') {
                return read_unquoted_string();
            }

            return token{token_type::invalid, std::string(1, c), pos};
        }

        // ===== PARSER IMPLEMENTATION =====

        snbt_parser::snbt_parser(std::string_view input)
            : tokenizer(input) {
            advance_token();
        }

        void snbt_parser::advance_token() {
            current_token = tokenizer.next_token();
        }

        void snbt_parser::expect(token_type expected) {
            if (current_token.type != expected) {
                throw std::runtime_error("Unexpected token, expected different type");
            }
            advance_token();
        }

        std::optional<uint32_t> snbt_parser::parse_hex_code(std::string_view str, size_t digits) {
            if (str.size() < digits)
                return std::nullopt;
            
            uint32_t value = 0;
            for (size_t i = 0; i < digits; i++) {
                char c = str[i];
                int digit_val;
                if (c >= '0' && c <= '9')
                    digit_val = c - '0';
                else if (c >= 'a' && c <= 'f')
                    digit_val = c - 'a' + 10;
                else if (c >= 'A' && c <= 'F')
                    digit_val = c - 'A' + 10;
                else
                    return std::nullopt;
                value = (value << 4) | digit_val;
            }
            return value;
        }

        std::string snbt_parser::parse_unicode_escape(std::string_view str, size_t& i) {
            if (str[i] != '\\' || i + 1 >= str.size())
                return "";

            i++; // skip backslash
            char escape_char = str[i];

            if (escape_char == 'x') {
                if (i + 2 >= str.size())
                    throw std::runtime_error("Invalid \\x escape sequence");
                auto code = parse_hex_code(str.substr(i + 1), 2);
                if (!code)
                    throw std::runtime_error("Invalid \\x escape sequence");
                i += 3;
                return std::string(1, static_cast<char>(*code));
            } else if (escape_char == 'u') {
                if (i + 4 >= str.size())
                    throw std::runtime_error("Invalid \\u escape sequence");
                auto code = parse_hex_code(str.substr(i + 1), 4);
                if (!code)
                    throw std::runtime_error("Invalid \\u escape sequence");
                i += 5;
                // Convert to UTF-8
                if (*code < 0x80) {
                    return std::string(1, static_cast<char>(*code));
                } else if (*code < 0x800) {
                    return std::string{
                        static_cast<char>(0xC0 | ((*code >> 6) & 0x1F)),
                        static_cast<char>(0x80 | (*code & 0x3F))
                    };
                } else {
                    return std::string{
                        static_cast<char>(0xE0 | ((*code >> 12) & 0x0F)),
                        static_cast<char>(0x80 | ((*code >> 6) & 0x3F)),
                        static_cast<char>(0x80 | (*code & 0x3F))
                    };
                }
            } else if (escape_char == 'U') {
                if (i + 8 >= str.size())
                    throw std::runtime_error("Invalid \\U escape sequence");
                auto code = parse_hex_code(str.substr(i + 1), 8);
                if (!code)
                    throw std::runtime_error("Invalid \\U escape sequence");
                i += 9;
                // For simplicity, just return the character if it fits in a byte
                if (*code < 0x80)
                    return std::string(1, static_cast<char>(*code));
                throw std::runtime_error("\\U escape sequence out of basic ASCII range");
            }
            return "";
        }

        char snbt_parser::parse_escape_sequence(std::string_view str, size_t& i) {
            if (i >= str.size() || str[i] != '\\')
                return '\0';

            i++; // skip backslash
            if (i >= str.size())
                throw std::runtime_error("Unterminated escape sequence");

            char escape_char = str[i];
            i++;

            switch (escape_char) {
            case 'b':
                return '\b';
            case 'f':
                return '\f';
            case 'n':
                return '\n';
            case 'r':
                return '\r';
            case 't':
                return '\t';
            case 's':
                return ' ';
            case '\\':
                return '\\';
            case '\'':
                return '\'';
            case '"':
                return '"';
            default:
                throw std::runtime_error(std::string("Unknown escape sequence: \\") + escape_char);
            }
        }

        std::string snbt_parser::unescape_string(std::string_view escaped) {
            std::string result;
            size_t i = 0;

            while (i < escaped.size()) {
                if (escaped[i] == '\\') {
                    // Check for unicode escapes
                    if (i + 1 < escaped.size() && (escaped[i + 1] == 'x' || escaped[i + 1] == 'u' || escaped[i + 1] == 'U')) {
                        result += parse_unicode_escape(escaped, i);
                    } else {
                        result += parse_escape_sequence(escaped, i);
                    }
                } else {
                    result += escaped[i];
                    i++;
                }
            }

            return result;
        }

        nbt snbt_parser::parse_number(const std::string& number_str) {
            if (number_str.empty())
                throw std::runtime_error("Empty number");

            // Determine type and parse accordingly
            std::string num = number_str;
            char type_suffix = 0;
            bool is_unsigned = false;

            // Check for signedness suffix first
            if (num.size() >= 2) {
                if ((num[num.size() - 2] == 'u' || num[num.size() - 2] == 'U') &&
                    (num[num.size() - 1] == 'b' || num[num.size() - 1] == 'B' ||
                     num[num.size() - 1] == 's' || num[num.size() - 1] == 'S' ||
                     num[num.size() - 1] == 'i' || num[num.size() - 1] == 'I' ||
                     num[num.size() - 1] == 'l' || num[num.size() - 1] == 'L')) {
                    is_unsigned = true;
                    type_suffix = num[num.size() - 1];
                    num = num.substr(0, num.size() - 2);
                }
            }

            // Check for type suffix
            if (type_suffix == 0 && num.size() > 0) {
                char last = num.back();
                if (last == 'b' || last == 'B' || last == 's' || last == 'S' ||
                    last == 'i' || last == 'I' || last == 'l' || last == 'L' ||
                    last == 'f' || last == 'F' || last == 'd' || last == 'D') {
                    type_suffix = std::tolower(last);
                    num = num.substr(0, num.size() - 1);
                }
            }

            // Remove underscores
            num.erase(std::remove(num.begin(), num.end(), '_'), num.end());

            // Parse based on type suffix or content
            if (num.find_first_of(".eE") != std::string::npos) {
                // Floating point
                if (type_suffix == 'f' || type_suffix == 'd' || type_suffix == 0) {
                    double value;
                    auto result = std::from_chars(num.data(), num.data() + num.size(), value);
                    if (result.ec != std::errc())
                        throw std::runtime_error("Invalid floating point number");
                    
                    if (type_suffix == 'f')
                        return nbt(static_cast<float>(value));
                    else
                        return nbt(value);
                }
                throw std::runtime_error("Invalid floating point type suffix");
            }

            // Integer parsing
            int64_t int_value = 0;
            
            if (num.substr(0, 2) == "0x" || num.substr(0, 2) == "0X") {
                // Hexadecimal
                int_value = std::stoll(num, nullptr, 16);
            } else if (num.substr(0, 2) == "0b" || num.substr(0, 2) == "0B") {
                // Binary
                int_value = std::stoll(num.substr(2), nullptr, 2);
            } else {
                // Decimal
                try {
                    int_value = std::stoll(num);
                } catch (...) {
                    throw std::runtime_error("Invalid integer");
                }
            }

            // Convert to appropriate type and apply unsigned conversion if needed
            if (is_unsigned && int_value >= 0) {
                // For unsigned, we interpret the bit pattern as unsigned
                switch (type_suffix) {
                case 'b':
                    // Unsigned byte stored as signed byte
                    return nbt(static_cast<int8_t>(static_cast<uint8_t>(int_value)));
                case 's':
                    // Unsigned short stored as signed short
                    return nbt(static_cast<int16_t>(static_cast<uint16_t>(int_value)));
                case 'i':
                    // Unsigned int stored as signed int
                    return nbt(static_cast<int32_t>(static_cast<uint32_t>(int_value)));
                case 'l':
                    // Unsigned long stored as signed long
                    return nbt(static_cast<int64_t>(static_cast<uint64_t>(int_value)));
                default:
                    return nbt(static_cast<int32_t>(int_value));
                }
            }

            // Regular signed parsing
            switch (type_suffix) {
            case 'b':
                return nbt(static_cast<int8_t>(int_value));
            case 's':
                return nbt(static_cast<int16_t>(int_value));
            case 'i':
                return nbt(static_cast<int32_t>(int_value));
            case 'l':
                return nbt(static_cast<int64_t>(int_value));
            default:
                // Default to int if no suffix
                return nbt(static_cast<int32_t>(int_value));
            }
        }

        nbt snbt_parser::parse_compound() {
            expect(token_type::open_brace);

            std::unordered_map<std::string, nbt> compound_data;

            while (current_token.type != token_type::close_brace && current_token.type != token_type::end_of_input) {
                // Parse key
                std::string key;
                if (current_token.type == token_type::string) {
                    key = unescape_string(current_token.value);
                    advance_token();
                } else if (current_token.type == token_type::key) {
                    key = current_token.value;
                    advance_token();
                } else {
                    throw std::runtime_error("Expected key in compound");
                }

                expect(token_type::colon);

                // Parse value
                nbt value = parse_value();
                compound_data[key] = value;

                if (current_token.type == token_type::comma) {
                    advance_token();
                }
            }

            expect(token_type::close_brace);

            return nbt(std::move(compound_data));
        }

        nbt snbt_parser::parse_byte_array() {
            expect(token_type::open_bracket);
            expect(token_type::key); // Should be 'B'
            expect(token_type::semicolon);

            list_array<uint8_t> arr;

            while (current_token.type != token_type::close_bracket && current_token.type != token_type::end_of_input) {
                if (current_token.type == token_type::number) {
                    auto num = parse_number(current_token.value);
                    arr.push_back(num.as_byte());
                    advance_token();
                } else {
                    throw std::runtime_error("Expected number in byte array");
                }

                if (current_token.type == token_type::comma) {
                    advance_token();
                }
            }

            expect(token_type::close_bracket);

            return nbt(std::move(arr));
        }

        nbt snbt_parser::parse_int_array() {
            expect(token_type::open_bracket);
            expect(token_type::key); // Should be 'I'
            expect(token_type::semicolon);

            list_array<int32_t> arr;

            while (current_token.type != token_type::close_bracket && current_token.type != token_type::end_of_input) {
                if (current_token.type == token_type::number) {
                    auto num = parse_number(current_token.value);
                    arr.push_back(num.as_int());
                    advance_token();
                } else {
                    throw std::runtime_error("Expected number in int array");
                }

                if (current_token.type == token_type::comma) {
                    advance_token();
                }
            }

            expect(token_type::close_bracket);

            return nbt(std::move(arr));
        }

        nbt snbt_parser::parse_long_array() {
            expect(token_type::open_bracket);
            expect(token_type::key); // Should be 'L'
            expect(token_type::semicolon);

            list_array<int64_t> arr;

            while (current_token.type != token_type::close_bracket && current_token.type != token_type::end_of_input) {
                if (current_token.type == token_type::number) {
                    auto num = parse_number(current_token.value);
                    arr.push_back(num.as_long());
                    advance_token();
                } else {
                    throw std::runtime_error("Expected number in long array");
                }

                if (current_token.type == token_type::comma) {
                    advance_token();
                }
            }

            expect(token_type::close_bracket);

            return nbt(std::move(arr));
        }

        nbt snbt_parser::parse_list() {
            expect(token_type::open_bracket);

            // Check for typed arrays
            if (current_token.type == token_type::key) {
                if (current_token.value == "B") {
                    // This is actually a byte array, backtrack and parse differently
                    tokenizer.pos--;
                    return parse_byte_array();
                } else if (current_token.value == "I") {
                    tokenizer.pos--;
                    return parse_int_array();
                } else if (current_token.value == "L") {
                    tokenizer.pos--;
                    return parse_long_array();
                }
            }

            list_array<nbt> list_data;

            while (current_token.type != token_type::close_bracket && current_token.type != token_type::end_of_input) {
                nbt value = parse_value();
                list_data.push_back(value);

                if (current_token.type == token_type::comma) {
                    advance_token();
                }
            }

            expect(token_type::close_bracket);

            return nbt(std::move(list_data));
        }

        nbt snbt_parser::parse_value() {
            switch (current_token.type) {
            case token_type::open_brace:
                return parse_compound();

            case token_type::open_bracket: {
                // Peek ahead to determine if it's an array or list
                tokenizer.skip_whitespace();
                size_t saved_pos = tokenizer.pos;
                
                auto peek_next = tokenizer.next_token();
                tokenizer.pos = saved_pos;
                
                if (peek_next.type == token_type::key && 
                    (peek_next.value == "B" || peek_next.value == "I" || peek_next.value == "L")) {
                    char array_type = peek_next.value[0];
                    
                    if (array_type == 'B')
                        return parse_byte_array();
                    else if (array_type == 'I')
                        return parse_int_array();
                    else
                        return parse_long_array();
                }
                
                return parse_list();
            }

            case token_type::string: {
                std::string str = unescape_string(current_token.value);
                advance_token();
                return nbt(str);
            }

            case token_type::number: {
                std::string num = current_token.value;
                advance_token();
                return parse_number(num);
            }

            case token_type::key: {
                std::string key = current_token.value;
                
                // Check for operations
                if (key == "true") {
                    advance_token();
                    return nbt(static_cast<int8_t>(1));
                } else if (key == "false") {
                    advance_token();
                    return nbt(static_cast<int8_t>(0));
                } else if (key == "bool") {
                    advance_token();
                    expect(token_type::open_bracket);
                    nbt arg = parse_value();
                    expect(token_type::close_bracket);
                    
                    if (arg.is_byte()) {
                        return nbt(static_cast<int8_t>(arg.get_byte() ? 1 : 0));
                    } else if (arg.is_numeric()) {
                        int64_t val = arg.as_long();
                        return nbt(static_cast<int8_t>(val != 0 ? 1 : 0));
                    } else {
                        throw std::runtime_error("bool() requires a numeric or boolean argument");
                    }
                } else if (key == "uuid") {
                    advance_token();
                    expect(token_type::open_bracket);
                    
                    if (current_token.type != token_type::string) {
                        throw std::runtime_error("uuid() requires a string argument");
                    }
                    
                    std::string uuid_str = unescape_string(current_token.value);
                    advance_token();
                    expect(token_type::close_bracket);
                    
                    // Convert UUID string to int array
                    base_objects::uuid uuid_val;
                    base_objects::uuid::from_uuid_string(uuid_val, uuid_str, true);
                    
                    return nbt(uuid_val);
                } else {
                    // Regular unquoted string
                    advance_token();
                    return nbt(key);
                }
            }

            default:
                throw std::runtime_error("Unexpected token in parse_value");
            }
        }

        nbt snbt_parser::parse() {
            return parse_value();
        }
    }

    // Public API implementation
    nbt parse_snbt(std::string_view snbt_string) {
        snbt_detail::snbt_parser parser(snbt_string);
        return parser.parse();
    }

    // ============================================================================
    // SNBT Read Stream Implementation
    // ============================================================================

    snbt_read_stream::snbt_read_stream(std::string_view input)
        : tokenizer(input), detected_type(snbt_tag_type::tag_end) {
        advance_token();
    }

    snbt_read_stream::~snbt_read_stream() = default;

    void snbt_read_stream::advance_token() {
        current_token = tokenizer.next_token();
    }

    void snbt_read_stream::detect_value_type() {
        if (readed) return;
        detect_type_from_token(current_token);
        readed = true;
    }

    snbt_tag_type snbt_read_stream::detect_type_from_token(const snbt_detail::token& tok) {
        switch (tok.type) {
            case snbt_detail::token_type::open_brace:
                return detected_type = snbt_tag_type::tag_compound;
            case snbt_detail::token_type::open_bracket:
                return detected_type = snbt_tag_type::tag_list;
            case snbt_detail::token_type::string:
                return detected_type = snbt_tag_type::tag_string;
            case snbt_detail::token_type::number: {
                const auto& val = tok.value;
                if (val.empty()) return detected_type = snbt_tag_type::tag_end;
                
                char last = val.back();
                if (last == 'b' || last == 'B') return detected_type = snbt_tag_type::tag_byte;
                if (last == 's' || last == 'S') return detected_type = snbt_tag_type::tag_short;
                if (last == 'l' || last == 'L') return detected_type = snbt_tag_type::tag_long;
                if (last == 'f' || last == 'F') return detected_type = snbt_tag_type::tag_float;
                if (last == 'd' || last == 'D') return detected_type = snbt_tag_type::tag_double;
                
                if (val.find('.') != std::string::npos || val.find('e') != std::string::npos || val.find('E') != std::string::npos) {
                    return detected_type = snbt_tag_type::tag_double;
                }
                return detected_type = snbt_tag_type::tag_int;
            }
            default:
                return detected_type = snbt_tag_type::tag_end;
        }
    }

    snbt_read_stream& snbt_read_stream::read_into(bool& res) {
        detect_value_type();
        if (current_token.type == snbt_detail::token_type::string && 
            (current_token.value == "true" || current_token.value == "false")) {
            res = (current_token.value == "true");
        } else {
            throw std::runtime_error("Expected boolean value");
        }
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(uint8_t& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = static_cast<uint8_t>(std::stoll(current_token.value));
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(uint16_t& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = static_cast<uint16_t>(std::stoll(current_token.value));
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(uint32_t& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = static_cast<uint32_t>(std::stoll(current_token.value));
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(uint64_t& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = std::stoull(current_token.value);
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(int8_t& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = static_cast<int8_t>(std::stoll(current_token.value));
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(int16_t& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = static_cast<int16_t>(std::stoll(current_token.value));
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(int32_t& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = static_cast<int32_t>(std::stoll(current_token.value));
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(int64_t& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = std::stoll(current_token.value);
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(float& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = std::stof(current_token.value);
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(double& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::number)
            throw std::runtime_error("Expected number");
        res = std::stod(current_token.value);
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(std::string& res) {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::string && 
            current_token.type != snbt_detail::token_type::key) {
            throw std::runtime_error("Expected string");
        }
        res = current_token.value;
        advance_token();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(nbt_convert& res) {
        res = nbt_convert::build(parse_to_nbt());
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(nbt& res) {
        res = parse_to_nbt();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(nbt_compound& res) {
        res = parse_to_nbt();
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(base_objects::uuid& res) {
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(base_objects::uuid_hex& res) {
        std::string str;
        read_into(str);
        base_objects::uuid::from_uuid_string(res, str, true);
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_into(base_objects::uuid_flat_hex& res) {
        std::string str;
        read_into(str);
        base_objects::uuid::from_uuid_string(res, str, true);
        return *this;
    }

    snbt_read_stream& snbt_read_stream::read_as(bool& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(uint8_t& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(uint16_t& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(uint32_t& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(uint64_t& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(int8_t& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(int16_t& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(int32_t& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(int64_t& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(float& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(double& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(std::string& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(nbt_convert& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(nbt& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(nbt_compound& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(base_objects::uuid& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(base_objects::uuid_hex& res) { return read_into(res); }
    snbt_read_stream& snbt_read_stream::read_as(base_objects::uuid_flat_hex& res) { return read_into(res); }

    void snbt_read_stream::skip() {
        detect_value_type();
        if (detected_type == snbt_tag_type::tag_compound) {
            auto comp = read_compound();
        } else if (detected_type == snbt_tag_type::tag_list) {
            auto list = read_list();
        } else {
            advance_token();
        }
    }

    snbt_tag_type snbt_read_stream::get_type() const {
        return detected_type;
    }

    snbt_read_compound_stream snbt_read_stream::read_compound() {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::open_brace)
            throw std::runtime_error("Expected '{'");
        advance_token();
        return snbt_read_compound_stream(std::move(tokenizer), false);
    }

    snbt_read_list_stream snbt_read_stream::read_list() {
        detect_value_type();
        if (current_token.type != snbt_detail::token_type::open_bracket)
            throw std::runtime_error("Expected '['");
        advance_token();
        return snbt_read_list_stream(std::move(tokenizer));
    }

    nbt snbt_read_stream::parse_to_nbt() {
        snbt_detail::snbt_parser parser(std::string(tokenizer.input.substr(tokenizer.pos)));
        return parser.parse();
    }

    template <class SIZE_FN, class FN>
    void snbt_read_stream::iterate(SIZE_FN&& size_callback, FN&& callback) {
        detect_value_type();
        if (detected_type == snbt_tag_type::tag_compound) {
            auto comp = read_compound();
            comp.iterate(std::forward<FN>(callback));
        } else if (detected_type == snbt_tag_type::tag_list) {
            auto list = read_list();
            list.iterable(std::forward<FN>(callback));
        }
    }

    // ============================================================================
    // SNBT Compound Stream Implementation
    // ============================================================================

    snbt_read_compound_stream::snbt_read_compound_stream(snbt_detail::snbt_tokenizer&& tok, bool strict)
        : tokenizer(std::move(tok)), reached_end(false), enable_collector_strict_order(strict) {
        advance_token();
    }

    snbt_read_compound_stream::~snbt_read_compound_stream() = default;

    void snbt_read_compound_stream::advance_token() {
        current_token = tokenizer.next_token();
    }

    bool snbt_read_compound_stream::is_reached_end() const noexcept {
        return reached_end;
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, bool& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, uint8_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, uint16_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, uint32_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, uint64_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, int8_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, int16_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, int32_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, int64_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, float& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, double& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, std::string& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, nbt_convert& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }
    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, nbt& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, nbt_compound& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, base_objects::uuid& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, base_objects::uuid_hex& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_into(const std::string& name, base_objects::uuid_flat_hex& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, bool& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, uint8_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, uint16_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, uint32_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, uint64_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, int8_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, int16_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, int32_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, int64_t& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, float& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, double& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, std::string& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, nbt_convert& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }
    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, nbt& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, nbt_compound& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, base_objects::uuid& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, base_objects::uuid_hex& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::collect_as(const std::string& name, base_objects::uuid_flat_hex& res) {
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });
    }

    snbt_read_compound_stream& snbt_read_compound_stream::make_collect() {
        return make_collect([](auto&, auto&) {});
    }

    snbt_read_compound_stream& snbt_read_compound_stream::force_all_collect() {
        std::unordered_set<std::string> collected;
        iterate([this, &collected](auto& name, auto& stream) {
            if (auto it = automated_collector.find(name); it != automated_collector.end()) {
                it->second(stream);
                collected.insert(name);
            } else {
                throw std::runtime_error("Uncollected: " + name);
            }
        });
        for (auto& [name, _] : automated_collector) {
            if (!collected.count(name))
                throw std::runtime_error("Missing: " + name);
        }
        return *this;
    }

    // ============================================================================
    // SNBT List Stream Implementation
    // ============================================================================

    snbt_read_list_stream::snbt_read_list_stream(snbt_detail::snbt_tokenizer&& tok)
        : tokenizer(std::move(tok)), current_item(0), items(0) {
        advance_token();
    }

    snbt_read_list_stream::~snbt_read_list_stream() = default;

    void snbt_read_list_stream::advance_token() {
        current_token = tokenizer.next_token();
    }

    snbt_tag_type snbt_read_list_stream::get_items_type() const {
        return items_type;
    }

    int32_t snbt_read_list_stream::size() const noexcept {
        return items;
    }

    int32_t snbt_read_list_stream::current_index() const noexcept {
        return current_item;
    }

    #define SNBT_READ_LIST_IMPL(TypeName) \
    snbt_read_list_stream& snbt_read_list_stream::read_one_into(TypeName& res) { \
        snbt_read_stream reader(std::string(tokenizer.get_remaining_input())); \
        reader.read_into(res); \
        current_item++; \
        return *this; \
    }

    SNBT_READ_LIST_IMPL(bool)
    SNBT_READ_LIST_IMPL(uint8_t)
    SNBT_READ_LIST_IMPL(uint16_t)
    SNBT_READ_LIST_IMPL(uint32_t)
    SNBT_READ_LIST_IMPL(uint64_t)
    SNBT_READ_LIST_IMPL(int8_t)
    SNBT_READ_LIST_IMPL(int16_t)
    SNBT_READ_LIST_IMPL(int32_t)
    SNBT_READ_LIST_IMPL(int64_t)
    SNBT_READ_LIST_IMPL(float)
    SNBT_READ_LIST_IMPL(double)
    SNBT_READ_LIST_IMPL(std::string)
    SNBT_READ_LIST_IMPL(nbt)
    SNBT_READ_LIST_IMPL(base_objects::uuid)
    SNBT_READ_LIST_IMPL(base_objects::uuid_hex)
    SNBT_READ_LIST_IMPL(base_objects::uuid_flat_hex)

    #undef SNBT_READ_LIST_IMPL

    #define SNBT_READ_LIST_AS_IMPL(TypeName) \
    snbt_read_list_stream& snbt_read_list_stream::read_one_as(TypeName& res) { \
        return read_one_into(res); \
    }

    SNBT_READ_LIST_AS_IMPL(bool)
    SNBT_READ_LIST_AS_IMPL(uint8_t)
    SNBT_READ_LIST_AS_IMPL(uint16_t)
    SNBT_READ_LIST_AS_IMPL(uint32_t)
    SNBT_READ_LIST_AS_IMPL(uint64_t)
    SNBT_READ_LIST_AS_IMPL(int8_t)
    SNBT_READ_LIST_AS_IMPL(int16_t)
    SNBT_READ_LIST_AS_IMPL(int32_t)
    SNBT_READ_LIST_AS_IMPL(int64_t)
    SNBT_READ_LIST_AS_IMPL(float)
    SNBT_READ_LIST_AS_IMPL(double)
    SNBT_READ_LIST_AS_IMPL(std::string)
    SNBT_READ_LIST_AS_IMPL(nbt)
    SNBT_READ_LIST_AS_IMPL(base_objects::uuid)
    SNBT_READ_LIST_AS_IMPL(base_objects::uuid_hex)
    SNBT_READ_LIST_AS_IMPL(base_objects::uuid_flat_hex)

    #undef SNBT_READ_LIST_AS_IMPL

    // ============================================================================
    // SNBT Write Stream Implementation
    // ============================================================================

    snbt_write_stream::snbt_write_stream(bool pretty_print) : pretty_print(pretty_print) {}

    snbt_write_stream::~snbt_write_stream() = default;

    void snbt_write_stream::write_indent() {
        if (pretty_print) output.append(depth * 2, ' ');
    }

    void snbt_write_stream::write_escaped_string(std::string_view str) {
        output.push_back('"');
        for (char c : str) {
            switch (c) {
                case '"': output.append("\\\""); break;
                case '\\': output.append("\\\\"); break;
                case '\n': output.append("\\n"); break;
                case '\r': output.append("\\r"); break;
                case '\t': output.append("\\t"); break;
                default: output.push_back(c);
            }
        }
        output.push_back('"');
    }

    void snbt_write_stream::write(bool res) { output.append(res ? "true" : "false"); }
    void snbt_write_stream::write(uint8_t res) { output.append(std::to_string(res)).push_back('b'); }
    void snbt_write_stream::write(uint16_t res) { output.append(std::to_string(res)).push_back('s'); }
    void snbt_write_stream::write(uint32_t res) { output.append(std::to_string(res)); }
    void snbt_write_stream::write(uint64_t res) { output.append(std::to_string(res)).push_back('l'); }
    void snbt_write_stream::write(int8_t res) { output.append(std::to_string(res)).push_back('b'); }
    void snbt_write_stream::write(int16_t res) { output.append(std::to_string(res)).push_back('s'); }
    void snbt_write_stream::write(int32_t res) { output.append(std::to_string(res)); }
    void snbt_write_stream::write(int64_t res) { output.append(std::to_string(res)).push_back('l'); }
    void snbt_write_stream::write(float res) { output.append(std::to_string(res)).push_back('f'); }
    void snbt_write_stream::write(double res) { output.append(std::to_string(res)).push_back('d'); }
    void snbt_write_stream::write(const std::string& res) { write_escaped_string(res); }
    void snbt_write_stream::write(std::string_view res) { write_escaped_string(res); }
    void snbt_write_stream::write(const nbt_convert& res) { output.append(res.to_snbt()); }
    void snbt_write_stream::write(const nbt& res) { output.append(res.as_snbt()); }
    void snbt_write_stream::write(const nbt_compound& res) { output.append(((nbt)res).as_snbt()); }

    void snbt_write_stream::write_array(const int8_t* arr, size_t size) {
        output.append("[B;");
        for (size_t i = 0; i < size; ++i) {
            if (i > 0) output.push_back(',');
            output.append(std::to_string(arr[i])).push_back('b');
        }
        output.push_back(']');
    }

    void snbt_write_stream::write_array(const uint8_t* arr, size_t size) {
        output.append("[B;");
        for (size_t i = 0; i < size; ++i) {
            if (i > 0) output.push_back(',');
            output.append(std::to_string(arr[i])).push_back('b');
        }
        output.push_back(']');
    }

    void snbt_write_stream::write_array(const int32_t* arr, size_t size) {
        output.append("[I;");
        for (size_t i = 0; i < size; ++i) {
            if (i > 0) output.push_back(',');
            output.append(std::to_string(arr[i]));
        }
        output.push_back(']');
    }

    void snbt_write_stream::write_array(const int64_t* arr, size_t size) {
        output.append("[L;");
        for (size_t i = 0; i < size; ++i) {
            if (i > 0) output.push_back(',');
            output.append(std::to_string(arr[i])).push_back('l');
        }
        output.push_back(']');
    }

    std::string snbt_write_stream::get_output() const {
        return output;
    }

    std::string snbt_write_stream::take_output() {
        return std::move(output);
    }

    snbt_write_compound_stream snbt_write_stream::write_compound() {
        output.push_back('{');
        return snbt_write_compound_stream(output, depth + 1, pretty_print);
    }

    snbt_write_list_stream snbt_write_stream::write_list() {
        output.push_back('[');
        return snbt_write_list_stream(output, depth + 1, pretty_print);
    }

    // ============================================================================
    // SNBT List Stream Implementation
    // ============================================================================

    snbt_write_list_stream::snbt_write_list_stream(std::string& output, uint16_t depth, bool pretty_print)
        : output(output), depth(depth), pretty_print(pretty_print), first_item(true) {}

    snbt_write_list_stream::~snbt_write_list_stream() {
        if (pretty_print && !first_item)
            write_indent();
        output.push_back(']');
    }

    void snbt_write_list_stream::write_indent() {
        if (pretty_print) {
            output.push_back('\n');
            for (uint16_t i = 0; i < depth; ++i)
                output.append("  ");
        }
    }

    void snbt_write_list_stream::write_escaped_string(std::string_view str) {
        output.push_back('"');
        for (char c : str) {
            switch (c) {
                case '"': output.append("\\\""); break;
                case '\\': output.append("\\\\"); break;
                case '\n': output.append("\\n"); break;
                case '\r': output.append("\\r"); break;
                case '\t': output.append("\\t"); break;
                default: output.push_back(c); break;
            }
        }
        output.push_back('"');
    }

#define SNBT_WRITE_LIST_IMPL(TypeName, Suffix) \
    snbt_write_list_stream& snbt_write_list_stream::write(TypeName res) { \
        if (!first_item) output.push_back(','); \
        if (pretty_print && !first_item) write_indent(); \
        first_item = false; \
        if constexpr (std::is_same_v<TypeName, bool>) { \
            output.append(res ? "true" : "false"); \
        } else { \
            output.append(std::to_string(res)); \
            if constexpr (!std::is_same_v<TypeName, uint32_t> && !std::is_same_v<TypeName, int32_t> && !std::is_same_v<TypeName, float> && !std::is_same_v<TypeName, double>) { \
                output.push_back(Suffix); \
            } else if constexpr (std::is_same_v<TypeName, float>) { \
                output.push_back('f'); \
            } else if constexpr (std::is_same_v<TypeName, double>) { \
                output.push_back('d'); \
            } \
        } \
        return *this; \
    }

    snbt_write_list_stream& snbt_write_list_stream::write(bool res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(res ? "true" : "false");
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(uint8_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res)).push_back('b');
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(uint16_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res)).push_back('s');
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(uint32_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res));
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(uint64_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res)).push_back('l');
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(int8_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res)).push_back('b');
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(int16_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res)).push_back('s');
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(int32_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res));
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(int64_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res)).push_back('l');
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(float res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res)).push_back('f');
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(double res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(std::to_string(res)).push_back('d');
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(const std::string& res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        write_escaped_string(res);
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(std::string_view res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        write_escaped_string(res);
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(const nbt_convert& res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(res.to_snbt());
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(const nbt& res) {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.append(res.as_snbt());
        return *this;
    }

    snbt_write_list_stream& snbt_write_list_stream::write(const nbt_compound& res) {
        return write((nbt)res);
    }

    snbt_write_compound_stream snbt_write_list_stream::write_compound() {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.push_back('{');
        return snbt_write_compound_stream(output, depth + 1, pretty_print);
    }

    snbt_write_list_stream snbt_write_list_stream::write_list() {
        if (!first_item) output.push_back(',');
        if (pretty_print && !first_item) write_indent();
        first_item = false;
        output.push_back('[');
        return snbt_write_list_stream(output, depth + 1, pretty_print);
    }

    // ============================================================================
    // SNBT Compound Stream Implementation
    // ============================================================================

    snbt_write_compound_stream::snbt_write_compound_stream(std::string& output, uint16_t depth, bool pretty_print)
        : output(output), depth(depth), pretty_print(pretty_print), first_item(true) {}

    snbt_write_compound_stream::~snbt_write_compound_stream() {
        if (pretty_print && !first_item)
            write_indent();
        output.push_back('}');
    }

    void snbt_write_compound_stream::write_indent() {
        if (pretty_print) {
            output.push_back('\n');
            for (uint16_t i = 0; i < depth; ++i)
                output.append("  ");
        }
    }

    void snbt_write_compound_stream::write_escaped_string(std::string_view str) {
        output.push_back('"');
        for (char c : str) {
            switch (c) {
                case '"': output.append("\\\""); break;
                case '\\': output.append("\\\\"); break;
                case '\n': output.append("\\n"); break;
                case '\r': output.append("\\r"); break;
                case '\t': output.append("\\t"); break;
                default: output.push_back(c); break;
            }
        }
        output.push_back('"');
    }

    void snbt_write_compound_stream::write_key(std::string_view key) {
        write_escaped_string(key);
        output.push_back(':');
        if (pretty_print)
            output.push_back(' ');
    }

#define SNBT_WRITE_COMPOUND_IMPL(TypeName) \
    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, TypeName res) { \
        if (!first_item) output.push_back(','); \
        if (pretty_print) write_indent(); \
        first_item = false; \
        write_key(key); \
        if constexpr (std::is_same_v<TypeName, bool>) { \
            output.append(res ? "true" : "false"); \
        } else if constexpr (std::is_same_v<TypeName, uint8_t>) { \
            output.append(std::to_string(res)).push_back('b'); \
        } else if constexpr (std::is_same_v<TypeName, uint16_t>) { \
            output.append(std::to_string(res)).push_back('s'); \
        } else if constexpr (std::is_same_v<TypeName, uint32_t>) { \
            output.append(std::to_string(res)); \
        } else if constexpr (std::is_same_v<TypeName, uint64_t>) { \
            output.append(std::to_string(res)).push_back('l'); \
        } else if constexpr (std::is_same_v<TypeName, int8_t>) { \
            output.append(std::to_string(res)).push_back('b'); \
        } else if constexpr (std::is_same_v<TypeName, int16_t>) { \
            output.append(std::to_string(res)).push_back('s'); \
        } else if constexpr (std::is_same_v<TypeName, int32_t>) { \
            output.append(std::to_string(res)); \
        } else if constexpr (std::is_same_v<TypeName, int64_t>) { \
            output.append(std::to_string(res)).push_back('l'); \
        } else if constexpr (std::is_same_v<TypeName, float>) { \
            output.append(std::to_string(res)).push_back('f'); \
        } else if constexpr (std::is_same_v<TypeName, double>) { \
            output.append(std::to_string(res)).push_back('d'); \
        } \
        return *this; \
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, bool res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(res ? "true" : "false");
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, uint8_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res)).push_back('b');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, uint16_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res)).push_back('s');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, uint32_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res));
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, uint64_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res)).push_back('l');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, int8_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res)).push_back('b');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, int16_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res)).push_back('s');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, int32_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res));
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, int64_t res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res)).push_back('l');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, float res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res)).push_back('f');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, double res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(std::to_string(res)).push_back('d');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, const std::string& res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        write_escaped_string(res);
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, std::string_view res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        write_escaped_string(res);
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, const nbt_convert& res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(res.to_snbt());
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, const nbt& res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.append(res.as_snbt());
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, const nbt_compound& res) {
        return write(key, (nbt)res);
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, base_objects::uuid res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        write_escaped_string(res.to_string());
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, base_objects::uuid_hex res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        write_escaped_string(res.to_string());
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write(std::string_view key, base_objects::uuid_flat_hex res) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        write_escaped_string(res.to_string_flat());
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write_compound(std::string_view key) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.push_back('{');
        return *this;
    }

    snbt_write_compound_stream& snbt_write_compound_stream::write_list(std::string_view key) {
        if (!first_item) output.push_back(',');
        if (pretty_print) write_indent();
        first_item = false;
        write_key(key);
        output.push_back('[');
        return *this;
    }

    // ============================================================================
    // SNBT Collection Implementation
    // ============================================================================

    namespace snbt_collection {
#define SNBT_COLLECT_INTO_IMPL_RELAXED(TypeName)                                               \
    compound_relaxed& compound_relaxed::collect_into(const std::string& name, TypeName& res) { \
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });     \
    }

        SNBT_COLLECT_INTO_IMPL_RELAXED(bool)
        SNBT_COLLECT_INTO_IMPL_RELAXED(uint8_t)
        SNBT_COLLECT_INTO_IMPL_RELAXED(uint16_t)
        SNBT_COLLECT_INTO_IMPL_RELAXED(uint32_t)
        SNBT_COLLECT_INTO_IMPL_RELAXED(uint64_t)
        SNBT_COLLECT_INTO_IMPL_RELAXED(int8_t)
        SNBT_COLLECT_INTO_IMPL_RELAXED(int16_t)
        SNBT_COLLECT_INTO_IMPL_RELAXED(int32_t)
        SNBT_COLLECT_INTO_IMPL_RELAXED(int64_t)
        SNBT_COLLECT_INTO_IMPL_RELAXED(float)
        SNBT_COLLECT_INTO_IMPL_RELAXED(double)
        SNBT_COLLECT_INTO_IMPL_RELAXED(std::string)
        SNBT_COLLECT_INTO_IMPL_RELAXED(nbt_convert)
        SNBT_COLLECT_INTO_IMPL_RELAXED(nbt)
        SNBT_COLLECT_INTO_IMPL_RELAXED(nbt_compound)
        SNBT_COLLECT_INTO_IMPL_RELAXED(base_objects::uuid)
        SNBT_COLLECT_INTO_IMPL_RELAXED(base_objects::uuid_hex)
        SNBT_COLLECT_INTO_IMPL_RELAXED(base_objects::uuid_flat_hex)

#undef SNBT_COLLECT_INTO_IMPL_RELAXED

#define SNBT_COLLECT_AS_IMPL_RELAXED(TypeName)                                               \
    compound_relaxed& compound_relaxed::collect_as(const std::string& name, TypeName& res) { \
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });     \
    }

        SNBT_COLLECT_AS_IMPL_RELAXED(bool)
        SNBT_COLLECT_AS_IMPL_RELAXED(uint8_t)
        SNBT_COLLECT_AS_IMPL_RELAXED(uint16_t)
        SNBT_COLLECT_AS_IMPL_RELAXED(uint32_t)
        SNBT_COLLECT_AS_IMPL_RELAXED(uint64_t)
        SNBT_COLLECT_AS_IMPL_RELAXED(int8_t)
        SNBT_COLLECT_AS_IMPL_RELAXED(int16_t)
        SNBT_COLLECT_AS_IMPL_RELAXED(int32_t)
        SNBT_COLLECT_AS_IMPL_RELAXED(int64_t)
        SNBT_COLLECT_AS_IMPL_RELAXED(float)
        SNBT_COLLECT_AS_IMPL_RELAXED(double)
        SNBT_COLLECT_AS_IMPL_RELAXED(std::string)
        SNBT_COLLECT_AS_IMPL_RELAXED(nbt_convert)
        SNBT_COLLECT_AS_IMPL_RELAXED(nbt)
        SNBT_COLLECT_AS_IMPL_RELAXED(nbt_compound)
        SNBT_COLLECT_AS_IMPL_RELAXED(base_objects::uuid)
        SNBT_COLLECT_AS_IMPL_RELAXED(base_objects::uuid_hex)
        SNBT_COLLECT_AS_IMPL_RELAXED(base_objects::uuid_flat_hex)

#undef SNBT_COLLECT_AS_IMPL_RELAXED

        compound_relaxed& compound_relaxed::force_all_collect(snbt_read_stream& stream) {
            std::unordered_set<std::string> collected;
            stream.iterate([this, &collected](auto& name, auto& item_stream) {
                if (auto it = automated_collector.find(name); it != automated_collector.end()) {
                    it->second(item_stream);
                    collected.insert(name);
                } else {
                    throw std::runtime_error("Uncollected: " + name);
                }
            });
            for (auto& [name, _] : automated_collector) {
                if (!collected.count(name))
                    throw std::runtime_error("Missing: " + name);
            }
            return *this;
        }

        // compound_strict stubs (can be completed similarly)
        compound_strict& compound_strict::collect_into(const std::string& name, bool& res) {
            return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });
        }

        compound_strict& compound_strict::make_collect(snbt_read_stream& stream) {
            return make_collect(stream, [](auto&, auto&) {});
        }

        compound_strict& compound_strict::force_all_collect(snbt_read_stream& stream) {
            std::unordered_set<std::string> collected;
            stream.iterate([this, &collected](auto& name, auto& item_stream) {
                if (auto it = automated_collector.find(name); it != automated_collector.end()) {
                    it->second(item_stream);
                    collected.insert(name);
                }
            });
            for (auto& name : collector_strict_order_data) {
                if (!collected.count(name))
                    throw std::runtime_error("Missing strict: " + name);
            }
            return *this;
        }

#define SNBT_COLLECT_INTO_IMPL_STRICT(TypeName)                                              \
    compound_strict& compound_strict::collect_into(const std::string& name, TypeName& res) { \
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); });   \
    }

        SNBT_COLLECT_INTO_IMPL_STRICT(bool)
        SNBT_COLLECT_INTO_IMPL_STRICT(uint8_t)
        SNBT_COLLECT_INTO_IMPL_STRICT(uint16_t)
        SNBT_COLLECT_INTO_IMPL_STRICT(uint32_t)
        SNBT_COLLECT_INTO_IMPL_STRICT(uint64_t)
        SNBT_COLLECT_INTO_IMPL_STRICT(int8_t)
        SNBT_COLLECT_INTO_IMPL_STRICT(int16_t)
        SNBT_COLLECT_INTO_IMPL_STRICT(int32_t)
        SNBT_COLLECT_INTO_IMPL_STRICT(int64_t)
        SNBT_COLLECT_INTO_IMPL_STRICT(float)
        SNBT_COLLECT_INTO_IMPL_STRICT(double)
        SNBT_COLLECT_INTO_IMPL_STRICT(std::string)
        SNBT_COLLECT_INTO_IMPL_STRICT(nbt_convert)
        SNBT_COLLECT_INTO_IMPL_STRICT(nbt)
        SNBT_COLLECT_INTO_IMPL_STRICT(nbt_compound)
        SNBT_COLLECT_INTO_IMPL_STRICT(base_objects::uuid)
        SNBT_COLLECT_INTO_IMPL_STRICT(base_objects::uuid_hex)
        SNBT_COLLECT_INTO_IMPL_STRICT(base_objects::uuid_flat_hex)

#undef SNBT_COLLECT_INTO_IMPL_STRICT

#define SNBT_COLLECT_AS_IMPL_STRICT(TypeName)                                             \
    compound_strict& compound_strict::collect_as(const std::string& name, TypeName& res) { \
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); });   \
    }

        SNBT_COLLECT_AS_IMPL_STRICT(bool)
        SNBT_COLLECT_AS_IMPL_STRICT(uint8_t)
        SNBT_COLLECT_AS_IMPL_STRICT(uint16_t)
        SNBT_COLLECT_AS_IMPL_STRICT(uint32_t)
        SNBT_COLLECT_AS_IMPL_STRICT(uint64_t)
        SNBT_COLLECT_AS_IMPL_STRICT(int8_t)
        SNBT_COLLECT_AS_IMPL_STRICT(int16_t)
        SNBT_COLLECT_AS_IMPL_STRICT(int32_t)
        SNBT_COLLECT_AS_IMPL_STRICT(int64_t)
        SNBT_COLLECT_AS_IMPL_STRICT(float)
        SNBT_COLLECT_AS_IMPL_STRICT(double)
        SNBT_COLLECT_AS_IMPL_STRICT(std::string)
        SNBT_COLLECT_AS_IMPL_STRICT(nbt_convert)
        SNBT_COLLECT_AS_IMPL_STRICT(nbt)
        SNBT_COLLECT_AS_IMPL_STRICT(nbt_compound)
        SNBT_COLLECT_AS_IMPL_STRICT(base_objects::uuid)
        SNBT_COLLECT_AS_IMPL_STRICT(base_objects::uuid_hex)
        SNBT_COLLECT_AS_IMPL_STRICT(base_objects::uuid_flat_hex)

#undef SNBT_COLLECT_AS_IMPL_STRICT

#define SNBT_COLLECT_INTO_IMPL_FLEX(TypeName)                                                   \
    compound_flex& compound_flex::collect_into(const std::string& name, TypeName& res) {   \
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_into(res); }); \
    }

        SNBT_COLLECT_INTO_IMPL_FLEX(bool)
        SNBT_COLLECT_INTO_IMPL_FLEX(uint8_t)
        SNBT_COLLECT_INTO_IMPL_FLEX(uint16_t)
        SNBT_COLLECT_INTO_IMPL_FLEX(uint32_t)
        SNBT_COLLECT_INTO_IMPL_FLEX(uint64_t)
        SNBT_COLLECT_INTO_IMPL_FLEX(int8_t)
        SNBT_COLLECT_INTO_IMPL_FLEX(int16_t)
        SNBT_COLLECT_INTO_IMPL_FLEX(int32_t)
        SNBT_COLLECT_INTO_IMPL_FLEX(int64_t)
        SNBT_COLLECT_INTO_IMPL_FLEX(float)
        SNBT_COLLECT_INTO_IMPL_FLEX(double)
        SNBT_COLLECT_INTO_IMPL_FLEX(std::string)
        SNBT_COLLECT_INTO_IMPL_FLEX(nbt_convert)
        SNBT_COLLECT_INTO_IMPL_FLEX(nbt)
        SNBT_COLLECT_INTO_IMPL_FLEX(nbt_compound)
        SNBT_COLLECT_INTO_IMPL_FLEX(base_objects::uuid)
        SNBT_COLLECT_INTO_IMPL_FLEX(base_objects::uuid_hex)
        SNBT_COLLECT_INTO_IMPL_FLEX(base_objects::uuid_flat_hex)

#undef SNBT_COLLECT_INTO_IMPL_FLEX

#define SNBT_COLLECT_AS_IMPL_FLEX(TypeName)                                              \
    compound_flex& compound_flex::collect_as(const std::string& name, TypeName& res) {   \
        return collect(name, [&res](snbt_read_stream& stream) { stream.read_as(res); }); \
    }

        SNBT_COLLECT_AS_IMPL_FLEX(bool)
        SNBT_COLLECT_AS_IMPL_FLEX(uint8_t)
        SNBT_COLLECT_AS_IMPL_FLEX(uint16_t)
        SNBT_COLLECT_AS_IMPL_FLEX(uint32_t)
        SNBT_COLLECT_AS_IMPL_FLEX(uint64_t)
        SNBT_COLLECT_AS_IMPL_FLEX(int8_t)
        SNBT_COLLECT_AS_IMPL_FLEX(int16_t)
        SNBT_COLLECT_AS_IMPL_FLEX(int32_t)
        SNBT_COLLECT_AS_IMPL_FLEX(int64_t)
        SNBT_COLLECT_AS_IMPL_FLEX(float)
        SNBT_COLLECT_AS_IMPL_FLEX(double)
        SNBT_COLLECT_AS_IMPL_FLEX(std::string)
        SNBT_COLLECT_AS_IMPL_FLEX(nbt_convert)
        SNBT_COLLECT_AS_IMPL_FLEX(nbt)
        SNBT_COLLECT_AS_IMPL_FLEX(nbt_compound)
        SNBT_COLLECT_AS_IMPL_FLEX(base_objects::uuid)
        SNBT_COLLECT_AS_IMPL_FLEX(base_objects::uuid_hex)
        SNBT_COLLECT_AS_IMPL_FLEX(base_objects::uuid_flat_hex)

#undef SNBT_COLLECT_AS_IMPL_FLEX
#define SNBT_COLLECT_INTO_IMPL(TypeName)                                                            \
    compound_flex& compound_flex::collect_into_required(const std::string& name, TypeName& res) {   \
        return collect_required(name, [&res](snbt_read_stream& stream) { stream.read_into(res); }); \
    }

        SNBT_COLLECT_INTO_IMPL(bool)
        SNBT_COLLECT_INTO_IMPL(uint8_t)
        SNBT_COLLECT_INTO_IMPL(uint16_t)
        SNBT_COLLECT_INTO_IMPL(uint32_t)
        SNBT_COLLECT_INTO_IMPL(uint64_t)
        SNBT_COLLECT_INTO_IMPL(int8_t)
        SNBT_COLLECT_INTO_IMPL(int16_t)
        SNBT_COLLECT_INTO_IMPL(int32_t)
        SNBT_COLLECT_INTO_IMPL(int64_t)
        SNBT_COLLECT_INTO_IMPL(float)
        SNBT_COLLECT_INTO_IMPL(double)
        SNBT_COLLECT_INTO_IMPL(std::string)
        SNBT_COLLECT_INTO_IMPL(nbt_convert)
        SNBT_COLLECT_INTO_IMPL(nbt)
        SNBT_COLLECT_INTO_IMPL(nbt_compound)
        SNBT_COLLECT_INTO_IMPL(base_objects::uuid)
        SNBT_COLLECT_INTO_IMPL(base_objects::uuid_hex)
        SNBT_COLLECT_INTO_IMPL(base_objects::uuid_flat_hex)

#undef SNBT_COLLECT_INTO_IMPL

#define SNBT_COLLECT_AS_IMPL(TypeName)                                                            \
    compound_flex& compound_flex::collect_as_required(const std::string& name, TypeName& res) {   \
        return collect_required(name, [&res](snbt_read_stream& stream) { stream.read_as(res); }); \
    }

        SNBT_COLLECT_AS_IMPL(bool)
        SNBT_COLLECT_AS_IMPL(uint8_t)
        SNBT_COLLECT_AS_IMPL(uint16_t)
        SNBT_COLLECT_AS_IMPL(uint32_t)
        SNBT_COLLECT_AS_IMPL(uint64_t)
        SNBT_COLLECT_AS_IMPL(int8_t)
        SNBT_COLLECT_AS_IMPL(int16_t)
        SNBT_COLLECT_AS_IMPL(int32_t)
        SNBT_COLLECT_AS_IMPL(int64_t)
        SNBT_COLLECT_AS_IMPL(float)
        SNBT_COLLECT_AS_IMPL(double)
        SNBT_COLLECT_AS_IMPL(std::string)
        SNBT_COLLECT_AS_IMPL(nbt_convert)
        SNBT_COLLECT_AS_IMPL(nbt)
        SNBT_COLLECT_AS_IMPL(nbt_compound)
        SNBT_COLLECT_AS_IMPL(base_objects::uuid)
        SNBT_COLLECT_AS_IMPL(base_objects::uuid_hex)
        SNBT_COLLECT_AS_IMPL(base_objects::uuid_flat_hex)

#undef SNBT_COLLECT_AS_IMPL

        compound_flex& compound_flex::make_collect(snbt_read_stream& stream) {
            return make_collect(stream, [](auto&, auto&) {});
        }

        compound_flex& compound_flex::force_all_collect(snbt_read_stream& stream) {
            std::unordered_set<std::string> collected_required;
            collected_required.reserve(automated_collector_required.size());
            std::unordered_set<std::string> collected_items;
            collected_items.reserve(automated_collector.size());
            stream.iterate([this, &collected_items, &collected_required](auto& name, auto& stream) {
                if (auto it = automated_collector_required.find(name); it != automated_collector_required.end()) {
                    it->second(stream);
                    collected_required.emplace(name);
                } else if (auto normal_it = automated_collector.find(name); normal_it != automated_collector.end()) {
                    normal_it->second(stream);
                    collected_items.emplace(name);
                } else
                    throw std::runtime_error("Not all elements is collected, skipped item: " + name);
            });
            for (auto& [it, _] : automated_collector)
                if (!collected_items.contains(it))
                    throw std::runtime_error("Not all elements is collected, invalid format");

            for (auto& [it, _] : automated_collector_required)
                if (!collected_required.contains(it))
                    throw std::runtime_error("Not all required elements is collected, invalid format");
            return *this;
        }
    }

}
