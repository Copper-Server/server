#include <src/base_objects/container.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server::base_objects {
    void container::to_nbt(util::nbt_write_stream& stream) const {
        auto list = stream.write_list();

        for(auto& [id, slot] : *this){
            list.write([&](util::nbt_write_stream& item) {
                auto compound = item.write_compound();
                compound.write("Slot", id);
                slot.to_nbt_base(compound);
            });
        }
    }

    container container::from_nbt(util::nbt_read_stream& stream) {
        container res;
        auto list = stream.read_list();
        stream.iterate([&](util::nbt_read_stream& item) {
            util::nbt_collection::compound_flex flex_collect;
            int32_t slot_num;
            slot_data slot;
            flex_collect.collect_into_required("Slot", slot_num);
            slot.from_nbt_base(flex_collect);
            res[slot_num] = std::move(slot);
        });
        return res;
    }

    container container::from_nbt(util::nbt_read_stream& stream, int32_t max_size) {
        container res;
        auto list = stream.read_list();
        stream.iterate([&](util::nbt_read_stream& item) {
            util::nbt_collection::compound_flex flex_collect;
            int32_t slot_num;
            slot_data slot;
            flex_collect.collect_into_required("Slot", slot_num);
            slot.from_nbt_base(flex_collect);
            if (slot_num >= max_size) 
                throw std::out_of_range("The slot is out of container max size");
            res[slot_num] = std::move(slot);
        });
        return res;
    }
}