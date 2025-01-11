#ifndef SRC_UTIL_JSON_HELPERS
#define SRC_UTIL_JSON_HELPERS
#include <boost/json.hpp>
#include <filesystem>
#include <fstream>

namespace copper_server::util {
    inline std::string to_string(boost::json::kind type_kind) {
        return boost::json::to_string(type_kind);
    }

    class js_object;
    class js_array;

    class js_value {
        friend class js_object;
        friend class js_array;
        friend void pretty_print(std::ostream& os, js_value const& jv);
        boost::json::value& obj;
        std::string path;


    public:
        js_value(const std::string& path, boost::json::value& obj)
            : obj(obj), path(path) {}

        js_value(js_value&& move)
            : obj(move.obj), path(move.path) {}

        js_value& operator=(const boost::json::value& any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(const boost::json::object& any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(const boost::json::array& any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(const boost::json::string_view& any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(const boost::json::string& any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(boost::json::value&& any_val) {
            obj = std::move(any_val);
            return *this;
        }

        js_value& operator=(boost::json::object&& any_val) {
            obj = std::move(any_val);
            return *this;
        }

        js_value& operator=(boost::json::array&& any_val) {
            obj = std::move(any_val);
            return *this;
        }

        js_value& operator=(boost::json::string&& any_val) {
            obj = std::move(any_val);
            return *this;
        }

        js_value& operator=(bool any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(int64_t any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(int32_t any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(int16_t any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(int8_t any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(uint64_t any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(uint32_t any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(uint16_t any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(uint8_t any_val) {
            obj = any_val;
            return *this;
        }

        js_value& operator=(double any_val) {
            obj = any_val;
            return *this;
        }

        bool is_null() const noexcept {
            return obj.is_null();
        }

        bool is_string() const noexcept {
            return obj.is_string();
        }

        bool is_array() const noexcept {
            return obj.is_array();
        }

        bool is_object() const noexcept {
            return obj.is_object();
        }

        bool is_number() const noexcept {
            return obj.is_number();
        }

        bool is_floating() const noexcept {
            return obj.is_double();
        }

        bool is_integral() const noexcept {
            return obj.is_int64() | obj.is_uint64();
        }

        operator std::string() {
            auto sv = operator std::string_view();
            return std::string(sv.begin(), sv.end());
        }

        operator std::string_view() {
            return operator boost::json::string&();
        }

        operator boost::json::string&() {
            try {
                return obj.as_string();
            } catch (const boost::system::system_error& err) {
                auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::string), path, util::to_string(obj.kind()));
                throw std::runtime_error(text);
            }
        }

        operator boost::json::object&() {

            try {
                return obj.as_object();
            } catch (const boost::system::system_error& err) {
                auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::object), path, util::to_string(obj.kind()));
                throw std::runtime_error(text);
            }
        }

        operator boost::json::array&() {
            if (obj.is_array())
                return obj.get_array();
            else {
                auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::array), path, util::to_string(obj.kind()));
                throw std::runtime_error(text);
            }
        }

        operator bool() const {
            try {
                return obj.as_bool();
            } catch (const boost::system::system_error& err) {
                auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::bool_), path, util::to_string(obj.kind()));
                throw std::runtime_error(text);
            }
        }

        operator int64_t() const {
            try {
                return obj.to_number<int64_t>();
            } catch (const boost::system::system_error& err) {
                auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::int64), path, util::to_string(obj.kind()));
                throw std::runtime_error(text);
            }
        }

        operator uint64_t() const {
            try {
                return obj.to_number<uint64_t>();
            } catch (const boost::system::system_error& err) {
                auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::uint64), path, util::to_string(obj.kind()));
                throw std::runtime_error(text);
            }
        }

        operator int32_t() const {
            int64_t val = operator int64_t();
            if (int32_t check_overflow = val; val == check_overflow)
                return check_overflow;
            else {
                auto text = std::format("Value at {} is too big for int32", path);
                throw std::overflow_error(text);
            }
        }

        operator int16_t() const {
            int64_t val = operator int64_t();
            if (int16_t check_overflow = val; val == check_overflow)
                return check_overflow;
            else {
                auto text = std::format("Value at {} is too big for int16", path);
                throw std::overflow_error(text);
            }
        }

        operator int8_t() const {
            int64_t val = operator int64_t();
            if (int8_t check_overflow = val; val == check_overflow)
                return check_overflow;
            else {
                auto text = std::format("Value at {} is too big for int8", path);
                throw std::overflow_error(text);
            }
        }

        operator uint32_t() const {
            uint64_t val = operator uint64_t();
            if (uint32_t check_overflow = val; val == check_overflow)
                return check_overflow;
            else {
                auto text = std::format("Value at {} is too big for uint32", path);
                throw std::overflow_error(text);
            }
        }

        operator uint16_t() const {
            uint64_t val = operator uint64_t();
            if (uint16_t check_overflow = val; val == check_overflow)
                return check_overflow;
            else {
                auto text = std::format("Value at {} is too big for uint16", path);
                throw std::overflow_error(text);
            }
        }

        operator uint8_t() const {
            uint64_t val = operator uint64_t();
            if (uint8_t check_overflow = val; val == check_overflow)
                return check_overflow;
            else {
                auto text = std::format("Value at {} is too big for uint8", path);
                throw std::overflow_error(text);
            }
        }

        operator float() const {
            try {
                return obj.to_number<float>();
            } catch (const boost::system::system_error& err) {
                auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::double_), path, util::to_string(obj.kind()));
                throw std::runtime_error(text);
            }
        }

        operator double() const {
            try {
                return obj.to_number<float>();
            } catch (const boost::system::system_error& err) {
                auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::double_), path, util::to_string(obj.kind()));
                throw std::runtime_error(text);
            }
        }

        js_value& or_apply(const boost::json::value& any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(const boost::json::object& any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(const boost::json::array& any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(const boost::json::string_view& any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(const boost::json::string& any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(const char* any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(boost::json::value&& any_val) {
            if (obj.is_null())
                obj = std::move(any_val);
            return *this;
        }

        js_value& or_apply(boost::json::object&& any_val) {
            if (obj.is_null())
                obj = std::move(any_val);
            return *this;
        }

        js_value& or_apply(boost::json::array&& any_val) {
            if (obj.is_null())
                obj = std::move(any_val);
            return *this;
        }

        js_value& or_apply(boost::json::string&& any_val) {
            if (obj.is_null())
                obj = std::move(any_val);
            return *this;
        }

        js_value& or_apply(bool any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(int64_t any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(int32_t any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(int16_t any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(int8_t any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(uint64_t any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(uint32_t any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(uint16_t any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(uint8_t any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        js_value& or_apply(double any_val) {
            if (obj.is_null())
                obj = any_val;
            return *this;
        }

        boost::json::value& get() {
            return obj;
        }

        std::string to_text() const;

        [[noreturn]] void parsing_error(std::string_view error_message) const {
            auto text = std::format("Parsing error at {} : {}", path, error_message);
            throw std::runtime_error(text);
        }
    };

    class js_object {
        friend void pretty_print(std::ostream& os, const js_object& jv);
        boost::json::object& obj;
        std::string path;

        js_object(const std::string& path, boost::json::object& obj)
            : obj(obj), path(path) {}

    public:
        class js_iterator {
            std::string& inner_path;
            boost::json::object::iterator iterator;

        public:
            js_iterator(boost::json::object::iterator iterator, std::string& inner_path)
                : iterator(iterator), inner_path(inner_path) {}

            js_iterator(const js_iterator& iterator)
                : iterator(iterator.iterator), inner_path(iterator.inner_path) {}

            js_iterator(js_iterator&& iterator)
                : iterator(std::move(iterator.iterator)), inner_path(iterator.inner_path) {}

            std::pair<boost::json::string_view, js_value> operator*() {
                return {iterator->key(), js_value(inner_path + ":" + iterator->key_c_str(), iterator->value())};
            }

            js_iterator& operator++() {
                ++iterator;
                return *this;
            }

            js_iterator& operator--() {
                --iterator;
                return *this;
            }

            js_iterator operator++(int) {
                return js_iterator(iterator++, inner_path);
            }

            js_iterator operator--(int) {
                return js_iterator(iterator--, inner_path);
            }

            auto operator!=(const js_iterator& other) {
                return iterator != other.iterator;
            }

            auto operator==(const js_iterator& other) {
                return iterator != other.iterator;
            }
        };

        static js_object get_object(js_value& obj) {
            return js_object(obj.path, (boost::json::object&)obj.or_apply(boost::json::object()));
        }

        static js_object get_object(js_value&& obj) {
            return js_object(obj.path, (boost::json::object&)obj.or_apply(boost::json::object()));
        }

        static js_object get_object(boost::json::object& obj) {
            return js_object("root", obj);
        }

        static js_object get_object(boost::json::value& obj) {
            if (obj.is_null())
                obj = boost::json::object();
            return js_object("root", obj.as_object());
        }

        js_object(const js_object& copy)
            : obj(copy.obj), path(copy.path) {}

        js_object(js_object&& move)
            : obj(move.obj), path(move.path) {}

        js_value operator[](const boost::json::string_view& name) {
            return js_value(path + ":" + (std::string)name, obj[name]);
        }

        const js_value operator[](const boost::json::string_view& name) const {
            return js_value(path + ":" + (std::string)name, obj[name]);
        }

        js_value at(const boost::json::string_view& name) const {
            try {
                return js_value(path + ":" + (std::string)name, obj.at(name));
            } catch (const boost::system::system_error&) {
                auto text = std::format("Not found element {} at {}.", (std::string)name, path);
                throw std::runtime_error(text);
            }
        }

        bool contains(std::string_view name) const {
            return obj.contains(name);
        }

        js_iterator begin() {
            return {obj.begin(), path};
        }

        js_iterator end() {
            return {obj.end(), path};
        }

        bool empty() const {
            return obj.empty();
        }

        size_t size() const {
            return obj.size();
        }

        //required only to not invalidate references when adding values,
        // can be useful only when need to access multiply fields at once, do not use when not needed
        template <class... Args>
        void defile_values(Args... args) {
            (obj[args], ...);
        }

        boost::json::object& get() {
            return obj;
        }

        [[noreturn]] void parsing_error(std::string_view error_message) const {
            auto text = std::format("Parsing error at {} : {}", path, error_message);
            throw std::runtime_error(text);
        }
    };

    class js_array {
        friend void pretty_print(std::ostream& os, const js_array& jv);
        boost::json::array& obj;
        std::string path;

        js_array(const std::string& path, boost::json::array& obj)
            : obj(obj), path(path) {}

    public:
        class js_iterator {
            std::string& inner_path;
            boost::json::array::iterator beginning;
            boost::json::array::iterator iterator;

        public:
            js_iterator(boost::json::array::iterator iterator, boost::json::array::iterator beginning, std::string& inner_path)
                : iterator(iterator), beginning(beginning), inner_path(inner_path) {}

            js_iterator(const js_iterator& iterator)
                : iterator(iterator.iterator), beginning(iterator.beginning), inner_path(iterator.inner_path) {}

            js_iterator(js_iterator&& iterator)
                : iterator(std::move(iterator.iterator)), beginning(iterator.beginning), inner_path(iterator.inner_path) {}

            js_value operator*() {
                return js_value(inner_path + "[" + std::to_string(iterator - beginning) + "]", *iterator);
            }

            js_iterator& operator++() {
                ++iterator;
                return *this;
            }

            js_iterator& operator--() {
                --iterator;
                return *this;
            }

            js_iterator operator++(int) {
                return js_iterator(iterator++, beginning, inner_path);
            }

            js_iterator operator--(int) {
                return js_iterator(iterator--, beginning, inner_path);
            }

            auto operator!=(const js_iterator& other) {
                return iterator != other.iterator;
            }

            auto operator==(const js_iterator& other) {
                return iterator != other.iterator;
            }
        };

        static js_array get_array(js_value& obj) {
            return js_array(obj.path, (boost::json::array&)obj.or_apply(boost::json::array()));
        }

        static js_array get_array(js_value&& obj) {
            return js_array(obj.path, (boost::json::array&)obj.or_apply(boost::json::array()));
        }

        static js_array get_array(boost::json::array& obj) {
            return js_array("root", obj);
        }

        js_array(const js_array& copy)
            : obj(copy.obj), path(copy.path) {}

        js_array(js_array&& move)
            : obj(move.obj), path(move.path) {}

        js_value operator[](size_t index) {
            return js_value(path + "[" + std::to_string(index) + "]", obj.at(index));
        }

        js_value at(size_t index) {
            try {
                return js_value(path + "[" + std::to_string(index) + "]", obj.at(index));
            } catch (const boost::system::system_error&) {
                auto text = std::format("Not found element {} at {}.", index, path);
                throw std::runtime_error(text);
            }
        }

        size_t size() const {
            return obj.size();
        }

        bool empty() const {
            return obj.empty();
        }

        void resize(size_t new_size) {
            obj.resize(new_size);
        }

        void push_back(boost::json::value&& value) {
            obj.push_back(std::move(value));
        }

        void push_back(const boost::json::value& value) {
            obj.push_back(value);
        }

        void push_back(std::string_view value) {
            obj.push_back((boost::json::string_view)value);
        }

        js_iterator begin() {
            return {obj.begin(), obj.begin(), path};
        }

        js_iterator end() {
            return {obj.end(), obj.begin(), path};
        }

        boost::json::array& get() {
            return obj;
        }

        [[noreturn]] void parsing_error(std::string_view error_message) const {
            auto text = std::format("Parsing error at {} : {}", path, error_message);
            throw std::runtime_error(text);
        }
    };

    std::optional<boost::json::object> try_read_json_file(const std::filesystem::path& file_path);
    std::optional<boost::json::object> try_read_json_file(std::string_view from_memory);
    void pretty_print(std::ostream& os, const boost::json::value& jv, std::string* indent = nullptr);

    inline void pretty_print(std::ostream& os, const js_value& jv) {
        pretty_print(os, jv.obj);
    }

    inline void pretty_print(std::ostream& os, const js_object& jv) {
        pretty_print(os, jv.obj);
    }

    inline void pretty_print(std::ostream& os, const js_array& jv) {
        pretty_print(os, jv.obj);
    }

    inline std::string pretty_print(const boost::json::value& jv) {
        std::ostringstream os;
        pretty_print(os, jv);
        return os.str();
    }

    inline std::string pretty_print(const js_value& jv) {
        std::ostringstream os;
        pretty_print(os, jv);
        return os.str();
    }

    inline std::string pretty_print(const js_object& jv) {
        std::ostringstream os;
        pretty_print(os, jv);
        return os.str();
    }

    inline std::string pretty_print(const js_array& jv) {
        std::ostringstream os;
        pretty_print(os, jv);
        return os.str();
    }
}
#endif /* SRC_UTIL_JSON_HELPERS */
