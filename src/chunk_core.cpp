#include "chunk_core.hpp"
#include "library/enbt.hpp"
#include "log.hpp"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <iostream>
#include <ranges>

namespace crafted_craft {
    ChunkGenerator* global_generator;

    uint8_t ChunkCore::chunk_y_count = 15;
    std::atomic_bool ChunkCore::mod_cys_allow = 1;

    inline ENBT prepareBlock(base_objects::block& bl) {
        ENBT block_data{
            ENBT((uint16_t)bl.id) //block id
        };
        return block_data;
    }

    inline base_objects::block prepareBlock(ENBT& bl) {
        uint16_t id = std::get<uint16_t>(bl[0].content());

		if (bl[1].contains())
            return base_objects::block(id);
        else
            return id;
    }


    void ChunkCore::Save(std::filesystem::path chunkPath) {
		chunkPath += std::filesystem::path::preferred_separator;
		chunkPath += std::to_string(pos_x) + "_" + std::to_string(pos_z);
		ENBT tree_z(ENBT::Type_ID::Type::array, 16);
		ENBT tree_y(ENBT::Type_ID::Type::array, 16);
		ENBT tree_x(ENBT::Type_ID::Type::array, 16);
		ENBT tree(ENBT::Type_ID::Type::darray, chunk_y_count);
		for (uint8_t i = 0; i < chunk_y_count; i++) {
			ENBT& sub_chunk = tree[i];
			for (uint8_t x = 0; x < chunk_y_count; x++) {

				sub_chunk[x] = tree_x;
				ENBT& xit = sub_chunk[x];
				for (uint8_t y = 0; y < chunk_y_count; y++) {

					xit[y] = tree_y;
					ENBT& yit = sub_chunk[y];
					for (uint8_t z = 0; z < chunk_y_count; z++) {
						xit[y] = tree_z;
						sub_chunk[z] = prepareBlock(chunk_fragments[i][x][y][z]);


					}
				}
			}
		}

		std::ofstream f(chunkPath.c_str());
        ENBTHelper::InitializeVersion(f);
        ENBTHelper::WriteToken(f, tree);
        f.close();
    }

    void ChunkCluster::Save(std::filesystem::path clusterPath) {
		clusterPath += std::filesystem::path::preferred_separator;
		clusterPath += std::to_string(pos_x) + "_" + std::to_string(pos_z);
		if (!std::filesystem::exists(clusterPath)) std::filesystem::create_directory(clusterPath);
		for (auto& tmp : chunks)
			for (auto& chunk : tmp)
				chunk.Save(clusterPath);
    }

    void WorldClusters::Save(std::filesystem::path worldPath) {
        for (auto& tmp : clusters)
            for (auto& cluster : tmp.second)
                cluster.second->Save(worldPath);
    }

    void ChunkCore::Load(std::filesystem::path chunkPath) {
        chunkPath += std::filesystem::path::preferred_separator;
        chunkPath += std::to_string(pos_x) + "_" + std::to_string(pos_z);

        std::ifstream f(chunkPath.c_str());
		ENBTHelper::CheckVersion(f);
		ENBT tree = ENBTHelper::ReadToken(f);
		f.close();
		for (uint8_t i = 0; i < chunk_y_count; i++) {

			ENBT& sub_chunk = tree[i];
			for (uint8_t x = 0; x < chunk_y_count; x++) {

				ENBT& xit = sub_chunk[x];
				for (uint8_t y = 0; y < chunk_y_count; y++) {

					ENBT& yit = sub_chunk[y];
					for (uint8_t z = 0; z < chunk_y_count; z++)
						chunk_fragments[i][x][y][z] = prepareBlock(sub_chunk[z]);

				}
			}
		}
    }

    void ChunkCluster::Load(std::filesystem::path clusterPath) {
        if (!std::filesystem::exists(clusterPath))
            std::filesystem::create_directory(clusterPath);
        for (auto& tmp : chunks)
			for (auto& chunk : tmp)
				chunk.Load(clusterPath);
    }

    void WorldClusters::Load(std::filesystem::path worldPath) {
        for (auto& p : std::filesystem::recursive_directory_iterator(worldPath))
            if (p.is_directory()) {
                try {
                    std::u8string path_string{p.path().u8string()};
                    list_array<std::u8string> coords;
                    boost::split(coords, path_string, boost::algorithm::is_any_of(u8"_"), boost::algorithm::token_compress_on);
                    if (coords.size() != 2)
                        throw std::invalid_argument("Coordinates count will be equal 2");
                    int64_t x = std::stoll((const char*)coords[0].c_str());
                    int64_t z = std::stoll((const char*)coords[0].c_str());
                    (clusters[x][z] = new ChunkCluster(x, z))->Load(p.path());
                } catch (const std::bad_alloc& ex) {
                    log::error("WorldClusters::Load", "Fail load cluster, " + std::string(ex.what()));
                    std::exit(EXIT_FAILURE);
                } catch (const std::invalid_argument& arg) {
                    log::error("WorldClusters::Load", "Cluster folder: `" + p.path().string() + "` is invalid, cause: " + arg.what());
                } catch (const std::out_of_range& arg) {
                    log::error("WorldClusters::Load", "Cluster folder: `" + p.path().string() + "` is invalid, cause: " + arg.what());
                }
            }
    }

    void WorldClusterTalker::ClusterDeath() {
        race_lock.lock();
        world_handle = nullptr;
        race_lock.unlock();
		ClusterUnloaded(cluster_x, cluster_z);
    }

    WorldClusterTalker::WorldClusterTalker(class WorldClusters& world, block_pos_t x, block_pos_t z, block_pos_t distance)
        : world_handle(&world) {
        cluster_x = x;
        cluster_z = z;
		cluster_handle_distance = distance;
		if (world.existsCluster(x, z)) 
			world.SubEvents(this);
    }

    void WorldClusterTalker::NotifyBlockChanged(BlockEventData data) {
        if (world_handle) {
			race_lock.lock();
			world_handle->EventNotify({ this,data.block,data.x, data.y, data.z , BlockEvent::Reason::Changed });
			race_lock.unlock();
		}
		else {
            log::error("WorldClusterTalker::NotifyBlockChanged", "World death");
            std::exit(EXIT_FAILURE);
        }
    }

    void WorldClusterTalker::NotifyBlockPlaced(BlockEventData data) {
        if (world_handle) {
            race_lock.lock();
            world_handle->EventNotify({this, data.block, data.x, data.y, data.z, BlockEvent::Reason::Placed});
            race_lock.unlock();
        } else {
            log::error("WorldClusterTalker::NotifyBlockPlaced", "World death");
            std::exit(EXIT_FAILURE);
        }
    }

    void WorldClusterTalker::NotifyBlockRemoved(BlockEventData data) {
        if (world_handle) {
            race_lock.lock();
            world_handle->EventNotify({this, nullptr, data.x, data.y, data.z, BlockEvent::Reason::Removed});
            race_lock.unlock();
        } else {
            log::error("WorldClusterTalker::NotifyBlockRemoved", "World death");
            std::exit(EXIT_FAILURE);
        }
    }

    void WorldClusterTalker::NotifyBlockBroken(BlockEventData data) {
        if (world_handle) {
            race_lock.lock();
            world_handle->EventNotify({this, nullptr, data.x, data.y, data.z, BlockEvent::Reason::Broken});
            race_lock.unlock();
        } else {
            log::error("WorldClusterTalker::NotifyBlockBroken", "World death");
            std::exit(EXIT_FAILURE);
        }
    }

    const ChunkCluster& WorldClusterTalker::ClusterRequest(block_pos_t x, block_pos_t z) {
        if (world_handle) {
            race_lock.lock();
            auto& res = world_handle->getCluster(x, z);
            race_lock.unlock();
            return res;
        } else {
            log::error("WorldClusterTalker::ClusterRequest", "World death");
            std::exit(EXIT_FAILURE);
            throw std::exception("How you do this");
        }
    }

    const base_objects::block& WorldClusterTalker::BlockRequest(block_pos_t x, block_pos_t y, block_pos_t z) {
        if (world_handle) {
            race_lock.lock();
            auto& res = world_handle->LocalBlock(x, y, z);
            race_lock.unlock();
            return res;
        } else {
            log::error("WorldClusterTalker::BlockRequest", "World death");
            std::exit(EXIT_FAILURE);
            throw std::exception("How you do this");
        }
    }

    WorldClusterTalker::~WorldClusterTalker() {
        race_lock.lock();
        if (world_handle)
            world_handle->UnsubEvents(this);
        race_lock.unlock();
    }
}