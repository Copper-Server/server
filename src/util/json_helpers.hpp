#ifndef SRC_UTIL_JSON_HELPERS
#define SRC_UTIL_JSON_HELPERS
#include <boost/json.hpp>
#include <filesystem>
#include <fstream>

namespace crafted_craft {
    namespace util {


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

            operator std::string() {
                return operator boost::json::string&().c_str();
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
                try {
                    return obj.as_array();
                } catch (const boost::system::system_error& err) {
                    auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::array), path, util::to_string(obj.kind()));
                    throw std::runtime_error(text);
                }
            }

            operator bool() {
                try {
                    return obj.as_bool();
                } catch (const boost::system::system_error& err) {
                    auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::bool_), path, util::to_string(obj.kind()));
                    throw std::runtime_error(text);
                }
            }

            operator int64_t() {
                try {
                    if (obj.is_uint64())
                        if (obj.as_uint64() <= INT64_MAX)
                            return obj.as_uint64();
                        else
                            throw std::runtime_error(std::format("Excepted signed integer at {}.", path));
                    else
                        return obj.as_int64();
                } catch (const boost::system::system_error& err) {
                    auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::int64), path, util::to_string(obj.kind()));
                    throw std::runtime_error(text);
                }
            }

            operator uint64_t() {
                try {
                    if (obj.is_int64())
                        if (obj.as_int64() >= 0)
                            return obj.as_int64();
                        else
                            throw std::runtime_error(std::format("Excepted unsigned integer at {}.", path));
                    else
                        return obj.as_uint64();
                } catch (const boost::system::system_error& err) {
                    auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::uint64), path, util::to_string(obj.kind()));
                    throw std::runtime_error(text);
                }
            }

            operator int32_t() {
                int64_t val = operator int64_t();
                if (int32_t check_overflow = val; val == check_overflow)
                    return check_overflow;
                else {
                    auto text = std::format("Value at {} is too big for int32", path);
                    throw std::overflow_error(text);
                }
            }

            operator int16_t() {
                int64_t val = operator int64_t();
                if (int16_t check_overflow = val; val == check_overflow)
                    return check_overflow;
                else {
                    auto text = std::format("Value at {} is too big for int16", path);
                    throw std::overflow_error(text);
                }
            }

            operator int8_t() {
                int64_t val = operator int64_t();
                if (int8_t check_overflow = val; val == check_overflow)
                    return check_overflow;
                else {
                    auto text = std::format("Value at {} is too big for int8", path);
                    throw std::overflow_error(text);
                }
            }

            operator uint32_t() {
                uint64_t val = operator uint64_t();
                if (uint32_t check_overflow = val; val == check_overflow)
                    return check_overflow;
                else {
                    auto text = std::format("Value at {} is too big for uint32", path);
                    throw std::overflow_error(text);
                }
            }

            operator uint16_t() {
                uint64_t val = operator uint64_t();
                if (uint16_t check_overflow = val; val == check_overflow)
                    return check_overflow;
                else {
                    auto text = std::format("Value at {} is too big for uint16", path);
                    throw std::overflow_error(text);
                }
            }

            operator uint8_t() {
                uint64_t val = operator uint64_t();
                if (uint8_t check_overflow = val; val == check_overflow)
                    return check_overflow;
                else {
                    auto text = std::format("Value at {} is too big for uint8", path);
                    throw std::overflow_error(text);
                }
            }

            operator float() {
                try {
                    return obj.as_double();
                } catch (const boost::system::system_error& err) {
                    auto text = std::format("Excepted {} at {} but got {}", util::to_string(boost::json::kind::double_), path, util::to_string(obj.kind()));
                    throw std::runtime_error(text);
                }
            }

            operator double() {
                try {
                    return obj.as_double();
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

                std::pair<boost::json::string, js_value> operator*() {
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
                return js_object(obj.path, obj.or_apply(boost::json::object()));
            }

            static js_object get_object(js_value&& obj) {
                return js_object(obj.path, obj.or_apply(boost::json::object()));
            }

            static js_object get_object(boost::json::object& obj) {
                return js_object("root", obj);
            }

            js_object(const js_object& copy)
                : obj(copy.obj), path(copy.path) {}

            js_object(js_object&& move)
                : obj(move.obj), path(move.path) {}

            js_value operator[](const std::string& name) {
                if (!obj.contains(name))
                    obj.emplace(name, boost::json::value());
                return js_value(path + ":" + name, obj.at(name));
            }

            bool contains(const std::string& name) {
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
                return js_array(obj.path, obj.or_apply(boost::json::array()));
            }

            static js_array get_array(js_value&& obj) {
                return js_array(obj.path, obj.or_apply(boost::json::array()));
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

            js_iterator begin() {
                return {obj.begin(), obj.begin(), path};
            }

            js_iterator end() {
                return {obj.end(), obj.begin(), path};
            }
        };

        boost::json::object try_read_json_file(const std::filesystem::path& file_path);
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
}


#endif /* SRC_UTIL_JSON_HELPERS */
