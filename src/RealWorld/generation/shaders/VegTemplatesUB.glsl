/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_TEMPLATES_UB_GLSL
#define VEG_TEMPLATES_UB_GLSL
#include <RealWorld/constants/generation.glsl>

struct String {
    uint begin;
    uint count;
};

struct ProbabilisticRewriteRuleBodies {
    uint firstIndex;
    uint bodyCount;
};

const int k_rewriteableSymbolCount = 1;

struct VegTemplate {
    String axiom;
    ProbabilisticRewriteRuleBodies rules[k_rewriteableSymbolCount];
    float alpha;
    uint iterCount;
};

const int k_totalRewriteRuleBodyCount = 3;

#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
layout (set = 0, binding = k_vegTemplatesBinding, std430)
restrict uniform VegTemplatesUB {
    uint8_t     symbols[k_vegTemplatesSymbolCount];
    float       ruleProbs[k_totalRewriteRuleBodyCount];
    String      ruleBodies[k_totalRewriteRuleBodyCount];
    VegTemplate tmplts[k_vegTemplateCount];
} u_vegTmplts;

#endif // !VEG_TEMPLATES_UB_GLSL