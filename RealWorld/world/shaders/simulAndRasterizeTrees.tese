/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout(quads, equal_spacing) in;
layout(location = 0) patch in vec2  i_posTi;
layout(location = 1) patch in vec2  i_sizeTi;
layout(location = 2) patch in float i_angleNorm;
layout(location = 3) patch in uint  i_instanceIndex;

#include <RealWorld/world/shaders/normAngles.glsl>
#include <RealWorld/world/shaders/TreeDynamicsPC.glsl>

vec2 imPos(vec2 posTi){
    return fract(posTi / p_worldTexSizeTi) * p_worldTexSizeTi;
}

void main(){
    vec2 uvs = vec2(gl_TessCoord.x - 0.5, gl_TessCoord.y);
    vec2 posTi = imPos(i_posTi) + toCartesian(i_sizeTi * uvs, i_angleNorm);
    vec4 pos = p_mvpMat * vec4(posTi, 0.0, 1.0);
    pos.xy += vec2(i_instanceIndex & 1, i_instanceIndex >> 1) * 2.0;
    gl_Position = pos;
}
