/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_CHAT
#define SRC_BASE_OBJECTS_CHAT
#include <library/list_array.hpp>
#include <optional>
#include <src/base_objects/dye_color.hpp>
#include <src/util/nbt.hpp>
#include <string>
#include <vector>

namespace copper_server::base_objects {
    struct chat {
        struct click_event_s {
            std::string open_url;
            std::string run_command;
            std::string suggest_command;
            std::optional<int32_t> change_page;
            std::string copy_to_clipboard;

            ~click_event_s();
        };

        struct hover_event_s {
            struct show_item_s {
                std::optional<std::string> tag;
                std::string id;
                int32_t count = 0;
            };

            struct show_entity_s {
                std::optional<std::string> name;
                std::string type;
                std::string id;
            };

            std::unique_ptr<show_item_s> show_item;
            std::unique_ptr<show_entity_s> show_entity;
            std::string show_text;

            ~hover_event_s();
        };

        chat();
        chat(const char* text, bool is_translation = false);
        chat(std::initializer_list<chat> args);
        chat(const std::string& set_text, bool is_translation = false);
        chat(std::string&& set_text, bool is_translation = false);
        chat(const chat& copy);
        chat(chat&& copy) noexcept;
        chat& operator=(const chat& copy);
        chat& operator=(chat&& copy) noexcept;
        ~chat();

        chat& set_text(const std::string& set_text);
        chat& set_translation(const std::string& set_text);
        chat& set_color(const std::string& set_text);
        chat& set_color(dye_color color);
        chat& set_insertion(const std::string& set_text);
        chat& set_font(const std::string& set_text);
        chat& set_text(std::string&& set_text = "");
        chat& set_translation(std::string&& set_text = "");
        chat& set_color(std::string&& set_text = "");
        chat& set_insertion(std::string&& set_text = "");
        chat& set_font(std::string&& set_text = "");
        chat& set_bold();
        chat& set_italic();
        chat& set_underlined();
        chat& set_strikethrough();
        chat& set_obfuscated();
        chat& set_bold(bool is);
        chat& set_italic(bool is);
        chat& set_underlined(bool is);
        chat& set_strikethrough(bool is);
        chat& set_obfuscated(bool is);
        chat& set_hover_event_show_text(const std::string& _show_text);
        chat& set_hover_event_show_item(const std::string& _id, int32_t _count, const std::optional<std::string>& _tag = std::nullopt);
        chat& set_hover_event_show_entity(const std::string& _id, const std::string& _type, const std::optional<std::string>& _name = std::nullopt);
        chat& set_hover_event();
        chat& set_click_event_open_url(const std::string& _open_url);
        chat& set_click_event_run_command(const std::string& _run_command);
        chat& set_click_event_suggest_command(const std::string& _suggest_command);
        chat& set_click_event_change_page(int32_t _change_page);
        chat& set_click_event_copy_to_clipboard(const std::string& _copy_to_clipboard);
        chat& set_click_event();

        list_array<chat>& get_extra();
        const std::string& get_text() const;
        const std::string& get_translation() const;
        const std::string& get_color() const;
        const std::string& get_insertion() const;
        const std::string& get_font() const;
        const std::unique_ptr<hover_event_s>& get_hover_event() const;
        const std::unique_ptr<click_event_s>& get_click_event() const;
        std::optional<bool> get_bold();
        std::optional<bool> get_italic();
        std::optional<bool> get_underlined();
        std::optional<bool> get_strikethrough();
        std::optional<bool> get_obfuscated();

        std::string to_str() const;
        static chat from_str(std::string_view str);
        util::nbt to_nbt() const;

        void remove_color();
        void remove_color_recursive();
        bool empty() const;

        static chat parse_to_chat(std::string_view string);
        static chat from_nbt(const util::nbt& nbt);
        static chat from_nbt_with_format(const util::nbt& nbt, list_array<util::nbt>&&);
        std::string to_ansi_console() const;


        bool operator==(const chat&) const;
        bool operator!=(const chat& other) const;

        std::strong_ordering operator<=>(const chat& other) const {
            return operator==(other) ? std::strong_ordering::equal : std::strong_ordering::less;
        }

        operator bool() const {
            return !empty();
        }

    private:
        list_array<chat> extra;
        std::string text;
        std::string color;
        std::string insertion;
        std::string font;

        std::unique_ptr<click_event_s> click_event;

        std::unique_ptr<hover_event_s> hover_event;

        bool bold : 1 = false;
        bool italic : 1 = false;
        bool underlined : 1 = false;
        bool strikethrough : 1 = false;
        bool obfuscated : 1 = false;

        bool defined_bold : 1 = false;
        bool defined_italic : 1 = false;
        bool defined_underlined : 1 = false;
        bool defined_strikethrough : 1 = false;
        bool defined_obfuscated : 1 = false;

        bool text_is_translation : 1 = false;
    };
}

#endif /* SRC_BASE_OBJECTS_CHAT */
