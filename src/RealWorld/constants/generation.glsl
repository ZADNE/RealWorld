/*! 
 *  @author    Dubsky Tomas
 */
#ifndef GENERATION_GLSL
#define GENERATION_GLSL

const int k_genBorderWidth = 8;
const int k_genGroupSize = 16;
const int k_chunkGenSlots = 12;
const int k_maxVegPerChunk = 64;
const int k_branchGenSlots = k_chunkGenSlots * 256;

const int k_lSystemSpeciesCount = 6;
const int k_grassSpeciesCount = 4;
const int k_vegSpeciesCount = k_lSystemSpeciesCount + k_grassSpeciesCount;

#endif // !GENERATION_GLSL