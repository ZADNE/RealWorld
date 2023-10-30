/*!
 *  @author    Dubsky Tomas
 */
#version 460
layout (triangle_strip, max_vertices = 12) out;
layout (location = 0) out vec2 o_uv;
layout (location = 1) out uint o_branchIndex0wallType16;

layout (triangles) in;
layout (location = 0) in vec2 i_uv[];
layout (location = 1) in uint i_branchIndex0wallType16[];

void replicationPass(vec2 offset){
    for (int i = 0; i < 3; ++i){
        vec4 pos = gl_in[i].gl_Position;
        pos.xy += offset;
        gl_Position = pos;
        o_branchIndex0wallType16 = i_branchIndex0wallType16[0];
        o_uv = i_uv[i];
        EmitVertex();
    }
    EndPrimitive();
}

void main() {
    // The main copy (with analysis)
    vec2 offset = vec2(0.0);
    for (int i = 0; i < 3; ++i){
        const vec4 pos = gl_in[i].gl_Position;
        offset = mix( // Remember is the vertex was outside range
            offset,
            vec2(greaterThan(abs(pos.xy), vec2(1.0))) * sign(pos.xy),
            equal(offset, vec2(0.0))
        );
        gl_Position = pos;
        o_branchIndex0wallType16 = i_branchIndex0wallType16[0];
        o_uv = i_uv[i];
        EmitVertex();
    }
    EndPrimitive();
    offset *= -2.0;

    if (offset.x != 0.0){ // Horizontal replication
        replicationPass(vec2(offset.x, 0.0));
    }

    if (offset.y != 0.0){ // Vertical replication
        replicationPass(vec2(0.0, offset.y));
    }

    if (offset.x != 0.0 && offset.y != 0.0){ // Diagonal replication
        replicationPass(vec2(offset.x, offset.y));
    }
}
