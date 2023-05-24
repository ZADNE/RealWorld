#extension GL_EXT_scalar_block_layout : require
layout(std430, push_constant) uniform PlayerMovementPC {
    float acceleration;
    float maxWalkVelocity;
    float jumpVelocity;
    float walkDirection;
    vec2 jump_autojump;
};
