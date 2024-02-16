/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_TEMPLATES_UB_GLSL
#define VEG_TEMPLATES_UB_GLSL
#include <RealWorld/vegetation/shaders/Branch.glsl>

#extension GL_EXT_scalar_block_layout : require
layout (set = 0, binding = k_vegTemplatesBinding, std430)
restrict uniform VegTemplatesUB {
    Branch branches[k_vegTemplatesBranchCount];
} u_vegTemplates;

#endif // !VEG_TEMPLATES_UB_GLSL