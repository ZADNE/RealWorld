layout(std140, binding = 0) restrict uniform _0WorldDynamicsUIB {
    ivec2 globalPosTi;
    uint modifyTarget;
    uint modifyShape;
    uvec2 modifySetValue;
    float modifyDiameter;
    uint timeHash;
    ivec2 updateOrder[16];
};
