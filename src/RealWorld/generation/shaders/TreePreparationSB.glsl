/*!
 *  @author     Dubsky Tomas
 */
#ifndef TREE_PREPARATION_SB_GLSL
#define TREE_PREPARATION_SB_GLSL

#ifndef TreePreparationSB_ACCESS
#define TreePreparationSB_ACCESS readonly
#endif

struct TreeDescription{
    uint  templateRootIndex;    // Index to the template buffer
    uint  writeIndex;           // Index to the branch buffer
    uint  randomSeed;
    uint  padding;
    vec2  rootPosTi;
    float sizeFactor;
    float angleFactor;
};

layout (binding = TreePreparationSB_BINDING, std430)
TreePreparationSB_ACCESS restrict buffer TreePreparationSB {
    uvec4           b_dispatchSize;
    TreeDescription b_treeDescriptions[];
};

#endif // !TREE_PREPARATION_SB_GLSL