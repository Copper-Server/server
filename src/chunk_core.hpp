#pragma once
#include "base_objects/block.hpp"
#include "range_walk.hpp"
#include "util/task_management.hpp"
#include <filesystem>
#include <list>

namespace crafted_craft {
    typedef Block ChunkFragment[16][16][16];
	typedef int32_t block_pos_t;
	typedef uint32_t ublock_pos_t;

	class ChunkGenerator {
	public:
		virtual void operator()(ChunkFragment* chunk,size_t max_y, uint64_t pos_x, uint64_t pos_z) = 0;
	};

	extern ChunkGenerator* global_generator;

	struct ChunkCore {
		ChunkFragment* chunk_fragments = nullptr;

		static void ChangeMaxY(uint16_t max_y) {
			if (mod_cys_allow) 
				chunk_y_count = max_y / 16 + (bool)(max_y % 16);
			else
				throw std::exception("Modifying value is disallowed while one or more chunk created");
		}
		ChunkCore() = default;
		ChunkCore(block_pos_t set_pos_x, block_pos_t set_pos_z):pos_x(set_pos_x), pos_z(set_pos_z){
			mod_cys_allow = false;
			chunk_fragments = new ChunkFragment[chunk_y_count];
			global_generator->operator()(chunk_fragments, chunk_y_count, pos_x, pos_z);
		}
		~ChunkCore() {
			if(chunk_fragments)
				delete[] chunk_fragments;
		}
		Block& operator()(block_pos_t x, block_pos_t y, block_pos_t z) {
			x += 8;
			y += 8;
			z += 8;
			if ((block_pos_t)chunk_y_count * 8  <= y)
				throw std::out_of_range("y block pos out of max size range");
			return chunk_fragments[y/8][x][y%8][z];
		}
		void Save(std::filesystem::path corePath);
		void Load(std::filesystem::path corePath);
	private:
		block_pos_t pos_x : (sizeof(block_pos_t)*8-3) = 0;
		block_pos_t pos_z : (sizeof(block_pos_t)*8-3) = 0;



		static uint8_t chunk_y_count;
		static std::atomic_bool mod_cys_allow;//cys is chunk_y_size
	};

	class ChunkCluster {
	public:
		ChunkCore chunks[3][3];
		std::mutex thread_guard;
		const block_pos_t pos_x : (sizeof(block_pos_t)*8-6);
		const block_pos_t pos_z : (sizeof(block_pos_t)*8-6);
		
		ChunkCluster(block_pos_t set_pos_x, block_pos_t set_pos_z) : pos_x(set_pos_x), pos_z(set_pos_z) {
			for (block_pos_t x = 0; x < 3; x++)
				for (block_pos_t z = 0; z < 3; z++)
					chunks[x][z] = { (pos_x + x) * 3,(pos_z + z) * 3 };
		}

		void Save(std::filesystem::path clusterPath);
		void Load(std::filesystem::path clusterPath);

		Block& operator()(block_pos_t x, block_pos_t y, block_pos_t z) {
			return (chunks[x / 24][z / 24])(x % 24, y, z % 24);
		}
		ChunkCore& getChunk(block_pos_t x, block_pos_t z) {
			return chunks[x / 24][z / 24];
		}
	};

	class WorldClusterTalker {
	protected:
		friend class WorldClusters;
		class WorldClusters* world_handle;
		std::mutex race_lock;
		void ClusterDeath();
	public:
		block_pos_t cluster_x, cluster_z;
		block_pos_t cluster_handle_distance;

		WorldClusterTalker(class WorldClusters& world, block_pos_t cluster_x, block_pos_t cluster_z, block_pos_t cluster_handle_distance);

		virtual void ClusterUnloaded(block_pos_t cluster_x, block_pos_t cluster_z) = 0;
		virtual void BlockChanged(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) = 0;
		virtual void BlockSeted(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) = 0;
		virtual void BlockRemoved(block_pos_t x, block_pos_t y, block_pos_t z) = 0;
		virtual void BlockBreaked(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) = 0;
		virtual void BlocksChanged(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) = 0;

		const ChunkCluster& ClusterRequest(block_pos_t x, block_pos_t z);

		bool do_handele_nbt = false;
		block_pos_t nbt_handle_x = 0, nbt_handle_y = 0, nbt_handle_z = 0;
		virtual void BlockHandleNbtChanged(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) = 0;

		const Block& BlockRequest(block_pos_t x, block_pos_t y, block_pos_t z);

		struct BlockEventData {
			Block* block; block_pos_t x, y, z;
		};
		void NotifyBlockChanged(BlockEventData);
		void NotifyBlockSeted(BlockEventData);
		void NotifyBlockRemoved(BlockEventData);
		void NotifyBlockBreaked(BlockEventData);

		virtual ~WorldClusterTalker();
	};
	struct BlockEvent {
		//source used for prevent send ourself
		class WorldClusterTalker* source;
		Block* block; block_pos_t x, y, z;
		enum class Reason {
			Changed,
			Seted,
			Removed,
			Breaked,
			nbt_changed
		};
		Reason reason;
    };

    class WorldClusters {
        friend class WorldClusterTalker;
        std::unordered_map<block_pos_t, std::unordered_map<block_pos_t, ChunkCluster*>> clusters;

        struct UPDATE_BLOCK {
            block_pos_t x, y, z;
            Block::TickReason reason;

            bool operator==(const UPDATE_BLOCK& bl) const {
                return bl.x == x && bl.y == y && bl.z == z && bl.reason == reason;
            }

            bool operator!=(const UPDATE_BLOCK& bl) const {
                return !operator==(bl);
            }
        };

        std::list<UPDATE_BLOCK> to_be_updated;
        std::mutex to_be_updated_mut;

        Block& LocalBlock(block_pos_t x, block_pos_t y, block_pos_t z) {
            return getCluster(x / 128, z / 128)(x % 128, y, z % 128);
        }

        inline void breakBlock(UPDATE_BLOCK block_pos) {
            EventNotify({nullptr, nullptr, block_pos.x, block_pos.y, block_pos.z, BlockEvent::Reason::Breaked});
        }

        inline void removeBlock(UPDATE_BLOCK block_pos) {
            EventNotify({nullptr, nullptr, block_pos.x, block_pos.y, block_pos.z, BlockEvent::Reason::Removed});
        }

        inline void setBlock(UPDATE_BLOCK block_pos, Block seted_block) {
            EventNotify({nullptr, new Block(seted_block), block_pos.x, block_pos.y, block_pos.z, BlockEvent::Reason::Seted});
        }

        void setBlocks(std::list<UPDATE_BLOCK> blocks, Block set_block) {
        }

        inline void stateChanged(UPDATE_BLOCK block_pos, Block bock_state) {
            EventNotify({nullptr, new Block(bock_state), block_pos.x, block_pos.y, block_pos.z, BlockEvent::Reason::Changed});
        }

        bool CalculateCanMoveBlocks(std::list<UPDATE_BLOCK> blocks, uint8_t mx, uint8_t my, uint8_t mz) {
            if (!(mx | my | mz))
                return false;
            for (auto& it : blocks) {
                if (it.y + my < 0)
                    return false;
                if (!existsCluster(it.x + mx, it.z + mz))
                    return false;
                auto&& tmp = getCluster(it.x + mx, it.z + mz);
                tmp.thread_guard.lock();
                if (!tmp(it.x + mx, it.y + my, it.z + mz).canMove()) {
                    tmp.thread_guard.unlock();
                    return false;
                }
                tmp.thread_guard.unlock();
            }
            return true;
        }

        void MoveBlocks(std::list<UPDATE_BLOCK> blocks, uint8_t mx, uint8_t my, uint8_t mz) {
            if (!(mx | my | mz))
                return;
            for (auto& it : blocks) {
                if (it.y + my < 0) {
                    removeBlock(it);
                    continue;
                }
                auto& get_cluster = getCluster(it.x, it.z);
                auto& set_cluster = getCluster(it.x + mx, it.z + mz);
                if (&get_cluster == &set_cluster) {
                    get_cluster.thread_guard.lock();
                    get_cluster(it.x + mx, it.y + my, it.z + mz) = get_cluster(it.x, it.y, it.z);
                    setBlock({it.x + mx, it.y + my, it.z + mz}, get_cluster(it.x + mx, it.y + my, it.z + mz));
                    removeBlock(it);
                    get_cluster.thread_guard.unlock();
                } else {
                    get_cluster.thread_guard.lock();
                    set_cluster.thread_guard.lock();
                    set_cluster(it.x + mx, it.y + my, it.z + mz) = get_cluster(it.x, it.y, it.z);
                    setBlock({it.x + mx, it.y + my, it.z + mz}, set_cluster(it.x + mx, it.y + my, it.z + mz));
                    removeBlock(it);
                    get_cluster.thread_guard.unlock();
                    set_cluster.thread_guard.unlock();
                }
            }
        }

        static std::list<UPDATE_BLOCK> _UpdateBlockSelect(Block::TickAnswer to_be_calc, UPDATE_BLOCK tick_pos) {
            switch (to_be_calc.mode) {
            case Block::TickAnswer::PosMode::as_pos:
                return {
                    UPDATE_BLOCK(
                        to_be_calc.X + tick_pos.x,
                        to_be_calc.Y + tick_pos.y,
                        to_be_calc.Z + tick_pos.z
                    ),
                    UPDATE_BLOCK(
                        to_be_calc.x + tick_pos.x,
                        to_be_calc.y + tick_pos.y,
                        to_be_calc.z + tick_pos.z
                    ),
                };
            case Block::TickAnswer::PosMode::as_box: {
                std::list<UPDATE_BLOCK> res;
                for (auto x : RangeInt(tick_pos.x + to_be_calc.X, tick_pos.x + to_be_calc.x))
                    for (auto y : RangeInt(tick_pos.y + to_be_calc.Y, tick_pos.y + to_be_calc.y))
                        for (auto z : RangeInt(tick_pos.z + to_be_calc.Z, tick_pos.z + to_be_calc.z))
                            res.push_back({x, y, z});
                return res;
            }
            case Block::TickAnswer::PosMode::as_tunnels: {
                std::list<UPDATE_BLOCK> res;
                for (auto x : RangeInt(tick_pos.x + to_be_calc.X, tick_pos.x + to_be_calc.x))
                    res.push_back({x, tick_pos.y, tick_pos.z});
                for (auto y : RangeInt(tick_pos.y + to_be_calc.Y, tick_pos.y + to_be_calc.y))
                    res.push_back({tick_pos.x, y, tick_pos.z});
                for (auto z : RangeInt(tick_pos.z + to_be_calc.Z, tick_pos.z + to_be_calc.z))
                    res.push_back({tick_pos.x, tick_pos.y, z});
                res.unique();
                return res;
            }
            default:
                return {};
            }
        }

        static std::list<UPDATE_BLOCK> _UpdateBlockSelectMoveFreed(Block::TickAnswer to_be_calc, UPDATE_BLOCK tick_pos) {
            if (!(to_be_calc.move_x | to_be_calc.move_y | to_be_calc.move_z))
                return {};
            std::list<UPDATE_BLOCK> res;
            if (to_be_calc.move_x) {
                block_pos_t max_x = tick_pos.x + to_be_calc.move_x;
                auto&& tmp = _UpdateBlockSelect(to_be_calc, tick_pos);
                tmp.remove_if(
                    to_be_calc.move_x > 0 ? std::function([max_x](UPDATE_BLOCK& check) { return max_x < check.x; }) : std::function([max_x](UPDATE_BLOCK& check) { return max_x > check.x; })
                );
                res.insert(res.end(), tmp.begin(), tmp.end());
            }
            if (to_be_calc.move_y) {
                block_pos_t max_y = tick_pos.y + to_be_calc.move_y;
                auto&& tmp = _UpdateBlockSelect(to_be_calc, tick_pos);
                tmp.remove_if(
                    to_be_calc.move_y > 0 ? std::function([max_y](UPDATE_BLOCK& check) { return max_y < check.y; }) : std::function([max_y](UPDATE_BLOCK& check) { return max_y > check.y; })
                );
                res.insert(res.end(), tmp.begin(), tmp.end());
            }
            if (to_be_calc.move_z) {
                block_pos_t max_z = tick_pos.z + to_be_calc.move_z;
                auto&& tmp = _UpdateBlockSelect(to_be_calc, tick_pos);
                tmp.remove_if(
                    to_be_calc.move_z > 0 ? std::function([max_z](UPDATE_BLOCK& check) { return max_z < check.z; }) : std::function([max_z](UPDATE_BLOCK& check) { return max_z > check.z; })
                );
                res.insert(res.end(), tmp.begin(), tmp.end());
            }
            res.unique();
            return res;
        }

        void _UpdateBlockUpdateNow(size_t& updated_now, std::list<UPDATE_BLOCK>& on_updating_blocks, std::list<UPDATE_BLOCK>& add_blocks) {
            if (updated_now + add_blocks.size() <= max_one_tick_update_now) {
                on_updating_blocks.insert(on_updating_blocks.end(), add_blocks.begin(), add_blocks.end());
                updated_now += add_blocks.size();
            } else {
                to_be_updated_mut.lock();
                to_be_updated.insert(to_be_updated.end(), add_blocks.begin(), add_blocks.end());
                to_be_updated_mut.unlock();
            }
        }

        void UpdateBlock(UPDATE_BLOCK ub) {
            std::list<UPDATE_BLOCK> tmp;
            tmp.push_back(ub);
            std::list<UPDATE_BLOCK> ticked;
            size_t updated_now = 0;
            while (tmp.empty()) {
                auto& it = tmp.front();
                auto tanswer = LocalBlock(it.x, it.y, it.z).Tick(it.reason);
                switch (tanswer.behavior) {
                case Block::TickAnswer::Behavior::add_to_be_ticked_next:
                    addToBeUpdated(it.x, it.y, it.z);
                    break;
                case Block::TickAnswer::Behavior::check_can_move_and_move_blocks:
                    Task::start([this, tanswer, it]() {
                        auto&& temp = _UpdateBlockSelect(tanswer, it);
                        if (CalculateCanMoveBlocks(temp, tanswer.move_x, tanswer.move_y, tanswer.move_z))
                            MoveBlocks(temp, tanswer.move_x, tanswer.move_y, tanswer.move_z);
                    });
                    break;
                case Block::TickAnswer::Behavior::check_can_move_and_move_blocks_then_add_new_blocks:
                    Task::start([this, tanswer, it]() {
                        if (!tanswer.block)
                            return;
                        auto&& temp = _UpdateBlockSelect(tanswer, it);
                        if (CalculateCanMoveBlocks(temp, tanswer.move_x, tanswer.move_y, tanswer.move_z)) {
                            MoveBlocks(temp, tanswer.move_x, tanswer.move_y, tanswer.move_z);
                            setBlocks(_UpdateBlockSelectMoveFreed(tanswer, it), *tanswer.block);
                        }
                        delete tanswer.block;
                    });
                    break;
                case Block::TickAnswer::Behavior::move_blocks:
                    Task::start([this, tanswer, it]() {
                        MoveBlocks(
                            _UpdateBlockSelect(tanswer, it),
                            tanswer.move_x,
                            tanswer.move_y,
                            tanswer.move_z
                        );
                    });
                    break;
                case Block::TickAnswer::Behavior::move_blocks_then_add_new_blocks:
                    Task::start([this, tanswer, it]() {
                        if (!tanswer.block)
                            return;
                        MoveBlocks(_UpdateBlockSelect(tanswer, it), tanswer.move_x, tanswer.move_y, tanswer.move_z);
                        setBlocks(_UpdateBlockSelectMoveFreed(tanswer, it), *tanswer.block);
                        delete tanswer.block;
                    });
                    break;
                case Block::TickAnswer::Behavior::update_now: {
                    auto&& insert_value = _UpdateBlockSelect(tanswer, it);
                    _UpdateBlockUpdateNow(updated_now, tmp, insert_value);
                    break;
                }
                case Block::TickAnswer::Behavior::update_now_except_ourself: {
                    auto&& insert_value = _UpdateBlockSelect(tanswer, it);
                    insert_value.remove(it);
                    _UpdateBlockUpdateNow(updated_now, tmp, insert_value);
                    break;
                }
                case Block::TickAnswer::Behavior::update_now_except_ticked: {
                    ticked.push_back(it);
                    auto&& insert_value = _UpdateBlockSelect(tanswer, it);
                    insert_value.remove_if([&ticked](auto& x) { return std::find(ticked.begin(), ticked.end(), x) != ticked.end(); });
                    _UpdateBlockUpdateNow(updated_now, tmp, insert_value);
                    break;
                }
                case Block::TickAnswer::Behavior::clone:
                case Block::TickAnswer::Behavior::break_box:
                    breakBlock(it);
                    break;
                case Block::TickAnswer::Behavior::remove_box:
                    removeBlock(it);
                    break;
                case Block::TickAnswer::Behavior::_break:
                    breakBlock(it);
                    break;
                case Block::TickAnswer::Behavior::_remove:
                    removeBlock(it);
                    break;
                case Block::TickAnswer::Behavior::done:
                default:
                    break;
                }
                tmp.pop_front();
            }
        }

        std::list<WorldClusterTalker*> events;
        std::mutex events_mut;

        void SubEvents(WorldClusterTalker* handler) {
            if (handler) {
                events_mut.lock();
                if (std::find(events.begin(), events.end(), handler) == events.end())
                    events.push_back(handler);
                events_mut.unlock();
            }
        }

        void UnsubEvents(WorldClusterTalker* handler) {
            if (handler) {
                events_mut.lock();
                events.remove(handler);
                events_mut.unlock();
            }
        }

        void _EventNotifyHandler(BlockEvent data, WorldClusterTalker* handler) {
            switch (data.reason) {
            case BlockEvent::Reason::Changed:
                handler->BlockChanged(data.block, data.x, data.y, data.z);
                break;
            case BlockEvent::Reason::Seted:
                handler->BlockSeted(data.block, data.x, data.y, data.z);
                break;
            case BlockEvent::Reason::Removed:
                handler->BlockRemoved(data.x, data.y, data.z);
                break;
            case BlockEvent::Reason::Breaked:
                handler->BlockBreaked(data.block, data.x, data.y, data.z);
                break;
            case BlockEvent::Reason::nbt_changed:
                int64_t cluster_x_dif = handler->cluster_x - data.x / 24;
                int64_t cluster_z_dif = handler->cluster_z - data.z / 24;
                if (+cluster_x_dif <= handler->cluster_handle_distance)
                    if (+cluster_z_dif <= +handler->cluster_handle_distance)
                        handler->BlockHandleNbtChanged(data.block, data.x, data.y, data.z);
            }
        }

        void EventNotify(BlockEvent data) {
            switch (data.reason) {
            case BlockEvent::Reason::Changed:
            case BlockEvent::Reason::nbt_changed:
            case BlockEvent::Reason::Seted:
                LocalBlock(data.x, data.y, data.z) = *data.block;
                break;
            case BlockEvent::Reason::Removed:
            case BlockEvent::Reason::Breaked:
                LocalBlock(data.x, data.y, data.z) = Block();
                break;
            }
            Task::start([this, data]() {
                for (auto tmp : events)
                    if (data.source != tmp)
                        _EventNotifyHandler(data, tmp);
            });
        }

    public:
        size_t max_one_tick_update_now = 500;
        uint32_t world_id = 0;

        void addToBeUpdated(block_pos_t x, block_pos_t y, block_pos_t z) {
            to_be_updated_mut.lock();
            to_be_updated.push_back({x, y, z});
            to_be_updated_mut.unlock();
        }

        inline void UpdateBlock(block_pos_t x, block_pos_t y, block_pos_t z, Block::TickReason reason) {
            UpdateBlock({x, y, z, reason});
        }

        inline Block operator()(block_pos_t x, block_pos_t y, block_pos_t z) {
            return LocalBlock(x, y, z);
        }

        void Save(std::filesystem::path worldPath);
        void Load(std::filesystem::path worldPath);

        bool existsCluster(block_pos_t x, block_pos_t z) {
            if (!clusters.contains(x / 48))
                return false;
            if (!clusters[x / 48].contains(z / 48))
                return false;
            return clusters[x / 48][z / 48];
        }

        ChunkCluster& getCluster(block_pos_t x, block_pos_t z) {
            auto tmp = clusters[x / 48][z / 48];
            if (!tmp)
                tmp = clusters[x / 48][z / 48] = new ChunkCluster(x / 48, z / 48);
            return *tmp;
        }

        void UnloadCluster(block_pos_t x, block_pos_t z) {
            if (!existsCluster(x, z))
                return;
            auto&& tmp = clusters[x / 48][z / 48];
            tmp->thread_guard.lock();
            clusters[x / 48].erase(z / 48);
            tmp->thread_guard.unlock();
            delete tmp;
        }

        void UnloadCluster(block_pos_t x, block_pos_t z, std::filesystem::path worldPath) {
            if (!existsCluster(x, z))
                return;
            auto&& tmp = clusters[x / 48][z / 48];
            tmp->thread_guard.lock();
            clusters[x / 48][z / 48]->Save(worldPath);
            clusters[x / 48].erase(z / 48);
            tmp->thread_guard.unlock();
            delete tmp;
        }

        void LoadCluster(block_pos_t x, block_pos_t z, std::filesystem::path worldPath) {
            if (!existsCluster(x, z)) {
                auto tmp = new ChunkCluster(x, z);
                tmp->Load(worldPath);
                clusters[x / 48][z / 48] = tmp;
            }
        }

        static WorldClusters* getWorldByID(uint32_t world_id) {
            return nullptr;
        }

        void WorldTickUpdate() {
            std::list<UPDATE_BLOCK> tmp;

            to_be_updated_mut.lock();
            std::swap(tmp, to_be_updated);
            to_be_updated_mut.unlock();

            while (!tmp.empty()) {
                UpdateBlock(tmp.front());
                tmp.pop_front();
            }
        }

        WorldClusters() {}

        ~WorldClusters() {
            for (auto& it : clusters)
				for (auto& sub_it : it.second)
					if(sub_it.second)
						delete sub_it.second;
        }
    };
}