/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout(location = 0) out vec2  o_posTi;
layout(location = 1) out vec2  o_sizeTi;
layout(location = 2) out float o_angleNorm;
layout(location = 3) out uint  o_instanceIndex;

const int BranchesSBWrite_BINDING = 0;
const int BranchesSBRead_BINDING = 1;
#include <RealWorld/trees/shaders/BranchesSB.glsl>
#include <RealWorld/trees/shaders/normAngles.glsl>
#include <RealWorld/trees/shaders/TreeDynamicsPC.glsl>

#include <RealWorld/generation/external_shaders/float_hash.glsl>
const float k_third = 0.33333333333;

#define absAngleNorm x
#define relRestAngleNorm y
#define angleVelNorm z

void main(){
    // Indices
    const uint branchIndex = gl_VertexIndex >> 2;
    const uint instanceIndex = gl_VertexIndex & 0x3;

    // Outputs for next stage
    Branch b = b_branchesRead[branchIndex];
    o_posTi = b.absPosTi;
    o_sizeTi = vec2(b.radiusTi * 2.0, b.lengthTi);
    vec4 angles = unpackUnorm4x8(b.angles);
    o_angleNorm = angles.absAngleNorm;
    o_instanceIndex = instanceIndex;
}
