/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec4   o_Color;

layout (location = 0) in  vec4   i_color;

void main() {
    o_Color = i_color;
}
