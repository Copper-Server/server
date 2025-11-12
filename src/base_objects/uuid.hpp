#ifndef SRC_BASE_OBJECTS_UUID
#define SRC_BASE_OBJECTS_UUID
#include <cstdint>
#include <exception>
#include <string>

namespace enbt{
    struct raw_uuid;
}

namespace copper_server::base_objects {
    struct uuid { //in nbt stored as 4 ints
        enum class family_t {
            unknown = -1,
            ncs = 0,
            rfc_4122 = 1,
            microsoft = 2,
            future = 3,
        };

        constexpr bool is_null() const noexcept {
            return data[0] | data[1] | data[2] | data[3] | data[4] | data[5] | data[6] | data[7]
                   | data[8] | data[9] | data[10] | data[11] | data[12] | data[13] | data[14] | data[15];
        }

        constexpr family_t family() const noexcept {
            auto octet7 = data[8];
            if ((octet7 & 0x80) == 0x00)
                return family_t::ncs;
            else if ((octet7 & 0xC0) == 0x80)
                return family_t::rfc_4122;
            else if ((octet7 & 0xE0) == 0xC0)
                return family_t::microsoft;
            else if ((octet7 & 0xE0) == 0xE0)
                return family_t::future;
            else
                return family_t::unknown;
        }

        enum class version_t {
            unknown = -1,
            time_based = 1,
            dce_security = 2,
            name_based_md5 = 3,
            random_number_based = 4,
            name_based_sha1 = 5,
            sortable_time_based = 6,
            timestamp_and_random = 7,
            custom = 8,
            v1 = time_based,
            v2 = dce_security,
            v3 = name_based_md5,
            v4 = random_number_based,
            v5 = name_based_sha1,
            v6 = sortable_time_based,
            v7 = timestamp_and_random,
            v8 = custom,
        };

        constexpr version_t version() const noexcept {
            auto octet9 = data[6];
            if ((octet9 & 0xF0) == 0x10)
                return version_t::time_based;
            else if ((octet9 & 0xF0) == 0x20)
                return version_t::dce_security;
            else if ((octet9 & 0xF0) == 0x30)
                return version_t::name_based_md5;
            else if ((octet9 & 0xF0) == 0x40)
                return version_t::random_number_based;
            else if ((octet9 & 0xF0) == 0x50)
                return version_t::name_based_sha1;
            else if ((octet9 & 0xF0) == 0x60)
                return version_t::sortable_time_based;
            else if ((octet9 & 0xF0) == 0x70)
                return version_t::timestamp_and_random;
            else if ((octet9 & 0xF0) == 0x80)
                return version_t::custom;
            else
                return version_t::unknown;
        }

        constexpr void swap(uuid& rhs) noexcept {
            std::swap(data, rhs.data);
        }

        constexpr auto operator<=>(const uuid&) const = default;

        // Format: 8-4-4-4-12 (standard UUID format)
        constexpr std::string to_string() const {
            std::string res;
            res.reserve(37);
            for (std::size_t i = 0; i < 16; i++) {
                res += "0123456789abcdef"[data[i] >> 4];
                res += "0123456789abcdef"[data[i] & 0xF];
                if (i == 3 || i == 5 || i == 7 || i == 9)
                    res += '-';
            }
            return res;
        }

        // Format: only hex
        constexpr std::string to_string_flat() const {
            std::string res;
            res.reserve(32);
            for (std::size_t i = 0; i < 16; i++) {
                res += "0123456789abcdef"[data[i] >> 4];
                res += "0123456789abcdef"[data[i] & 0xF];
            }
            return res;
        }

        static uuid from_string(std::string_view view);//uses from_string_sha1

        static uuid from_string_sha1(std::string_view view);

        static uuid from_string_md5(std::string_view view);

        static std::string_view from_uuid_string(uuid& res, std::string_view view, bool skip_hyphens = true) {
            //parse standard UUID format that for from to_string function, skip -
            size_t j = 0, i = 0;
            for (; i < 36; i++) {
                if (view[i] == '-' && skip_hyphens)
                    continue;
                if (j == 16)
                    return view.substr(i);
                std::string byteString = std::string(view.substr(i, 2));
                res.data[j++] = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
            }
            if (j != 16)
                throw std::invalid_argument("Invalid UUID string format");
            else
                return view.substr(i);
        }

        static uuid generate_v4();

        static uuid generate_v7();

        static uuid generate_npc(); //uses generate_v7 and assigns version 2, should be used only for npc players to be correctly processed by clients in online mode

        static uuid generate_offline();//uses generate_v7 and assigns version 3, should be used for players or entities(inconsistent)

        static uuid create_offline(std::string_view name); //adds OfflinePlayer: to the string and uses from_string_md5

        //in online mode the server should always use clients uuid

        static uuid as_null();

        static uuid to_uuid(const enbt::raw_uuid&) noexcept;

        operator enbt::raw_uuid() const noexcept;

        uint8_t data[16];
    };

    struct uuid_hex : public uuid { //in nbt stored as string using to_string
        using uuid::uuid;
        using uuid::operator=;
    };

    struct uuid_flat_hex : public uuid { //in nbt stored as string using to_string_flat
        using uuid::uuid;
        using uuid::operator=;
    };
}

namespace std {
    template <>
    struct hash<copper_server::base_objects::uuid> {
        std::size_t operator()(const copper_server::base_objects::uuid& uuid) const noexcept {
            std::uint64_t parts[2];
            std::memcpy(&parts[0], uuid.data, 8);
            std::memcpy(&parts[1], uuid.data + 8, 8);
            return std::hash<std::uint64_t>()(parts[0]) ^ std::hash<std::uint64_t>()(parts[1]);
        }
    };
}


#endif /* SRC_BASE_OBJECTS_UUID */
