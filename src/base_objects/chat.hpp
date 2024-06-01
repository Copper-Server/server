#ifndef SRC_BASE_OBJECTS_CHAT
#define SRC_BASE_OBJECTS_CHAT
#include "../library/list_array.hpp"
#include "../protocolHelperNBT.hpp"
#include <optional>
#include <string>
#include <vector>

namespace crafted_craft {
    struct Chat {
        struct clickEventS {
            char* open_url;
            char* run_command;
            char* suggest_command;
            uint32_t* change_page;
            char* copy_to_clipboard;

            ~clickEventS() {
                if (open_url)
                    delete open_url;
                if (run_command)
                    delete run_command;
                if (suggest_command)
                    delete suggest_command;
                if (change_page)
                    delete change_page;
                if (copy_to_clipboard)
                    delete copy_to_clipboard;


                open_url = nullptr;
                run_command = nullptr;
                suggest_command = nullptr;
                change_page = nullptr;
                copy_to_clipboard = nullptr;
            }
        };

        struct hoverEventS {
            struct show_itemS {
                std::optional<std::string> tag;
                std::string id;
                int32_t count;
            }* show_item;

            struct show_entityS {
                std::optional<std::string> name;
                std::string type;
                std::string id;
            }* show_entity;

            char* show_text;

            ~hoverEventS() {
                if (show_text)
                    delete show_text;
                if (show_item)
                    delete show_item;
                if (show_entity)
                    delete show_entity;
            }
        };

        Chat() = default;

        Chat(const char* set_text, bool is_translation = false) {
            setString(text, set_text);
            text_is_translation = is_translation;
        }

        Chat(const std::string& set_text, bool is_translation = false) {
            setString(text, set_text);
            text_is_translation = is_translation;
        }

        Chat(const Chat& copy) {
            operator=(copy);
        }

        Chat(Chat&& copy) noexcept {
            operator=(std::move(copy));
        }

        Chat& operator=(const Chat& copy) {
            if (copy.text)
                setString(text, copy.text);
            if (copy.color)
                setString(color, copy.color);
            if (copy.insertion)
                setString(insertion, copy.insertion);
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
            extra = extra;
            return *this;
        }

        Chat& operator=(Chat&& copy) noexcept {
            text = copy.text;
            copy.text = nullptr;
            color = copy.color;
            copy.color = nullptr;
            insertion = copy.insertion;
            copy.insertion = nullptr;
            clickEvent = copy.clickEvent;
            copy.clickEvent = nullptr;
            hoverEvent = copy.hoverEvent;
            copy.hoverEvent = nullptr;
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
            extra = std::move(extra);
            return *this;
        }

        ~Chat() {
            if (text)
                delete text;
            if (color)
                delete color;
            if (insertion)
                delete insertion;
            if (clickEvent)
                delete clickEvent;
            if (hoverEvent)
                delete hoverEvent;
            text = nullptr;
            color = nullptr;
            insertion = nullptr;
            clickEvent = nullptr;
            hoverEvent = nullptr;
        }

        void SetText(const std::string& set_text = "") {
            setString(text, set_text);
            text_is_translation = false;
        }

        void SetTranslation(const std::string& set_text = "") {
            setString(text, set_text);
            text_is_translation = true;
        }

        void SetColor(const std::string& set_text = "") {
            setString(color, set_text);
        }

        void SetInsertion(const std::string& set_text = "") {
            setString(insertion, set_text);
        }

        void SetFont(const std::string& set_text = "") {
            setString(font, set_text);
        }

        void SetBold() {
            defined_bold = false;
        }

        void SetItalic() {
            defined_italic = false;
        }

        void SetUnderlined() {
            defined_underlined = false;
        }

        void SetStrikethrough() {
            defined_strikethrough = false;
        }

        void SetObfuscated() {
            defined_obfuscated = false;
        }

        void SetBold(bool is) {
            bold = is;
            defined_bold = true;
        }

        void SetItalic(bool is) {
            italic = is;
            defined_italic = true;
        }

        void SetUnderlined(bool is) {
            underlined = is;
            defined_underlined = true;
        }

        void SetStrikethrough(bool is) {
            strikethrough = is;
            defined_strikethrough = true;
        }

        void SetObfuscated(bool is) {
            obfuscated = is;
            defined_obfuscated = true;
        }

        void SetHoverEventShowText(const std::string& _show_text) {
            setHoverEvent(_show_text);
        }

        void SetHoverEventShowItem(const std::string& _id, int32_t _count, const std::optional<std::string>& _tag = std::nullopt) {
            hoverEventS::show_itemS* show_item = new hoverEventS::show_itemS;
            show_item->id = _id;
            show_item->count = _count;
            show_item->tag = _tag;
            setHoverEvent(show_item);
        }

        void SetHoverEventShowEntity(const std::string& _id, const std::string& _type, const std::optional<std::string>& _name = std::nullopt) {
            hoverEventS::show_entityS* show_entity = new hoverEventS::show_entityS;
            show_entity->id = _id;
            show_entity->type = _type;
            show_entity->name = _name;
            setHoverEvent(show_entity);
        }

        void SetHoverEvent() {
            if (hoverEvent)
                delete hoverEvent;
            hoverEvent = nullptr;
        }

        void SetClickEventOpenUrl(const std::string& _open_url) {
            if (!clickEvent)
                clickEvent = new clickEventS;

            if (clickEvent->run_command)
                delete clickEvent->run_command;

            if (clickEvent->suggest_command)
                delete clickEvent->suggest_command;

            if (clickEvent->change_page)
                delete clickEvent->change_page;
            setString(clickEvent->open_url, _open_url);
        }

        void SetClickEventRunCommand(const std::string& _run_command) {
            if (!clickEvent)
                clickEvent = new clickEventS;

            if (clickEvent->open_url)
                delete clickEvent->open_url;

            if (clickEvent->suggest_command)
                delete clickEvent->suggest_command;

            if (clickEvent->change_page)
                delete clickEvent->change_page;

            setString(clickEvent->run_command, _run_command);
        }

        void SetClickEventSuggestCommand(const std::string& _suggest_command) {
            if (!clickEvent)
                clickEvent = new clickEventS;

            if (clickEvent->open_url)
                delete clickEvent->open_url;

            if (clickEvent->run_command)
                delete clickEvent->run_command;

            if (clickEvent->change_page)
                delete clickEvent->change_page;

            setString(clickEvent->suggest_command, _suggest_command);
        }

        void SetClickEventChangePage(uint32_t _change_page) {
            if (!clickEvent)
                clickEvent = new clickEventS;
            if (!clickEvent->change_page)
                clickEvent->change_page = new uint32_t;
            *clickEvent->change_page = _change_page;
        }

        void SetClickEventCopyToClipboard(const std::string& _copy_to_clipboard) {
            if (!clickEvent)
                clickEvent = new clickEventS;
            if (clickEvent->copy_to_clipboard)
                delete clickEvent->copy_to_clipboard;
            clickEvent->copy_to_clipboard = nullptr;
            setString(clickEvent->copy_to_clipboard, _copy_to_clipboard);
        }

        void SetClickEvent() {
            if (clickEvent)
                delete clickEvent;
            clickEvent = nullptr;
        }

        list_array<Chat>& GetExtra() {
            return extra;
        }

        std::optional<const char*> GetText() const {
            if (text_is_translation || !text)
                return std::nullopt;
            return text;
        }

        std::optional<const char*> GetTranslation() const {
            if (!text_is_translation || !text)
                return std::nullopt;
            return text;
        }

        std::optional<const char*> GetColor() const {
            if (!color)
                return std::nullopt;
            return color;
        }

        std::optional<const char*> GetInsertion() const {
            if (!insertion)
                return std::nullopt;
            return insertion;
        }

        std::optional<const char*> GetFont() const {
            if (!font)
                return std::nullopt;
            return font;
        }

        std::optional<const hoverEventS*> GetHoverEvent() const {
            if (hoverEvent)
                return std::make_optional(hoverEvent);
            else
                return std::nullopt;
        }

        std::optional<const clickEventS*> GetClickEvent() const {
            if (clickEvent)
                return std::make_optional(clickEvent);
            else
                return std::nullopt;
        }

        std::optional<bool> GetBold() {
            std::optional<bool> result;
            if (defined_bold)
                return std::make_optional<bool>(bold);
            else
                return std::nullopt;
        }

        std::optional<bool> GetItalic() {
            if (defined_italic)
                return std::make_optional<bool>(italic);
            else
                return std::nullopt;
        }

        std::optional<bool> GetUnderlined() {
            if (defined_underlined)
                return std::make_optional<bool>(underlined);
            else
                return std::nullopt;
        }

        std::optional<bool> GetStrikethrough() {
            if (defined_strikethrough)
                return std::make_optional<bool>(strikethrough);
            else
                return std::nullopt;
        }

        std::optional<bool> GetObfuscated() {
            if (defined_obfuscated)
                return std::make_optional<bool>(obfuscated);
            else
                return std::nullopt;
        }

        std::string ToStr() const {
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

        ENBT ToENBT() const {
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
                }
                if (clickEvent->run_command) {
                    clickEvent_enbt["action"] = "run_command";
                    clickEvent_enbt["value"] = clickEvent->run_command;
                }
                if (clickEvent->suggest_command) {
                    clickEvent_enbt["action"] = "suggest_command";
                    clickEvent_enbt["value"] = clickEvent->suggest_command;
                }
                if (clickEvent->change_page) {
                    clickEvent_enbt["action"] = "change_page";
                    clickEvent_enbt["value"] = *clickEvent->change_page;
                }
                if (clickEvent->copy_to_clipboard) {
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
                }
                if (hoverEvent->show_entity) {
                    enbt::compound show_entity_enbt;
                    show_entity_enbt["type"] = hoverEvent->show_entity->type;
                    show_entity_enbt["id"] = hoverEvent->show_entity->id;
                    if (hoverEvent->show_entity->name) {
                        show_entity_enbt["name"] = *hoverEvent->show_entity->name;
                    }
                    hoverEvent_enbt["action"] = "show_entity";
                    hoverEvent_enbt["contents"] = std::move(show_entity_enbt);
                }
                if (hoverEvent->show_text) {
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
            return enbt;
        }

        list_array<uint8_t> ToTextComponent() const {
            return NBT::build(ToENBT()).get_as_normal();
        }

        void removeColor() {
            if (color)
                delete color;
        }

        void removeColorRecursive() {
            if (color)
                delete color;
            for (auto& it : extra)
                it.removeColorRecursive();
        }

        bool empty() const {
            return !text && extra.empty();
        }

        static Chat parseToChat(const std::string& string) {
            list_array<Chat> result;
            constexpr const char format_symbol = '§';
            Chat current_chat;
            bool format_command = false;

            std::string current_string;
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
                } else if (c == format_symbol) {
                    if (current_string.size()) {
                        current_chat.SetText(current_string);
                        result.push_back(current_chat);
                        current_chat.SetText();
                        current_string.clear();
                    }
                    format_command = true;
                } else
                    current_string += c;
            }
            if (current_string.size()) {
                current_chat.SetText(current_string);
                result.push_back(current_chat);
            }
            Chat final_chat;
            final_chat.GetExtra() = result;
            return final_chat;
        }


    private:
        list_array<Chat> extra;
        char* text = nullptr;
        char* color = nullptr;
        char* insertion = nullptr;
        char* font = nullptr;

        clickEventS* clickEvent = nullptr;

        hoverEventS* hoverEvent = nullptr;

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

        static void setString(char*& char_ptr, const std::string& string) {
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
                char_ptr = new char[str_len];
                for (size_t i = 0; i < str_len; i++)
                    char_ptr[i] = string[i];
            } else if (char_ptr) {
                delete[] char_ptr;
                char_ptr = nullptr;
            }
        }

        void setHoverEvent(hoverEventS::show_itemS* setHoverEvent) {
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

        void setHoverEvent(hoverEventS::show_entityS* setHoverEvent) {
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

        void setHoverEvent(const std::string& setHoverEvent) {
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
};
}

#endif /* SRC_BASE_OBJECTS_CHAT */
