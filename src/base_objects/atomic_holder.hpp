/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_ATOMIC_HOLDER
#define SRC_BASE_OBJECTS_ATOMIC_HOLDER
#include <atomic>
#include <stdexcept>

namespace copper_server::base_objects {

    template <typename T>
    class atomic_holder {
        T* data;
        std::atomic_size_t* ref_count;

        atomic_holder(T* data, std::atomic_size_t* ref_count)
            : data(data), ref_count(ref_count) {}

        void decrease_counter() {
            if (ref_count) {
                if (--(*ref_count) == 0) {
                    delete ref_count;
                    delete data;
                }
            }
        }

    public:
        atomic_holder()
            : data(), ref_count() {}

        atomic_holder(nullptr_t)
            : data(nullptr), ref_count(nullptr) {}

        atomic_holder(T* data)
            : data(data), ref_count(new std::atomic_size_t(1)) {}

        atomic_holder(const atomic_holder& other)
            : data(other.data), ref_count(other.ref_count) {
            if (ref_count) {
                (*ref_count)++;
            }
        }

        atomic_holder(atomic_holder&& other)
            : data(other.data), ref_count(other.ref_count) {
            other.data = nullptr;
            other.ref_count = nullptr;
        }

        atomic_holder& operator=(T* value) {
            if (data == value)
                return *this;
            decrease_counter();
            data = value;
            ref_count = data ? new std::atomic_size_t(1) : nullptr;
            return *this;
        }

        atomic_holder& operator=(const atomic_holder& other) {
            if (data == other.data)
                return *this;
            decrease_counter();
            data = other.data;
            ref_count = other.ref_count;
            if (ref_count)
                (*ref_count)++;
            return *this;
        }

        atomic_holder& operator=(atomic_holder&& other) {
            if (data == other.data)
                return *this;
            decrease_counter();
            data = other.data;
            ref_count = other.ref_count;
            other.data = nullptr;
            other.ref_count = nullptr;
            return *this;
        }

        ~atomic_holder() {
            decrease_counter();
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

        bool operator==(const atomic_holder& other) const {
            return data == other.data;
        }

        bool operator!=(const atomic_holder& other) const {
            return data != other.data;
        }

        operator bool() const {
            return data != nullptr;
        }

        bool operator!() const {
            return data == nullptr;
        }

        bool is_last() {
            return ref_count ? *ref_count == 1 : false;
        }

        size_t use_count() const {
            return ref_count ? ref_count->load() : size_t(0);
        }

        void reset() {
            decrease_counter();
            data = nullptr;
            ref_count = nullptr;
        }
    };
}
#endif /* SRC_BASE_OBJECTS_ATOMIC_HOLDER */
