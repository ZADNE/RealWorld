/*!
 *  @author     Dubsky Tomas
 */
#ifndef TREE_TEMPLATES_UB_GLSL
#define TREE_TEMPLATES_UB_GLSL

#include <RealWorld/trees/shaders/Branch.glsl>

layout(set = 0, binding = TreeTemplatesUB_BINDING, std430)
restrict buffer TreeTemplatesUB {
    Branch b_branches[];
};

#endif // TREE_TEMPLATES_UB_GLSL