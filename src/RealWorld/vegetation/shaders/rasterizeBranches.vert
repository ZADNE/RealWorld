/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec2  o_posTi;
layout (location = 1) out vec2  o_sizeTi;
layout (location = 2) out float o_startAngleNorm;
layout (location = 3) out float o_endAngleNorm;
layout (location = 4) out uint  o_branchIndex0wallType16;

const int BranchVectorSBWrite_BINDING = 0;
const int BranchVectorSBRead_BINDING = 1;
#include <RealWorld/vegetation/shaders/BranchVectorSB.glsl>
#include <RealWorld/vegetation/shaders/normAngles.glsl>
#include <RealWorld/vegetation/shaders/VegDynamicsPC.glsl>

#include <RealWorld/generation/external_shaders/snoise.glsl>
const float k_third = 0.33333333333;

void main(){
    // Load this branch and parent branch
    Branch b = b_branchesRead[gl_VertexIndex];
    const Branch parent = b_branchesRead[b.parentIndex];

    // Outputs for next stage
    o_posTi = b.absPosTi;
    o_sizeTi = vec2(b.radiusTi * 2.0, b.lengthTi);
    o_endAngleNorm = b.absAngleNorm;
    o_startAngleNorm = parent.absAngleNorm;
    o_branchIndex0wallType16 = gl_VertexIndex | (b.wallType << 16);

    // Simulation
    float wind = snoise(vec2(b.absPosTi.x * 0.001, p_timeSec * 0.1), 0.0);
    wind += 0.5 * snoise(vec2(b.absPosTi.x * 0.001, p_timeSec * 0.1 * 2.0), 0.0);

    float volume = k_pi * b.radiusTi * b.radiusTi * b.lengthTi;
    float weight = volume * b.density;

    vec2 force = vec2(wind * b.radiusTi * b.lengthTi * 0.1, weight * -0.01);

    float forceAngle = atan(force.y, force.x);
    float forceSize  = length(force);

    float momentOfInertia = k_third * weight * b.lengthTi *
                            b.lengthTi;
    momentOfInertia = momentOfInertia == 0.0 ? 1.0 : momentOfInertia;

    float angularAcc =
        b.lengthTi * forceSize *
        sin(forceAngle - b.absAngleNorm * k_2pi) /
        momentOfInertia;

    float angleDiffToRestNorm = angularDifference(
        fract(parent.absAngleNorm + b.relRestAngleNorm), b.absAngleNorm
    );

    angularAcc += b.stiffness * angleDiffToRestNorm - b.angleVelNorm * 0.875;

    // If not root - TO BE FIXED
    if (b.parentIndex != gl_VertexIndex){
        b.angleVelNorm += angularAcc;
        b.absAngleNorm += b.angleVelNorm;
        b.absAngleNorm = fract(b.absAngleNorm);
    }

    b.absPosTi = parent.absPosTi + toCartesian(parent.lengthTi, parent.absAngleNorm);


    // Store the modified branch
    b_branchesWrite[gl_VertexIndex] = b;
}
