﻿/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/tile_properties/blockTransformationProperties.hpp>









//To be edited with propertiesHelper.txt
constexpr std::array<glm::uvec4, 256> k_wallTransformationProperties = {
//              Properties                                  The first applicable rule   The last applicable rule    Padding
    glm::uvec4{ STONEb,                                     6,                          6,                          0},
    glm::uvec4{ DIRTb,                                      0,                          1,                          0},
    glm::uvec4{ GRASSb,                                     7,                          9,                          0},
    glm::uvec4{ STONEb,                                     6,                          6,                          0},
    glm::uvec4{ DIRTb,                                      6,                          6,                          0},
    glm::uvec4{ DIRTb,                                      2,                          3,                          0},
    glm::uvec4{ GRASSb,                                     10,                         12,                         0},
    glm::uvec4{ DIRTb,                                      4,                          5,                          0},
    glm::uvec4{ GRASSb,                                     13,                         15,                         0},
    glm::uvec4{ GRASSb,                                     8,                          9,                          0},
    glm::uvec4{ HALLOWb | STONEb,                           ~0,                         0,                          0},
    glm::uvec4{ HALLOWb | DIRTb,                            ~0,                         0,                          0},
    glm::uvec4{ HALLOWb | GRASSb,                           ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0},
    glm::uvec4{ 0,                                          ~0,                         0,                          0}
};
