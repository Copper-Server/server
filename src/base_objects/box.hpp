#ifndef SRC_BASE_OBJECTS_BOX
#define SRC_BASE_OBJECTS_BOX
#include <memory>

namespace copper_server::base_objects {

    template <class T>
    struct box {
        using value_type = T;
        std::shared_ptr<T> ptr;

        constexpr box() {}

        constexpr box(box&& b)
            : ptr(std::move(b.ptr)) {}

        constexpr box(std::shared_ptr<T>&& b)
            : ptr(std::move(b)) {}

        constexpr box(const box& b) : ptr(b.ptr) {}

        constexpr box(const std::shared_ptr<T>& b) : ptr(b) {}


        ~box() = default;

        constexpr box& operator=(const box& box);

        constexpr box& operator=(box&& box) {
            ptr = std::move(box.ptr);
            return *this;
        }

        constexpr T* operator->() {
            return &*ptr;
        }

        constexpr const T* operator->() const {
            return &*ptr;
        }

        constexpr T& operator*() {
            return *ptr;
        }

        constexpr const T& operator*() const {
            return *ptr;
        }

        auto operator<=>(const box& other) const = default;
    };

    template <class T>
    constexpr bool operator==(const box<T>& l, const box<T>& r) noexcept {
        return l.ptr == r.ptr;
    }

    template <class T>
    constexpr bool operator!=(const box<T>& l, const box<T>& r) noexcept {
        return l.ptr != r.ptr;
    }

    template <class T>
    constexpr box<T>& box<T>::operator=(const box<T>& box) {
        ptr = std::make_shared<T>(*box);
        return *this;
    }
}
#endif /* SRC_BASE_OBJECTS_BOX */
