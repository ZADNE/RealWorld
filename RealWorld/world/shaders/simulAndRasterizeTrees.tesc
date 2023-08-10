/*!
 *  @author    Dubsky Tomas
 */
#version 460
layout(vertices = 1) out;
layout(location = 0) patch out vec2  o_posTi;
layout(location = 1) patch out vec2  o_sizeTi;
layout(location = 2) patch out float o_angleNorm;
layout(location = 3) patch out uint  o_instanceIndex;

layout(location = 0) in vec2  i_posTi[];
layout(location = 1) in vec2  i_sizeTi[];
layout(location = 2) in float i_angleNorm[];
layout(location = 3) in uint  i_instanceIndex[];

void main() {
    gl_TessLevelOuter[0] = 1.0f;
    gl_TessLevelOuter[1] = 1.0f;
    gl_TessLevelOuter[2] = 1.0f;
    gl_TessLevelOuter[3] = 1.0f;
    gl_TessLevelInner[0] = 0.0f;
    gl_TessLevelInner[1] = 0.0f;

    o_posTi         = i_posTi[gl_InvocationID];
    o_sizeTi        = i_sizeTi[gl_InvocationID];
    o_angleNorm     = i_angleNorm[gl_InvocationID];
    o_instanceIndex = i_instanceIndex[gl_InvocationID];
}
