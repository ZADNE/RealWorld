/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/pipelines/Vertex.hpp>

/**
* @brief Represents a dynamic light that can be added into the world.
*/
struct ExternalLight {
    ExternalLight(const glm::ivec2& posPx, re::Color col): posPx(posPx), col(col) {}

    glm::ivec2 posPx;   //Position of the center of the light in pixels
    re::Color col;      //RGB = color of the light, A = intensity of the light
    glm::uint padding;
};
