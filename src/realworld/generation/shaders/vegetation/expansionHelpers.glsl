/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_EXPANSION_HELPERS_GLSL
#define RW_EXPANSION_HELPERS_GLSL

void emitBranch(
    uint outIndex, vec2 posTi, float tipAbsAngNorm, float relRestAngNorm,
    uint parentOffset, uint wallType, vec2 sizeTi, vec2 densityStiffness
){
    b_vegPrep.vegIndex[outIndex] = gl_WorkGroupID.x;
    b_vegPrep.absPosTi[outIndex] = posTi;
    b_vegPrep.absAngNorm[outIndex] = tipAbsAngNorm;
    b_vegPrep.parentOffset15wallType31[outIndex] = packBranchParentOffsetWallType(parentOffset, wallType);
    b_vegPrep.relRestAngNorm[outIndex] = relRestAngNorm;
    b_vegPrep.radiusTi[outIndex] = sizeTi.x;
    b_vegPrep.lengthTi[outIndex] = sizeTi.y;
    b_vegPrep.densityStiffness[outIndex] = densityStiffness;
}

uint reserveSpaceInPrepBuf(uint instanceIndex, uint chunkIndex, uint branchCount){
    uint outIndex = atomicAdd(b_vegPrep.branchDispatchSize.x, branchCount);
    b_vegPrep.prepIndexOfFirstBranch[instanceIndex] = outIndex;
    b_vegPrep.vegOffsetWithinChunk[instanceIndex] =
        atomicAdd(b_vegPrep.branchOfChunk[chunkIndex], branchCount);
    return outIndex;
}

#endif // !RW_EXPANSION_HELPERS_GLSL