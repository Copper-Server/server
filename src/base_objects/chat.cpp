
#include <src/base_objects/chat.hpp>
#include <src/protocolHelperNBT.hpp>
#include <src/util/conversions.hpp>
#include <src/util/json_helpers.hpp>
#include <utf8.h>

namespace copper_server {
    Chat Chat::parseToChat(const std::string& string) {
        list_array<Chat> result;

        constexpr const uint32_t format_symbol = U'§';
        constexpr const unsigned char format_symbol_parts[2] = {0xC2, 0xA7};
        Chat current_chat;

        bool format_command = false;
        bool got_first_part_format_symbol = false;
        bool got_slash = false;
        bool got_slash_except_part_format = false;
        bool got_utf_code_point = false;
        bool got_big_utf_code_point = false;
        bool got_variable_utf_code_point = false;

        std::string current_string;
        current_string.reserve(string.size());
        std::string utf_code_point;
        for (char c : string) {
            if (format_command) {
                switch (c) {
                case '0':
                    current_chat.SetColor("black");
                    break;
                case '1':
                    current_chat.SetColor("dark_blue");
                    break;
                case '2':
                    current_chat.SetColor("dark_green");
                    break;
                case '3':
                    current_chat.SetColor("dark_aqua");
                    break;
                case '4':
                    current_chat.SetColor("dark_red");
                    break;
                case '5':
                    current_chat.SetColor("dark_purple");
                    break;
                case '6':
                    current_chat.SetColor("gold");
                    break;
                case '7':
                    current_chat.SetColor("gray");
                    break;
                case '8':
                    current_chat.SetColor("dark_gray");
                    break;
                case '9':
                    current_chat.SetColor("blue");
                    break;
                case 'a':
                    current_chat.SetColor("green");
                    break;
                case 'b':
                    current_chat.SetColor("aqua");
                    break;
                case 'c':
                    current_chat.SetColor("red");
                    break;
                case 'd':
                    current_chat.SetColor("light_purple");
                    break;
                case 'e':
                    current_chat.SetColor("yellow");
                    break;
                case 'f':
                    current_chat.SetColor("white");
                    break;
                case 'k':
                    current_chat.SetObfuscated(true);
                    break;
                case 'l':
                    current_chat.SetBold(true);
                    break;
                case 'm':
                    current_chat.SetStrikethrough(true);
                    break;
                case 'n':
                    current_chat.SetUnderlined(true);
                    break;
                case 'o':
                    current_chat.SetItalic(true);
                    break;
                case 'r':
                    current_chat.removeColor();
                    current_chat.SetBold(false);
                    current_chat.SetItalic(false);
                    current_chat.SetUnderlined(false);
                    current_chat.SetStrikethrough(false);
                    current_chat.SetObfuscated(false);
                    break;
                default:
                    break;
                }
                format_command = false;
            } else if (got_utf_code_point) {
                utf_code_point += c;
                if (utf_code_point.size() == 4) {
                    utf8::utfchar16_t code_point = std::stoi(utf_code_point, nullptr, 16);
                    char utf8_code_point[4];
                    current_string += std::string(utf8_code_point, utf8::utf16to8(&code_point, &code_point + 1, utf8_code_point));
                    got_utf_code_point = false;
                    utf_code_point.clear();
                }
            } else if (got_big_utf_code_point) {
                utf_code_point += c;
                if (utf_code_point.size() == 8) {
                    utf8::utfchar32_t code_point = std::stoi(utf_code_point, nullptr, 16);
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
                    current_chat.SetText(current_string);
                    result.push_back(current_chat);
                    current_chat.SetText();
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
            current_chat.SetText(current_string);
            result.push_back(current_chat);
        }
        Chat final_chat;
        final_chat.GetExtra() = result;
        return final_chat;
    }

    std::string Chat::to_ansi_console() const {
        std::string result;
        if (color) {
            if (strcmp(color, "black") == 0)
                result += "\033[30m";
            else if (strcmp(color, "dark_blue") == 0)
                result += "\033[34m";
            else if (strcmp(color, "dark_green") == 0)
                result += "\033[32m";
            else if (strcmp(color, "dark_aqua") == 0)
                result += "\033[36m";
            else if (strcmp(color, "dark_red") == 0)
                result += "\033[31m";
            else if (strcmp(color, "dark_purple") == 0)
                result += "\033[35m";
            else if (strcmp(color, "gold") == 0)
                result += "\033[33m";
            else if (strcmp(color, "gray") == 0)
                result += "\033[37m";
            else if (strcmp(color, "dark_gray") == 0)
                result += "\033[90m";
            else if (strcmp(color, "blue") == 0)
                result += "\033[94m";
            else if (strcmp(color, "green") == 0)
                result += "\033[92m";
            else if (strcmp(color, "aqua") == 0)
                result += "\033[96m";
            else if (strcmp(color, "red") == 0)
                result += "\033[91m";
            else if (strcmp(color, "light_purple") == 0)
                result += "\033[95m";
            else if (strcmp(color, "yellow") == 0)
                result += "\033[93m";
            else if (strcmp(color, "white") == 0)
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
        if (text)
            result += text;
        if (bold || italic || underlined || strikethrough || obfuscated)
            result += "\033[0m";

        for (auto& it : extra)
            result += it.to_ansi_console();
        return result;
    }

    Chat Chat::fromEnbt(const enbt::value& enbt) {
        if (enbt.is_string())
            return Chat((std::string)enbt);
        Chat result;
        auto entry = enbt.as_compound();

        if (entry.contains("text"))
            result.SetText(entry["text"]);
        else if (entry.contains("translate"))
            result.SetTranslation(entry["translate"]);

        if (entry.contains("color"))
            result.SetColor(entry["color"]);

        if (entry.contains("insertion"))
            result.SetInsertion(entry["insertion"]);

        if (entry.contains("bold"))
            result.SetBold(entry["bold"]);

        if (entry.contains("italic"))
            result.SetItalic(entry["italic"]);

        if (entry.contains("underlined"))
            result.SetUnderlined(entry["underlined"]);

        if (entry.contains("strikethrough"))
            result.SetStrikethrough(entry["strikethrough"]);

        if (entry.contains("obfuscated"))
            result.SetObfuscated(entry["obfuscated"]);

        if (entry.contains("insertion"))
            result.SetInsertion(entry["insertion"]);

        if (entry.contains("insertion"))
            result.SetInsertion(entry["insertion"]);

        if (entry.contains("clickEvent")) {
            auto click_event = entry["clickEvent"].as_compound();
            const std::string& action = (const std::string&)click_event["action"];
            auto& value = click_event["value"];
            if (action == "open_url")
                result.SetClickEventOpenUrl(value);
            else if (action == "run_command")

                result.SetClickEventRunCommand(value);

            else if (action == "suggest_command")
                result.SetClickEventSuggestCommand(value);

            else if (action == "change_page")
                result.SetClickEventChangePage(value);
            else if (action == "copy_to_clipboard")
                result.SetClickEventCopyToClipboard(value);
        }
        if (entry.contains("hoverEvent")) {
            auto hover_event = entry["hoverEvent"].as_compound();
            const std::string& action = (const std::string&)hover_event["action"];
            auto& content = hover_event["content"];
            if (action == "show_item") {
                if (content.contains("tag"))
                    result.SetHoverEventShowItem(content["id"], content["count"], (std::string)content["tag"]);
                else
                    result.SetHoverEventShowItem(content["id"], content["count"], std::nullopt);
            } else if (action == "show_entity") {
                if (content.contains("name"))
                    result.SetHoverEventShowEntity(content["id"], content["type"], (std::string)content["name"]);
                else
                    result.SetHoverEventShowItem(content["id"], content["type"], std::nullopt);
            } else if (action == "show_text")
                result.SetHoverEventShowText(content);
        }

        if (entry.contains("extra")) {
            auto& extra_arr = result.GetExtra();
            auto extra = entry["extra"].as_fixed_array();
            extra_arr.reserve(extra.size());
            for (auto& it : extra)
                extra_arr.push_back(Chat::fromEnbt(it));
        }
        return result;
    }

    Chat Chat::fromTextComponent(const list_array<uint8_t>& enbt) {
        return fromEnbt(NBT::build(enbt).get_as_enbt());
    }

    std::string Chat::ToStr() const {
        std::string str = "{";
        if (text) {
            if (text_is_translation)
                str += "\"translation\":\"";
            else
                str += "\"text\":\"";
            str += text;
            str += "\"";
        } else {
            str += "\"text\":\"\"";
        }
        if (color) {
            str += ',';
            str += "\"color\":\"";
            str += color;
            str += "\"";
        }
        if (insertion) {
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
        if (clickEvent) {
            str += ',';
            str += "\"clickEvent\":{";
            if (clickEvent->open_url) {
                str += "\"action\":\"open_url\",";
                str += "\"value\":\"";
                str += clickEvent->open_url;
                str += "\"";
            }
            if (clickEvent->run_command) {
                str += "\"action\":\"run_command\",";
                str += "\"value\":\"";
                str += clickEvent->run_command;
                str += "\"";
            }
            if (clickEvent->suggest_command) {
                str += "\"action\":\"suggest_command\",";
                str += "\"value\":\"";
                str += clickEvent->suggest_command;
                str += "\"";
            }
            if (clickEvent->change_page) {
                str += "\"action\":\"change_page\",";
                str += "\"value\":";
                str += std::to_string(*clickEvent->change_page);
            }
            if (clickEvent->copy_to_clipboard) {
                str += "\"action\":\"copy_to_clipboard\",";
                str += "\"value\":\"";
                str += clickEvent->copy_to_clipboard;
                str += "\"";
            }
            str += '}';
        }
        if (hoverEvent) {
            str += ',';
            str += "\"hoverEvent\":{";
            if (hoverEvent->show_item) {
                str += "\"action\":\"show_item\",";
                str += "\"contents\":{";
                str += "\"id\":\"";
                str += hoverEvent->show_item->id;
                str += "\",";
                str += "\"count\":";
                str += std::to_string(hoverEvent->show_item->count);
                if (hoverEvent->show_item->tag) {
                    str += ",\"tag\":\"";
                    str += *hoverEvent->show_item->tag;
                    str += "\"";
                }
                str += '}';
            }
            if (hoverEvent->show_entity) {
                str += "\"action\":\"show_entity\",";
                str += "\"contents\":{";
                str += "\"type\":\"";
                str += hoverEvent->show_entity->type;
                str += "\",";
                str += "\"id\":\"";
                str += hoverEvent->show_entity->id;
                if (hoverEvent->show_entity->name) {
                    str += "\",\"name\":\"";
                    str += *hoverEvent->show_entity->name;
                }
                str += "\"}";
            }
            if (hoverEvent->show_text) {
                str += "\"action\":\"show_text\",";
                str += "\"contents\":\"";
                str += hoverEvent->show_text;
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
                str += it.ToStr();
                need_comma = true;
            }
        }
        str += '}';
        return str;
    }

    Chat fromJson(util::js_object&& json) {
        using namespace util;
        Chat result;
        if (json.contains("text"))
            result.SetText(util::conversions::string::to_direct(json["text"]));
        else if (json.contains("translate"))
            result.SetTranslation(json["translate"]);

        if (json.contains("color"))
            result.SetColor(json["color"]);

        if (json.contains("insertion"))
            result.SetInsertion(json["insertion"]);

        if (json.contains("bold"))
            result.SetBold(json["bold"]);

        if (json.contains("italic"))
            result.SetItalic(json["italic"]);

        if (json.contains("underlined"))
            result.SetUnderlined(json["underlined"]);

        if (json.contains("strikethrough"))
            result.SetStrikethrough(json["strikethrough"]);

        if (json.contains("obfuscated"))
            result.SetObfuscated(json["obfuscated"]);

        if (json.contains("insertion"))
            result.SetInsertion(json["insertion"]);

        if (json.contains("insertion"))
            result.SetInsertion(json["insertion"]);

        if (json.contains("clickEvent")) {
            auto click_event = js_object::get_object(json["clickEvent"]);
            std::string action = click_event["action"];
            auto value = click_event["value"];
            if (action == "open_url")
                result.SetClickEventOpenUrl(value);
            else if (action == "run_command")

                result.SetClickEventRunCommand(value);

            else if (action == "suggest_command")
                result.SetClickEventSuggestCommand(value);

            else if (action == "change_page")
                result.SetClickEventChangePage(value);
            else if (action == "copy_to_clipboard")
                result.SetClickEventCopyToClipboard(value);
        }
        if (json.contains("hoverEvent")) {
            auto hover_event = js_object::get_object(json["hoverEvent"]);
            std::string action = hover_event["action"];
            auto content = hover_event["content"];
            if (action == "show_item") {
                auto content_obj = js_object::get_object(content);
                if (content_obj.contains("tag"))
                    result.SetHoverEventShowItem(content_obj["id"], content_obj["count"], (std::string)content_obj["tag"]);
                else
                    result.SetHoverEventShowItem(content_obj["id"], content_obj["count"], std::nullopt);
            } else if (action == "show_entity") {
                auto content_obj = js_object::get_object(content);
                if (content_obj.contains("name"))
                    result.SetHoverEventShowEntity(content_obj["id"], content_obj["type"], (std::string)content_obj["name"]);
                else
                    result.SetHoverEventShowItem(content_obj["id"], content_obj["type"], std::nullopt);
            } else if (action == "show_text")
                result.SetHoverEventShowText(util::conversions::string::to_direct(content));
        }

        if (json.contains("extra")) {
            auto& extra_arr = result.GetExtra();
            auto extra = js_array::get_array(json["extra"]);
            extra_arr.reserve(extra.size());
            for (auto it : extra)
                extra_arr.push_back(fromJson(js_object::get_object(it)));
        }
        return result;
    }

    Chat Chat::fromStr(const std::string& str) {
        auto json_hold = boost::json::parse(str);
        if (json_hold.is_string())
            return Chat::parseToChat(json_hold.as_string().c_str());
        else
            return fromJson(util::js_object::get_object(json_hold));
    }

    enbt::value Chat::ToENBT() const {
        enbt::compound enbt;
        if (text) {
            if (text_is_translation)
                enbt["translate"] = text;
            else
                enbt["text"] = text;
        } else {
            enbt["text"] = "";
        }
        if (color)
            enbt["color"] = color;
        if (insertion)
            enbt["insertion"] = insertion;
        if (defined_bold)
            enbt["bold"] = bold;
        if (defined_italic)
            enbt["italic"] = italic;
        if (defined_underlined)
            enbt["underlined"] = underlined;
        if (defined_strikethrough)
            enbt["strikethrough"] = strikethrough;
        if (defined_obfuscated)
            enbt["obfuscated"] = obfuscated;
        if (clickEvent) {
            enbt::compound clickEvent_enbt;
            if (clickEvent->open_url) {
                clickEvent_enbt["action"] = "open_url";
                clickEvent_enbt["value"] = clickEvent->open_url;
            } else if (clickEvent->run_command) {
                clickEvent_enbt["action"] = "run_command";
                clickEvent_enbt["value"] = clickEvent->run_command;
            } else if (clickEvent->suggest_command) {
                clickEvent_enbt["action"] = "suggest_command";
                clickEvent_enbt["value"] = clickEvent->suggest_command;
            } else if (clickEvent->change_page) {
                clickEvent_enbt["action"] = "change_page";
                clickEvent_enbt["value"] = *clickEvent->change_page;
            } else if (clickEvent->copy_to_clipboard) {
                clickEvent_enbt["action"] = "copy_to_clipboard";
                clickEvent_enbt["value"] = clickEvent->copy_to_clipboard;
            }
            enbt["clickEvent"] = std::move(clickEvent_enbt);
        }
        if (hoverEvent) {
            enbt::compound hoverEvent_enbt;
            if (hoverEvent->show_item) {
                enbt::compound show_item_enbt;
                show_item_enbt["id"] = hoverEvent->show_item->id;
                show_item_enbt["count"] = hoverEvent->show_item->count;
                if (hoverEvent->show_item->tag) {
                    show_item_enbt["tag"] = *hoverEvent->show_item->tag;
                }
                hoverEvent_enbt["action"] = "show_item";
                hoverEvent_enbt["contents"] = std::move(show_item_enbt);
            } else if (hoverEvent->show_entity) {
                enbt::compound show_entity_enbt;
                show_entity_enbt["type"] = hoverEvent->show_entity->type;
                show_entity_enbt["id"] = hoverEvent->show_entity->id;
                if (hoverEvent->show_entity->name) {
                    show_entity_enbt["name"] = *hoverEvent->show_entity->name;
                }
                hoverEvent_enbt["action"] = "show_entity";
                hoverEvent_enbt["contents"] = std::move(show_entity_enbt);
            } else if (hoverEvent->show_text) {
                hoverEvent_enbt["action"] = "show_text";
                hoverEvent_enbt["contents"] = hoverEvent->show_text;
            }
            enbt["hoverEvent"] = std::move(hoverEvent_enbt);
        }

        if (extra.size()) {
            enbt::fixed_array extra_enbt(extra.size());
            size_t i = 0;
            for (auto& it : extra)
                extra_enbt.set(i++, it.ToENBT());
            enbt["extra"] = std::move(extra_enbt);
        }
        if (enbt.size() == 1) {
            if (enbt.contains("text"))
                return enbt["text"];
        }
        return enbt;
    }

    list_array<uint8_t> Chat::ToTextComponent() const {
        return NBT::build(ToENBT()).get_as_normal();
    }

    void Chat::setString(char*& char_ptr, const std::string& string) {
        if (string.contains("\"{}")) {
            std::string new_string;
            for (auto& it : string) {
                if (it == '\"' || it == '{' || it == '}')
                    new_string += '\\';
                new_string += it;
            }
            setString(char_ptr, new_string);
            return;
        }
        if (size_t str_len = string.size(); str_len) {
            str_len++;
            if (char_ptr)
                delete[] char_ptr;
            char_ptr = new char[str_len + 1];
            for (size_t i = 0; i < str_len; i++)
                char_ptr[i] = string[i];
            char_ptr[str_len] = 0;
        } else if (char_ptr) {
            delete[] char_ptr;
            char_ptr = nullptr;
        }
    }

    void Chat::setHoverEvent(hoverEventS::show_itemS* setHoverEvent) {
        if (!hoverEvent)
            hoverEvent = new hoverEventS;
        if (hoverEvent->show_item)
            delete hoverEvent->show_item;
        hoverEvent->show_item = setHoverEvent;

        if (hoverEvent->show_entity)
            delete hoverEvent->show_entity;
        hoverEvent->show_entity = nullptr;

        if (hoverEvent->show_text)
            delete hoverEvent->show_text;

        hoverEvent->show_text = nullptr;
    }

    void Chat::setHoverEvent(hoverEventS::show_entityS* setHoverEvent) {
        if (!hoverEvent)
            hoverEvent = new hoverEventS;
        if (hoverEvent->show_entity)
            delete hoverEvent->show_entity;
        hoverEvent->show_entity = setHoverEvent;

        if (hoverEvent->show_item)
            delete hoverEvent->show_item;
        hoverEvent->show_item = nullptr;

        if (hoverEvent->show_text)
            delete hoverEvent->show_text;

        hoverEvent->show_text = nullptr;
    }

    void Chat::setHoverEvent(const std::string& setHoverEvent) {
        if (!hoverEvent)
            hoverEvent = new hoverEventS;
        if (hoverEvent->show_text)
            delete hoverEvent->show_text;
        hoverEvent->show_text = nullptr;
        setString(hoverEvent->show_text, setHoverEvent);

        if (hoverEvent->show_item)
            delete hoverEvent->show_item;
        hoverEvent->show_item = nullptr;

        if (hoverEvent->show_entity)
            delete hoverEvent->show_entity;
        hoverEvent->show_entity = nullptr;
    }

    bool Chat::operator==(const Chat& other) const {
        if (
            defined_bold != other.defined_bold
            | defined_italic != other.defined_italic
            | defined_underlined != other.defined_underlined
            | defined_strikethrough != other.defined_strikethrough
            | defined_obfuscated != other.defined_obfuscated
        )
            return false;

        if (
            (!text != !other.text)
            | (!color != !other.color)
            | (!insertion != !other.insertion)
            | (!font != !other.font)
            | (!clickEvent != !other.clickEvent)
            | (!hoverEvent != !other.hoverEvent)
        )
            return false;

        if (text) {
            if (text_is_translation != other.text_is_translation)
                return false;
            if (strcmp(text, other.text))
                return false;
        }
        if (color)
            if (strcmp(color, other.color))
                return false;
        if (insertion)
            if (strcmp(insertion, other.insertion))
                return false;
        if (font)
            if (strcmp(font, other.font))
                return false;

        if (clickEvent) {
            if (
                (!clickEvent->change_page != !other.clickEvent->change_page)
                | (!clickEvent->copy_to_clipboard != !other.clickEvent->copy_to_clipboard)
                | (!clickEvent->open_url != !other.clickEvent->open_url)
                | (!clickEvent->run_command != !other.clickEvent->run_command)
                | (!clickEvent->suggest_command != !other.clickEvent->suggest_command)
            )
                return false;

            if (clickEvent->change_page)
                if (*clickEvent->change_page != *other.clickEvent->change_page)
                    return false;

            if (clickEvent->copy_to_clipboard)
                if (strcmp(clickEvent->copy_to_clipboard, other.clickEvent->copy_to_clipboard))
                    return false;
            if (clickEvent->open_url)
                if (strcmp(clickEvent->open_url, other.clickEvent->open_url))
                    return false;
            if (clickEvent->run_command)
                if (strcmp(clickEvent->run_command, other.clickEvent->run_command))
                    return false;
            if (clickEvent->suggest_command)
                if (strcmp(clickEvent->suggest_command, other.clickEvent->suggest_command))
                    return false;
        }
        if (hoverEvent) {
            if (
                (!hoverEvent->show_entity != !other.hoverEvent->show_entity)
                | (!hoverEvent->show_item != !other.hoverEvent->show_item)
                | (!hoverEvent->show_text != !other.hoverEvent->show_text)
            )
                return false;
            if (hoverEvent->show_text)
                if (strcmp(hoverEvent->show_text, other.hoverEvent->show_text))
                    return false;

            if (hoverEvent->show_entity) {
                if (hoverEvent->show_entity->id != other.hoverEvent->show_entity->id)
                    return false;
                if (hoverEvent->show_entity->name != other.hoverEvent->show_entity->name)
                    return false;
                if (hoverEvent->show_entity->type != other.hoverEvent->show_entity->type)
                    return false;
            }

            if (hoverEvent->show_item) {
                if (hoverEvent->show_item->id != other.hoverEvent->show_item->id)
                    return false;
                if (hoverEvent->show_item->count != other.hoverEvent->show_item->count)
                    return false;
                if (hoverEvent->show_item->tag != other.hoverEvent->show_item->tag)
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
}