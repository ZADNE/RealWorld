layout(std140, binding = 0) uniform PlayerMovementUIB {
    float acceleration;
    float maxWalkVelocity;
    float jumpVelocity;
    float walkDirection;
    vec2 jump_autojump;
};
