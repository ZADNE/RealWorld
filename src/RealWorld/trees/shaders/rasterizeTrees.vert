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

    // Simulation
    if (instanceIndex == 0){
        const Branch parent = b_branchesRead[b.parentIndex];
        vec4 parentAngles = unpackUnorm4x8(parent.angles);
        /*float         wind   = hash11(p_timeSec);
        wind += 0.3 * hash11(p_timeSec * 10.0);

        float volume = k_pi * b.radiusTi * b.radiusTi * b.lengthTi;
        float weight = volume * b.density;

        vec2 force = vec2(wind * b.radiusTi * b.lengthTi, weight * -0.01);

        float forceAngle = atan(force.y, force.x);
        float forceSize  = length(force);

        float momentOfInertia = k_third * weight * b.lengthTi *
                                b.lengthTi;
        momentOfInertia = momentOfInertia == 0.0 ? 1.0 : momentOfInertia;

        float angularAcc =
            b.lengthTi * forceSize *
            sin(forceAngle - angles.absAngleNorm * k_2pi) /
            momentOfInertia;

        float angleDiffToRestNorm = angularDifference(
            fract(parentAngles.absAngleNorm + angles.relRestAngleNorm), angles.absAngleNorm
        );

        angularAcc += b.stiffness * angleDiffToRestNorm - angles.angleVelNorm * 0.2;

        angles.angleVelNorm += angularAcc;
        angles.absAngleNorm += angles.angleVelNorm;
        angles.absAngleNorm = fract(angles.absAngleNorm);
        b.angles = packUnorm4x8(angles);*/

        b.absPosTi +=
            vec2(0.000, 1.0); +
            parent.absPosTi +
            toCartesian(b.lengthTi, parentAngles.absAngleNorm + angles.relRestAngleNorm);

        // Store the modified branch
        b_branchesWrite[branchIndex] = b;
    }
}
