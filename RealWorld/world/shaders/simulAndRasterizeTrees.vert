/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout(location = 0) out vec4  o_posSizeTi;
layout(location = 1) out float o_angleNorm;

const int BranchesSBWrite_BINDING = 0;
const int BranchesSBRead_BINDING = 1;
#include <RealWorld/world/shaders/BranchesSB.glsl>
#include <RealWorld/world/shaders/TreeDynamicsPC.glsl>

#include <RealWorld/generation/external_shaders/float_hash.glsl>

const float k_pi =      3.14159265359;
const float k_2pi =     6.28318530718;
const float k_third =   0.33333333333;

vec2 toCartesian(float len, float angleNorm) {
    float angle = angleNorm * k_2pi;
    return {len * cos(angle), len * sin(angle)};
}

float angularDifference(float target, float current) {
    float diff = target - current;
    diff += (diff > 0.5f) ? -1.0f : (diff < -0.5f) ? 1.0f : 0.0f;
    return diff;
}

float absAngleNorm(vec4 angles){ return angles.x; }
float relRestAngleNorm(vec4 angles){ return angles.y; }
float angleVelNorm(vec4 angles){ return angles.z; }

void main(){
    Branch b = b_branchesRead[gl_VertexIndex];
    vec4 angles = unpackUnorm4x8(b.angles);
    if (gl_InstanceID == 0){
        const Branch parent = b_branchesRead[b.parentIndex];
        float         wind   = hash11(p_timeSec);
        wind += 0.3 * hash11(p_timeSec * 10.0);

        float volume = k_pi * b.radiusTi * b.radiusTi * b.lengthTi;
        float weight = volume * b.density;

        vec2 force = vec2{wind * b.radiusTi * b.lengthTi, weight * -0.01};

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

        angularAcc += b.stiffness * angleDiffToRestNorm - b.angleVelNorm * 0.2;

        b.angleVelNorm += angularAcc;
        b.absAngleNorm += b.angleVelNorm;
        b.absAngleNorm = fract(b.absAngleNorm);

        b.absPosTi =
            parent.absPosTi +
            toCartesian(b.lengthTi, parent.absAngleNorm + b.relRestAngleNorm);

        // Store the modified branch
        b_branchesWrite[i] = b;
    }
    o_posSizeTi = vec4(b.absPosTi, b.radiusTi, b.lengthTi);
    o_angleNorm = 
}
