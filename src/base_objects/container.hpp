#ifndef SRC_BASE_OBJECTS_CONTAINER
#define SRC_BASE_OBJECTS_CONTAINER
#include <unordered_map>
#include <src/base_objects/slot.hpp>

namespace copper_server::util {
    class nbt_read_stream;
    class nbt_write_stream;
}

namespace copper_server::base_objects {
    struct container : public std::unordered_map<int32_t, slot_data> {
        using std::unordered_map<int32_t, slot_data>::unordered_map;
        using std::unordered_map<int32_t, slot_data>::operator=;


        void to_nbt(util::nbt_write_stream& stream) const;
        static container from_nbt(util::nbt_read_stream& stream);
        static container from_nbt(util::nbt_read_stream& stream, int32_t max_size);
    };

    template <int32_t sized_s>
    struct container_sized : public container {
        container_sized() : container() {
            reserve(sized_s);
        }

        container_sized(container&& dat) : container(std::move(dat)) {}

        container_sized(const container& dat) : container(dat) {}

        using container::container;
        using container::operator=;

        void to_nbt(util::nbt_write_stream& stream) const{
            container::to_nbt(stream);
        }

        static container_sized from_nbt(util::nbt_read_stream& stream) {
            return container::from_nbt(stream, sized_s);
        }

        constexpr size_t max_size() const noexcept {
            return sized_s;
        }
    };
}

#endif /* SRC_BASE_OBJECTS_CONTAINER */
