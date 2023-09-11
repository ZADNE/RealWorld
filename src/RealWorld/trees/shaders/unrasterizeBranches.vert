/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout(location = 0) out vec2  o_posTi;
layout(location = 1) out vec2  o_sizeTi;
layout(location = 2) out float o_angleNorm;

const int BranchesSBWrite_BINDING = 0;
const int BranchesSBRead_BINDING = 1;
#include <RealWorld/trees/shaders/BranchesSB.glsl>
#include <RealWorld/trees/shaders/normAngles.glsl>
#include <RealWorld/trees/shaders/TreeDynamicsPC.glsl>

#include <RealWorld/generation/external_shaders/float_hash.glsl>
const float k_third = 0.33333333333;

void main(){
    // Outputs for next stage
    Branch b = b_branchesRead[gl_VertexIndex];
    o_posTi = b.absPosTi;
    o_sizeTi = vec2(b.radiusTi * 2.0, b.lengthTi);
    o_angleNorm = b.absAngleNorm;
}
