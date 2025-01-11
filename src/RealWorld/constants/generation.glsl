/** 
 *  @author    Dubsky Tomas
 */
#ifndef GENERATION_GLSL
#define GENERATION_GLSL
#include <RealShaders/CppIntegration.glsl>

constexpr int k_genBorderWidth = 8;
constexpr int k_genGroupSize = 16;
constexpr int k_chunkGenSlots = 12;
constexpr int k_maxVegPerChunk = 64;
constexpr int k_branchGenSlots = k_chunkGenSlots * 256;

constexpr int k_lSystemSpeciesCount = 6;
constexpr int k_grassSpeciesCount = 4;
constexpr int k_vegSpeciesCount = k_lSystemSpeciesCount + k_grassSpeciesCount;

#endif // !GENERATION_GLSL