#ifndef SRC_BASE_OBJECTS_PTR_OPTIONAL
#define SRC_BASE_OBJECTS_PTR_OPTIONAL
#include <stdexcept>

namespace copper_server::base_objects {

    template <typename T>
    class ptr_optional {
        T* data;

    public:
        ptr_optional()
            : data() {}

        ptr_optional(T* data)
            : data(data) {}

        ptr_optional(const ptr_optional& other) {
            if (other.data)
                data = new T(*other.data);
        }

        ptr_optional(ptr_optional&& other) {
            data = other.data;
            other.data = nullptr;
        }

        ptr_optional& operator=(T* data) {
            if (this->data == data)
                return *this;
            if (this->data)
                delete this->data;
            this->data = data;
            return *this;
        }

        ptr_optional& operator=(const ptr_optional& other) {
            if (data == other.data)
                return *this;
            if (data)
                delete data;
            if (other.data)
                data = new T(*other.data);
            return *this;
        }

        ptr_optional& operator=(ptr_optional&& other) {
            if (data == other.data)
                return *this;
            if (data)
                delete data;
            data = other.data;
            other.data = nullptr;
            return *this;
        }

        ~ptr_optional() {
            if (data)
                delete data;
        }

        T* operator->() {
            if (!data)
                throw std::runtime_error("Data is nullptr");
            return data;
        }

        T& operator*() {
            if (!data)
                throw std::runtime_error("Data is nullptr");
            return *data;
        }

        T* operator->() const {
            if (!data)
                throw std::runtime_error("Data is nullptr");
            return data;
        }

        T& operator*() const {
            if (!data)
                throw std::runtime_error("Data is nullptr");
            return *data;
        }

        bool operator==(const ptr_optional& other) const {
            return data == other.data;
        }

        bool operator!=(const ptr_optional& other) const {
            return data != other.data;
        }

        operator bool() const {
            return data != nullptr;
        }

        bool operator!() const {
            return data == nullptr;
        }
    };
}

#endif /* SRC_BASE_OBJECTS_PTR_OPTIONAL */
