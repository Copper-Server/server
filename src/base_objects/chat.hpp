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

        Chat(std::initializer_list<Chat> args) {
            bool first = true;
            for (auto& it : args) {
                if (first) {
                    operator=(it);
                    first = false;
                } else
                    extra.push_back(it);
            }
        }

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
            extra = copy.extra;
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
            extra = std::move(copy.extra);
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

        std::string ToStr() const;
        static Chat fromStr(const std::string& str);
            ENBT ToENBT() const;
        list_array<uint8_t> ToTextComponent() const;

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

        static Chat parseToChat(const std::string& string);
        static Chat fromTextComponent(const list_array<uint8_t>& enbt);
        static Chat fromEnbt(const ENBT& enbt);
        std::string to_ansi_console() const;

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

        static void setString(char*& char_ptr, const std::string& string);
        void setHoverEvent(hoverEventS::show_itemS* setHoverEvent);
        void setHoverEvent(hoverEventS::show_entityS* setHoverEvent);
        void setHoverEvent(const std::string& setHoverEvent);
    };
}

#endif /* SRC_BASE_OBJECTS_CHAT */
