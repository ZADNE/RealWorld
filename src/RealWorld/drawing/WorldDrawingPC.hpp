/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace rw {

struct WorldDrawingPC {
    glm::mat4  minimapViewMat;
    glm::vec2  uvRectSize;
    glm::vec2  uvRectOffset;
    glm::ivec2 botLeftTi;
    glm::ivec2 worldTexMask;
    glm::vec2  minimapOffset;
    glm::vec2  minimapSize;
    glm::vec4  skyColor;
};

} // namespace rw
