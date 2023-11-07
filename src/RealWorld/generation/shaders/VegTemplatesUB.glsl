/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_TEMPLATES_UB_GLSL
#define VEG_TEMPLATES_UB_GLSL

#include <RealWorld/vegetation/shaders/Branch.glsl>

#extension GL_EXT_scalar_block_layout : require
layout (set = 0, binding = VegTemplatesUB_BINDING, std430)
restrict uniform VegTemplatesUB {
    Branch u_vegTemplateBranches[k_vegTemplatesBranchCount];
};

#endif // !VEG_TEMPLATES_UB_GLSL