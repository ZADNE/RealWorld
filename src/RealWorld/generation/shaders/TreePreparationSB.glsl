/*!
 *  @author     Dubsky Tomas
 */
#ifndef TREE_PREPARATION_SB_GLSL
#define TREE_PREPARATION_SB_GLSL

#ifndef TreePreparationSB_ACCESS
#define TreePreparationSB_ACCESS readonly
#endif

layout(binding = TreePreparationSB_BINDING, std430)
TreePreparationSB_ACCESS restrict buffer TreePreparationSB {
    uvec4 b_dispatchSize;
};

#endif // !TREE_PREPARATION_SB_GLSL