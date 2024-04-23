#pragma once
#include <unordered_map>
#include "mc_version.hpp"

namespace crafted_craft {
    namespace data {
		struct ClientAdaptived {
			char* struct_data;
			short struct_len;
			McVersion support_mc_from;
			McVersion support_mc_to;
		};
		struct ClientAdaptivedContainer {
			ClientAdaptivedContainer(std::initializer_list<ClientAdaptived> init) {
				all_items = new ClientAdaptived[init.size()];
				variants_count = init.size();
				size_t i = 0;
				for (const auto& it : init)
					all_items[i++] = it;
			}

			const ClientAdaptived* begin() const {
				return all_items;
			}
			const ClientAdaptived* end() const {
				return all_items + variants_count;
			}
			size_t size() const {
				return variants_count;
			}
			const ClientAdaptived& operator[](size_t pos) const {
				return all_items[pos];
			}
		private:
			ClientAdaptived* all_items;
			size_t variants_count;
		};

	}
}