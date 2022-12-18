/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>

 /**
  * @brief Contains all push constants needed for drawing
 */
struct WorldDrawerPushConstants {
    //Updated when view / world texture changes \|/
    glm::mat4 viewMat;
    glm::ivec2 worldTexMask;
    int viewWidthTi;
    glm::uint padding0;
    //Updated every step \|/
    glm::ivec2 analysisOffsetTi;
    glm::vec2 botLeftPxModTilePx;
    glm::ivec2 drawShadowsReadOffsetTi;
    glm::ivec2 botLeftTi;
    glm::ivec2 addLightOffsetPx;
    glm::uint lightCount;
};
