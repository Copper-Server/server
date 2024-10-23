
#include "json_helpers.hpp"
#include "../log.hpp"
#include <format>

namespace crafted_craft {
    namespace util {
        std::optional<boost::json::object> try_read_json_file(const std::filesystem::path& file_path) {
            boost::json::object data;
            {
                std::ifstream file(file_path);
                if (file.is_open()) {
                    boost::system::error_code ec;
                    auto result = boost::json::parse(file, ec);
                    data = result.is_object() ? std::move(result).as_object() : boost::json::object{{"root", std::move(result)}};
                    file.close();
                    if (ec) {
                        if (ec.message() == "incomplete JSON")
                            if (std::filesystem::is_empty(file_path))
                                return data;
                        std::string err_string =
                            ec.has_location()
                                ? std::format("Failed to read {} file because:\n{}\n On:\n{}", file_path.string(), ec.message(), ec.location().to_string())
                                : std::format("Failed to read {} file because:\n{}", file_path.string(), ec.message());
                        log::warn("server", err_string);
                        return std::nullopt;
                    }
                }
            }
            return data;
        }

        std::optional<boost::json::object> try_read_json_file(std::string_view from_memory) {
            boost::json::object data;
            {
                boost::system::error_code ec;
                auto result = boost::json::parse(from_memory, ec);
                data = result.is_object() ? std::move(result).as_object() : boost::json::object{{"root", std::move(result)}};
                if (ec) {
                    std::string err_string = ec.has_location()
                                                 ? std::format("Failed to read json because:\n{}\n On:\n{}", ec.message(), ec.location().to_string())
                                                 : std::format("Failed to read json because:\n{}", ec.message());
                    log::warn("server", err_string);
                    return std::nullopt;
                }
            }
            return data;
        }

        void pretty_print(std::ostream& os, const boost::json::value& jv, std::string* indent) {
            std::string indent_;
            if (!indent)
                indent = &indent_;

            switch (jv.kind()) {
            case boost::json::kind::object: {
                os << "{\n";
                indent->append(4, ' ');

                std::vector<boost::json::string> keys;

                for (auto const& v : jv.get_object()) {
                    keys.push_back(v.key());
                }

                std::sort(keys.begin(), keys.end());

                std::size_t i = 0, n = keys.size();

                for (auto const& k : keys) {
                    os << *indent << boost::json::serialize(k) << ": ";
                    pretty_print(os, jv.at(k), indent);
                    os << (++i == n ? "\n" : ",\n");
                }

                indent->resize(indent->size() - 4);
                os << *indent << "}";
                break;
            }
            case boost::json::kind::array: {
                os << "[\n";
                indent->append(4, ' ');

                auto const& a = jv.get_array();

                std::size_t i = 0, n = a.size();

                for (auto const& v : a) {
                    os << *indent;
                    pretty_print(os, v, indent);
                    os << (++i == n ? "\n" : ",\n");
                }

                indent->resize(indent->size() - 4);
                os << *indent << "]";
                break;
            }

            default: {
                os << boost::json::serialize(jv);
                break;
            }
            }

            if (indent->empty())
                os << "\n";
        }

        std::string js_value::to_text() const {
            return boost::json::serialize(obj);
        }
    }
}