#ifndef SRC_BASE_OBJECTS_ANY_BLOCK
#define SRC_BASE_OBJECTS_ANY_BLOCK

#include <src/api/ecs.hpp>
#include <src/base_objects/block.hpp>
#include <variant>

namespace copper_server::base_objects {
    struct any_block : public std::variant<block, api::ecs::entity> {
        using std::variant<block, api::ecs::entity>::variant;
        using std::variant<block, api::ecs::entity>::operator=;

        any_block(const any_block& value) {
            std::visit(
                [this]<class T>(const T& it) {
                    if constexpr (std::is_same_v<T, block>)
                        *this = it;
                    else {
                        auto res = it.copy_and_wait();
                        if (!res)
                            throw std::runtime_error("Failed to copy block_entity");
                        *this = *res;
                    }
                },
                value
            );
        }

        any_block& operator=(const any_block& value) {
            std::visit(
                [this]<class T>(const T& it) {
                    if constexpr (std::is_same_v<T, block>)
                        *this = it;
                    else {
                        auto res = it.copy_and_wait();
                        if (!res)
                            throw std::runtime_error("Failed to copy block_entity");
                        *this = *res;
                    }
                },
                value
            );
            return *this;
        }
    };
}

#endif /* SRC_BASE_OBJECTS_ANY_BLOCK */
