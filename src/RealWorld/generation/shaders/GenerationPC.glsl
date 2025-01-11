/**
 *  @author     Dubsky Tomas
 */
#ifndef GENERATION_PC_GLSL
#define GENERATION_PC_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (push_constant, scalar)
uniform GenerationPC {
    ivec2 chunkTi[k_chunkGenSlots];
    ivec2 worldTexSizeCh;
    int   seed;
    uint  storeSegment;
    uint  edgeConsolidationPromote;
    uint  edgeConsolidationReduce;
} RE_GLSL_ONLY(p_);

#ifdef VULKAN

uint blockImgLoadLayer(){
    return (1 - p_.storeSegment) * k_chunkGenSlots + gl_WorkGroupID.z;
}

uint blockImgStoreLayer(){
    return p_.storeSegment * k_chunkGenSlots + gl_WorkGroupID.z;
}

uint wallImgLayer(){
    return 2 * k_chunkGenSlots + gl_WorkGroupID.z;
}

#endif

#endif // !GENERATION_PC_GLSL