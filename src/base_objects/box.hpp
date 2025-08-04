#ifndef SRC_BASE_OBJECTS_BOX
#define SRC_BASE_OBJECTS_BOX
#include <memory>

namespace copper_server::base_objects {

    template <class T>
    struct box {
        using element_type = T;
        std::unique_ptr<T> ptr;

        constexpr box(box&& b)
            : ptr(std::move(b.ptr)) {}

        constexpr box(std::unique_ptr<T>&& b)
            : ptr(std::move(b)) {}

        ~box() = default;

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
    };
}
#endif /* SRC_BASE_OBJECTS_BOX */
