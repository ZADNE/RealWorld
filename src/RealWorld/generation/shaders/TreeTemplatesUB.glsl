/*!
 *  @author     Dubsky Tomas
 */
#ifndef TREE_TEMPLATES_UB_GLSL
#define TREE_TEMPLATES_UB_GLSL

#include <RealWorld/constants/tree.glsl>
#include <RealWorld/trees/shaders/Branch.glsl>

#extension GL_EXT_scalar_block_layout : require
layout(set = 0, binding = TreeTemplatesUB_BINDING, std430)
restrict uniform TreeTemplatesUB {
    Branch u_treeTemplateBranches[k_treeTemplatesBranchCount];
};

#endif // TREE_TEMPLATES_UB_GLSL