/*! 
 *  @author    Dubsky Tomas
 */
#ifndef EXPAND_GRASS_GLSL
#define EXPAND_GRASS_GLSL

shared uint s_outIndex;

struct GrassTemplate{
    float rootCount;
};

const GrassTemplate k_grassTemplates[k_grassSpeciesCount] = {
    {16.0}
};


const int k_randomOffsetTi = iChunkTi.x - 1;
int randomOffset(){
    return int(random()) & k_randomOffsetTi;
}

void expandGrass(){
    uint grassIndex = s_inst.templateIndex - k_lSystemSpeciesCount;
    if (gl_LocalInvocationID.x == 0){
        // Reserve space for branches in the preparation buffer
        uint rootCount = uint(k_grassTemplates[grassIndex].rootCount * s_inst.growth);
        s_branchCount = 2 * rootCount;
        s_outIndex = reserveSpaceInPrepBuf(
            gl_WorkGroupID.x, s_inst.chunkIndex, s_branchCount
        );
    }
    barrier();

    // Output branches
    for (uint i = gl_LocalInvocationID.x; i < s_branchCount; i += gl_WorkGroupSize.x){
        ivec2 pTi = ivec2(s_inst.rootPosTi.x + randomOffset(), 0);
        float xPx = tiToPx(pTi.x);
        vec2 biomeClimate = biomeClimate(xPx, p_seed);
        Biome biome = biomeStructure(biomeClimate);
        float horizonPx = horizon(xPx, biome, p_seed).r;
        pTi.y = int(pxToTi(horizonPx));

        bool stemBranch = bool(i & 1);

        float angleNorm = stemBranch ? 0.0 : 0.25;
        vec2 sizeTi = stemBranch ? vec2(0.5, 22.0) : vec2(0.0, 0.0);
        sizeTi.y *= 1.0 + (randomFloat() - 0.5) * 0.375;

        emitBranch(
            s_outIndex + i,
            vec2(pTi),
            angleNorm,
            angleNorm,
            i & 1,
            k_wheatWl,
            sizeTi,
            vec2(8.0, 0.015625)
        );
    }
}

#endif // !EXPAND_GRASS_GLSL