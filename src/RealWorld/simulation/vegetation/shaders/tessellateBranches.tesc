/*!
 *  @author    Dubsky Tomas
 */
#version 460
layout (vertices = 1) out;
layout (location = 0) patch out vec2  o_p0Ti; // Start
layout (location = 1) patch out vec2  o_p1Ti; // Control
layout (location = 2) patch out vec2  o_p2Ti; // End
layout (location = 3) patch out vec2  o_sizeTi;
layout (location = 4) patch out uint  o_branchIndex15wallType31;

layout (location = 0) in vec2  i_posTi[];
layout (location = 1) in vec2  i_sizeTi[];
layout (location = 2) in float i_startAngleNorm[];
layout (location = 3) in float i_endAngleNorm[];
layout (location = 4) in uint  i_branchIndex15wallType31[];

#include <RealWorld/simulation/vegetation/shaders/VegDynamicsPC.glsl>
#include <RealWorld/simulation/vegetation/shaders/normAngles.glsl>

vec2 imPos(vec2 posTi){
    return fract(posTi / p_.worldTexSizeTi) * p_.worldTexSizeTi;
}

void main() {
    float lengthTi = i_sizeTi[gl_InvocationID].y;
    float aDiff = abs(angularDifference(i_startAngleNorm[gl_InvocationID], i_endAngleNorm[gl_InvocationID]));
    float subdivs =
        (lengthTi > 0.0)
        ? max(lengthTi * aDiff * 3.0, 1.0) // max to avoid the branch from disappearing if aDiff == 0
        : 0.0;                       // Discard root zero-length 'branch'
    gl_TessLevelOuter[0] = subdivs;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = subdivs;
    gl_TessLevelOuter[3] = 1.0;
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = subdivs;

    o_p0Ti = imPos(i_posTi[gl_InvocationID]);
    o_p1Ti = o_p0Ti + toCartesian(lengthTi * 0.5, i_startAngleNorm[gl_InvocationID]);
    o_p2Ti = o_p0Ti + toCartesian(lengthTi, i_endAngleNorm[gl_InvocationID]);
    o_sizeTi = i_sizeTi[gl_InvocationID];
    o_branchIndex15wallType31 = i_branchIndex15wallType31[gl_InvocationID];
}
