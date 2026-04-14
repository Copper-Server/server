/*
 * Copyright 2026-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTIL_SNBT_STREAM
#define SRC_UTIL_SNBT_STREAM

#include <cstdint>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace copper_server::util {
    class nbt;
    class snbt_read_stream;
    class snbt_read_compound_stream;
    class snbt_read_list_stream;

    namespace snbt_detail {
        // Token types for SNBT parsing
        enum class token_type {
            number,
            string,
            key,
            open_brace,    // {
            close_brace,   // }
            open_bracket,  // [
            close_bracket, // ]
            comma,         // ,
colon,         // :
            semicolon,     // ;
            end_of_input,
            invalid
        };

        struct token {
            token_type type;
            std::string value;
            size_t position;
        };

        class snbt_tokenizer {
        public:
            explicit snbt_tokenizer(std::string_view input);

            token next_token();
            token peek_token();
            void skip_whitespace();

            // Public accessor for remaining input
            std::string_view get_remaining_input() const {
                return input.substr(pos);
            }

        private:
            std::string_view input;
            size_t pos;
            std::optional<token> cached_token;

            token read_token();
            token read_string(char quote);
            token read_unquoted_string();
            token read_number();
            char current() const;
            char peek(size_t offset = 1) const;
            bool is_at_end() const;
            void advance();

            friend class snbt_parser;
            friend class snbt_read_stream;
            friend class snbt_read_list_stream;
        };

        class snbt_parser {
        public:
            explicit snbt_parser(std::string_view input);

            nbt parse();

        private:
            snbt_tokenizer tokenizer;

            nbt parse_value();
            nbt parse_compound();
            nbt parse_list();
            nbt parse_byte_array();
            nbt parse_int_array();
            nbt parse_long_array();
            nbt parse_number(const std::string& number_str);

            token current_token;

            void advance_token();
            void expect(token_type expected);

            // String processing
            static std::string unescape_string(std::string_view escaped);
            static char parse_escape_sequence(std::string_view str, size_t& i);

            // Number parsing helpers
            static std::optional<uint32_t> parse_hex_code(std::string_view str, size_t digits);
            static std::string parse_unicode_escape(std::string_view str, size_t& i);
        };
    }

    enum class snbt_tag_type {
        tag_end = 0,
        tag_byte = 1,
        tag_short = 2,
        tag_int = 3,
        tag_long = 4,
        tag_float = 5,
        tag_double = 6,
        tag_byte_array = 7,
        tag_string = 8,
        tag_list = 9,
        tag_compound = 10,
        tag_int_array = 11,
        tag_long_array = 12,
        tag_boolean = 13 // Pseudo-type for SNBT true/false
    };

    // Streaming SNBT reader - reads SNBT values on-demand without loading entire structure
    class snbt_read_stream {
        snbt_detail::snbt_tokenizer tokenizer;
        snbt_detail::token current_token;
        snbt_tag_type detected_type;
        bool readed = false;

        void advance_token();
        void detect_value_type();
        snbt_tag_type detect_type_from_token(const snbt_detail::token& tok);

        friend class snbt_read_compound_stream;
        friend class snbt_read_list_stream;

    public:
        explicit snbt_read_stream(std::string_view input);
        ~snbt_read_stream();

        // Read primitive types
        snbt_read_stream& read_into(bool& res);
        snbt_read_stream& read_into(uint8_t& res);
        snbt_read_stream& read_into(uint16_t& res);
        snbt_read_stream& read_into(uint32_t& res);
        snbt_read_stream& read_into(uint64_t& res);
        snbt_read_stream& read_into(int8_t& res);
        snbt_read_stream& read_into(int16_t& res);
        snbt_read_stream& read_into(int32_t& res);
        snbt_read_stream& read_into(int64_t& res);
        snbt_read_stream& read_into(float& res);
        snbt_read_stream& read_into(double& res);
        snbt_read_stream& read_into(std::string& res);
        snbt_read_stream& read_into(nbt_convert& res);
        snbt_read_stream& read_into(nbt& res);
        snbt_read_stream& read_into(nbt_compound& res);
        snbt_read_stream& read_into(base_objects::uuid& res);
        snbt_read_stream& read_into(base_objects::uuid_hex& res);
        snbt_read_stream& read_into(base_objects::uuid_flat_hex& res);

        // Read with type conversion
        snbt_read_stream& read_as(bool& res);
        snbt_read_stream& read_as(uint8_t& res);
        snbt_read_stream& read_as(uint16_t& res);
        snbt_read_stream& read_as(uint32_t& res);
        snbt_read_stream& read_as(uint64_t& res);
        snbt_read_stream& read_as(int8_t& res);
        snbt_read_stream& read_as(int16_t& res);
        snbt_read_stream& read_as(int32_t& res);
        snbt_read_stream& read_as(int64_t& res);
        snbt_read_stream& read_as(float& res);
        snbt_read_stream& read_as(double& res);
        snbt_read_stream& read_as(std::string& res);
        snbt_read_stream& read_as(nbt_convert& res);
        snbt_read_stream& read_as(nbt& res);
        snbt_read_stream& read_as(nbt_compound& res);
        snbt_read_stream& read_as(base_objects::uuid& res);
        snbt_read_stream& read_as(base_objects::uuid_hex& res);
        snbt_read_stream& read_as(base_objects::uuid_flat_hex& res);

        template <class T>
        T read_into() {
            T res;
            read_into(res);
            return res;
        }

        template <class T>
        T read_as() {
            T res;
            read_as(res);
            return res;
        }

        void skip();
        snbt_tag_type get_type() const;

        // Stream-based iteration
        snbt_read_compound_stream read_compound();
        snbt_read_list_stream read_list();

        template <class FN_first, class FN_second>
        void double_pass_read(FN_first&& first_pass, FN_second&& second_pass)
            requires(std::is_invocable_v<FN_first, snbt_read_stream&> && std::is_invocable_v<FN_second, snbt_read_stream&>)
        {
            nbt temp = parse_to_nbt();
            std::string snbt_str = temp.as_snbt();

            {
                snbt_read_stream stream(snbt_str);
                first_pass(stream);
            }
            {
                snbt_read_stream stream(snbt_str);
                second_pass(stream);
            }
        }

        template <class FN>
        void iterate(FN&& callback)
            requires(std::is_invocable_v<FN, std::string_view, snbt_read_stream&> || std::is_invocable_v<FN, const std::string&, snbt_read_stream&>)
        {
            iterate([](size_t) {}, std::move(callback));
        }

        template <class FN>
        void iterate(FN&& callback)
            requires(std::is_invocable_v<FN, snbt_read_stream&>)
        {
            iterate([](size_t) {}, std::move(callback));
        }

        template <class SIZE_FN, class FN>
        void iterate(SIZE_FN&& size_callback, FN&& callback);

        // Parse entire structure to NBT
        nbt parse_to_nbt();
    };

    // Streaming compound reader
    class snbt_read_compound_stream {
        snbt_detail::snbt_tokenizer tokenizer;
        snbt_detail::token current_token;
        bool reached_end = false;
        bool enable_collector_strict_order = false;

        std::unordered_map<std::string, std::function<void(snbt_read_stream&)>> automated_collector;
        std::vector<std::string> collector_strict_order_data;

        void advance_token();

        friend class snbt_read_stream;

    public:
        snbt_read_compound_stream(snbt_detail::snbt_tokenizer&& tokenizer, bool enable_collector_strict_order = false);
        ~snbt_read_compound_stream();

        bool is_reached_end() const noexcept;

        // Iteration with callbacks
        template <class FN>
        snbt_read_compound_stream& read(FN&& fn)
            requires(std::is_invocable_v<FN, std::string&, snbt_read_stream&>)
        {
            iterate(std::move(fn));
            return *this;
        }

        template <class FN>
        snbt_read_compound_stream& iterable(FN&& fn)
            requires(std::is_invocable_v<FN, std::string&, snbt_read_stream&>)
        {
            iterate(std::move(fn));
            return *this;
        }

        // Collection pattern - define fields to collect on-demand
        template <class FN>
        snbt_read_compound_stream& collect(const std::string& name, FN&& fn)
            requires(std::is_invocable_v<FN, snbt_read_stream&>)
        {
            automated_collector[name] = std::forward<FN>(fn);
            if (enable_collector_strict_order)
                collector_strict_order_data.push_back(name);
            return *this;
        }

        snbt_read_compound_stream& collect_into(const std::string& name, bool& res);
        snbt_read_compound_stream& collect_into(const std::string& name, uint8_t& res);
        snbt_read_compound_stream& collect_into(const std::string& name, uint16_t& res);
        snbt_read_compound_stream& collect_into(const std::string& name, uint32_t& res);
        snbt_read_compound_stream& collect_into(const std::string& name, uint64_t& res);
        snbt_read_compound_stream& collect_into(const std::string& name, int8_t& res);
        snbt_read_compound_stream& collect_into(const std::string& name, int16_t& res);
        snbt_read_compound_stream& collect_into(const std::string& name, int32_t& res);
        snbt_read_compound_stream& collect_into(const std::string& name, int64_t& res);
        snbt_read_compound_stream& collect_into(const std::string& name, float& res);
        snbt_read_compound_stream& collect_into(const std::string& name, double& res);
        snbt_read_compound_stream& collect_into(const std::string& name, std::string& res);
        snbt_read_compound_stream& collect_into(const std::string& name, nbt_convert& res);
        snbt_read_compound_stream& collect_into(const std::string& name, nbt& res);
        snbt_read_compound_stream& collect_into(const std::string& name, nbt_compound& res);
        snbt_read_compound_stream& collect_into(const std::string& name, base_objects::uuid& res);
        snbt_read_compound_stream& collect_into(const std::string& name, base_objects::uuid_hex& res);
        snbt_read_compound_stream& collect_into(const std::string& name, base_objects::uuid_flat_hex& res);

        snbt_read_compound_stream& collect_as(const std::string& name, bool& res);
        snbt_read_compound_stream& collect_as(const std::string& name, uint8_t& res);
        snbt_read_compound_stream& collect_as(const std::string& name, uint16_t& res);
        snbt_read_compound_stream& collect_as(const std::string& name, uint32_t& res);
        snbt_read_compound_stream& collect_as(const std::string& name, uint64_t& res);
        snbt_read_compound_stream& collect_as(const std::string& name, int8_t& res);
        snbt_read_compound_stream& collect_as(const std::string& name, int16_t& res);
        snbt_read_compound_stream& collect_as(const std::string& name, int32_t& res);
        snbt_read_compound_stream& collect_as(const std::string& name, int64_t& res);
        snbt_read_compound_stream& collect_as(const std::string& name, float& res);
        snbt_read_compound_stream& collect_as(const std::string& name, double& res);
        snbt_read_compound_stream& collect_as(const std::string& name, std::string& res);
        snbt_read_compound_stream& collect_as(const std::string& name, nbt_convert& res);
        snbt_read_compound_stream& collect_as(const std::string& name, nbt& res);
        snbt_read_compound_stream& collect_as(const std::string& name, nbt_compound& res);
        snbt_read_compound_stream& collect_as(const std::string& name, base_objects::uuid& res);
        snbt_read_compound_stream& collect_as(const std::string& name, base_objects::uuid_hex& res);
        snbt_read_compound_stream& collect_as(const std::string& name, base_objects::uuid_flat_hex& res);

        template <class FN>
        snbt_read_compound_stream& collect_iterate(const std::string& name, FN&& fn)
            requires(
                std::is_invocable_v<FN, snbt_read_stream&>
                || std::is_invocable_v<FN, std::string_view, snbt_read_stream&>
                || std::is_invocable_v<FN, const std::string&, snbt_read_stream&>
            )
        {
            automated_collector[name] = [fn](snbt_read_stream& stream) {
                stream.iterate(fn);
            };
            if (enable_collector_strict_order)
                collector_strict_order_data.push_back(name);
            return *this;
        }

        template <class FN>
        snbt_read_compound_stream& make_collect(FN&& on_uncollected)
            requires(std::is_invocable_v<FN, const std::string&, snbt_read_stream&>)
        {
            iterate([this, &on_uncollected](auto& name, auto& stream) {
                if (auto it = automated_collector.find(name); it != automated_collector.end())
                    it->second(stream);
                else
                    on_uncollected(name, stream);
            });
            return *this;
        }

        snbt_read_compound_stream& make_collect();
        snbt_read_compound_stream& force_all_collect();

    private:
        template <class FN>
        void iterate(FN&& callback)
            requires(std::is_invocable_v<FN, std::string&, snbt_read_stream&>)
        {
            // Iterate through all key-value pairs in the compound
            while (current_token.type != snbt_detail::token_type::close_brace && current_token.type != snbt_detail::token_type::end_of_input) {

                if (current_token.type != snbt_detail::token_type::key && current_token.type != snbt_detail::token_type::string) {
                    throw std::runtime_error("Expected key in compound");
                }

                std::string key = current_token.value;
                advance_token();

                if (current_token.type != snbt_detail::token_type::colon)
                    throw std::runtime_error("Expected ':' after key");
                advance_token();

                snbt_read_stream value_stream(std::string(tokenizer.input.substr(tokenizer.pos)));
                callback(key, value_stream);

                // Advance past the value
                if (current_token.type == snbt_detail::token_type::comma) {
                    advance_token();
                } else if (current_token.type != snbt_detail::token_type::close_brace) {
                    throw std::runtime_error("Expected ',' or '}' in compound");
                }
            }

            if (current_token.type != snbt_detail::token_type::close_brace)
                throw std::runtime_error("Expected '}'");
            reached_end = true;
        }
    };

    // Streaming list reader
    class snbt_read_list_stream {
        snbt_detail::snbt_tokenizer tokenizer;
        snbt_detail::token current_token;
        int32_t current_item = 0;
        int32_t items = 0;
        snbt_tag_type items_type = snbt_tag_type::tag_end;
        char array_prefix = '\0'; // 'B', 'I', 'L' for typed arrays
        bool reached_end = false;

        void advance_token();
        void skip_value();

        friend class snbt_read_stream;

    public:
        snbt_read_list_stream(snbt_detail::snbt_tokenizer&& tokenizer);
        ~snbt_read_list_stream();

        snbt_tag_type get_items_type() const;
        int32_t size() const noexcept;
        int32_t current_index() const noexcept;

        snbt_read_list_stream& read_one_into(bool& res);
        snbt_read_list_stream& read_one_into(uint8_t& res);
        snbt_read_list_stream& read_one_into(uint16_t& res);
        snbt_read_list_stream& read_one_into(uint32_t& res);
        snbt_read_list_stream& read_one_into(uint64_t& res);
        snbt_read_list_stream& read_one_into(int8_t& res);
        snbt_read_list_stream& read_one_into(int16_t& res);
        snbt_read_list_stream& read_one_into(int32_t& res);
        snbt_read_list_stream& read_one_into(int64_t& res);
        snbt_read_list_stream& read_one_into(float& res);
        snbt_read_list_stream& read_one_into(double& res);
        snbt_read_list_stream& read_one_into(std::string& res);
        snbt_read_list_stream& read_one_into(nbt_convert& res);
        snbt_read_list_stream& read_one_into(nbt& res);
        snbt_read_list_stream& read_one_into(nbt_compound& res);
        snbt_read_list_stream& read_one_into(base_objects::uuid& res);
        snbt_read_list_stream& read_one_into(base_objects::uuid_hex& res);
        snbt_read_list_stream& read_one_into(base_objects::uuid_flat_hex& res);

        snbt_read_list_stream& read_one_as(bool& res);
        snbt_read_list_stream& read_one_as(uint8_t& res);
        snbt_read_list_stream& read_one_as(uint16_t& res);
        snbt_read_list_stream& read_one_as(uint32_t& res);
        snbt_read_list_stream& read_one_as(uint64_t& res);
        snbt_read_list_stream& read_one_as(int8_t& res);
        snbt_read_list_stream& read_one_as(int16_t& res);
        snbt_read_list_stream& read_one_as(int32_t& res);
        snbt_read_list_stream& read_one_as(int64_t& res);
        snbt_read_list_stream& read_one_as(float& res);
        snbt_read_list_stream& read_one_as(double& res);
        snbt_read_list_stream& read_one_as(std::string& res);
        snbt_read_list_stream& read_one_as(nbt_convert& res);
        snbt_read_list_stream& read_one_as(nbt& res);
        snbt_read_list_stream& read_one_as(nbt_compound& res);
        snbt_read_list_stream& read_one_as(base_objects::uuid& res);
        snbt_read_list_stream& read_one_as(base_objects::uuid_hex& res);
        snbt_read_list_stream& read_one_as(base_objects::uuid_flat_hex& res);

        template <class FN>
        snbt_read_list_stream& read_one(FN&& fn)
            requires(std::is_invocable_v<FN, snbt_read_stream&>)
        {
            if (current_item >= items)
                throw std::out_of_range("List index out of range");

            snbt_read_stream reader(std::move(tokenizer), get_items_type());
            fn(reader);
            current_item++;

            // Resync tokenizer
            if (current_item < items) {
                if (current_token.type != snbt_detail::token_type::comma)
                    throw std::runtime_error("Expected comma in list");
                advance_token();
            }
            return *this;
        }

        template <class FN>
        snbt_read_list_stream& iterable(FN&& fn)
            requires(std::is_invocable_v<FN, snbt_read_stream&>)
        {
            while (current_item < items)
                read_one(std::forward<FN>(fn));
            return *this;
        }
    };

    // Forward declarations
    class snbt_write_list_stream;
    class snbt_write_compound_stream;

    // Streaming SNBT writer
    class snbt_write_stream {
        std::string output;
        uint16_t depth = 0;
        bool pretty_print = false;

        void write_indent();
        void write_escaped_string(std::string_view str);

        friend class snbt_write_list_stream;
        friend class snbt_write_compound_stream;

    public:
        snbt_write_stream(bool pretty_print = false);
        ~snbt_write_stream();

        void write(bool res);
        void write(uint8_t res);
        void write(uint16_t res);
        void write(uint32_t res);
        void write(uint64_t res);
        void write(int8_t res);
        void write(int16_t res);
        void write(int32_t res);
        void write(int64_t res);
        void write(float res);
        void write(double res);
        void write(const std::string& res);
        void write(std::string_view res);
        void write(const nbt_convert& res);
        void write(const nbt& res);
        void write(const nbt_compound& res);

        void write_array(const int8_t* arr, size_t size);
        void write_array(const uint8_t* arr, size_t size);
        void write_array(const int32_t* arr, size_t size);
        void write_array(const int64_t* arr, size_t size);

        snbt_write_compound_stream write_compound();
        snbt_write_list_stream write_list();

        std::string get_output() const;
        std::string take_output();
    };

    class snbt_write_list_stream {
        std::string& output;
        uint16_t depth = 0;
        bool pretty_print = false;
        bool first_item = true;

    public:
        snbt_write_list_stream(std::string& output, uint16_t depth, bool pretty_print);
        ~snbt_write_list_stream();

        snbt_write_list_stream& write(bool res);
        snbt_write_list_stream& write(uint8_t res);
        snbt_write_list_stream& write(uint16_t res);
        snbt_write_list_stream& write(uint32_t res);
        snbt_write_list_stream& write(uint64_t res);
        snbt_write_list_stream& write(int8_t res);
        snbt_write_list_stream& write(int16_t res);
        snbt_write_list_stream& write(int32_t res);
        snbt_write_list_stream& write(int64_t res);
        snbt_write_list_stream& write(float res);
        snbt_write_list_stream& write(double res);
        snbt_write_list_stream& write(const std::string& res);
        snbt_write_list_stream& write(std::string_view res);
        snbt_write_list_stream& write(const nbt_convert& res);
        snbt_write_list_stream& write(const nbt& res);
        snbt_write_list_stream& write(const nbt_compound& res);

        snbt_write_compound_stream write_compound();
        snbt_write_list_stream write_list();

    private:
        void write_indent();
        void write_escaped_string(std::string_view str);
    };

    class snbt_write_compound_stream {
        std::string& output;
        uint16_t depth = 0;
        bool pretty_print = false;
        bool first_item = true;

    public:
        snbt_write_compound_stream(std::string& output, uint16_t depth, bool pretty_print);
        ~snbt_write_compound_stream();

        snbt_write_compound_stream& write(std::string_view key, bool res);
        snbt_write_compound_stream& write(std::string_view key, uint8_t res);
        snbt_write_compound_stream& write(std::string_view key, uint16_t res);
        snbt_write_compound_stream& write(std::string_view key, uint32_t res);
        snbt_write_compound_stream& write(std::string_view key, uint64_t res);
        snbt_write_compound_stream& write(std::string_view key, int8_t res);
        snbt_write_compound_stream& write(std::string_view key, int16_t res);
        snbt_write_compound_stream& write(std::string_view key, int32_t res);
        snbt_write_compound_stream& write(std::string_view key, int64_t res);
        snbt_write_compound_stream& write(std::string_view key, float res);
        snbt_write_compound_stream& write(std::string_view key, double res);
        snbt_write_compound_stream& write(std::string_view key, const std::string& res);
        snbt_write_compound_stream& write(std::string_view key, std::string_view res);
        snbt_write_compound_stream& write(std::string_view key, const nbt_convert& res);
        snbt_write_compound_stream& write(std::string_view key, const nbt& res);
        snbt_write_compound_stream& write(std::string_view key, const nbt_compound& res);

        snbt_write_compound_stream& write(std::string_view key, base_objects::uuid res);
        snbt_write_compound_stream& write(std::string_view key, base_objects::uuid_hex res);
        snbt_write_compound_stream& write(std::string_view key, base_objects::uuid_flat_hex res);

        snbt_write_compound_stream& write_compound(std::string_view key);
        snbt_write_compound_stream& write_list(std::string_view key);

    private:
        void write_indent();
        void write_escaped_string(std::string_view str);
        void write_key(std::string_view key);
    };

    namespace snbt_collection {
        // Similar to nbt_collection for compound handling
        class compound_relaxed {
            std::unordered_map<std::string, std::function<void(snbt_read_stream&)>> automated_collector;

        public:
            compound_relaxed() = default;
            ~compound_relaxed() = default;

            template <class FN>
            compound_relaxed& collect(const std::string& name, FN&& fn)
                requires(std::is_invocable_v<FN, snbt_read_stream&>)
            {
                automated_collector[name] = std::forward<FN>(fn);
                return *this;
            }

            compound_relaxed& collect_into(const std::string& name, bool& res);
            compound_relaxed& collect_into(const std::string& name, uint8_t& res);
            compound_relaxed& collect_into(const std::string& name, uint16_t& res);
            compound_relaxed& collect_into(const std::string& name, uint32_t& res);
            compound_relaxed& collect_into(const std::string& name, uint64_t& res);
            compound_relaxed& collect_into(const std::string& name, int8_t& res);
            compound_relaxed& collect_into(const std::string& name, int16_t& res);
            compound_relaxed& collect_into(const std::string& name, int32_t& res);
            compound_relaxed& collect_into(const std::string& name, int64_t& res);
            compound_relaxed& collect_into(const std::string& name, float& res);
            compound_relaxed& collect_into(const std::string& name, double& res);
            compound_relaxed& collect_into(const std::string& name, std::string& res);
            compound_relaxed& collect_into(const std::string& name, nbt_convert& res);
            compound_relaxed& collect_into(const std::string& name, nbt& res);
            compound_relaxed& collect_into(const std::string& name, nbt_compound& res);
            compound_relaxed& collect_into(const std::string& name, base_objects::uuid& res);
            compound_relaxed& collect_into(const std::string& name, base_objects::uuid_hex& res);
            compound_relaxed& collect_into(const std::string& name, base_objects::uuid_flat_hex& res);

            compound_relaxed& collect_as(const std::string& name, bool& res);
            compound_relaxed& collect_as(const std::string& name, uint8_t& res);
            compound_relaxed& collect_as(const std::string& name, uint16_t& res);
            compound_relaxed& collect_as(const std::string& name, uint32_t& res);
            compound_relaxed& collect_as(const std::string& name, uint64_t& res);
            compound_relaxed& collect_as(const std::string& name, int8_t& res);
            compound_relaxed& collect_as(const std::string& name, int16_t& res);
            compound_relaxed& collect_as(const std::string& name, int32_t& res);
            compound_relaxed& collect_as(const std::string& name, int64_t& res);
            compound_relaxed& collect_as(const std::string& name, float& res);
            compound_relaxed& collect_as(const std::string& name, double& res);
            compound_relaxed& collect_as(const std::string& name, std::string& res);
            compound_relaxed& collect_as(const std::string& name, nbt_convert& res);
            compound_relaxed& collect_as(const std::string& name, nbt& res);
            compound_relaxed& collect_as(const std::string& name, nbt_compound& res);
            compound_relaxed& collect_as(const std::string& name, base_objects::uuid& res);
            compound_relaxed& collect_as(const std::string& name, base_objects::uuid_hex& res);
            compound_relaxed& collect_as(const std::string& name, base_objects::uuid_flat_hex& res);

            template <class FN>
            compound_relaxed& make_collect(snbt_read_stream& stream, FN&& on_uncollected)
                requires(std::is_invocable_v<FN, const std::string&, snbt_read_stream&>)
            {
                stream.iterate([this, &on_uncollected](auto name, auto& item_stream) {
                    if (auto it = automated_collector.find(name); it != automated_collector.end())
                        it->second(item_stream);
                    else
                        on_uncollected(name, item_stream);
                });
                return *this;
            }

            compound_relaxed& make_collect(snbt_read_stream& stream) {
                return make_collect(stream, [](auto&, auto&) {});
            }

            compound_relaxed& force_all_collect(snbt_read_stream& stream);
        };

        class compound_strict {
            std::unordered_map<std::string, std::function<void(snbt_read_stream&)>> automated_collector;
            std::vector<std::string> collector_strict_order_data;

        public:
            compound_strict() = default;
            ~compound_strict() = default;

            template <class FN>
            compound_strict& collect(const std::string& name, FN&& fn)
                requires(std::is_invocable_v<FN, snbt_read_stream&>)
            {
                automated_collector[name] = std::forward<FN>(fn);
                collector_strict_order_data.push_back(name);
                return *this;
            }

            compound_strict& collect_into(const std::string& name, bool& res);
            compound_strict& collect_into(const std::string& name, uint8_t& res);
            compound_strict& collect_into(const std::string& name, uint16_t& res);
            compound_strict& collect_into(const std::string& name, uint32_t& res);
            compound_strict& collect_into(const std::string& name, uint64_t& res);
            compound_strict& collect_into(const std::string& name, int8_t& res);
            compound_strict& collect_into(const std::string& name, int16_t& res);
            compound_strict& collect_into(const std::string& name, int32_t& res);
            compound_strict& collect_into(const std::string& name, int64_t& res);
            compound_strict& collect_into(const std::string& name, float& res);
            compound_strict& collect_into(const std::string& name, double& res);
            compound_strict& collect_into(const std::string& name, std::string& res);
            compound_strict& collect_into(const std::string& name, nbt_convert& res);
            compound_strict& collect_into(const std::string& name, nbt& res);
            compound_strict& collect_into(const std::string& name, nbt_compound& res);
            compound_strict& collect_into(const std::string& name, base_objects::uuid& res);
            compound_strict& collect_into(const std::string& name, base_objects::uuid_hex& res);
            compound_strict& collect_into(const std::string& name, base_objects::uuid_flat_hex& res);

            compound_strict& collect_as(const std::string& name, bool& res);
            compound_strict& collect_as(const std::string& name, uint8_t& res);
            compound_strict& collect_as(const std::string& name, uint16_t& res);
            compound_strict& collect_as(const std::string& name, uint32_t& res);
            compound_strict& collect_as(const std::string& name, uint64_t& res);
            compound_strict& collect_as(const std::string& name, int8_t& res);
            compound_strict& collect_as(const std::string& name, int16_t& res);
            compound_strict& collect_as(const std::string& name, int32_t& res);
            compound_strict& collect_as(const std::string& name, int64_t& res);
            compound_strict& collect_as(const std::string& name, float& res);
            compound_strict& collect_as(const std::string& name, double& res);
            compound_strict& collect_as(const std::string& name, std::string& res);
            compound_strict& collect_as(const std::string& name, nbt_convert& res);
            compound_strict& collect_as(const std::string& name, nbt& res);
            compound_strict& collect_as(const std::string& name, nbt_compound& res);
            compound_strict& collect_as(const std::string& name, base_objects::uuid& res);
            compound_strict& collect_as(const std::string& name, base_objects::uuid_hex& res);
            compound_strict& collect_as(const std::string& name, base_objects::uuid_flat_hex& res);

            template <class FN>
            compound_strict& make_collect(snbt_read_stream& stream, FN&& on_uncollected)
                requires(std::is_invocable_v<FN, const std::string&, snbt_read_stream&>)
            {
                stream.iterate([this, &on_uncollected](auto name, auto& item_stream) {
                    if (auto it = automated_collector.find(name); it != automated_collector.end())
                        it->second(item_stream);
                    else
                        on_uncollected(name, item_stream);
                });
                return *this;
            }

            compound_strict& make_collect(snbt_read_stream& stream);
            compound_strict& force_all_collect(snbt_read_stream& stream);
        };

        class compound_flex {
            std::unordered_map<std::string, std::function<void(snbt_read_stream&)>> automated_collector;
            std::unordered_map<std::string, std::function<void(snbt_read_stream&)>> automated_collector_required;

        public:
            compound_flex() = default;
            ~compound_flex() = default;

            template <class FN>
            compound_flex& collect(const std::string& name, FN&& fn)
                requires(std::is_invocable_v<FN, snbt_read_stream&>)
            {
                automated_collector[name] = std::forward<FN>(fn);
                return *this;
            }

            template <class FN>
            compound_flex& collect_required(const std::string& name, FN&& fn)
                requires(std::is_invocable_v<FN, snbt_read_stream&>)
            {
                automated_collector_required[name] = std::forward<FN>(fn);
                return *this;
            }

            compound_flex& collect_into(const std::string& name, bool& res);
            compound_flex& collect_into(const std::string& name, uint8_t& res);
            compound_flex& collect_into(const std::string& name, uint16_t& res);
            compound_flex& collect_into(const std::string& name, uint32_t& res);
            compound_flex& collect_into(const std::string& name, uint64_t& res);
            compound_flex& collect_into(const std::string& name, int8_t& res);
            compound_flex& collect_into(const std::string& name, int16_t& res);
            compound_flex& collect_into(const std::string& name, int32_t& res);
            compound_flex& collect_into(const std::string& name, int64_t& res);
            compound_flex& collect_into(const std::string& name, float& res);
            compound_flex& collect_into(const std::string& name, double& res);
            compound_flex& collect_into(const std::string& name, std::string& res);
            compound_flex& collect_into(const std::string& name, nbt_convert& res);
            compound_flex& collect_into(const std::string& name, nbt& res);
            compound_flex& collect_into(const std::string& name, nbt_compound& res);
            compound_flex& collect_into(const std::string& name, base_objects::uuid& res);
            compound_flex& collect_into(const std::string& name, base_objects::uuid_hex& res);
            compound_flex& collect_into(const std::string& name, base_objects::uuid_flat_hex& res);

            compound_flex& collect_as(const std::string& name, bool& res);
            compound_flex& collect_as(const std::string& name, uint8_t& res);
            compound_flex& collect_as(const std::string& name, uint16_t& res);
            compound_flex& collect_as(const std::string& name, uint32_t& res);
            compound_flex& collect_as(const std::string& name, uint64_t& res);
            compound_flex& collect_as(const std::string& name, int8_t& res);
            compound_flex& collect_as(const std::string& name, int16_t& res);
            compound_flex& collect_as(const std::string& name, int32_t& res);
            compound_flex& collect_as(const std::string& name, int64_t& res);
            compound_flex& collect_as(const std::string& name, float& res);
            compound_flex& collect_as(const std::string& name, double& res);
            compound_flex& collect_as(const std::string& name, std::string& res);
            compound_flex& collect_as(const std::string& name, nbt_convert& res);
            compound_flex& collect_as(const std::string& name, nbt& res);
            compound_flex& collect_as(const std::string& name, nbt_compound& res);
            compound_flex& collect_as(const std::string& name, base_objects::uuid& res);
            compound_flex& collect_as(const std::string& name, base_objects::uuid_hex& res);
            compound_flex& collect_as(const std::string& name, base_objects::uuid_flat_hex& res);

            template <class FN>
            compound_flex& collect_iterate(const std::string& name, FN&& fn)
                requires(
                    std::is_invocable_v<FN, snbt_read_stream&>
                    || std::is_invocable_v<FN, std::string_view, snbt_read_stream&>
                    || std::is_invocable_v<FN, const std::string&, snbt_read_stream&>
                )
            {
                automated_collector[name] = [fn](snbt_read_stream& stream) {
                    stream.iterate(fn);
                };
                return *this;
            }

            compound_flex& collect_into_required(const std::string& name, bool& res);
            compound_flex& collect_into_required(const std::string& name, uint8_t& res);
            compound_flex& collect_into_required(const std::string& name, uint16_t& res);
            compound_flex& collect_into_required(const std::string& name, uint32_t& res);
            compound_flex& collect_into_required(const std::string& name, uint64_t& res);
            compound_flex& collect_into_required(const std::string& name, int8_t& res);
            compound_flex& collect_into_required(const std::string& name, int16_t& res);
            compound_flex& collect_into_required(const std::string& name, int32_t& res);
            compound_flex& collect_into_required(const std::string& name, int64_t& res);
            compound_flex& collect_into_required(const std::string& name, float& res);
            compound_flex& collect_into_required(const std::string& name, double& res);
            compound_flex& collect_into_required(const std::string& name, std::string& res);
            compound_flex& collect_into_required(const std::string& name, nbt_convert& res);
            compound_flex& collect_into_required(const std::string& name, nbt& res);
            compound_flex& collect_into_required(const std::string& name, nbt_compound& res);
            compound_flex& collect_into_required(const std::string& name, base_objects::uuid& res);
            compound_flex& collect_into_required(const std::string& name, base_objects::uuid_hex& res);
            compound_flex& collect_into_required(const std::string& name, base_objects::uuid_flat_hex& res);

            compound_flex& collect_as_required(const std::string& name, bool& res);
            compound_flex& collect_as_required(const std::string& name, uint8_t& res);
            compound_flex& collect_as_required(const std::string& name, uint16_t& res);
            compound_flex& collect_as_required(const std::string& name, uint32_t& res);
            compound_flex& collect_as_required(const std::string& name, uint64_t& res);
            compound_flex& collect_as_required(const std::string& name, int8_t& res);
            compound_flex& collect_as_required(const std::string& name, int16_t& res);
            compound_flex& collect_as_required(const std::string& name, int32_t& res);
            compound_flex& collect_as_required(const std::string& name, int64_t& res);
            compound_flex& collect_as_required(const std::string& name, float& res);
            compound_flex& collect_as_required(const std::string& name, double& res);
            compound_flex& collect_as_required(const std::string& name, std::string& res);
            compound_flex& collect_as_required(const std::string& name, nbt_convert& res);
            compound_flex& collect_as_required(const std::string& name, nbt& res);
            compound_flex& collect_as_required(const std::string& name, nbt_compound& res);
            compound_flex& collect_as_required(const std::string& name, base_objects::uuid& res);
            compound_flex& collect_as_required(const std::string& name, base_objects::uuid_hex& res);
            compound_flex& collect_as_required(const std::string& name, base_objects::uuid_flat_hex& res);

            template <class FN>
            compound_flex& collect_iterate_required(const std::string& name, FN&& fn)
                requires(
                    std::is_invocable_v<FN, snbt_read_stream&>
                    || std::is_invocable_v<FN, std::string_view, snbt_read_stream&>
                    || std::is_invocable_v<FN, const std::string&, snbt_read_stream&>
                )
            {
                automated_collector_required[name] = [fn](snbt_read_stream& stream) {
                    stream.iterate(fn);
                };
                return *this;
            }

            template <class FN>
            compound_flex& make_collect(snbt_read_stream& stream, FN&& on_uncollected)
                requires(std::is_invocable_v<FN, const std::string&, snbt_read_stream&>)
            {
                std::unordered_set<std::string> collected_required;
                collected_required.reserve(automated_collector_required.size());
                stream.iterate([this, &on_uncollected, &collected_required](const std::string& name, snbt_read_stream& stream) {
                    if (auto it = automated_collector_required.find(name); it != automated_collector_required.end()) {
                        it->second(stream);
                        collected_required.emplace(name);
                    } else if (auto normal_it = automated_collector.find(name); normal_it != automated_collector.end())
                        normal_it->second(stream);
                    else
                        on_uncollected(name, stream);
                });
                for (auto& [it, _] : automated_collector_required)
                    if (!collected_required.contains(it))
                        throw std::runtime_error("Not all required elements is collected, invalid format");
                return *this;
            }

            compound_flex& make_collect(snbt_read_stream& stream);
            compound_flex& force_all_collect(snbt_read_stream& stream);
        };
    }

    // Public API
    nbt parse_snbt(std::string_view snbt_string);
}

#endif /* SRC_UTIL_SNBT_STREAM */
