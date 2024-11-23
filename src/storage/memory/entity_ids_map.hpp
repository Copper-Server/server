#ifndef SRC_STORAGE_MEMORY_ENTITY_IDS_MAP
#define SRC_STORAGE_MEMORY_ENTITY_IDS_MAP
#include <boost/bimap.hpp>
#include <library/enbt/enbt.hpp>
#include <library/fast_task.hpp>
#include <random>
#include <shared_mutex>

namespace copper_server {
    namespace storage {
        namespace memory {
            class entity_ids_map_storage {
                boost::bimap<int32_t, enbt::raw_uuid> ids;
                fast_task::task_mutex mutex;
                int32_t id_allocator = 0;

            public:
                //generates random UUID allocates, guarantees uniqueness
                [[nodiscard]] std::pair<int32_t, enbt::raw_uuid> allocate_id() {
                    enbt::raw_uuid uuid = enbt::raw_uuid::generate_v4();
                    std::unique_lock lock(mutex);
                    if (ids.size() == INT32_MAX)
                        throw std::runtime_error("Too many registered UUID's, can't allocate more");
                    while (ids.right.find(uuid) != ids.right.end())
                        uuid = enbt::raw_uuid::generate_v4();
                    while (ids.left.find(id_allocator) != ids.left.end())
                        id_allocator++;
                    ids.left.insert({id_allocator, uuid});
                    return {id_allocator, uuid};
                }

                [[nodiscard]] int32_t allocate_id(const enbt::raw_uuid& uuid) {
                    std::unique_lock lock(mutex);
                    if (ids.size() == INT32_MAX)
                        throw std::runtime_error("Too many registered UUID's, can't allocate more");
                    if (ids.right.find(uuid) != ids.right.end())
                        throw std::invalid_argument("UUID already registered");
                    while (ids.left.find(id_allocator) != ids.left.end())
                        id_allocator++;
                    ids.left.insert({id_allocator, uuid});
                    return id_allocator++;
                }

                void remove_id(int32_t id) {
                    std::unique_lock lock(mutex);
                    ids.left.erase(id);
                }

                [[nodiscard]] int32_t remove_id(const enbt::raw_uuid& uuid) {
                    std::unique_lock lock(mutex);
                    auto it = ids.right.find(uuid);
                    if (it == ids.right.end())
                        return -1;
                    int32_t id = it->second;
                    ids.right.erase(it);
                    return id;
                }

                [[nodiscard]] int32_t get_id(const enbt::raw_uuid& uuid) {
                    std::unique_lock lock(mutex);
                    auto it = ids.right.find(uuid);
                    if (it == ids.right.end())
                        return -1;
                    return it->second;
                }

                [[nodiscard]] enbt::raw_uuid get_uuid(int32_t id) {
                    std::unique_lock lock(mutex);
                    auto it = ids.left.find(id);
                    if (it == ids.left.end())
                        return enbt::raw_uuid();
                    return it->second;
                }

                [[nodiscard]] bool has_id(int32_t id) {
                    std::unique_lock lock(mutex);
                    return ids.left.find(id) != ids.left.end();
                }

                [[nodiscard]] bool has_uuid(const enbt::raw_uuid& uuid) {
                    std::unique_lock lock(mutex);
                    return ids.right.find(uuid) != ids.right.end();
                }
            };
        }
    }
}
#endif /* SRC_STORAGE_MEMORY_ENTITY_IDS_MAP */
