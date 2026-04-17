/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/unordered/unordered_flat_map.hpp>
#include <src/api/bin/ecs/deletion_system.hpp>
#include <src/api/bin/ecs/manager.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/ecs/entity_definition.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/log.hpp>
#include <stacktrace>

namespace copper_server::api::ecs {
    void relation_visitor::context_t::make_unlink(ecs::entity self, ecs::entity target_holder) const {
        on_unlink(component, self, target_holder);
    }

    relation_visitor::relation_visitor(std::move_only_function<void(ecs::entity target, relation_type type, context_t& context)>&& callback)
        : callback(std::move(callback)) {}

    void relation_visitor::push(entity e, relation_type type) {
        callback(e, type, context);
    }

    struct entity_allocation_request {
        const entity_recipe& recipe;
        std::optional<world*> world_id;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        entity result;
        bool ready = false;
    };

    struct entity_creation_request {
        std::unique_ptr<detail::components_holder> components;
        std::optional<world*> world_id;
        entity_recipe calculated_recipe;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        entity result;
        bool ready = false;
    };

    struct entity_copy_request {
        entity other_entity;
        std::optional<world*> world_id;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        std::optional<entity> result;
        std::exception_ptr ex;
        bool ready = false;
    };

    struct entity_transfer_request {
        uint32_t id;
        uint32_t generation;
        std::optional<world*> world_id;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        bool ready = false;
        bool success = false;
    };

    moodycamel::ConcurrentQueue<entity_allocation_request*> allocation_queue;
    moodycamel::ConcurrentQueue<entity_creation_request*> creation_queue;
    moodycamel::ConcurrentQueue<entity_copy_request*> copy_queue;
    moodycamel::ConcurrentQueue<entity_transfer_request*> transfer_queue;

    world_local_registry::world_local_registry(int32_t id) : world_ptr(manager::instance().enable_world(id)) {
    }

    world_local_registry::~world_local_registry() {
        manager::instance().disable_world(world_ptr->id);
    }

    fast_task::future_ptr<bool> world_local_registry::register_entity_async(entity& entity) {
        auto request = std::make_unique<entity_transfer_request>(entity.id, entity.generation, world_ptr);
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
        auto& man = manager::instance();
        auto& record = man.records.at(entity.id);
        if (record.world_owner == world_ptr)
            return fast_task::future<bool>::make_ready(true);
        return register_entity_async(entity);
    }

    fast_task::future_ptr<entity> world_local_registry::allocate_entity_async(const entity_recipe& recipe) {
        if (!recipe.is_frozen())
            throw std::runtime_error("The recipe should be frozen before creating an entity!");
        auto request = std::make_unique<entity_allocation_request>(recipe, world_ptr);
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
        auto& man = manager::instance();
        auto& record = man.records.at(entity.id);
        if (record.world_owner == world_ptr)
            return true;

        return register_entity_async(entity).get();
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
        size_t in_degree;

        system_node(std::unique_ptr<system_interface> instance, const detail::system_info& info, size_t in_degree = 0)
            : instance(std::move(instance)), info(info), in_degree(in_degree) {}

        system_node(system_node&&) = default;
        system_node& operator=(system_node&&) = default;
        system_node(const system_node&) = delete;
        system_node& operator=(const system_node&) = delete;
    };

    struct scheduler::scheduler_data {

        struct tick_group {
            std::vector<system_node> systems;
            std::unordered_map<size_t, std::vector<size_t>> dependency_graph;
            bool graph_is_dirty = false;

            tick_group() = default;
            tick_group(tick_group&&) = default;
            tick_group& operator=(tick_group&&) = default;
            tick_group(const tick_group&) = delete;
            tick_group& operator=(const tick_group&) = delete;

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

        fast_task::task_rw_mutex groups_mutex;
        boost::unordered_flat_map<tick_phase, tick_group> groups;
    };

    scheduler::scheduler() : data(std::make_unique<scheduler_data>()) {
        add_system<deletion_system>(tick_phase::early_processing);
    }

    scheduler::~scheduler() {}

    namespace mutation_processing {
        struct prepared_move_op {
            uint32_t entity_id;
            archetype* from_archetype;
            archetype* to_archetype;
            std::unordered_map<component_id, std::vector<char>> data; // Owns the new component data
        };

        struct prepared_in_place_update_op {
            uint32_t entity_id;
            std::unordered_map<component_id, std::vector<char>> data;
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
            std::vector<prepared_move_op> prepared_moves;
            std::vector<prepared_in_place_update_op> prepared_updates;
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

        void process_destruction_queue(world& w) {
            std::vector<entity_destroy_queue_item> items = collect_all_(w.entity_destroy_queue);
            if (items.empty())
                return;

            std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
                return a.id < b.id;
            });
            auto last = std::unique(items.begin(), items.end(), [](const auto& a, const auto& b) {
                return a.id == b.id && a.generation == b.generation;
            });
            items.erase(last, items.end());

            std::unordered_map<archetype*, std::vector<uint32_t>> groups;
            auto& man = manager::instance();

            for (const auto& item : items) {
                if (man.has_entity(item.id, item.generation)) {
                    auto& record = man.records.at(item.id);
                    groups[record.type].push_back(item.id);
                }
            }
            fast_task::future_tool::for_each_move(std::move(groups), [](std::pair<archetype*, std::vector<uint32_t>>&& group) {
                auto& man = manager::instance();
                for (auto& it : group.second)
                    man.deallocate_entity(it);
            });
        }

        std::vector<std::vector<prepared_move_op>> distribute_disjoint_moves(std::vector<prepared_move_op>&& all_moves) {
            std::vector<std::vector<prepared_move_op>> batches;
            std::vector<std::unordered_set<archetype*>> batch_locks;

            batches.reserve(16);
            batch_locks.reserve(16);

            for (auto& move : all_moves) {
                bool placed = false;

                for (size_t i = 0; i < batches.size(); ++i) {
                    auto& locks = batch_locks[i];

                    if (!locks.contains(move.from_archetype) && !locks.contains(move.to_archetype)) {
                        locks.insert(move.from_archetype);
                        locks.insert(move.to_archetype);

                        batches[i].push_back(std::move(move));
                        placed = true;
                        break;
                    }
                }

                if (!placed) {
                    batch_locks.emplace_back();
                    batch_locks.back().insert(move.from_archetype);
                    batch_locks.back().insert(move.to_archetype);

                    batches.emplace_back();
                    batches.back().push_back(std::move(move));
                }
            }

            return batches;
        }

        std::vector<std::vector<parallel_transfer_op>> distribute_disjoint_transfers(std::vector<parallel_transfer_op>&& all_transfers) {
            std::vector<std::vector<parallel_transfer_op>> batches;
            std::vector<std::unordered_set<archetype*>> batch_locks;

            batches.reserve(16);
            batch_locks.reserve(16);

            for (auto& transfer : all_transfers) {
                bool placed = false;
                auto source_archetype = manager::instance().records[transfer.request->id].type;

                for (size_t i = 0; i < batches.size(); ++i) {
                    auto& locks = batch_locks[i];

                    if (!locks.contains(transfer.target_archetype) && !locks.contains(source_archetype)) {
                        locks.insert(transfer.target_archetype);
                        locks.insert(source_archetype);

                        batches[i].push_back(std::move(transfer));
                        placed = true;
                        break;
                    }
                }

                if (!placed) {
                    batch_locks.emplace_back();
                    batch_locks.back().insert(transfer.target_archetype);
                    batch_locks.back().insert(source_archetype);

                    batches.emplace_back();
                    batches.back().push_back(std::move(transfer));
                }
            }

            return batches;
        }

        grouped_mutation_ops prepare_mutations(world& w) {
            grouped_mutation_ops res;
            std::unordered_map<uint32_t, prepared_move_op> combined_changes;
            consume_all_(w.mutation_queue, [&](detail::mutation_queue_item item) {
                if (!manager::instance().has_entity(item.entity_id, item.generation))
                    return;

                auto& record = manager::instance().records.at(item.entity_id);
                archetype* from_archetype = record.type;
                world* in_world = record.world_owner;

                if (auto current_op = combined_changes.find(item.entity_id); combined_changes.end() != current_op) {
                    auto& op = current_op->second;
                    if (!item.remove && op.to_archetype->component_index_map.contains(item.component)) {
                        if (auto comp = op.data.find(item.component); op.data.end() != comp) {
                            detail::component_info_registry.at(item.component).destroy(comp->second.data());
                            comp->second = std::move(item.data);
                        } else
                            op.data.emplace(item.component, std::move(item.data));
                    } else {
                        archetype* to_archetype = nullptr;
                        if (item.remove) {
                            if (auto it = op.to_archetype->remove_transition_cache.find(item.component); it == op.to_archetype->remove_transition_cache.end())
                                to_archetype = in_world->map_new_archetype_without(op.to_archetype, item.component);
                        } else if (auto it = op.to_archetype->add_transition_cache.find(item.component); it == op.to_archetype->add_transition_cache.end())
                            to_archetype = in_world->map_new_archetype_with(op.to_archetype, item.component);

                        prepared_move_op& update = combined_changes[item.entity_id];
                        update.to_archetype = to_archetype;
                        update.from_archetype = from_archetype;

                        if (item.remove) {
                            if (auto comp = op.data.find(item.component); op.data.end() != comp)
                                detail::component_info_registry.at(item.component).destroy(comp->second.data());
                        } else {
                            if (auto comp = op.data.find(item.component); op.data.end() != comp) {
                                detail::component_info_registry.at(item.component).destroy(comp->second.data());
                                comp->second = std::move(item.data);
                            } else
                                op.data.emplace(item.component, std::move(item.data));
                        }
                    }
                } else {
                    if (!item.remove && from_archetype->component_index_map.contains(item.component)) {
                        prepared_move_op& update = combined_changes[item.entity_id];
                        update.entity_id = item.entity_id;
                        update.to_archetype = from_archetype;
                        update.from_archetype = from_archetype;
                        update.data.emplace(item.component, std::move(item.data));
                    } else {
                        archetype* to_archetype = nullptr;
                        if (item.remove) {
                            if (auto it = from_archetype->remove_transition_cache.find(item.component); it == from_archetype->remove_transition_cache.end())
                                to_archetype = in_world->map_new_archetype_without(from_archetype, item.component);
                        } else if (auto it = from_archetype->add_transition_cache.find(item.component); it == from_archetype->add_transition_cache.end())
                            to_archetype = in_world->map_new_archetype_with(from_archetype, item.component);

                        prepared_move_op& update = combined_changes[item.entity_id];
                        update.entity_id = item.entity_id;
                        update.to_archetype = to_archetype;
                        update.from_archetype = from_archetype;
                        if (!item.remove)
                            update.data.emplace(item.component, std::move(item.data));
                    }
                }
            });

            size_t in_place_count = 0;
            for (auto& [e, data] : combined_changes)
                in_place_count += data.to_archetype == data.from_archetype;

            res.prepared_moves.reserve(combined_changes.size() - in_place_count);
            res.prepared_updates.reserve(in_place_count);
            for (auto& [e, data] : combined_changes)
                if (data.to_archetype == data.from_archetype)
                    res.prepared_updates.emplace_back(data.entity_id, std::move(data.data));
                else
                    res.prepared_moves.emplace_back(std::move(data));
            return res;
        }

        std::vector<parallel_allocation_op> prepare_allocation_requests() {
            std::vector<parallel_allocation_op> res;
            res.reserve(allocation_queue.size_approx());
            consume_all_(allocation_queue, [&](entity_allocation_request* item) {
                if (item->world_id)
                    res.emplace_back(item, item->world_id.value()->map_get_archetype(item->recipe.get_ids()), *item->world_id);
                else
                    res.emplace_back(item, manager::instance().limbo.map_get_archetype(item->recipe.get_ids()), &manager::instance().limbo);
            });
            return res;
        }

        std::vector<parallel_creation_op> prepare_creation_requests() {
            std::vector<parallel_creation_op> res;
            res.reserve(creation_queue.size_approx());
            consume_all_(creation_queue, [&](entity_creation_request* item) {
                if (item->world_id)
                    res.emplace_back(item, item->world_id.value()->map_get_archetype(item->calculated_recipe.get_ids()), *item->world_id);
                else
                    res.emplace_back(item, manager::instance().limbo.map_get_archetype(item->calculated_recipe.get_ids()), &manager::instance().limbo);
            });
            return res;
        }

        std::vector<parallel_copy_op> prepare_copy_requests() {
            std::vector<parallel_copy_op> res;
            res.reserve(copy_queue.size_approx());
            consume_all_(copy_queue, [&](entity_copy_request* item) {
                res.emplace_back(item, manager::instance().records.at(item->other_entity.id).type);
            });
            return res;
        }

        void execute_mutations(grouped_mutation_ops mutations, ::moodycamel::ConcurrentQueue<uint32_t>& arch_type_changes) {
            auto assign_ops = fast_task::future_tool::for_each_move(mutations.prepared_updates, [](prepared_in_place_update_op&& update) {
                auto& record = manager::instance().records.at(update.entity_id);
                for (auto& [comp_id, data] : update.data) {
                    auto& component_info = detail::component_info_registry.at(comp_id);
                    auto component_index = record.type->component_index_map.at(comp_id);
                    void* dest_ptr = record.chunk->memory_block.get() + record.type->layout.component_offsets[component_index] + (record.chunk_index * component_info.size);

                    component_info.destroy(dest_ptr);
                    component_info.move_construct(dest_ptr, (void*)data.data());
                    record.type->mark_dirty(record.chunk, component_index, record.chunk_index);
                }
            });


            for (auto& batch : distribute_disjoint_moves(std::move(mutations.prepared_moves))) {
                std::sort(batch.begin(), batch.end(), [](const prepared_move_op& a, const prepared_move_op& b) {
                    if (a.from_archetype != b.from_archetype)
                        return a.from_archetype < b.from_archetype;
                    return a.to_archetype < b.to_archetype;
                });
                fast_task::future_tool::for_each_move(batch, [&](prepared_move_op&& move) {
                    auto& record = manager::instance().records[move.entity_id];
                    archetype* expected = nullptr;
                    if (record.archetype_before_mutation.compare_exchange_strong(expected, record.type, std::memory_order_relaxed))
                        arch_type_changes.enqueue(move.entity_id);

                    manager::instance().move_entity(move.entity_id, move.to_archetype, record.world_owner);

                    for (auto& [comp_id, data] : move.data) {
                        auto& component_info = detail::component_info_registry.at(comp_id);
                        auto component_index = record.type->component_index_map.at(comp_id);
                        void* dest_ptr = record.chunk->memory_block.get() + record.type->layout.component_offsets[component_index] + (record.chunk_index * component_info.size);

                        component_info.move(dest_ptr, (void*)data.data());
                        record.type->mark_dirty(record.chunk, component_index, record.chunk_index);
                    }
                })->wait();
            }
            assign_ops->wait();
        }

        void process_transfers() {
            std::vector<parallel_transfer_op> res;
            res.reserve(transfer_queue.size_approx());
            consume_all_(transfer_queue, [&res](entity_transfer_request* transfer) {
                if (manager::instance().records.size() > transfer->id) {
                    auto& record = manager::instance().records[transfer->id];
                    if (record.generation == transfer->generation) {

                        if (transfer->world_id)
                            res.emplace_back(transfer, transfer->world_id.value()->map_get_archetype(record.type->whole_component_ids), *transfer->world_id);
                        else
                            res.emplace_back(transfer, manager::instance().limbo.map_get_archetype(record.type->whole_component_ids), &manager::instance().limbo);
                    }
                }
            });


            fast_task::future_tool::for_each_move(distribute_disjoint_transfers(std::move(res)), [](std::vector<parallel_transfer_op>&& transfers) {
                for (auto& transfer : transfers) {
                    fast_task::unique_lock lock(transfer.request->mut);
                    transfer.request->success = false;

                    manager::instance().move_entity(transfer.request->id, transfer.target_archetype, transfer.w);

                    transfer.request->ready = true;
                    transfer.request->cv.notify_one();
                }
            })->wait();
        }

        void process_dirty_marking(world& w) {
            parallel_drain(w.marking_queue, [](entity_dirty_mark_item&& mark) {
                auto& record = manager::instance().records.at(mark.id);
                if (record.generation != mark.generation)
                    if (auto component_index = record.type->component_index_map.find(mark.component); component_index != record.type->component_index_map.end())
                        record.type->mark_dirty(record.chunk, component_index->second, record.chunk_index);
            });
        }

        void process_allocation(std::vector<parallel_allocation_op>&& creation_requests, fast_task::unique_lock<fast_task::task_rw_mutex>& world_guard) {
            fast_task::relock_guard relock(world_guard);
            fast_task::future_tool::for_each_move(std::move(creation_requests), [&](parallel_allocation_op&& reg) {
                auto [item, arch, world] = reg;
                {
                    fast_task::lock_guard arch_lock(arch->arch_mutex);
                    item->result = manager::instance().allocate_entity(arch, world);
                }
                fast_task::lock_guard message_lock(item->mut);

                const auto& defaults = item->recipe.get_defaults();
                if (!defaults.empty()) {
                    auto& record = manager::instance().records.at(item->result.id);

                    for (auto& [id, src_ptr] : defaults) {
                        auto component_index = arch->component_index_map.at(id);
                        const auto& info = detail::component_info_registry.at(id);

                        size_t offset = arch->layout.component_offsets[component_index];
                        void* dest_ptr = record.chunk->memory_block.get() + offset + (record.chunk_index * info.size);

                        info.copy_assign(dest_ptr, src_ptr);
                        arch->mark_dirty(record.chunk, component_index, record.chunk_index);
                    }
                }

                item->ready = true;
                item->cv.notify_one();
            })->wait();
        }

        void process_creation(std::vector<parallel_creation_op>&& creation_requests, fast_task::unique_lock<fast_task::task_rw_mutex>& world_guard) {
            fast_task::relock_guard relock(world_guard);
            fast_task::future_tool::for_each_move(std::move(creation_requests), [&](parallel_creation_op&& reg) {
                auto [item, arch, world] = reg;
                {
                    fast_task::lock_guard arch_lock(arch->arch_mutex);
                    item->result = manager::instance().allocate_entity(arch, world);
                }
                fast_task::lock_guard message_lock(item->mut);

                const auto& defaults = item->calculated_recipe.get_defaults();
                if (!defaults.empty()) {
                    auto& record = manager::instance().records.at(item->result.id);

                    for (auto& [id, src_ptr] : defaults) {
                        auto component_index = arch->component_index_map.at(id);
                        const auto& info = detail::component_info_registry.at(id);

                        size_t offset = arch->layout.component_offsets[component_index];
                        void* dest_ptr = record.chunk->memory_block.get() + offset + (record.chunk_index * info.size);

                        info.copy_assign(dest_ptr, src_ptr);
                        arch->mark_dirty(record.chunk, component_index, record.chunk_index);
                    }
                }

                auto& record = manager::instance().records.at(item->result.id);
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
                if (!arch) {
                    fast_task::lock_guard message_lock(item->mut);
                    item->ready = true;
                    item->cv.notify_one();
                    return;
                }
                auto& other_record = manager::instance().records.at(item->other_entity.id);
                {
                    fast_task::lock_guard arch_lock(arch->arch_mutex);
                    item->result = manager::instance().allocate_entity(arch, other_record.world_owner);
                }
                fast_task::lock_guard message_lock(item->mut);
                auto& record = manager::instance().records.at(item->result->id);
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

        void proceed_mutations(fast_task::unique_lock<fast_task::task_rw_mutex>& world_guard) {
            process_transfers();
            process_allocation(prepare_allocation_requests(), world_guard);
            process_creation(prepare_creation_requests(), world_guard);
            process_copy(prepare_copy_requests());
        }

        void proceed_mutations(world& w) {
            process_destruction_queue(w);
            execute_mutations(prepare_mutations(w), w.arch_type_changes);
            process_dirty_marking(w);
        }
    }

    void global_registry::global_tick() {
        fast_task::unique_lock world_guard(manager::instance().manager_mutex);
        mutation_processing::proceed_mutations(world_guard);
    }

    void scheduler::execute_frame(world_local_registry& registry, tick_phase phase) {
        fast_task::unique_lock data_world_guard(data->groups_mutex);
        auto& current_group = data->groups[phase];
        data_world_guard.unlock();

        world& current_world = *registry.get_ecs_world_ref();
        mutation_processing::proceed_mutations(*registry.get_ecs_world_ref());


        if (current_group.graph_is_dirty)
            current_group.build_tree();
        {
            data_world_guard.lock();
            current_group.proceed_tree(registry);
            data_world_guard.unlock();
        }

        fast_task::future_tool::for_each_wait(current_world.archetypes, [](const std::unique_ptr<archetype>& archetype_ptr) {
            for (std::unique_ptr<chunk>& ch : archetype_ptr->chunks)
                for (auto& flags : archetype_ptr->dirty_flags(ch.get()))
                    for (auto& flag_word : flags)
                        flag_word.store(0, std::memory_order_relaxed);
        });

        mutation_processing::parallel_drain(current_world.arch_type_changes, [](uint32_t id) {
            manager::instance().records[id].archetype_before_mutation = nullptr;
        });
    }

    void scheduler::add_system_impl(std::unique_ptr<system_interface> system, const detail::system_info& info, tick_phase phase) {
        fast_task::unique_lock data_world_guard(data->groups_mutex);
        auto& current_group = data->groups[phase];
        current_group.systems.push_back(system_node(std::move(system), info));
        current_group.graph_is_dirty = true;
    }

    namespace detail {
        void report_fault_destruction(const std::type_info& info) {
            api::log::error("ecs", "Unrecognized exception while trying to destruct the ecs component: " + std::string(info.name()) + "\nStacktrace: \n\t" + std::to_string(std::stacktrace::current()));
        }

        void report_fault_destruction(const std::exception& ex, const std::type_info& info) {
            api::log::error("ecs", "Caught c++ exception while trying to destruct the ecs component: " + std::string(info.name()) + "\n" + ex.what() + "\nStacktrace: \n\t" + std::to_string(std::stacktrace::current()));
        }

        std::atomic<component_id> next_component_id = 0;
        std::vector<component_type_info> component_info_registry;
        fast_task::mutex registry_mutex;

        world* get_queues_for_entity(uint32_t id, uint32_t generation) {
            if (!manager::instance().has_entity(id, generation))
                return nullptr;
            auto& record = manager::instance().records.at(id);
            return record.world_owner;
        }

        void* get_entity_component(uint32_t id, uint32_t generation, component_id component_id) {
            auto& record = manager::instance().records.at(id);
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
            auto world = get_queues_for_entity(command.entity_id, command.generation);
            if (!world)
                return;
            else if (!world->mutation_queue.enqueue(std::move(command)))
                throw std::bad_alloc();
        }

        void queue_remove_entity_component(uint32_t id, uint32_t generation, component_id component_id) {
            auto check_comp = static_cast<const com::type_definition*>(detail::get_entity_component(id, generation, detail::get_component_id<com::type_definition>()));
            if (check_comp) {
                switch (check_comp->type->get_remove_action(component_id)) {
                case entity_definition::component_remove_act::optional:
                    break;
                case entity_definition::component_remove_act::locked:
                    throw std::runtime_error("Removing this component is not allowed");
                case entity_definition::component_remove_act::reset_on_remove: {
                    auto& defaults = check_comp->type->get_recipe().get_defaults();
                    auto component = get_entity_component(id, generation, component_id);
                    if (component) {
                        if (auto it = defaults.find(component_id); defaults.end() != it) {
                            auto& info = component_info_registry.at(component_id);
                            info.copy_assign(component, it->second);
                        } else
                            component_info_registry.at(component_id).reset(component);
                        queue_mark_dirty(id, generation, component_id);
                    } else {
                        auto& info = component_info_registry.at(component_id);
                        mutation_queue_item queue{id, generation, component_id};
                        queue.data.resize(info.size);
                        info.construct(queue.data.data());
                        if (auto it = defaults.find(component_id); defaults.end() != it)
                            info.copy_assign(component, it->second);
                        queue_command(std::move(queue));
                    }
                    return;
                }
                }
            }
            queue_command(mutation_queue_item{id, generation, component_id, true});
        }

        void queue_destroy_entity(uint32_t id, uint32_t generation) {
            auto world = get_queues_for_entity(id, generation);
            if (!world)
                return;
            else if (!world->entity_destroy_queue.enqueue(entity_destroy_queue_item{id, generation}))
                throw std::bad_alloc();
        }

        void queue_mark_dirty(uint32_t id, uint32_t generation, component_id component_id) {
            auto world = get_queues_for_entity(id, generation);
            if (!world)
                return;
            else if (!world->marking_queue.enqueue(entity_dirty_mark_item{id, generation, component_id}))
                throw std::bad_alloc();
        }

        bool has_entity_component(uint32_t id, uint32_t generation, whole_component_id component_id) {
            auto& record = manager::instance().records.at(id);
            if (record.generation == generation)
                return record.type->whole_component_presence_helper.contains(component_id);
            else
                return false;
        }

        bool is_valid(uint32_t id, uint32_t generation) {
            auto& records = manager::instance().records;
            if (records.size() <= id)
                return false;
            return records.at(id).generation == generation;
        }

        std::optional<int32_t> get_entity_assigned_to_world(uint32_t id, uint32_t generation) {
            auto& record = manager::instance().records.at(id);
            if (record.generation == generation && record.world_owner != &manager::instance().limbo)
                return record.world_owner->id;
            else
                return std::nullopt;
        }

        void request_all_childs(uint32_t id, uint32_t generation, relation_visitor& visitor) {
            auto& record = manager::instance().records.at(id);
            if (record.generation == generation && record.world_owner != &manager::instance().limbo)
                record.type->request_all_relations(record, visitor);
        }

        size_t get_entity_archetype_id(uint32_t id, uint32_t generation) { //this function returns pointer as integer instead of hash, because the hash could collide with other types. better be safe than oops
            auto& record = manager::instance().records.at(id);
            if (record.generation == generation)
                return std::bit_cast<size_t>(record.type);
            else
                return 0;
        }

        archetype_layout get_archetype_layout(uint32_t id, uint32_t generation) {
            auto& record = manager::instance().records.at(id);
            if (record.generation == generation)
                return {record.type->component_ids, record.type->component_index_map, record.type->layout.component_offsets};
            else
                return {};
        }

        void* get_entity_component_by_offset(uint32_t id, uint32_t generation, size_t offset, size_t type_size) {
            auto& record = manager::instance().records.at(id);
            if (record.generation != generation)
                return nullptr;
            return record.chunk->memory_block.get() + offset + (record.chunk_index * type_size);
        }

        fast_task::future_ptr<entity> create_entity(std::optional<world*> world_id, std::unique_ptr<components_holder> components) {
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

        fast_task::future_ptr<entity> create_entity(std::optional<world*> world_id, const api::ecs::entity_recipe& base_recipe, std::unique_ptr<components_holder> components) {
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

        fast_task::future_ptr<std::optional<entity>> copy_entity(std::optional<world*> w, const api::ecs::entity& base_entity) {
            auto request = std::make_unique<entity_copy_request>(base_entity, w);

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

        entity load_ecs_entity(const std::string& named_id, util::nbt_read_stream& stream, std::optional<world*> world_id) {
            return get_entity_definition(named_id).from_nbt(stream, world_id);
        }

        void store_ecs_entity(const std::string& named_id, util::nbt_write_stream& stream, entity ee) {
            get_entity_definition(named_id).to_nbt(stream, ee);
        }

        struct iteration_handle::iteration_data {
            fast_task::shared_lock<fast_task::task_rw_mutex> main_lock;
            size_t current_archetype_index = 0;
            size_t current_chunk_index = 0;
            std::vector<void*> component_arrays;

            iteration_data(fast_task::task_rw_mutex& init) : main_lock(init) {
            }

            iteration_data(iteration_data&& mov) = delete;

            // CRITICAL: after changing check mark_component_dirty for correctness
            std::pair<size_t, void**> next(iteration_topology& topology) {
                while (current_archetype_index < topology.arch_data.size()) {
                    iteration_topology::arch_data_t& adata = topology.arch_data[current_archetype_index];
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

            bool is_end(iteration_topology& topology) const {
                return current_archetype_index >= topology.arch_data.size();
            }

            // CRITICAL: this function depends on next() function, changes on using current_chunk_index
            //  would impact the current chunk retrial
            //  this function used for explicit dirty marking
            void mark_component_dirty(iteration_topology& topology, component_id component, size_t entity_index) {
                topology.mark_component_dirty(current_archetype_index, current_chunk_index, component, entity_index);
            }

            bool is_entity_match(iteration_topology& topology, size_t entity_index) const {
                return topology.is_entity_match(current_archetype_index, current_chunk_index, entity_index);
            }

            std::pair<uint32_t, uint32_t> get_current_entity(iteration_topology& topology, size_t entity_index) {
                return topology.get_current_entity(current_archetype_index, current_chunk_index, entity_index);
            }

            structural_changes get_component_change_state(iteration_topology& topology, size_t entity_index, component_id cid) {
                return topology.get_component_change_state(current_archetype_index, current_chunk_index, entity_index, cid);
            }

            preserved_state preserve_state() const {
                return {current_archetype_index, current_chunk_index};
            }
        };

        iteration_handle::iteration_handle() = default;
        iteration_handle::iteration_handle(const std::shared_ptr<iteration_topology>& topology)
            : topology(topology),
              data(std::make_unique<iteration_data>(manager::instance().manager_mutex)) {}

        iteration_handle::iteration_handle(iteration_handle&& other) noexcept {
            topology = std::move(other.topology);
            data = std::move(other.data);
            other.topology = nullptr;
            other.data = nullptr;
        }

        iteration_handle::~iteration_handle() = default;

        iteration_handle& iteration_handle::operator=(iteration_handle&& other) noexcept {
            topology = std::move(other.topology);
            data = std::move(other.data);
            return *this;
        }

        std::pair<size_t, void**> iteration_handle::next() {
            if (data)
                return data->next(*topology);
            else
                return {0, nullptr};
        }

        bool iteration_handle::is_end() const {
            if (data)
                return data->is_end(*topology);
            return true;
        }

        void iteration_handle::mark_component_dirty(component_id component, size_t index) {
            if (data)
                data->mark_component_dirty(*topology, component, index);
        }

        bool iteration_handle::is_entity_match(size_t entity_index) const {
            if (data)
                return data->is_entity_match(*topology, entity_index);
            else
                return false;
        }

        std::pair<uint32_t, uint32_t> iteration_handle::get_current_entity(size_t entity_index) {
            if (data)
                return data->get_current_entity(*topology, entity_index);
            else
                return {0, -1};
        }

        structural_changes iteration_handle::get_component_change_state(size_t entity_index, component_id cid) {
            if (data)
                return data->get_component_change_state(*topology, entity_index, cid);
            else
                return structural_changes::no_changes;
        }

        void iteration_handle::preserved_state::mark_component_dirty(iteration_handle& handle, component_id cid, size_t index) {
            if (handle.data)
                handle.topology->mark_component_dirty(archetype_index, chunk_index, cid, index);
        }

        bool iteration_handle::preserved_state::is_entity_match(iteration_handle& handle, size_t current_index_in_chunk) const {
            if (handle.data)
                return handle.topology->is_entity_match(archetype_index, chunk_index, current_index_in_chunk);
            else
                return false;
        }

        std::pair<uint32_t, uint32_t> iteration_handle::preserved_state::get_current_entity(iteration_handle& handle, size_t current_index_in_chunk) {
            if (handle.data)
                return handle.topology->get_current_entity(archetype_index, chunk_index, current_index_in_chunk);
            else
                return {0, -1};
        }

        structural_changes iteration_handle::preserved_state::get_component_change_state(iteration_handle& handle, size_t entity_index, component_id cid) {
            if (handle.data)
                return handle.topology->get_component_change_state(archetype_index, chunk_index, entity_index, cid);
            else
                return structural_changes::no_changes;
        }

        iteration_handle::preserved_state iteration_handle::preserve_state() {
            if (data)
                return data->preserve_state();
            else
                return {0, 0};
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

        int32_t get_world_id(world* w) {
            return w->id;
        }

        int32_t get_world_id(world_local_registry& w) {
            return w.get_id();
        }

        world* get_world_by_id(int32_t id) {
            return manager::instance().get_world(id);
        }

        world* register_world(int32_t id) {
            return manager::instance().enable_world(id);
        }

        void unregister_world(world* w) {
            return manager::instance().disable_world(w->id);
        }

        size_t get_state_version(std::optional<int32_t> world_id) {
            if (!world_id)
                return manager::instance().state_version.load();
            else
                return get_world_by_id(*world_id)->state_version.load();
        }

        void iteration_topology::calculate_data(
            std::span<component_id> components,
            std::span<component_id> clean_components,
            std::span<component_id> dirty_components,
            std::span<component_id> mark_dirty_components,
            std::span<component_id> with_changes_
        ) {
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

        void iteration_topology::mark_component_dirty(size_t archetype_index, size_t chunk_index, component_id component, size_t entity_index) {
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

        bool iteration_topology::is_entity_match(size_t archetype_index, size_t chunk_index, size_t entity_index) const {
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
                auto& record = manager::instance().records.at(id);
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

        std::pair<uint32_t, uint32_t> iteration_topology::get_current_entity(size_t archetype_index, size_t chunk_index, size_t entity_index) {
            if (chunk_index == 0)
                return {0, UINT32_MAX};

            auto& active_arch_data = arch_data[archetype_index];
            auto& active_chunk = active_arch_data.type->chunks[chunk_index - 1];
            auto id = active_chunk.get()->entities()[entity_index];

            auto& record = manager::instance().records.at(id);
            return {id, record.generation};
        }

        structural_changes iteration_topology::get_component_change_state(size_t archetype_index, size_t chunk_index, size_t entity_index, component_id cid) {
            auto& active_arch_data = arch_data[archetype_index];
            auto& active_chunk = active_arch_data.type->chunks[chunk_index - 1];
            auto id = active_chunk.get()->entities()[entity_index];

            auto& record = manager::instance().records.at(id);

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

        std::shared_ptr<iteration_topology> iterate_components(
            int32_t world_id,
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<component_id> writes_components,
            std::span<component_id> with_dirty_components,
            std::span<component_id> with_clean_components,
            std::span<component_id> with_changes,
            std::span<whole_component_id> with_tag_components,
            std::span<whole_component_id> without_tag_components
        ) {
            auto data = std::make_shared<iteration_topology>();
            if (auto it = manager::instance().worlds.find(world_id); it != manager::instance().worlds.end())
                for (const auto& archetype_ptr : it->second.archetypes)
                    if (archetype_ptr->matches_query(components, with_components, without_components, with_tag_components, without_tag_components))
                        data->arch_data.push_back(archetype_ptr.get());

            data->calculate_data(components, with_clean_components, with_dirty_components, writes_components, with_changes);
            return data;
        }

        std::shared_ptr<iteration_topology> iterate_components_global(
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<component_id> writes_components,
            std::span<component_id> with_dirty_components,
            std::span<component_id> with_clean_components,
            std::span<component_id> with_changes,
            std::span<whole_component_id> with_tag_components,
            std::span<whole_component_id> without_tag_components
        ) {
            auto data = std::make_shared<iteration_topology>();

            for (const auto& archetype_ptr : manager::instance().limbo.archetypes)
                if (archetype_ptr->matches_query(components, with_components, without_components, with_tag_components, without_tag_components))
                    data->arch_data.push_back(archetype_ptr.get());

            for (const auto& [id, world] : manager::instance().worlds)
                for (const auto& archetype_ptr : world.archetypes)
                    if (archetype_ptr->matches_query(components, with_components, without_components, with_tag_components, without_tag_components))
                        data->arch_data.push_back(archetype_ptr.get());


            data->calculate_data(components, with_clean_components, with_dirty_components, writes_components, with_changes);
            return data;
        }

        iteration_handle make_handle(const std::shared_ptr<iteration_topology>& topology, size_t component_count) {
            iteration_handle res(topology);
            res.data->component_arrays.resize(component_count);
            return res;
        }
    }

    void entity::destroy() {
        if (id != UINT32_MAX) {
            add<com::dead_mark>();
            id = UINT32_MAX;
        }
    }

    std::optional<entity> entity::copy_and_wait() const {
        return detail::copy_entity(std::nullopt, *this)->take();
    }

    util::nbt entity::get_nbt() const {
        std::stringstream ss;
        util::nbt_write_stream nws(ss);
        get<api::ecs::com::type_definition>().type->to_nbt(nws, *this);
        size_t res_size = 0;
        return util::nbt_convert::readNBT((uint8_t*)ss.view().data(), ss.view().size(), res_size).get_as_nbt();
    }

    bool entity_ref::try_resolve() {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std::decay_t<decltype(it)>, base_objects::uuid>) {
                    auto res = api::entity_id_map::get_entity(it);
                    if (!res)
                        return false;
                    value = *res;
                }
                return true;
            },
            value
        );
    }

    entity entity_ref::get_entity() {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std::decay_t<decltype(it)>, base_objects::uuid>) {
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

    entity entity_ref::get_entity() const {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std::decay_t<decltype(it)>, base_objects::uuid>) {
                    return *api::entity_id_map::get_entity(it);
                } else
                    return it;
            },
            value
        );
    }

    base_objects::uuid entity_ref::get_uuid() const {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std::decay_t<decltype(it)>, base_objects::uuid>) {
                    return it;
                } else
                    return it.template get<com::entities::uuid>().id;
            },
            value
        );
    }

    bool entity_ref::is_resolved() {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std::decay_t<decltype(it)>, base_objects::uuid>) {
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

    bool entity_ref::is_valid() const {
        return std::visit(
            [this](auto& it) {
                if constexpr (std::is_same_v<std::decay_t<decltype(it)>, base_objects::uuid>) {
                    return true;
                } else
                    return it.is_valid();
            },
            value
        );
    }

    bool entity_ref::operator==(const entity_ref& other) const {
        return std::visit(
            [&other]<class T0>(const T0& it) {
                return std::visit(
                    [&it]<class T1>(const T1& other_it) {
                        if constexpr (std::is_same_v<T0, T1>) {
                            return it == other_it;
                        } else if constexpr (std::is_same_v<T0, base_objects::uuid>) {
                            return it == other_it.template get<com::entities::uuid>().id;
                        } else
                            return it.template get<com::entities::uuid>().id == other_it;
                    },
                    other.value
                );
            },
            value
        );
    }

    bool entity_ref::operator!=(const entity_ref& other) const {
        return !(*this == other);
    }

    bool entity_ref::operator==(const entity& other) const {
        return *this == entity_ref{other};
    }

    bool entity_ref::operator!=(const entity& other) const {
        return !(*this == other);
    }

    bool entity_ref::operator==(const base_objects::uuid& other) const {
        return *this == entity_ref{other};
    }

    bool entity_ref::operator!=(const base_objects::uuid& other) const {
        return !(*this == other);
    }

    unique_entity::~unique_entity() {
        if (id != UINT32_MAX)
            add<com::dead_mark>();
    }

    entity unique_entity::release() {
        entity self = *this;
        id = UINT32_MAX;
        return self;
    }

    bool unique_entity::has_value() {
        return id != UINT32_MAX;
    }
}
