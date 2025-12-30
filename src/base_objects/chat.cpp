/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/chat.hpp>
#include <src/util/conversions.hpp>
#include <src/util/json_helpers.hpp>
#include <utf8.h>

namespace copper_server::base_objects {

    chat from_json(util::js_object&& json) {
        chat result;
        if (json.contains("text"))
            result.set_text(util::conversions::string::to_direct(json["text"]));
        else if (json.contains("translate"))
            result.set_translation(json["translate"]);

        if (json.contains("color"))
            result.set_color(json["color"]);

        if (json.contains("insertion"))
            result.set_insertion(json["insertion"]);

        if (json.contains("bold"))
            result.set_bold(json["bold"]);

        if (json.contains("italic"))
            result.set_italic(json["italic"]);

        if (json.contains("underlined"))
            result.set_underlined(json["underlined"]);

        if (json.contains("strikethrough"))
            result.set_strikethrough(json["strikethrough"]);

        if (json.contains("obfuscated"))
            result.set_obfuscated(json["obfuscated"]);

        if (json.contains("font"))
            result.set_font(json["font"]);

        if (json.contains("clickEvent")) {
            auto click_event = util::js_object::get_object(json["clickEvent"]);
            std::string action = click_event["action"];
            auto value = click_event["value"];
            if (action == "open_url")
                result.set_click_event_open_url(value);
            else if (action == "run_command")

                result.set_click_event_run_command(value);

            else if (action == "suggest_command")
                result.set_click_event_suggest_command(value);

            else if (action == "change_page")
                result.set_click_event_change_page(value);
            else if (action == "copy_to_clipboard")
                result.set_click_event_copy_to_clipboard(value);
        }
        if (json.contains("hoverEvent")) {
            auto hover_event = util::js_object::get_object(json["hoverEvent"]);
            std::string action = hover_event["action"];
            auto content = hover_event["content"];
            if (action == "show_item") {
                auto content_obj = util::js_object::get_object(content);
                if (content_obj.contains("tag"))
                    result.set_hover_event_show_item(content_obj["id"], content_obj["count"], (std::string)content_obj["tag"]);
                else
                    result.set_hover_event_show_item(content_obj["id"], content_obj["count"], std::nullopt);
            } else if (action == "show_entity") {
                auto content_obj = util::js_object::get_object(content);
                if (content_obj.contains("name"))
                    result.set_hover_event_show_entity(content_obj["id"], content_obj["type"], (std::string)content_obj["name"]);
                else
                    result.set_hover_event_show_item(content_obj["id"], content_obj["type"], std::nullopt);
            } else if (action == "show_text")
                result.set_hover_event_show_text(util::conversions::string::to_direct(content));
        }

        if (json.contains("extra")) {
            auto& extra_arr = result.get_extra();
            auto extra = util::js_array::get_array(json["extra"]);
            extra_arr.reserve(extra.size());
            for (auto it : extra)
                extra_arr.push_back(from_json(util::js_object::get_object(it)));
        }
        return result;
    }

    void formater(list_array<util::nbt>& items, chat& it) {
        if (it.get_text().empty() && items.size()) {
            std::string_view str(it.get_text());
            if (auto res = str.find("%"); res != str.npos) {
                chat insert[3]{it, {}, it};
                insert[0].set_text(std::string(str.substr(0, res)));
                insert[1] = chat::from_nbt(items.take_back());
                insert[2].set_text(std::string(str.substr(res + 1)));
                for (auto& i : insert[1].get_extra())
                    formater(items, i);

                if (res + 1 < str.size()) {
                    insert[2].set_text(std::string(str.substr(res + 1)));
                    it.get_extra().push_back(insert[0]);
                    it.get_extra().push_back(insert[1]);
                    it.get_extra().push_back(insert[2]);
                } else {
                    it.get_extra().push_back(insert[0]);
                    it.get_extra().push_back(insert[1]);
                }
            }
        }
        for (auto& i : it.get_extra())
            formater(items, i);
    };

    chat::click_event_s::~click_event_s() = default;
    chat::hover_event_s::~hover_event_s() = default;

    chat::chat() = default;

    chat::chat(const char* set_text, bool is_translation) {
        text = set_text;
        text_is_translation = is_translation;
    }

    chat::chat(std::initializer_list<chat> args) {
        bool first = true;
        for (auto& it : args) {
            if (first) {
                operator=(it);
                first = false;
            } else
                extra.push_back(it);
        }
    }

    chat::chat(const std::string& set_text, bool is_translation) {
        text = set_text;
        text_is_translation = is_translation;
    }

    chat::chat(std::string&& set_text, bool is_translation) {
        text = std::move(set_text);
        text_is_translation = is_translation;
    }

    chat::chat(const chat& copy) {
        operator=(copy);
    }

    chat::chat(chat&& copy) noexcept {
        operator=(std::move(copy));
    }

    chat& chat::operator=(const chat& copy) {
        text = copy.text;
        ;
        color = copy.color;
        insertion = copy.insertion;
        defined_bold = copy.defined_bold;
        defined_italic = copy.defined_italic;
        defined_underlined = copy.defined_underlined;
        defined_strikethrough = copy.defined_strikethrough;
        defined_obfuscated = copy.defined_obfuscated;
        bold = copy.bold;
        italic = copy.italic;
        underlined = copy.underlined;
        strikethrough = copy.strikethrough;
        obfuscated = copy.obfuscated;
        text_is_translation = copy.text_is_translation;
        extra = copy.extra;
        return *this;
    }

    chat& chat::operator=(chat&& copy) noexcept {
        text = std::move(copy.text);
        color = std::move(copy.color);
        insertion = std::move(copy.insertion);
        click_event = std::move(copy.click_event);
        hover_event = std::move(copy.hover_event);
        defined_bold = copy.defined_bold;
        defined_italic = copy.defined_italic;
        defined_underlined = copy.defined_underlined;
        defined_strikethrough = copy.defined_strikethrough;
        defined_obfuscated = copy.defined_obfuscated;
        bold = copy.bold;
        italic = copy.italic;
        underlined = copy.underlined;
        strikethrough = copy.strikethrough;
        obfuscated = copy.obfuscated;
        text_is_translation = copy.text_is_translation;
        extra = std::move(copy.extra);
        return *this;
    }

    chat::~chat() = default;

    chat& chat::set_text(const std::string& set_text) {
        text = set_text;
        ;
        text_is_translation = false;
        return *this;
    }

    chat& chat::set_translation(const std::string& set_text) {
        text = set_text;
        text_is_translation = true;
        return *this;
    }

    chat& chat::set_color(const std::string& set_text) {
        color = set_text;
        return *this;
    }

    chat& chat::set_color(dye_color _color) {
        switch (_color) {
        case dye_color::white:
            set_color("white");
            break;
        case dye_color::orange:
            set_color("orange");
            break;
        case dye_color::magenta:
            set_color("magenta");
            break;
        case dye_color::light_blue:
            set_color("light_blue");
            break;
        case dye_color::yellow:
            set_color("yellow");
            break;
        case dye_color::lime:
            set_color("lime");
            break;
        case dye_color::pink:
            set_color("pink");
            break;
        case dye_color::gray:
            set_color("gray");
            break;
        case dye_color::light_gray:
            set_color("light_gray");
            break;
        case dye_color::cyan:
            set_color("cyan");
            break;
        case dye_color::purple:
            set_color("purple");
            break;
        case dye_color::blue:
            set_color("blue");
            break;
        case dye_color::brown:
            set_color("brown");
            break;
        case dye_color::green:
            set_color("green");
            break;
        case dye_color::red:
            set_color("red");
            break;
        case dye_color::black:
            set_color("black");
            break;
        default:
            set_color();
            break;
        }
        return *this;
    }

    chat& chat::set_insertion(const std::string& set_text) {
        insertion = set_text;
        return *this;
    }

    chat& chat::set_font(const std::string& set_text) {
        font = set_text;
        return *this;
    }

    chat& chat::set_text(std::string&& set_text) {
        text = std::move(set_text);
        text_is_translation = false;
        return *this;
    }

    chat& chat::set_translation(std::string&& set_text) {
        text = std::move(set_text);
        text_is_translation = true;
        return *this;
    }

    chat& chat::set_color(std::string&& set_text) {
        color = std::move(set_text);
        return *this;
    }

    chat& chat::set_insertion(std::string&& set_text) {
        insertion = std::move(set_text);
        return *this;
    }

    chat& chat::set_font(std::string&& set_text) {
        font = std::move(set_text);
        return *this;
    }

    chat& chat::set_bold() {
        defined_bold = false;
        return *this;
    }

    chat& chat::set_italic() {
        defined_italic = false;
        return *this;
    }

    chat& chat::set_underlined() {
        defined_underlined = false;
        return *this;
    }

    chat& chat::set_strikethrough() {
        defined_strikethrough = false;
        return *this;
    }

    chat& chat::set_obfuscated() {
        defined_obfuscated = false;
        return *this;
    }

    chat& chat::set_bold(bool is) {
        bold = is;
        defined_bold = true;
        return *this;
    }

    chat& chat::set_italic(bool is) {
        italic = is;
        defined_italic = true;
        return *this;
    }

    chat& chat::set_underlined(bool is) {
        underlined = is;
        defined_underlined = true;
        return *this;
    }

    chat& chat::set_strikethrough(bool is) {
        strikethrough = is;
        defined_strikethrough = true;
        return *this;
    }

    chat& chat::set_obfuscated(bool is) {
        obfuscated = is;
        defined_obfuscated = true;
        return *this;
    }

    chat& chat::set_hover_event_show_text(const std::string& _show_text) {
        if (!hover_event)
            hover_event = std::make_unique<hover_event_s>();
        hover_event->show_entity.reset();
        hover_event->show_item.reset();
        hover_event->show_text = _show_text;
        return *this;
    }

    chat& chat::set_hover_event_show_item(const std::string& _id, int32_t _count, const std::optional<std::string>& _tag) {
        if (!hover_event)
            hover_event = std::make_unique<hover_event_s>();
        hover_event->show_entity.reset();
        hover_event->show_item = std::make_unique<hover_event_s::show_item_s>();
        hover_event->show_text.clear();

        hover_event->show_item->id = _id;
        hover_event->show_item->count = _count;
        hover_event->show_item->tag = _tag;
        return *this;
    }

    chat& chat::set_hover_event_show_entity(const std::string& _id, const std::string& _type, const std::optional<std::string>& _name) {
        if (!hover_event)
            hover_event = std::make_unique<hover_event_s>();
        hover_event->show_entity = std::make_unique<hover_event_s::show_entity_s>();
        hover_event->show_item.reset();
        hover_event->show_text.clear();

        hover_event->show_entity->id = _id;
        hover_event->show_entity->type = _type;
        hover_event->show_entity->name = _name;
        return *this;
    }

    chat& chat::set_hover_event() {
        hover_event.reset();
        return *this;
    }

    chat& chat::set_click_event_open_url(const std::string& _open_url) {
        if (!click_event)
            click_event = std::make_unique<click_event_s>();

        click_event->run_command.clear();
        click_event->suggest_command.clear();
        click_event->copy_to_clipboard.clear();
        click_event->change_page = std::nullopt;
        click_event->open_url = _open_url;
        return *this;
    }

    chat& chat::set_click_event_run_command(const std::string& _run_command) {
        if (!click_event)
            click_event = std::make_unique<click_event_s>();

        click_event->run_command.clear();
        click_event->suggest_command.clear();
        click_event->copy_to_clipboard.clear();
        click_event->change_page = std::nullopt;
        click_event->open_url.clear();

        click_event->run_command = _run_command;
        return *this;
    }

    chat& chat::set_click_event_suggest_command(const std::string& _suggest_command) {
        if (!click_event)
            click_event = std::make_unique<click_event_s>();

        click_event->run_command.clear();
        click_event->copy_to_clipboard.clear();
        click_event->change_page = std::nullopt;
        click_event->open_url.clear();
        click_event->suggest_command = _suggest_command;
        return *this;
    }

    chat& chat::set_click_event_change_page(int32_t _change_page) {
        if (!click_event)
            click_event = std::make_unique<click_event_s>();
        click_event->run_command.clear();
        click_event->suggest_command.clear();
        click_event->copy_to_clipboard.clear();
        click_event->open_url.clear();
        click_event->change_page = _change_page;
        return *this;
    }

    chat& chat::set_click_event_copy_to_clipboard(const std::string& _copy_to_clipboard) {
        if (!click_event)
            click_event = std::make_unique<click_event_s>();
        click_event->run_command.clear();
        click_event->suggest_command.clear();
        click_event->copy_to_clipboard.clear();
        click_event->change_page = std::nullopt;
        click_event->open_url.clear();
        click_event->copy_to_clipboard = _copy_to_clipboard;
        return *this;
    }

    chat& chat::set_click_event() {
        click_event.reset();
        return *this;
    }

    list_array<chat>& chat::get_extra() {
        return extra;
    }

    const std::string& chat::get_text() const {
        static const std::string empty;
        if (text_is_translation || text.empty())
            return empty;
        return text;
    }

    const std::string& chat::get_translation() const {
        static const std::string empty;
        if (!text_is_translation || text.empty())
            return empty;
        return text;
    }

    const std::string& chat::get_color() const {
        static const std::string empty;
        if (color.empty())
            return empty;
        return color;
    }

    const std::string& chat::get_insertion() const {
        static const std::string empty;
        if (insertion.empty())
            return empty;
        return insertion;
    }

    const std::string& chat::get_font() const {
        static const std::string empty;
        if (font.empty())
            return empty;
        return font;
    }

    const std::unique_ptr<chat::hover_event_s>& chat::get_hover_event() const {
        return hover_event;
    }

    const std::unique_ptr<chat::click_event_s>& chat::get_click_event() const {
        return click_event;
    }

    std::optional<bool> chat::get_bold() {
        if (defined_bold)
            return std::make_optional<bool>((bool)bold);
        else
            return std::nullopt;
    }

    std::optional<bool> chat::get_italic() {
        if (defined_italic)
            return std::make_optional<bool>((bool)italic);
        else
            return std::nullopt;
    }

    std::optional<bool> chat::get_underlined() {
        if (defined_underlined)
            return std::make_optional<bool>((bool)underlined);
        else
            return std::nullopt;
    }

    std::optional<bool> chat::get_strikethrough() {
        if (defined_strikethrough)
            return std::make_optional<bool>((bool)strikethrough);
        else
            return std::nullopt;
    }

    std::optional<bool> chat::get_obfuscated() {
        if (defined_obfuscated)
            return std::make_optional<bool>((bool)obfuscated);
        else
            return std::nullopt;
    }

    std::string chat::to_str() const {
        std::string str = "{";
        if (!text.empty()) {
            if (text_is_translation)
                str += "\"translation\":\"";
            else
                str += "\"text\":\"";
            str += text;
            str += "\"";
        } else {
            str += "\"text\":\"\"";
        }
        if (!color.empty()) {
            str += ',';
            str += "\"color\":\"";
            str += color;
            str += "\"";
        }
        if (!insertion.empty()) {
            str += ',';
            str += "\"insertion\":\"";
            str += insertion;
            str += "\"";
        }
        if (defined_bold) {
            str += ',';
            str += "\"bold\":\"";
            str += bold ? "true" : "false";
            str += "\"";
        }
        if (defined_italic) {
            str += ',';
            str += "\"italic\":\"";
            str += italic ? "true" : "false";
            str += "\"";
        }
        if (defined_underlined) {
            str += ',';
            str += "\"underlined\":\"";
            str += underlined ? "true" : "false";
            str += "\"";
        }
        if (defined_strikethrough) {
            str += ',';
            str += "\"strikethrough\":\"";
            str += strikethrough ? "true" : "false";
            str += "\"";
        }
        if (defined_obfuscated) {
            str += ',';
            str += "\"obfuscated\":\"";
            str += obfuscated ? "true" : "false";
            str += "\"";
        }
        if (click_event) {
            str += ',';
            str += "\"clickEvent\":{";
            if (!click_event->open_url.empty()) {
                str += "\"action\":\"open_url\",";
                str += "\"value\":\"";
                str += click_event->open_url;
                str += "\"";
            }
            if (!click_event->run_command.empty()) {
                str += "\"action\":\"run_command\",";
                str += "\"value\":\"";
                str += click_event->run_command;
                str += "\"";
            }
            if (!click_event->suggest_command.empty()) {
                str += "\"action\":\"suggest_command\",";
                str += "\"value\":\"";
                str += click_event->suggest_command;
                str += "\"";
            }
            if (!click_event->change_page) {
                str += "\"action\":\"change_page\",";
                str += "\"value\":";
                str += std::to_string(*click_event->change_page);
            }
            if (!click_event->copy_to_clipboard.empty()) {
                str += "\"action\":\"copy_to_clipboard\",";
                str += "\"value\":\"";
                str += click_event->copy_to_clipboard;
                str += "\"";
            }
            str += '}';
        }
        if (hover_event) {
            str += ',';
            str += "\"hoverEvent\":{";
            if (hover_event->show_item) {
                str += "\"action\":\"show_item\",";
                str += "\"contents\":{";
                str += "\"id\":\"";
                str += hover_event->show_item->id;
                str += "\",";
                str += "\"count\":";
                str += std::to_string(hover_event->show_item->count);
                if (hover_event->show_item->tag) {
                    str += ",\"tag\":\"";
                    str += *hover_event->show_item->tag;
                    str += "\"";
                }
                str += '}';
            }
            if (hover_event->show_entity) {
                str += "\"action\":\"show_entity\",";
                str += "\"contents\":{";
                str += "\"type\":\"";
                str += hover_event->show_entity->type;
                str += "\",";
                str += "\"id\":\"";
                str += hover_event->show_entity->id;
                if (hover_event->show_entity->name) {
                    str += "\",\"name\":\"";
                    str += *hover_event->show_entity->name;
                }
                str += "\"}";
            }
            if (!hover_event->show_text.empty()) {
                str += "\"action\":\"show_text\",";
                str += "\"contents\":\"";
                str += hover_event->show_text;
                str += "\"";
            }
            str += '}';
        }

        if (extra.size()) {
            str += ',';
            str += "\"extra\":[";
            bool need_comma = false;
            for (auto& it : extra) {
                if (need_comma)
                    str += ',';
                str += it.to_str();
                need_comma = true;
            }
        }
        str += '}';
        return str;
    }

    chat chat::from_str(std::string_view str) {
        auto json_hold = boost::json::parse(str);
        if (json_hold.is_string())
            return chat::parse_to_chat(json_hold.as_string().c_str());
        else
            return from_json(util::js_object::get_object(json_hold));
    }

    util::nbt chat::to_nbt() const {
        util::nbt_compound nbt;
        if (!text.empty()) {
            if (text_is_translation)
                nbt["translate"] = text;
            else
                nbt["text"] = text;
        } else {
            nbt["text"] = "";
        }
        if (!color.empty())
            nbt["color"] = color;
        if (!insertion.empty())
            nbt["insertion"] = insertion;
        if (defined_bold)
            nbt["bold"] = bold;
        if (defined_italic)
            nbt["italic"] = italic;
        if (defined_underlined)
            nbt["underlined"] = underlined;
        if (defined_strikethrough)
            nbt["strikethrough"] = strikethrough;
        if (defined_obfuscated)
            nbt["obfuscated"] = obfuscated;
        if (click_event) {
            util::nbt_compound click_event_nbt;
            if (!click_event->open_url.empty()) {
                click_event_nbt["action"] = "open_url";
                click_event_nbt["value"] = click_event->open_url;
            } else if (!click_event->run_command.empty()) {
                click_event_nbt["action"] = "run_command";
                click_event_nbt["value"] = click_event->run_command;
            } else if (!click_event->suggest_command.empty()) {
                click_event_nbt["action"] = "suggest_command";
                click_event_nbt["value"] = click_event->suggest_command;
            } else if (click_event->change_page) {
                click_event_nbt["action"] = "change_page";
                click_event_nbt["value"] = *click_event->change_page;
            } else if (!click_event->copy_to_clipboard.empty()) {
                click_event_nbt["action"] = "copy_to_clipboard";
                click_event_nbt["value"] = click_event->copy_to_clipboard;
            }
            nbt["clickEvent"] = click_event_nbt.take_map();
        }
        if (hover_event) {
            util::nbt_compound hover_event_nbt;
            if (hover_event->show_item) {
                util::nbt_compound show_item_nbt;
                show_item_nbt["id"] = hover_event->show_item->id;
                show_item_nbt["count"] = hover_event->show_item->count;
                if (hover_event->show_item->tag) {
                    show_item_nbt["tag"] = *hover_event->show_item->tag;
                }
                hover_event_nbt["action"] = "show_item";
                hover_event_nbt["contents"] = show_item_nbt.take_map();
            } else if (hover_event->show_entity) {
                util::nbt_compound show_entity_nbt;
                show_entity_nbt["type"] = hover_event->show_entity->type;
                show_entity_nbt["id"] = hover_event->show_entity->id;
                if (hover_event->show_entity->name) {
                    show_entity_nbt["name"] = *hover_event->show_entity->name;
                }
                hover_event_nbt["action"] = "show_entity";
                hover_event_nbt["contents"] = show_entity_nbt.take_map();
            } else if (!hover_event->show_text.empty()) {
                hover_event_nbt["action"] = "show_text";
                hover_event_nbt["contents"] = hover_event->show_text;
            }
            nbt["hoverEvent"] = hover_event_nbt.take_map();
        }

        if (extra.size()) {
            list_array<util::nbt> extra_nbt;
            extra_nbt.reserve(extra.size());
            size_t i = 0;
            for (auto& it : extra)
                extra_nbt.push_back(it.to_nbt());
            nbt["extra"] = std::move(extra_nbt);
        }
        if (nbt.size() == 1) {
            if (nbt.contains("text"))
                return nbt["text"];
        }
        return nbt.take_map();
    }

    void chat::remove_color() {
        color.clear();
    }

    void chat::remove_color_recursive() {
        color.clear();
        for (auto& it : extra)
            it.remove_color_recursive();
    }

    bool chat::empty() const {
        return text.empty() && extra.empty();
    }

    chat chat::parse_to_chat(std::string_view string) {
        list_array<chat> result;

        constexpr const char format_symbol_parts[2] = {(char)(unsigned char)194, (char)(unsigned char)167}; //{(char)0x_c2, (char)0x_a7};
        chat current_chat;

        bool format_command = false;
        bool got_first_part_format_symbol = false;
        bool got_slash = false;
        bool got_slash_except_part_format = false;
        bool got_utf_code_point = false;
        bool got_big_utf_code_point = false;

        std::string current_string;
        current_string.reserve(string.size());
        std::string utf_code_point;
        for (char c : string) {
            if (format_command) {
                switch (c) {
                case '0':
                    current_chat.set_color("black");
                    break;
                case '1':
                    current_chat.set_color("dark_blue");
                    break;
                case '2':
                    current_chat.set_color("dark_green");
                    break;
                case '3':
                    current_chat.set_color("dark_aqua");
                    break;
                case '4':
                    current_chat.set_color("dark_red");
                    break;
                case '5':
                    current_chat.set_color("dark_purple");
                    break;
                case '6':
                    current_chat.set_color("gold");
                    break;
                case '7':
                    current_chat.set_color("gray");
                    break;
                case '8':
                    current_chat.set_color("dark_gray");
                    break;
                case '9':
                    current_chat.set_color("blue");
                    break;
                case 'a':
                    current_chat.set_color("green");
                    break;
                case 'b':
                    current_chat.set_color("aqua");
                    break;
                case 'c':
                    current_chat.set_color("red");
                    break;
                case 'd':
                    current_chat.set_color("light_purple");
                    break;
                case 'e':
                    current_chat.set_color("yellow");
                    break;
                case 'f':
                    current_chat.set_color("white");
                    break;
                case 'k':
                    current_chat.set_obfuscated(true);
                    break;
                case 'l':
                    current_chat.set_bold(true);
                    break;
                case 'm':
                    current_chat.set_strikethrough(true);
                    break;
                case 'n':
                    current_chat.set_underlined(true);
                    break;
                case 'o':
                    current_chat.set_italic(true);
                    break;
                case 'r':
                    current_chat.remove_color();
                    current_chat.set_bold(false);
                    current_chat.set_italic(false);
                    current_chat.set_underlined(false);
                    current_chat.set_strikethrough(false);
                    current_chat.set_obfuscated(false);
                    break;
                default:
                    break;
                }
                format_command = false;
            } else if (got_utf_code_point) {
                utf_code_point += c;
                if (utf_code_point.size() == 4) {
                    utf8::utfchar16_t code_point = (utf8::utfchar16_t)std::stoi(utf_code_point, nullptr, 16);
                    char utf8_code_point[4];
                    current_string += std::string(utf8_code_point, utf8::utf16to8(&code_point, &code_point + 1, utf8_code_point));
                    got_utf_code_point = false;
                    utf_code_point.clear();
                }
            } else if (got_big_utf_code_point) {
                utf_code_point += c;
                if (utf_code_point.size() == 8) {
                    utf8::utfchar32_t code_point = (utf8::utfchar32_t)std::stoi(utf_code_point, nullptr, 16);
                    char utf8_code_point[4];
                    current_string += std::string(utf8_code_point, utf8::utf32to8(&code_point, &code_point + 1, utf8_code_point));
                    got_big_utf_code_point = false;
                    utf_code_point.clear();
                }
            } else if (got_slash) {
                if (got_slash_except_part_format) {
                    if (c != format_symbol_parts[1]) {
                        current_string += format_symbol_parts[1];
                        current_string += c;
                        got_slash_except_part_format = false;
                        got_slash = false;
                        continue;
                    }
                }
                switch (c) {
                case 'a':
                    current_string += '\a';
                    break;
                case 'n':
                    current_string += '\n';
                    break;
                case 't':
                    current_string += '\t';
                    break;
                case 'r':
                    current_string += '\r';
                    break;
                case 'f':
                    current_string += '\f';
                    break;
                case 'b':
                    current_string += '\b';
                    break;
                case '\\':
                    current_string += '\\';
                    break;
                case '\'':
                    current_string += '\'';
                    break;
                case '\"':
                    current_string += '\"';
                    break;
                case 'v':
                    current_string += '\v';
                    break;
                case 'u':
                    got_utf_code_point = true;
                    break;
                case 'U':
                    got_big_utf_code_point = true;
                    break;
                case format_symbol_parts[0]:
                    current_string += format_symbol_parts[0];
                    got_slash_except_part_format = true;
                    break;
                case format_symbol_parts[1]:
                    current_string += format_symbol_parts[1];
                    got_slash_except_part_format = false;
                    break;
                default:
                    current_string += '\\';
                    current_string += c;
                    break;
                }
                if (c != format_symbol_parts[0])
                    got_slash = false;
            } else if (c == format_symbol_parts[0]) {
                got_first_part_format_symbol = true;
            } else if (c == format_symbol_parts[1]) {
                if (current_string.size()) {
                    current_chat.set_text(current_string);
                    result.push_back(current_chat);
                    current_chat.set_text();
                    current_string.clear();
                }
                got_first_part_format_symbol = false;
                format_command = true;
            } else if (got_first_part_format_symbol) {
                current_string += format_symbol_parts[0];
                current_string += c;
                got_first_part_format_symbol = false;
            } else if (c == '\\') {
                got_slash = true;
            } else
                current_string += c;
        }
        if (got_slash) {
            current_string += '\\';
            got_slash = false;
        }
        if (current_string.size()) {
            current_chat.set_text(current_string);
            result.push_back(current_chat);
        }
        chat final_chat;
        final_chat.get_extra() = result;
        return final_chat;
    }

    chat chat::from_nbt(const util::nbt& nbt) {
        if (nbt.is_string())
            return chat(nbt.get_string());
        chat result;
        auto& entry = nbt.get_compound();

        if (entry.contains("text"))
            result.set_text(entry.at("text").get_string());
        else if (entry.contains("translate"))
            result.set_translation(entry.at("translate").get_string());

        if (entry.contains("color"))
            result.set_color(entry.at("color").get_string());

        if (entry.contains("insertion"))
            result.set_insertion(entry.at("insertion").get_string());

        if (entry.contains("bold"))
            result.set_bold(entry.at("bold").get_byte());

        if (entry.contains("italic"))
            result.set_italic(entry.at("italic").get_byte());

        if (entry.contains("underlined"))
            result.set_underlined(entry.at("underlined").get_byte());

        if (entry.contains("strikethrough"))
            result.set_strikethrough(entry.at("strikethrough").get_byte());

        if (entry.contains("obfuscated"))
            result.set_obfuscated(entry.at("obfuscated").get_byte());

        if (entry.contains("font"))
            result.set_font(entry.at("font").get_string());

        if (entry.contains("clickEvent")) {
            auto& click_event = entry.at("clickEvent").get_compound();
            const std::string& action = click_event.at("action").get_string();
            auto& value = click_event.at("value");
            if (action == "open_url")
                result.set_click_event_open_url(value.get_string());
            else if (action == "run_command")

                result.set_click_event_run_command(value.get_string());

            else if (action == "suggest_command")
                result.set_click_event_suggest_command(value.get_string());

            else if (action == "change_page")
                result.set_click_event_change_page(value.get_int());
            else if (action == "copy_to_clipboard")
                result.set_click_event_copy_to_clipboard(value.get_string());
        }
        if (entry.contains("hoverEvent")) {
            auto& hover_event = entry.at("hoverEvent").get_compound();
            const std::string& action = hover_event.at("action").get_string();
            auto& content = hover_event.at("content");
            if (action == "show_item") {
                if (content.get_compound().contains("tag"))
                    result.set_hover_event_show_item(content.at("id").get_string(), content.at("count").get_int(), content.at("tag").get_string());
                else
                    result.set_hover_event_show_item(content.at("id").get_string(), content.at("count").get_int(), std::nullopt);
            } else if (action == "show_entity") {
                if (content.get_compound().contains("name"))
                    result.set_hover_event_show_entity(content.at("id").get_string(), content.at("type").get_string(), content.at("name").get_string());
                else
                    result.set_hover_event_show_entity(content.at("id").get_string(), content.at("type").get_string(), std::nullopt);
            } else if (action == "show_text")
                result.set_hover_event_show_text(content.get_string());
        }

        if (entry.contains("extra")) {
            auto& extra_arr = result.get_extra();
            auto& extra = entry.at("extra").get_list();
            extra_arr.reserve(extra.size());
            for (auto& it : extra)
                extra_arr.push_back(chat::from_nbt(it));
        }
        return result;
    }

    chat chat::from_nbt_with_format(const util::nbt& nbt, list_array<util::nbt>&& items) {
        auto res = from_nbt(nbt);
        formater(items, res);
        return res;
    }

    std::string chat::to_ansi_console() const {
        std::string result;
        if (!color.empty()) {
            if (color == "black")
                result += "\033[30m";
            else if (color == "dark_blue")
                result += "\033[34m";
            else if (color == "dark_green")
                result += "\033[32m";
            else if (color == "dark_aqua")
                result += "\033[36m";
            else if (color == "dark_red")
                result += "\033[31m";
            else if (color == "dark_purple")
                result += "\033[35m";
            else if (color == "gold")
                result += "\033[33m";
            else if (color == "gray")
                result += "\033[37m";
            else if (color == "dark_gray")
                result += "\033[90m";
            else if (color == "blue")
                result += "\033[94m";
            else if (color == "green")
                result += "\033[92m";
            else if (color == "aqua")
                result += "\033[96m";
            else if (color == "red")
                result += "\033[91m";
            else if (color == "light_purple")
                result += "\033[95m";
            else if (color == "yellow")
                result += "\033[93m";
            else if (color == "white")
                result += "\033[97m";
        }
        if (bold)
            result += "\033[1m";
        if (italic)
            result += "\033[3m";
        if (underlined)
            result += "\033[4m";
        if (strikethrough)
            result += "\033[9m";
        if (obfuscated)
            result += "\033[8m";
        if (!text.empty())
            result += text;
        if (bold || italic || underlined || strikethrough || obfuscated)
            result += "\033[0m";

        for (auto& it : extra)
            result += it.to_ansi_console();
        return result;
    }

    bool chat::operator==(const chat& other) const {
        if (
            defined_bold != other.defined_bold
            || defined_italic != other.defined_italic
            || defined_underlined != other.defined_underlined
            || defined_strikethrough != other.defined_strikethrough
            || defined_obfuscated != other.defined_obfuscated
        )
            return false;

        if (bool(click_event) != bool(other.click_event))
            return false;
        if (bool(hover_event) != bool(other.hover_event))
            return false;


        if (!text.empty()) {
            if (text_is_translation != other.text_is_translation)
                return false;
            if (text != other.text)
                return false;
        }
        if (!color.empty())
            if (color != other.color)
                return false;
        if (!insertion.empty())
            if (insertion != other.insertion)
                return false;
        if (!font.empty())
            if (font != other.font)
                return false;

        if (click_event) {
            if (click_event->change_page)
                if (*click_event->change_page != *other.click_event->change_page)
                    return false;
            if (click_event->copy_to_clipboard != other.click_event->copy_to_clipboard)
                return false;
            if (click_event->open_url != other.click_event->open_url)
                return false;
            if (click_event->run_command != other.click_event->run_command)
                return false;
            if (click_event->suggest_command != other.click_event->suggest_command)
                return false;
        }
        if (hover_event) {
            if (
                (!hover_event->show_entity != !other.hover_event->show_entity)
                | (!hover_event->show_item != !other.hover_event->show_item)
            )
                return false;
            if (hover_event->show_text != other.hover_event->show_text)
                return false;

            if (hover_event->show_entity) {
                if (hover_event->show_entity->id != other.hover_event->show_entity->id)
                    return false;
                if (hover_event->show_entity->name != other.hover_event->show_entity->name)
                    return false;
                if (hover_event->show_entity->type != other.hover_event->show_entity->type)
                    return false;
            }

            if (hover_event->show_item) {
                if (hover_event->show_item->id != other.hover_event->show_item->id)
                    return false;
                if (hover_event->show_item->count != other.hover_event->show_item->count)
                    return false;
                if (hover_event->show_item->tag != other.hover_event->show_item->tag)
                    return false;
            }
        }

        if (defined_bold)
            if (bold != other.bold)
                return false;
        if (defined_italic)
            if (italic != other.italic)
                return false;
        if (defined_underlined)
            if (underlined != other.underlined)
                return false;
        if (defined_strikethrough)
            if (strikethrough != other.strikethrough)
                return false;
        if (defined_obfuscated)
            if (obfuscated != other.obfuscated)
                return false;

        if (extra != other.extra)
            return false;
        return true;
    }

    bool chat::operator!=(const chat& other) const {
        return !operator==(other);
    }
}