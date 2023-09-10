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

#include <RealWorld/generation/external_shaders/snoise.glsl>
const float k_third = 0.33333333333;

#define absAngleNorm x
#define relRestAngleNorm y
#define angleVelNorm z

void main(){
    // Outputs for next stage
    Branch b = b_branchesRead[gl_VertexIndex];
    o_posTi = b.absPosTi;
    o_sizeTi = vec2(b.radiusTi * 2.0, b.lengthTi);
    o_angleNorm = b.angles.absAngleNorm;

    // Simulation
    const Branch parent = b_branchesRead[b.parentIndex];
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
        sin(forceAngle - b.angles.absAngleNorm * k_2pi) /
        momentOfInertia;

    float angleDiffToRestNorm = angularDifference(
        fract(parent.angles.absAngleNorm + b.angles.relRestAngleNorm), b.angles.absAngleNorm
    );

    angularAcc += b.stiffness * angleDiffToRestNorm - b.angles.angleVelNorm * 0.875;

    b.angles.angleVelNorm += angularAcc;
    b.angles.absAngleNorm += b.angles.angleVelNorm;
    b.angles.absAngleNorm = fract(b.angles.absAngleNorm);

    b.absPosTi = parent.absPosTi + toCartesian(parent.lengthTi, parent.angles.absAngleNorm);

    // Store the modified branch
    b_branchesWrite[gl_VertexIndex] = b;
}
