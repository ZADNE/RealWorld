/** 
 *  @author    Dubsky Tomas
 */
#ifndef EXPAND_GRASS_GLSL
#define EXPAND_GRASS_GLSL

shared uint s_outIndex;

struct GrassTemplate{
    uint  wallType;
};

const GrassTemplate k_grassTemplates[k_grassSpeciesCount] = {
    {k_tallGrassWl},
    {k_coldTallGrassWl},
    {k_mudTallGrassWl},
    {k_dryTallGrassWl}
};


const int k_randomOffsetTi = iChunkTi.x - 1;
int randomOffset(){
    return int(random()) & k_randomOffsetTi;
}

void expandGrass(){
    uint grassIndex = s_inst.templateIndex - k_lSystemSpeciesCount;
    if (gl_LocalInvocationID.x == 0){
        // Reserve space for branches in the preparation buffer
        uint rootCount = uint(32.0 * s_inst.growth);
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
        vec2 biomeClimate = calcBiomeClimate(xPx, p_.seed);
        Biome biome = calcBiomeStructure(biomeClimate);
        float horizonPx = calcHorizon(xPx, biome, p_.seed).r;
        pTi.y = int(pxToTi(horizonPx));

        bool stemBranch = bool(i & 1);

        float angleNorm = stemBranch ? 0.0 : 0.25 + (randomFloat() - 0.5) * 0.05;
        vec2 sizeTi = stemBranch ? vec2(0.5, 16.0 + randomFloat() * 10.0) : vec2(0.0, 0.0);

        emitBranch(
            s_outIndex + i,
            vec2(pTi),
            angleNorm,
            angleNorm,
            i & 1,
            k_grassTemplates[grassIndex].wallType,
            sizeTi,
            vec2(8.0, 0.015625)
        );
    }
}

#endif // !EXPAND_GRASS_GLSL