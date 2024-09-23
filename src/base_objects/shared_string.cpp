#include "shared_string.hpp"

namespace crafted_craft {
    namespace base_objects {
        fast_task::protected_value<std::unordered_set<shared_string>> shared_string::global;

        shared_string shared_string::make_shared(std::string_view str) {
            shared_string res(new std::string(str), nullptr);
            return global.set([&](std::unordered_set<shared_string>& val) {
                auto it = val.find(res);
                if (it == val.end()) {
                    res.count = new std::atomic_size_t(1);
                    return *val.insert(std::move(res)).first;
                } else
                    return *it;
            });
        }

        const shared_string empty;

        void shared_string::leave() {
            if (count) {
                if (*count-- == 1) {
                    global.set([this](std::unordered_set<shared_string>& val) {
                        val.erase(*this);
                    });
                    delete ref;
                    delete count;
                    count = nullptr;
                    ref = nullptr;
                }
            } else if (ref) {
                delete ref;
                ref = nullptr;
            }
        }

        shared_string::shared_string(std::string* ref, std::atomic_size_t* count)
            : ref(ref), count(count) {}

        shared_string::shared_string()
            : shared_string(make_shared("")) {}

        shared_string::shared_string(const char* str, size_t len)
            : ref(nullptr), count(nullptr) {
            *this =
                (len == size_t(-1))
                    ? make_shared(std::string_view(str))
                    : make_shared(std::string_view(str, len));
        }

        shared_string::shared_string(std::string_view str)
            : shared_string(make_shared(str)) {}

        shared_string::shared_string(const std::string& str)
            : shared_string(make_shared(str)) {}

        shared_string::shared_string(std::string&& str)
            : shared_string(make_shared(str)) {}

        shared_string::shared_string(const shared_string& str)
            : ref(str.ref), count(str.count) {
            ++*count;
        }

        shared_string::shared_string(shared_string&& str) noexcept
            : ref(str.ref), count(str.count) {
            if (this == &str)
                return;
            str.count = nullptr;
            str.ref = nullptr;
        }

        shared_string::~shared_string() {
            leave();
        }

        shared_string& shared_string::operator=(const shared_string& str) {
            if (this != &str) {
                leave();
                ref = str.ref;
                count = str.count;
                ++*count;
            }
            return *this;
        }

        shared_string& shared_string::operator=(shared_string&& str) {
            if (this != &str) {
                ref = str.ref;
                count = str.count;
                str.ref = nullptr;
                str.count = nullptr;
            }
            return *this;
        }

        std::string shared_string::operator+(const shared_string& str) const {
            return *ref + *str.ref;
        }

        std::string shared_string::operator+(std::string_view str) const {
            return *ref + (std::string)str;
        }

        const std::string& shared_string::get() const noexcept {
            return *ref;
        }
    }
}