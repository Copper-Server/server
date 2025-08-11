/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_PTR_OPTIONAL
#define SRC_BASE_OBJECTS_PTR_OPTIONAL
#include <stdexcept>

namespace copper_server::base_objects {

    template <typename T>
    class ptr_optional {
        T* data;

    public:
        ptr_optional()
            : data(nullptr) {}

        ptr_optional(const T& data)
            : data(new T(data)) {}

        ptr_optional(T&& data)
            : data(new T(std::move(data))) {}

        ptr_optional(T* data)
            : data(data) {}

        ptr_optional(const ptr_optional& other) {
            if (other.data)
                data = new T(*other.data);
            else
                data = nullptr;
        }

        ptr_optional(ptr_optional&& other) {
            data = other.data;
            other.data = nullptr;
        }

        ptr_optional& operator=(T* value) {
            if (data == value)
                return *this;
            if (data)
                delete data;
            data = value;
            return *this;
        }

        ptr_optional& operator=(const ptr_optional& other) {
            if (data == other.data)
                return *this;
            if (data)
                delete data;
            if (other.data)
                data = new T(*other.data);
            else
                data = nullptr;
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

        void reset() {
            if (data)
                delete data;
            data = nullptr;
        }
    };
}

#endif /* SRC_BASE_OBJECTS_PTR_OPTIONAL */
