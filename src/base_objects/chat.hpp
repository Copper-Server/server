#ifndef SRC_BASE_OBJECTS_CHAT
#define SRC_BASE_OBJECTS_CHAT
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <optional>
#include <string>
#include <vector>

namespace copper_server {
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

        Chat();
        Chat(std::initializer_list<Chat> args);
        Chat(const char* set_text, bool is_translation = false);
        Chat(const std::string& set_text, bool is_translation = false);
        Chat(const Chat& copy);
        Chat(Chat&& copy) noexcept;
        Chat& operator=(const Chat& copy);
        Chat& operator=(Chat&& copy) noexcept;
        ~Chat();

        Chat& SetText(const std::string& set_text = "");
        Chat& SetTranslation(const std::string& set_text = "");
        Chat& SetColor(const std::string& set_text = "");
        Chat& SetInsertion(const std::string& set_text = "");
        Chat& SetFont(const std::string& set_text = "");
        Chat& SetBold();
        Chat& SetItalic();
        Chat& SetUnderlined();
        Chat& SetStrikethrough();
        Chat& SetObfuscated();
        Chat& SetBold(bool is);
        Chat& SetItalic(bool is);
        Chat& SetUnderlined(bool is);
        Chat& SetStrikethrough(bool is);
        Chat& SetObfuscated(bool is);
        Chat& SetHoverEventShowText(const std::string& _show_text);
        Chat& SetHoverEventShowItem(const std::string& _id, int32_t _count, const std::optional<std::string>& _tag = std::nullopt);
        Chat& SetHoverEventShowEntity(const std::string& _id, const std::string& _type, const std::optional<std::string>& _name = std::nullopt);
        Chat& SetHoverEvent();
        Chat& SetClickEventOpenUrl(const std::string& _open_url);
        Chat& SetClickEventRunCommand(const std::string& _run_command);
        Chat& SetClickEventSuggestCommand(const std::string& _suggest_command);
        Chat& SetClickEventChangePage(uint32_t _change_page);
        Chat& SetClickEventCopyToClipboard(const std::string& _copy_to_clipboard);
        Chat& SetClickEvent();

        list_array<Chat>& GetExtra();
        std::optional<const char*> GetText() const;
        std::optional<const char*> GetTranslation() const;
        std::optional<const char*> GetColor() const;
        std::optional<const char*> GetInsertion() const;
        std::optional<const char*> GetFont() const;
        std::optional<const hoverEventS*> GetHoverEvent() const;
        std::optional<const clickEventS*> GetClickEvent() const;
        std::optional<bool> GetBold();
        std::optional<bool> GetItalic();
        std::optional<bool> GetUnderlined();
        std::optional<bool> GetStrikethrough();
        std::optional<bool> GetObfuscated();

        std::string ToStr() const;
        static Chat fromStr(const std::string& str);
        enbt::value ToENBT() const;

        void removeColor();
        void removeColorRecursive();
        bool empty() const;

        static Chat parseToChat(const std::string& string);
        static Chat fromEnbt(const enbt::value& enbt);
        static Chat from_enbt_with_format(const enbt::value& enbt, list_array<enbt::value>&&);
        std::string to_ansi_console() const;


        bool operator==(const Chat&) const;
        bool operator!=(const Chat& other) const;

        std::strong_ordering operator<=>(const Chat& other) const {
            return operator==(other) ? std::strong_ordering::equal : std::strong_ordering::less;
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

        static void setString(char*& char_ptr, const std::string& string);
        void setHoverEvent(hoverEventS::show_itemS* setHoverEvent);
        void setHoverEvent(hoverEventS::show_entityS* setHoverEvent);
        void setHoverEvent(const std::string& setHoverEvent);
    };
}

#endif /* SRC_BASE_OBJECTS_CHAT */
