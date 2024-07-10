#ifndef SRC_BASE_OBJECTS_SHARED_STRING
#define SRC_BASE_OBJECTS_SHARED_STRING
#include "../library/fast_task.hpp"
#include <atomic>
#include <string>
#include <unordered_set>

namespace crafted_craft {
    namespace base_objects {
        class shared_string {
            static fast_task::protected_value<std::unordered_set<shared_string>> global;
            static shared_string make_shared(const std::string_view& str);
            void leave();

            std::string* ref;
            std::atomic_size_t* count;

            shared_string(std::string*, std::atomic_size_t*);

        public:
            shared_string();
            shared_string(const char* str, size_t len = -1);
            shared_string(const std::string_view& str);
            shared_string(const std::string& str);
            shared_string(const shared_string& str);
            shared_string(shared_string&& str) noexcept;

            ~shared_string();

            shared_string& operator=(const std::string_view& str) {
                return *this = make_shared(str);
            }

            shared_string& operator=(const std::string& str) {
                return *this = make_shared(str);
            }

            shared_string& operator=(const shared_string& str);
            shared_string& operator=(shared_string&& str);

            std::string operator+(const shared_string& str) const;
            std::string operator+(const std::string& str) const;

            bool operator==(const shared_string& str) const noexcept {
                if (str.ref == ref)
                    return true;
                return *str.ref == *ref;
            }

            bool operator==(const std::string& str) const noexcept {
                return str == *ref;
            }

            bool operator!=(const shared_string& str) const noexcept {
                if (str.ref == ref)
                    return false;
                return *str.ref != *ref;
            }

            bool operator!=(const std::string& str) const noexcept {
                return str != *ref;
            }

            std::strong_ordering operator<=>(const shared_string& str) const noexcept {
                if (str.ref == ref)
                    return std::strong_ordering::equal;
                return *str.ref <=> *ref;
            }

            std::strong_ordering operator<=>(const std::string& str) const noexcept {
                return str <=> *ref;
            }

            const std::string& get() const noexcept;

            bool empty() const noexcept {
                return get().empty();
            }
        };
    }
}

namespace std {
    template <>
    struct hash<crafted_craft::base_objects::shared_string> {
        size_t operator()(const crafted_craft::base_objects::shared_string& value) const noexcept {
            return std::hash<std::string>()(value.get());
        }
    };
} // namespace std


#endif /* SRC_BASE_OBJECTS_SHARED_STRING */
