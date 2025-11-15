/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/bin/ecs/manager.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/entity_id_map.hpp>

namespace copper_server::api::ecs {

    manager management;

    struct entity_allocation_request {
        const entity_recipe& recipe;
        std::optional<int32_t> world_id;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        entity result;
        bool ready = false;
    };

    struct entity_creation_request {
        std::unique_ptr<detail::components_holder> components;
        std::optional<int32_t> world_id;
        entity_recipe calculated_recipe;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        entity result;
        bool ready = false;
    };

    struct entity_copy_request {
        entity other_entity;
        std::optional<int32_t> world_id;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        std::optional<entity> result;
        std::exception_ptr ex;
        bool ready = false;
    };

    struct entity_transfer_request {
        int32_t id;
        uint32_t generation;
        std::optional<int32_t> world_id;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        bool ready = false;
        bool success = false;
    };

    struct relation_change_item {
        component_id component;
        entity parent;
        entity child;
        bool is_add;
    };

    moodycamel::ConcurrentQueue<entity_allocation_request*> allocation_queue;
    moodycamel::ConcurrentQueue<entity_creation_request*> creation_queue;
    moodycamel::ConcurrentQueue<entity_copy_request*> copy_queue;
    moodycamel::ConcurrentQueue<entity_transfer_request*> transfer_queue;
    moodycamel::ConcurrentQueue<relation_change_item> relation_change_queue;

    world_local_registry::world_local_registry(int32_t id) : id(id) {
        management.enable_world(id);
    }

    world_local_registry::~world_local_registry() {
        management.disable_world(id);
    }

    fast_task::future_ptr<bool> world_local_registry::register_entity_async(entity& entity) {
        auto request = std::make_unique<entity_transfer_request>(entity.id, entity.generation, id);
        if (!transfer_queue.enqueue(request.get()))
            return fast_task::make_ready_future(false);
        return fast_task::future<bool>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return req->success;
        });
    }

    fast_task::future_ptr<bool> world_local_registry::unregister_entity_async(entity& entity) {
        auto request = std::make_unique<entity_transfer_request>(entity.id, entity.generation);
        if (!transfer_queue.enqueue(request.get()))
            return fast_task::make_ready_future(false);
        return fast_task::future<bool>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return req->success;
        });
    }

    fast_task::future_ptr<bool> world_local_registry::transfer_entity_async(entity& entity) {
        return register_entity_async(entity);
    }

    fast_task::future_ptr<entity> world_local_registry::allocate_entity_async(const entity_recipe& recipe) {
        if (!recipe.is_frozen())
            throw std::runtime_error("The recipe should be frozen before creating an entity!");
        auto request = std::make_unique<entity_allocation_request>(recipe, id);
        if (!allocation_queue.enqueue(request.get()))
            throw std::bad_alloc();
        return fast_task::future<entity>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return std::move(req->result);
        });
    }

    bool world_local_registry::register_entity_and_block(entity& entity) {
        return register_entity_async(entity).get();
    }

    bool world_local_registry::unregister_entity_and_block(entity& entity) {
        return unregister_entity_async(entity).get();
    }

    bool world_local_registry::transfer_entity_and_block(entity& entity) {
        return transfer_entity_async(entity).get();
    }

    entity world_local_registry::allocate_entity_and_wait(const entity_recipe& recipe) {
        return allocate_entity_async(recipe)->take();
    }

    fast_task::future_ptr<entity> global_registry::allocate_entity_async(const entity_recipe& recipe) {
        if (!recipe.is_frozen())
            throw std::runtime_error("The recipe should be frozen before creating an entity!");
        auto request = std::make_unique<entity_allocation_request>(recipe);
        if (!allocation_queue.enqueue(request.get()))
            throw std::bad_alloc();
        return fast_task::future<entity>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return std::move(req->result);
        });
    }

    entity global_registry::allocate_entity_and_wait(const entity_recipe& recipe) {
        return global_registry::allocate_entity_async(recipe)->take();
    }

    struct system_node {
        std::unique_ptr<system_interface> instance;
        const detail::system_info& info;
        size_t in_degree = 0;
    };

    struct scheduler::scheduler_data {
        std::vector<system_node> systems;
        std::unordered_map<size_t, std::vector<size_t>> dependency_graph;
        bool graph_is_dirty = false;

        void build_tree() {
            dependency_graph.clear();
            for (auto& node : systems)
                node.in_degree = 0;

            std::unordered_map<component_id, std::vector<size_t>> component_writers;
            std::unordered_map<component_id, std::vector<size_t>> component_readers;

            for (size_t i = 0; i < systems.size(); ++i) {
                const auto& info = systems[i].info;

                for (const auto& comp_id : info.write_dependencies)
                    component_writers[comp_id].push_back(i);
                for (const auto& comp_id : info.read_dependencies)
                    component_readers[comp_id].push_back(i);
            }


            bit_list_array<> dependency_added(systems.size());

            for (size_t predecessor_idx = 0; predecessor_idx < systems.size(); ++predecessor_idx) {
                const auto& predecessor_info = systems[predecessor_idx].info;

                dependency_added.zero_all();
                dependency_added.set(predecessor_idx, true);

                for (const auto& comp_id : predecessor_info.write_dependencies) {
                    if (component_readers.count(comp_id)) {
                        for (const auto successor_idx : component_readers.at(comp_id)) {
                            if (!dependency_added[successor_idx]) {
                                dependency_graph[predecessor_idx].push_back(successor_idx);
                                systems[successor_idx].in_degree++;
                                dependency_added[successor_idx] = true;
                            }
                        }
                    }

                    if (component_writers.count(comp_id)) {
                        for (const auto successor_idx : component_writers.at(comp_id)) {
                            if (!dependency_added.get_unchecked(successor_idx)) {
                                dependency_graph[predecessor_idx].push_back(successor_idx);
                                systems[successor_idx].in_degree++;
                                dependency_added.set(successor_idx, true);
                            }
                        }
                    }
                }
            }

            graph_is_dirty = false;
        }

        void proceed_tree(world_local_registry& registry) {
            std::vector<size_t> current_in_degrees;
            current_in_degrees.reserve(systems.size());
            for (const auto& node : systems) {
                current_in_degrees.push_back(node.in_degree);
            }

            size_t systems_executed_count = 0;
            list_array<size_t> ready_queue;
            for (size_t i = 0; i < systems.size(); ++i)
                if (current_in_degrees[i] == 0)
                    ready_queue.push_back(i);

            while (!ready_queue.empty()) {
                std::vector<size_t> systems_in_current_stage;
                while (!ready_queue.empty()) {
                    systems_in_current_stage.push_back(ready_queue.front());
                    ready_queue.pop_front();
                }

                fast_task::future_tool::for_each_wait(systems_in_current_stage, [&](size_t system_index) {
                    systems[system_index].instance->tick(registry);
                });

                systems_executed_count += systems_in_current_stage.size();

                for (size_t completed_system_index : systems_in_current_stage) {
                    if (dependency_graph.count(completed_system_index)) {
                        for (size_t dependent_index : dependency_graph.at(completed_system_index)) {
                            current_in_degrees[dependent_index]--;
                            if (current_in_degrees[dependent_index] == 0)
                                ready_queue.push_back(dependent_index);
                        }
                    }
                }
            }

            if (systems_executed_count < systems.size()) {
                std::string error_message = "Circular dependency detected in systems! The following systems could not be executed:\n";
                for (size_t i = 0; i < systems.size(); ++i)
                    if (current_in_degrees[i] > 0)
                        error_message += "\tSystem " + std::string(systems[i].info.info.name()) + "\n";
                throw std::runtime_error(error_message);
            }
        }
    };

    scheduler::scheduler() : data(std::make_unique<scheduler_data>()) {}

    scheduler::~scheduler() {}

    namespace mutation_processing {
        struct pre_allocated_mutation_op {
            int32_t entity_id;
            component_id component;
            archetype* to_archetype;
            std::vector<char> data; //if nullptr the component will be removed
        };

        struct prepared_move_op {
            int32_t entity_id;
            archetype* from_archetype;
            archetype* to_archetype;
            component_id comp_id;
            std::vector<char> data; // Owns the new component data
        };

        struct prepared_in_place_update_op {
            int32_t entity_id;
            component_id comp_id;
            std::vector<char> data;
        };

        struct parallel_allocation_op {
            entity_allocation_request* request;
            archetype* target_archetype;
            world* w;
        };

        struct parallel_creation_op {
            entity_creation_request* request;
            archetype* target_archetype;
            world* w;
        };

        struct parallel_copy_op {
            entity_copy_request* request;
            archetype* target_archetype;
        };

        struct parallel_transfer_op {
            entity_transfer_request* request;
            archetype* target_archetype;
            world* w;
        };

        struct grouped_mutation_ops {
            ::moodycamel::ConcurrentQueue<prepared_move_op> prepared_moves;
            ::moodycamel::ConcurrentQueue<prepared_in_place_update_op> prepared_updates;
        };

        template <class T, class FN>
        void consume_all_(::moodycamel::ConcurrentQueue<T>& queue, FN&& callback) {
            T item;
            while (queue.try_dequeue(item))
                callback(std::move(item));
        }

        template <class T, class FN>
        void parallel_drain(::moodycamel::ConcurrentQueue<T>& queue, FN&& process_func) {
            auto num_workers = std::thread::hardware_concurrency();
            std::vector<fast_task::future_ptr<void>> worker_futures;
            worker_futures.reserve(num_workers);

            for (decltype(num_workers) i = 0; i < num_workers; ++i) {
                worker_futures.push_back(fast_task::future<void>::start([&]() {
                    T item;
                    while (queue.try_dequeue(item))
                        process_func(std::move(item));
                }));
            }

            fast_task::future_tool::wait_all(std::move(worker_futures));
        }

        template <class T>
        std::vector<T> collect_all_(::moodycamel::ConcurrentQueue<T>& queue) {
            std::vector<T> res;
            res.reserve(queue.size_approx());
            T item;
            while (queue.try_dequeue(item))
                res.emplace_back(std::move(item));
            return res;
        }

        void process_relation_changes() {
            std::lock_guard guard(management.relation_mutex);
            consume_all_(relation_change_queue, [](relation_change_item&& item) {
                if (!management.has_entity(item.parent.id, item.parent.generation))
                    return;
                if (!management.has_entity(item.child.id, item.child.generation))
                    return;
                if (item.is_add)
                    management.add_entity_relation(item.parent.id, item.child.id, item.component);
                else
                    management.remove_entity_relation(item.parent.id, item.child.id, item.component);
            });
        }

        void process_destruction_queue(world& w) {
            consume_all_(w.entity_destroy_queue, [](entity_destroy_queue_item&& item) {
                if (management.has_entity(item.id, item.generation))
                    management.deallocate_entity(item.id);
            });
        }

        std::vector<std::vector<prepared_move_op>> group_disjoint_moves(std::vector<prepared_move_op>&& all_moves) {
            std::vector<std::vector<prepared_move_op>> parallel_batches;

            while (!all_moves.empty()) {
                std::vector<prepared_move_op> current_batch;
                std::unordered_set<archetype*> archetypes_in_batch;

                auto new_end = std::remove_if(all_moves.begin(), all_moves.end(), [&](prepared_move_op& move_op) {
                    bool has_conflict = archetypes_in_batch.count(move_op.from_archetype) || archetypes_in_batch.count(move_op.to_archetype);

                    if (!has_conflict) {
                        archetypes_in_batch.insert(move_op.from_archetype);
                        archetypes_in_batch.insert(move_op.to_archetype);
                        current_batch.push_back(std::move(move_op));
                        return true;
                    }
                    return false;
                });

                all_moves.erase(new_end, all_moves.end());
                parallel_batches.push_back(std::move(current_batch));
            }
            return parallel_batches;
        }

        grouped_mutation_ops prepare_mutations(world& w) {
            grouped_mutation_ops res;
            consume_all_(w.mutation_queue, [&](detail::mutation_queue_item&& item) {
                if (!management.has_entity(item.entity_id, item.generation))
                    return;

                auto& record = management.records.at(item.entity_id);
                archetype* from_archetype = record.type;
                world* in_world = record.world_owner;

                if (!item.data.empty() && from_archetype->component_index_map.count(item.component)) {
                    prepared_in_place_update_op update;
                    update.entity_id = item.entity_id;
                    update.comp_id = item.component;
                    update.data = std::move(item.data);

                    res.prepared_updates.enqueue(std::move(update));
                    return;
                }


                archetype* to_archetype = nullptr;
                if (item.data.empty()) {
                    if (auto it = from_archetype->remove_transition_cache.find(item.component); it == from_archetype->remove_transition_cache.end()) {
                        to_archetype = in_world->map_new_archetype(from_archetype, item.component);
                    }
                } else {
                    if (auto it = from_archetype->add_transition_cache.find(item.component); it == from_archetype->add_transition_cache.end())
                        to_archetype = in_world->map_new_archetype(from_archetype, item.component);
                }
                res.prepared_moves.enqueue(
                    prepared_move_op{
                        item.entity_id,
                        from_archetype,
                        to_archetype,
                        item.component,
                        std::move(item.data)
                    }
                );
            });
            return res;
        }

        std::vector<parallel_allocation_op> prepare_allocation_requests() {
            std::vector<parallel_allocation_op> res;
            res.reserve(allocation_queue.size_approx());
            consume_all_(allocation_queue, [&](entity_allocation_request* item) {
                if (item->world_id)
                    if (auto it = management.worlds.find(*item->world_id); it != management.worlds.end()) {
                        res.emplace_back(item, it->second.map_get_archetype(item->recipe.get_ids()), &it->second);
                        return;
                    }

                res.emplace_back(item, management.limbo.map_get_archetype(item->recipe.get_ids()), &management.limbo);
            });
            return res;
        }

        std::vector<parallel_creation_op> prepare_creation_requests() {
            std::vector<parallel_creation_op> res;
            res.reserve(creation_queue.size_approx());
            consume_all_(creation_queue, [&](entity_creation_request* item) {
                if (item->world_id)
                    if (auto it = management.worlds.find(*item->world_id); it != management.worlds.end()) {
                        res.emplace_back(item, it->second.map_get_archetype(item->calculated_recipe.get_ids()), &it->second);
                        return;
                    }
                res.emplace_back(item, management.limbo.map_get_archetype(item->calculated_recipe.get_ids()), &management.limbo);
            });
            return res;
        }

        std::vector<parallel_copy_op> prepare_copy_requests() {
            std::vector<parallel_copy_op> res;
            res.reserve(copy_queue.size_approx());
            consume_all_(copy_queue, [&](entity_copy_request* item) {
                res.emplace_back(item, management.records.at(item->other_entity.id).type);
            });
            return res;
        }

        void execute_mutations(grouped_mutation_ops mutations, ::moodycamel::ConcurrentQueue<int32_t>& arch_type_changes) {
            consume_all_(mutations.prepared_updates, [](prepared_in_place_update_op&& update) {
                auto& record = management.records.at(update.entity_id);
                auto& component_info = detail::component_info_registry.at(update.comp_id);
                auto component_index = record.type->component_index_map.at(update.comp_id);
                void* dest_ptr = record.chunk->memory_block.get() + record.type->layout.component_offsets[component_index] + (record.chunk_index * component_info.size);

                component_info.destroy(dest_ptr);
                component_info.move_construct(dest_ptr, (void*)update.data.data());
                record.type->mark_dirty(record.chunk, component_index, record.chunk_index);
            });


            for (auto& batch : group_disjoint_moves(collect_all_(mutations.prepared_moves))) {
                fast_task::future_tool::for_each_move(batch, [&](prepared_move_op&& move) {
                    archetype* arch1 = move.from_archetype;
                    archetype* arch2 = move.to_archetype;
                    if (arch1 == arch2)
                        return;

                    if (reinterpret_cast<uintptr_t>(arch1) > reinterpret_cast<uintptr_t>(arch2))
                        std::swap(arch1, arch2);

                    {
                        auto& record = management.records[move.entity_id];
                        archetype* expected = nullptr;
                        if (record.archetype_before_mutation.compare_exchange_strong(expected, record.type, std::memory_order_relaxed))
                            arch_type_changes.enqueue(move.entity_id);
                    }
                    std::unique_lock lock1(arch1->arch_mutex);
                    std::unique_lock lock2(arch2->arch_mutex);

                    auto& record = management.records[move.entity_id];
                    management.move_entity(move.entity_id, move.to_archetype, record.world_owner);
                    lock2.unlock();
                    lock1.unlock();

                    if (move.data.size()) {
                        auto& component_info = detail::component_info_registry.at(move.comp_id);
                        auto component_index = record.type->component_index_map.at(move.comp_id);
                        void* dest_ptr = record.chunk->memory_block.get() + record.type->layout.component_offsets[component_index] + (record.chunk_index * component_info.size);

                        component_info.move(dest_ptr, (void*)move.data.data());
                        record.type->mark_dirty(record.chunk, component_index, record.chunk_index);
                    }
                })->wait();
            }
        }

        void process_transfers() {
            std::vector<parallel_transfer_op> res;
            res.reserve(transfer_queue.size_approx());
            consume_all_(transfer_queue, [&res](entity_transfer_request* transfer) {
                if (management.records.size() > transfer->id) {
                    auto& record = management.records[transfer->id];
                    if (record.generation == transfer->generation) {

                        if (transfer->world_id)
                            if (auto it = management.worlds.find(*transfer->world_id); it != management.worlds.end()) {
                                res.emplace_back(transfer, it->second.map_get_archetype(record.type->component_ids), &it->second);
                                return;
                            }
                        res.emplace_back(transfer, management.limbo.map_get_archetype(record.type->component_ids), &management.limbo);
                    }
                }
            });


            fast_task::future_tool::for_each_move(std::move(res), [](parallel_transfer_op&& transfer) {
                fast_task::unique_lock lock(transfer.request->mut);
                transfer.request->success = false;
                auto arch1 = management.records[transfer.request->id].type;
                auto arch2 = transfer.target_archetype;


                if (reinterpret_cast<uintptr_t>(arch1) > reinterpret_cast<uintptr_t>(arch2))
                    std::swap(arch1, arch2);

                std::unique_lock lock1(arch1->arch_mutex);
                std::unique_lock lock2(arch2->arch_mutex);

                management.move_entity(transfer.request->id, transfer.target_archetype, transfer.w);
                lock2.unlock();
                lock1.unlock();


                transfer.request->ready = true;
                transfer.request->cv.notify_one();
            });
        }

        void process_dirty_marking(world& w) {
            parallel_drain(w.marking_queue, [](entity_dirty_mark_item&& mark) {
                auto& record = management.records.at(mark.id);
                if (record.generation != mark.generation)
                    if (auto component_index = record.type->component_index_map.find(mark.component); component_index != record.type->component_index_map.end())
                        record.type->mark_dirty(record.chunk, component_index->second, record.chunk_index);
            });
        }

        void process_allocation(std::vector<parallel_allocation_op>&& creation_requests) {
            fast_task::future_tool::for_each_move(std::move(creation_requests), [&](parallel_allocation_op&& reg) {
                auto [item, arch, world] = reg;
                fast_task::lock_guard message_lock(item->mut);
                fast_task::lock_guard arch_lock(arch->arch_mutex);
                item->result = management.allocate_entity(arch, world);
                item->ready = true;
                item->cv.notify_one();
            })->wait();
        }

        void process_creation(std::vector<parallel_creation_op>&& creation_requests) {
            fast_task::future_tool::for_each_move(std::move(creation_requests), [&](parallel_creation_op&& reg) {
                auto [item, arch, world] = reg;
                fast_task::lock_guard message_lock(item->mut);
                fast_task::lock_guard arch_lock(arch->arch_mutex);
                item->result = management.allocate_entity(arch, world);


                auto& record = management.records.at(item->result.id);
                for (auto& [id, ptr] : reg.request->components->components_reference) {
                    auto& component_info = detail::component_info_registry.at(id);
                    auto component_index = record.type->component_index_map.at(id);
                    void* dest_ptr = record.chunk->memory_block.get() + record.type->layout.component_offsets[component_index] + (record.chunk_index * component_info.size);

                    component_info.move(dest_ptr, (void*)ptr);
                    record.type->mark_dirty(record.chunk, component_index, record.chunk_index);
                }


                item->ready = true;
                item->cv.notify_one();
            })->wait();
        }

        void process_copy(std::vector<parallel_copy_op>&& copy_requests) {
            fast_task::future_tool::for_each_move(std::move(copy_requests), [&](parallel_copy_op&& reg) {
                auto [item, arch] = reg;
                fast_task::lock_guard message_lock(item->mut);
                if (!arch) {
                    item->ready = true;
                    item->cv.notify_one();
                    return;
                }
                fast_task::lock_guard arch_lock(arch->arch_mutex);
                auto& other_record = management.records.at(item->other_entity.id);
                item->result = management.allocate_entity(arch, other_record.world_owner);

                auto& record = management.records.at(item->result->id);
                for (auto& [id, component_index] : arch->component_index_map) {
                    auto& component_info = detail::component_info_registry.at(id);
                    void* src_ptr = other_record.chunk->memory_block.get() + arch->layout.component_offsets[component_index] + (other_record.chunk_index * component_info.size);
                    void* dest_ptr = record /*  */.chunk->memory_block.get() + arch->layout.component_offsets[component_index] + (record.chunk_index * component_info.size);
                    try {
                        component_info.copy_assign(dest_ptr, src_ptr);
                    } catch (...) {
                        item->ex = std::current_exception();
                        break;
                    }
                    record.type->mark_dirty(record.chunk, component_index, record.chunk_index);
                }


                item->ready = true;
                item->cv.notify_one();
            })->wait();
        }

        void proceed_mutations() {
            process_relation_changes();
            process_transfers();
            process_allocation(prepare_allocation_requests());
            process_creation(prepare_creation_requests());
            process_copy(prepare_copy_requests());
        }

        void proceed_mutations(world& w) {
            process_destruction_queue(w);
            execute_mutations(prepare_mutations(w), w.arch_type_changes);
            process_dirty_marking(w);
        }
    }

    void global_registry::global_tick() {
        fast_task::unique_lock world_guard(management.manager_mutex);
        mutation_processing::proceed_mutations();
    }

    void scheduler::execute_frame(world_local_registry& registry) {
        fast_task::shared_lock world_guard(management.manager_mutex);
        world& current_world = management.worlds.at(registry.get_id());
        mutation_processing::proceed_mutations(current_world);

        if (data->graph_is_dirty)
            data->build_tree();
        {
            fast_task::relock_guard relock(world_guard);
            data->proceed_tree(registry);
        }

        fast_task::future_tool::for_each_wait(current_world.archetypes, [](const std::unique_ptr<archetype>& archetype_ptr) {
            for (std::unique_ptr<chunk>& ch : archetype_ptr->chunks)
                for (auto& flags : archetype_ptr->dirty_flags(ch.get()))
                    for (auto& flag_word : flags)
                        flag_word.store(0, std::memory_order_relaxed);
        });

        mutation_processing::parallel_drain(current_world.arch_type_changes, [](int32_t id) {
            management.records[id].archetype_before_mutation = nullptr;
        });
    }

    void scheduler::add_system_impl(std::unique_ptr<system_interface> system, const detail::system_info& info) {
        fast_task::unique_lock world_guard(management.manager_mutex);
        data->systems.push_back(system_node(std::move(system), info));
        data->graph_is_dirty = true;
    }

    namespace detail {
        std::atomic<component_id> next_component_id = 0;
        std::vector<component_type_info> component_info_registry;
        fast_task::mutex registry_mutex;

        world* get_queues_for_entity(int32_t id, uint32_t generation) {
            if (!management.has_entity(id, generation))
                return nullptr;
            auto& record = management.records.at(id);
            return record.world_owner;
        }

        void* get_entity_component(int32_t id, uint32_t generation, component_id component_id) {
            auto& record = management.records.at(id);
            if (record.generation != generation)
                return nullptr;
            auto arch = record.type;
            auto it = arch->component_index_map.find(component_id);
            if (it == arch->component_index_map.end())
                return nullptr;
            else {
                size_t base_offset = record.type->layout.component_offsets[it->second];
                const auto& type_info = detail::component_info_registry[component_id];
                return record.chunk->memory_block.get() + base_offset + (record.chunk_index * type_info.size);
            }
        }

        void queue_command(detail::mutation_queue_item&& command) {
            if (!get_queues_for_entity(command.entity_id, command.generation) //TODO add check
                     ->mutation_queue.enqueue(std::move(command)))
                throw std::bad_alloc();
        }

        void queue_destroy_entity(int32_t id, uint32_t generation) {
            if (!get_queues_for_entity(id, generation) //TODO add check
                     ->entity_destroy_queue.enqueue(entity_destroy_queue_item{id, generation}))
                throw std::bad_alloc();
        }

        void queue_mark_dirty(int32_t id, uint32_t generation, component_id component_id) {
            if (!get_queues_for_entity(id, generation) //TODO add check
                     ->marking_queue.enqueue(entity_dirty_mark_item{id, generation, component_id}))
                throw std::bad_alloc();
        }

        bool queue_add_relation(component_id component_id, entity parent, entity child) {
            return relation_change_queue.enqueue({component_id, parent, child, true});
        }

        bool queue_remove_relation(component_id component_id, entity parent, entity child) {
            return relation_change_queue.enqueue({component_id, parent, child, false});
        }

        bool has_relation(component_id component_id, entity parent, entity child) {
            fast_task::shared_lock lock(management.manager_mutex);
            if (!management.has_entity(parent.id, parent.generation) || !management.has_entity(child.id, child.generation))
                return false;
            if (auto com = management.relation_index.find(component_id); com != management.relation_index.end())
                if (auto it = com->second.find(parent.id); it != com->second.end())
                    if (auto item = std::find(it->second.begin(), it->second.end(), child.id); item != it->second.end())
                        return true;
            return false;
        }

        bool has_entity_component(int32_t id, uint32_t generation, component_id component_id) {
            auto& record = management.records.at(id);
            if (record.generation == generation)
                return record.type->component_index_map.contains(component_id);
            else
                return false;
        }

        std::optional<int32_t> get_entity_assigned_to_world(int32_t id, uint32_t generation) {
            auto& record = management.records.at(id);
            if (record.generation == generation && record.world_owner != &management.limbo)
                return record.world_owner->id;
            else
                return std::nullopt;
        }

        fast_task::future_ptr<entity> create_entity(std::optional<int32_t> world_id, std::unique_ptr<components_holder> components) {
            auto request = std::make_unique<entity_creation_request>(std::move(components), world_id);
            for (auto [id, ptr] : request->components->components_reference)
                request->calculated_recipe.with(id);
            request->calculated_recipe.freeze();

            if (!creation_queue.enqueue(request.get()))
                throw std::bad_alloc();
            return fast_task::future<entity>::start([req = std::move(request)]() {
                fast_task::mutex_unify unify(req->mut);
                fast_task::unique_lock lock(unify);
                while (!req->ready)
                    req->cv.wait(lock);
                return std::move(req->result);
            });
        }

        fast_task::future_ptr<entity> create_entity(std::optional<int32_t> world_id, const api::ecs::entity_recipe& base_recipe, std::unique_ptr<components_holder> components) {
            auto request = std::make_unique<entity_creation_request>(std::move(components), world_id);
            request->calculated_recipe.with(base_recipe);
            for (auto [id, ptr] : request->components->components_reference)
                request->calculated_recipe.with(id);
            request->calculated_recipe.freeze();

            if (!creation_queue.enqueue(request.get()))
                throw std::bad_alloc();
            return fast_task::future<entity>::start([req = std::move(request)]() {
                fast_task::mutex_unify unify(req->mut);
                fast_task::unique_lock lock(unify);
                while (!req->ready)
                    req->cv.wait(lock);
                return std::move(req->result);
            });
        }

        fast_task::future_ptr<std::optional<entity>> copy_entity(std::optional<int32_t> world_id, const api::ecs::entity& base_entity) {
            auto request = std::make_unique<entity_copy_request>(base_entity, world_id);

            if (!copy_queue.enqueue(request.get()))
                throw std::bad_alloc();
            return fast_task::future<std::optional<entity>>::start([req = std::move(request)]() {
                fast_task::mutex_unify unify(req->mut);
                fast_task::unique_lock lock(unify);
                while (!req->ready)
                    req->cv.wait(lock);
                if (req->ex)
                    std::rethrow_exception(req->ex);
                return std::move(*req->result);
            });
        }

        fast_task::task_rw_mutex& immediate_lock() {
            return management.manager_mutex;
        }

        struct iteration_handle::iteration_data {
            struct arch_data_t {
                archetype* type;
                std::vector<size_t> required_layout_offsets;
                std::vector<uint32_t> required_clean_comp_indices; // Use correct type
                std::vector<uint32_t> required_dirty_comp_indices; // Use correct type
                std::vector<uint32_t> make_dirty_comp_indices;     // Use correct type

                arch_data_t(archetype* type) : type(type) {}
            };

            fast_task::shared_lock<fast_task::task_rw_mutex> main_lock;
            size_t current_archetype_index = 0;
            size_t current_chunk_index = 0;
            std::vector<arch_data_t> arch_data;
            std::vector<component_id> with_changes;
            std::vector<void*> component_arrays;

            void calculate_data(std::span<component_id> components, std::span<component_id> clean_components, std::span<component_id> dirty_components, std::span<component_id> mark_dirty_components, std::span<component_id> with_changes_) {
                for (auto& res : arch_data) {
                    auto& index_map = res.type->component_index_map;
                    auto& offsets = res.type->layout.component_offsets;

                    res.required_layout_offsets.reserve(components.size());
                    for (auto component : components)
                        res.required_layout_offsets.push_back(offsets[index_map.at(component)]);

                    res.required_clean_comp_indices.reserve(clean_components.size());
                    for (auto component : clean_components)
                        if (auto it = index_map.find(component); it != index_map.end())
                            res.required_clean_comp_indices.push_back(it->second);

                    res.required_dirty_comp_indices.reserve(dirty_components.size());
                    for (auto component : dirty_components)
                        if (auto it = index_map.find(component); it != index_map.end())
                            res.required_dirty_comp_indices.push_back(it->second);

                    res.make_dirty_comp_indices.reserve(mark_dirty_components.size());
                    for (auto component : mark_dirty_components)
                        if (auto it = index_map.find(component); it != index_map.end())
                            res.make_dirty_comp_indices.push_back(it->second);
                }
                with_changes = {with_changes_.begin(), with_changes_.end()};
            }

            // CRITICAL: after changing check mark_component_dirty for correctness
            std::pair<size_t, void**> next() {
                while (current_archetype_index < arch_data.size()) {
                    arch_data_t& adata = arch_data[current_archetype_index];
                    archetype* archetype = adata.type;
                    while (current_chunk_index < archetype->chunks.size()) {
                        std::unique_ptr<chunk>& chunk = archetype->chunks[current_chunk_index];

                        bool dirty_match = true;
                        if (!adata.required_dirty_comp_indices.empty()) {
                            dirty_match = false;
                            for (uint32_t comp_index : adata.required_dirty_comp_indices) {
                                auto flags = archetype->dirty_flags(chunk.get(), comp_index);
                                uint64_t combined_mask = 0;
                                for (const auto& flag_word : flags)
                                    combined_mask |= flag_word.load(std::memory_order_relaxed);

                                if (combined_mask != 0) {
                                    dirty_match = true;
                                    break;
                                }
                            }
                        }
                        if (!adata.required_clean_comp_indices.empty()) {
                            dirty_match = false;
                            for (uint32_t comp_index : adata.required_clean_comp_indices) {
                                auto flags = archetype->dirty_flags(chunk.get(), comp_index);
                                uint64_t combined_mask = 0;
                                for (const auto& flag_word : flags)
                                    combined_mask |= flag_word.load(std::memory_order_relaxed);

                                if (combined_mask == 0) {
                                    dirty_match = true;
                                    break;
                                }
                            }
                        }

                        if (dirty_match) {
                            for (uint32_t comp_index : adata.make_dirty_comp_indices)
                                archetype->mark_dirty_entities(chunk.get(), comp_index);

                            for (size_t i = 0; i < adata.required_layout_offsets.size(); i++)
                                component_arrays[i] = chunk->memory_block.get() + adata.required_layout_offsets[i];

                            ++current_chunk_index;
                            return {chunk->entity_count, component_arrays.data()};
                        }
                        ++current_chunk_index;
                    }
                    ++current_archetype_index;
                    current_chunk_index = 0;
                }

                return {0, nullptr};
            }

            bool is_end() const {
                return current_archetype_index >= arch_data.size();
            }

            // CRITICAL: this function depends on next() function, changes on using current_chunk_index
            //  would impact the current chunk retrial
            //  this function used for explicit dirty marking
            void mark_component_dirty(component_id component, size_t entity_index) {
                mark_component_dirty(current_archetype_index, current_chunk_index, component, entity_index);
            }

            bool is_entity_match(size_t entity_index) const {
                return is_entity_match(current_archetype_index, current_chunk_index, entity_index);
            }

            std::pair<int32_t, uint32_t> get_current_entity(size_t entity_index) {
                return get_current_entity(current_archetype_index, current_chunk_index, entity_index);
            }

            structural_changes get_component_change_state(size_t entity_index, component_id cid) {
                return get_component_change_state(current_archetype_index, current_chunk_index, entity_index, cid);
            }

            void mark_component_dirty(size_t archetype_index, size_t chunk_index, component_id component, size_t entity_index) {
                if (archetype_index >= arch_data.size())
                    return;

                archetype* archetype = arch_data[archetype_index].type;

                if (chunk_index == 0)
                    return;

                std::unique_ptr<chunk>& chunk = archetype->chunks[chunk_index - 1];

                auto it = archetype->component_index_map.find(component);
                if (it != archetype->component_index_map.end()) {
                    uint32_t component_index_in_archetype = it->second;
                    archetype->mark_dirty(chunk.get(), component_index_in_archetype, entity_index);
                }
            }

            bool is_entity_match(size_t archetype_index, size_t chunk_index, size_t entity_index) const {
                if (chunk_index == 0)
                    return false;

                auto& active_arch_data = arch_data[archetype_index];
                auto& active_chunk = active_arch_data.type->chunks[chunk_index - 1];
                if (active_arch_data.required_dirty_comp_indices.size()) {
                    for (uint32_t comp_index : active_arch_data.required_dirty_comp_indices)
                        if (!active_arch_data.type->is_dirty(active_chunk.get(), comp_index, entity_index))
                            return false;
                }

                if (with_changes.size()) {
                    auto id = active_chunk.get()->entities()[entity_index];
                    auto& record = management.records.at(id);
                    archetype* before_arch = record.archetype_before_mutation.load(std::memory_order_relaxed);
                    if (record.archetype_before_mutation.load(std::memory_order_relaxed) != nullptr) {
                        for (auto component_id_ : with_changes) {
                            bool existed_before = before_arch->component_index_map.contains(component_id_);
                            bool exists_after = record.type->component_index_map.contains(component_id_);

                            if (!existed_before && exists_after)
                                return true;
                            else if (existed_before && !exists_after)
                                return true;
                        }
                    }
                    return false;
                }
                return true;
            }

            std::pair<int32_t, uint32_t> get_current_entity(size_t archetype_index, size_t chunk_index, size_t entity_index) {
                if (chunk_index == 0)
                    return {0, UINT32_MAX};

                auto& active_arch_data = arch_data[archetype_index];
                auto& active_chunk = active_arch_data.type->chunks[chunk_index - 1];
                auto id = active_chunk.get()->entities()[entity_index];

                auto& record = management.records.at(id);
                return {id, record.generation};
            }

            structural_changes get_component_change_state(size_t archetype_index, size_t chunk_index, size_t entity_index, component_id cid) {
                auto& active_arch_data = arch_data[archetype_index];
                auto& active_chunk = active_arch_data.type->chunks[chunk_index - 1];
                auto id = active_chunk.get()->entities()[entity_index];

                auto& record = management.records.at(id);

                archetype* before_arch = record.archetype_before_mutation.load(std::memory_order_relaxed);
                if (before_arch == nullptr) {
                    auto it = active_arch_data.type->component_index_map.find(cid);
                    if (it != active_arch_data.type->component_index_map.end()) {
                        uint32_t component_index_in_archetype = it->second;
                        if (active_arch_data.type->is_dirty(active_chunk.get(), component_index_in_archetype, entity_index))
                            return structural_changes::modified;
                    }
                    return structural_changes::no_changes;
                }

                archetype* after_arch = record.type;

                bool existed_before = before_arch->component_index_map.contains(cid);
                bool exists_after = after_arch->component_index_map.contains(cid);

                if (!existed_before && exists_after)
                    return structural_changes::added;
                else if (existed_before && !exists_after) {
                    return structural_changes::removed;
                } else if (existed_before && exists_after) {
                    auto it = active_arch_data.type->component_index_map.find(cid);
                    if (it != active_arch_data.type->component_index_map.end()) {
                        uint32_t component_index_in_archetype = it->second;
                        if (active_arch_data.type->is_dirty(active_chunk.get(), component_index_in_archetype, entity_index))
                            return structural_changes::modified;
                    }
                    return structural_changes::no_changes;
                } else
                    return structural_changes::no_changes;
            }

            preserved_state preserve_state() const {
                return {current_archetype_index, current_chunk_index};
            }
        };

        struct iteration_handle::relational_iteration_data {
            fast_task::shared_lock<fast_task::task_rw_mutex> main_lock;

            struct item {
                std::vector<void*> component_pointers;
                archetype* source_archetype;
                entity e;
            };

            std::vector<component_id> auto_mark_dirty;

            std::vector<item> matched_entities;
            size_t current_index = 0;

            void calculate_data(
                std::span<component_id> components,
                std::span<component_id> with_components,
                std::span<component_id> without_components,
                std::span<component_id> writes_components,
                std::span<component_id> with_dirty_components,
                std::span<component_id> with_clean_components,
                std::span<component_id> with_changes,
                std::span<std::pair<component_id, entity>> relations,
                std::optional<int32_t> world_id = std::nullopt
            ) {
                fast_task::shared_lock guard(management.relation_mutex);
                std::vector<int32_t> candidate_ids = management.get_relation_query(relations);
                auto_mark_dirty = {writes_components.begin(), writes_components.end()};


                for (int32_t entity_id : candidate_ids) {
                    auto& record = management.records.at(entity_id);
                    if (world_id)
                        if (record.world_owner->id != *world_id)
                            continue;
                    archetype* arch = record.type;

                    if (!arch->matches_query(components, with_components, without_components))
                        continue;

                    bool dirty_check_passed = true;
                    for (auto cid : with_dirty_components) {
                        if (!arch->is_dirty(record.chunk, arch->component_index_map.at(cid), record.chunk_index)) {
                            dirty_check_passed = false;
                            break;
                        }
                    }
                    if (!dirty_check_passed)
                        continue;
                    bool clean_check_passed = true;
                    for (auto cid : with_clean_components) {
                        if (arch->is_dirty(record.chunk, arch->component_index_map.at(cid), record.chunk_index)) {
                            clean_check_passed = false;
                            break;
                        }
                    }
                    if (!clean_check_passed)
                        continue;

                    if (with_changes.size()) {
                        bool changes_check_passed = false;

                        archetype* before_arch = record.archetype_before_mutation.load(std::memory_order_relaxed);
                        if (record.archetype_before_mutation.load(std::memory_order_relaxed) != nullptr) {
                            for (auto component_id_ : with_changes) {
                                bool existed_before = before_arch->component_index_map.contains(component_id_);
                                bool exists_after = record.type->component_index_map.contains(component_id_);

                                if (!existed_before && exists_after) {
                                    changes_check_passed = true;
                                    break;
                                } else if (existed_before && !exists_after) {
                                    changes_check_passed = true;
                                    break;
                                }
                            }
                        }
                        if (!changes_check_passed)
                            continue;
                    }

                    item new_item;
                    new_item.e = {entity_id, record.generation};
                    new_item.source_archetype = arch;
                    new_item.component_pointers.reserve(components.size());

                    for (auto cid : components)
                        new_item.component_pointers.push_back(get_entity_component(entity_id, record.generation, cid));
                    matched_entities.push_back(std::move(new_item));
                }
            }

            std::pair<size_t, void**> next() {
                if (is_end())
                    return {0, nullptr};
                return {1, matched_entities[current_index++].component_pointers.data()};
            }

            bool is_end() const {
                return current_index >= matched_entities.size();
            }

            void mark_component_dirty(component_id component, size_t entity_index) {
                mark_component_dirty(0, current_index - 1, component, entity_index);
            }

            bool is_entity_match(size_t entity_index) const {
                return true;
            }

            std::pair<int32_t, uint32_t> get_current_entity(size_t entity_index) {
                return get_current_entity(0, current_index - 1, entity_index);
            }

            structural_changes get_component_change_state(size_t entity_index, component_id cid) {
                return get_component_change_state(0, current_index - 1, entity_index, cid);
            }

            void mark_component_dirty(size_t archetype_index, size_t chunk_index, component_id component, size_t entity_index) {
                archetype* archetype = matched_entities[chunk_index].source_archetype;
                auto chunk = management.records.at(matched_entities[chunk_index].e.id).chunk;
                auto it = archetype->component_index_map.find(component);
                if (it != archetype->component_index_map.end()) {
                    uint32_t component_index_in_archetype = it->second;
                    archetype->mark_dirty(chunk, component_index_in_archetype, entity_index);
                }
            }

            bool is_entity_match(size_t archetype_index, size_t chunk_index, size_t entity_index) const {
                return true;
            }

            std::pair<int32_t, uint32_t> get_current_entity(size_t archetype_index, size_t chunk_index, size_t entity_index) {
                auto& e = matched_entities[chunk_index].e;
                return {e.id, e.generation};
            }

            structural_changes get_component_change_state(size_t archetype_index, size_t chunk_index, size_t entity_index, component_id cid) {
                auto& record = management.records.at(matched_entities[chunk_index].e.id);
                auto& active_chunk = record.chunk;

                archetype* before_arch = record.archetype_before_mutation.load(std::memory_order_relaxed);
                if (before_arch == nullptr) {
                    auto it = record.type->component_index_map.find(cid);
                    if (it != record.type->component_index_map.end()) {
                        uint32_t component_index_in_archetype = it->second;
                        if (record.type->is_dirty(active_chunk, component_index_in_archetype, entity_index))
                            return structural_changes::modified;
                    }
                    return structural_changes::no_changes;
                }

                archetype* after_arch = record.type;

                bool existed_before = before_arch->component_index_map.contains(cid);
                bool exists_after = after_arch->component_index_map.contains(cid);

                if (!existed_before && exists_after)
                    return structural_changes::added;
                else if (existed_before && !exists_after) {
                    return structural_changes::removed;
                } else if (existed_before && exists_after) {
                    auto it = record.type->component_index_map.find(cid);
                    if (it != record.type->component_index_map.end()) {
                        uint32_t component_index_in_archetype = it->second;
                        if (record.type->is_dirty(active_chunk, component_index_in_archetype, entity_index))
                            return structural_changes::modified;
                    }
                    return structural_changes::no_changes;
                } else
                    return structural_changes::no_changes;
            }

            preserved_state preserve_state() const {
                return {0, current_index - 1};
            }
        };

        iteration_handle::iteration_handle(iteration_handle&& other) noexcept {
            data = other.data;
            other.data = nullptr;
        }

        iteration_handle::~iteration_handle() {
            std::visit(
                []<class T>(T& ptr) {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        delete ptr;
                },
                data
            );
        }

        std::pair<size_t, void**> iteration_handle::next() {
            return std::visit(
                []<class T>(T& ptr) -> std::pair<size_t, void**> {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->next();
                    else
                        return {0, nullptr};
                },
                data
            );
        }

        bool iteration_handle::is_end() const {
            return std::visit(
                []<class T>(const T& ptr) -> bool {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->is_end();
                    else
                        return true;
                },
                data
            );
        }

        void iteration_handle::mark_component_dirty(component_id component, size_t index) {
            std::visit(
                [&]<class T>(T& ptr) {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->mark_component_dirty(component, index);
                },
                data
            );
        }

        bool iteration_handle::is_entity_match(size_t entity_index) const {
            return std::visit(
                [entity_index]<class T>(const T& ptr) -> bool {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->is_entity_match(entity_index);
                    else
                        return false;
                },
                data
            );
        }

        std::pair<int32_t, uint32_t> iteration_handle::get_current_entity(size_t entity_index) {
            return std::visit(
                [entity_index]<class T>(T& ptr) -> std::pair<int32_t, uint32_t> {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->get_current_entity(entity_index);
                    else
                        return {0, -1};
                },
                data
            );
        }

        structural_changes iteration_handle::get_component_change_state(size_t entity_index, component_id cid) {
            return std::visit(
                [entity_index, cid]<class T>(T& ptr) -> structural_changes {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->get_component_change_state(entity_index, cid);
                    else
                        return structural_changes::no_changes;
                },
                data
            );
        }

        void iteration_handle::preserved_state::mark_component_dirty(iteration_handle& handle, component_id cid, size_t index) {
            std::visit(
                [this, cid, index]<class T>(T& ptr) {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        ptr->mark_component_dirty(archetype_index, chunk_index, cid, index);
                },
                handle.data
            );
        }

        bool iteration_handle::preserved_state::is_entity_match(iteration_handle& handle, size_t current_index_in_chunk) const {
            return std::visit(
                [this, current_index_in_chunk]<class T>(T& ptr) -> bool {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->is_entity_match(archetype_index, chunk_index, current_index_in_chunk);
                    else
                        return false;
                },
                handle.data
            );
        }

        std::pair<int32_t, uint32_t> iteration_handle::preserved_state::get_current_entity(iteration_handle& handle, size_t current_index_in_chunk) {
            return std::visit(
                [this, current_index_in_chunk]<class T>(T& ptr) -> std::pair<int32_t, uint32_t> {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->get_current_entity(archetype_index, chunk_index, current_index_in_chunk);
                    else
                        return {0, -1};
                },
                handle.data
            );
        }

        structural_changes iteration_handle::preserved_state::get_component_change_state(iteration_handle& handle, size_t entity_index, component_id cid) {
            return std::visit(
                [this, entity_index, cid]<class T>(T& ptr) -> structural_changes {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->get_component_change_state(archetype_index, chunk_index, entity_index, cid);
                    else
                        return structural_changes::no_changes;
                },
                handle.data
            );
        }

        iteration_handle::preserved_state iteration_handle::preserve_state() {
            return std::visit(
                []<class T>(T& ptr) -> iteration_handle::preserved_state {
                    if constexpr (!std::is_same_v<T, std::nullptr_t>)
                        return ptr->preserve_state();
                    else
                        return {0, 0};
                },
                data
            );
        }

        entity iterator_view::current_entity() {
            auto it = handle.get_current_entity(index);
            return {it.first, it.second};
        }

        entity iterator_view_chunk::current_entity(size_t index) {
            auto it = handle.get_current_entity(index);
            return {it.first, it.second};
        }

        entity iterator_view_chunk_parallel::current_entity(size_t index) {
            auto it = state.get_current_entity(handle, index);
            return {it.first, it.second};
        }

        iteration_handle iterate_components(
            int32_t world_id,
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<component_id> writes_components,
            std::span<component_id> with_dirty_components,
            std::span<component_id> with_clean_components,
            std::span<component_id> with_changes,
            std::span<std::pair<component_id, entity>> with_relation
        ) {
            iteration_handle handle;
            if (with_relation.size()) {
                auto data = std::make_unique<iteration_handle::relational_iteration_data>(management.manager_mutex);
                data->calculate_data(components, with_components, without_components, writes_components, with_dirty_components, with_clean_components, with_changes, with_relation, world_id);
                handle.data = data.release();
            } else {
                auto data = std::make_unique<iteration_handle::iteration_data>(management.manager_mutex);

                if (auto it = management.worlds.find(world_id); it != management.worlds.end())
                    for (const auto& archetype_ptr : it->second.archetypes)
                        if (archetype_ptr->matches_query(components, with_components, without_components))
                            data->arch_data.push_back(archetype_ptr.get());

                data->calculate_data(components, with_clean_components, with_dirty_components, writes_components, with_changes);
                data->component_arrays.resize(components.size());
                handle.data = data.release();
            }
            return handle;
        }

        iteration_handle iterate_components_global(
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<component_id> writes_components,
            std::span<component_id> with_dirty_components,
            std::span<component_id> with_clean_components,
            std::span<component_id> with_changes,
            std::span<std::pair<component_id, entity>> with_relation
        ) {
            iteration_handle handle;

            if (with_relation.size()) {
                auto data = std::make_unique<iteration_handle::relational_iteration_data>(management.manager_mutex);
                data->calculate_data(components, with_components, without_components, writes_components, with_dirty_components, with_clean_components, with_changes, with_relation);
                handle.data = data.release();
            } else {
                auto data = std::make_unique<iteration_handle::iteration_data>(management.manager_mutex);

                for (const auto& archetype_ptr : management.limbo.archetypes)
                    if (archetype_ptr->matches_query(components, with_components, without_components))
                        data->arch_data.push_back(archetype_ptr.get());

                for (const auto& [id, world] : management.worlds)
                    for (const auto& archetype_ptr : world.archetypes)
                        if (archetype_ptr->matches_query(components, with_components, without_components))
                            data->arch_data.push_back(archetype_ptr.get());


                data->calculate_data(components, with_clean_components, with_dirty_components, writes_components, with_changes);
                data->component_arrays.resize(components.size());
                handle.data = data.release();
            }
            return handle;
        }
    }

    entity_recipe& entity_recipe::freeze() {
        if (is_frozen_)
            return *this;
        is_frozen_ = true;
        std::sort(component_ids.begin(), component_ids.end());
        component_ids.erase(
            std::unique(component_ids.begin(), component_ids.end()),
            component_ids.end()
        );
        hash = archetype_hash{}(component_ids);
        return *this;
    }

    const std::vector<component_id>& entity_recipe::get_ids() const {
        return component_ids;
    }

    size_t entity_recipe::get_hash() const {
        assert(is_frozen_ && "Cannot get hash from an unfrozen recipe!");
        return hash;
    }

    std::optional<entity> entity::copy_and_wait() const {
        return detail::copy_entity(std::nullopt, *this)->take();
    }

    entity entity_ref::get_entity() {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std ::decay_t<decltype(it)>, base_objects::uuid>) {
                    auto res = api::entity_id_map::get_entity(it);
                    if (!res)
                        throw std::runtime_error("The entity is not loaded");
                    value = *res;
                    return *res;
                } else
                    return it;
            },
            value
        );
    }

    base_objects::uuid entity_ref::get_uuid() {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std ::decay_t<decltype(it)>, base_objects::uuid>) {
                    return it;
                } else
                    return it.get<com::uuid>().id;
            },
            value
        );
    }

    bool entity_ref::is_resolved() {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std ::decay_t<decltype(it)>, base_objects::uuid>) {
                    auto res = api::entity_id_map::get_entity(it);
                    if (!res)
                        return false;
                    value = *res;
                    return true;
                } else
                    return true;
            },
            value
        );
    }
}
