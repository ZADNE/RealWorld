/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/simulation/vegetation/shaders/BranchSB_glsl.hpp>

namespace rw {

// NOLINTBEGIN: Poor man's reflection

struct BranchSerialized {
    static_assert(
        sizeof(glsl::BranchSB) == 11796480,
        "Layout of branch members probably changed - fix me"
    );
    template<typename T>
    using NoRef = std::remove_reference_t<T>;
    using B     = glsl::BranchSB;

    NoRef<decltype(*B::absPosTi[0])> absPosTi;
    NoRef<decltype(*B::absAngNorm[0])> absAngNorm;

    NoRef<decltype(*B::parentOffset15wallType31)> parentOffset15wallType31;
    NoRef<decltype(*B::relRestAngNorm)> relRestAngNorm;
    NoRef<decltype(*B::angVel)> angVel;
    NoRef<decltype(*B::radiusTi)> radiusTi;
    NoRef<decltype(*B::lengthTi)> lengthTi;
    NoRef<decltype(*B::densityStiffness)> densityStiffness;
    NoRef<decltype(*B::raster)> raster;

    static constexpr size_t memberCount() {
        static_assert(
            sizeof(glsl::BranchSB) == 11796480,
            "Also fix the count of members here"
        );
        return 9;
    }
};

// NOLINTEND

} // namespace rw
