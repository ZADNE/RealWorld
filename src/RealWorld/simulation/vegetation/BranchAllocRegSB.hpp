/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <memory>

#include <glm/vec2.hpp>

#include <RealWorld/simulation/vegetation/shaders/BranchAllocRegSB_glsl.hpp>

namespace rw {

/**
 * @brief Allocates BranchAllocRegSB initializes it
 * @details It is allocated on heap because it would take too much space on stack.
 */
inline std::unique_ptr<glsl::BranchAllocRegSB> createBranchAllocRegSB() {
    auto ptr    = std::make_unique<glsl::BranchAllocRegSB>();
    size_t size = std::size(ptr->allocIndexOfTheChunk);
    std::fill_n(&ptr->allocIndexOfTheChunk[0], size, -1);
    return ptr;
}

} // namespace rw
