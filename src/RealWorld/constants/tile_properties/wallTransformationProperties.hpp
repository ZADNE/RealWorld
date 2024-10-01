﻿/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/tile_properties/blockTransformationProperties.hpp>

namespace rw {

// clang-format off: Custom alignment of columns

constexpr std::array<glm::uvec4, 256> k_wallTransformationProperties = {
//                              Properties                                  The first applicable rule   The last applicable rule    Padding
/*stone*/           glm::uvec4{ STONEb,                                     6,                          6,                          0},
/*dirt*/            glm::uvec4{ DIRTb,                                      0,                          1,                          0},
/*grass*/           glm::uvec4{ GRASSb,                                     7,                          9,                          0},
/*coldStone*/       glm::uvec4{ STONEb,                                     6,                          6,                          0},
/*sand*/            glm::uvec4{ DIRTb,                                      6,                          6,                          0},
/*coldDirt*/        glm::uvec4{ DIRTb,                                      2,                          3,                          0},
/*coldGrass*/       glm::uvec4{ GRASSb,                                     10,                         12,                         0},
/*mud*/             glm::uvec4{ DIRTb,                                      4,                          5,                          0},
/*mudGrass*/        glm::uvec4{ GRASSb,                                     13,                         15,                         0},
/*dryGrass*/        glm::uvec4{ GRASSb,                                     8,                          9,                          0},
/*hallowStone*/     glm::uvec4{ HALLOWb | STONEb,                           ~0,                         0,                          0},
/*hallowDirt*/      glm::uvec4{ HALLOWb | DIRTb,                            ~0,                         0,                          0},
/*hallowGrass*/     glm::uvec4{ HALLOWb | GRASSb,                           ~0,                         0,                          0},
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
/*oakWood*/         glm::uvec4{ WOODb,                                      20,                         21,                         0},
/*acaciaWood*/      glm::uvec4{ WOODb,                                      20,                         21,                         0},
/*coniferousWood*/  glm::uvec4{ WOODb,                                      20,                         21,                         0},
/*palmWood*/        glm::uvec4{ WOODb,                                      20,                         21,                         0},
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
/*tallGrass*/       glm::uvec4{ GRASSb,                                     20,                         21,                         0},
/*coldTallGrass*/   glm::uvec4{ GRASSb,                                     20,                         21,                         0},
/*mudTallGrass*/    glm::uvec4{ GRASSb,                                     20,                         21,                         0},
/*dryTallGrass*/    glm::uvec4{ GRASSb,                                     20,                         21,                         0},
/*leaf*/            glm::uvec4{ 0,                                          16,                         17,                         0},
/*dryLeaf*/         glm::uvec4{ 0,                                          18,                         19,                         0},
/*needle*/          glm::uvec4{ 0,                                          18,                         18,                         0},
/*palmLeaf*/        glm::uvec4{ 0,                                          18,                         18,                         0},
/*withy*/           glm::uvec4{ 0,                                          20,                         21,                         0},
/*cactus*/          glm::uvec4{ 0,                                          20,                         21,                         0},
/*burningWood*/     glm::uvec4{ BURNINGb,                                   22,                         24,                         0},
/*burntWood*/       glm::uvec4{ 0,                                          ~0,                         0,                          0},
/*hallowWood*/      glm::uvec4{ WOODb | HALLOWb,                            20,                         20,                         0},
                    glm::uvec4{ 0,                                          ~0,                         0,                          0},
                    glm::uvec4{ 0,                                          ~0,                         0,                          0},
/*air*/             glm::uvec4{ 0,                                          ~0,                         0,                          0}
};

// clang-format on

} // namespace rw
