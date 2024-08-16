/*!
 *  @author     Dubsky Tomas
 */
#ifndef DROPPED_TILES_SB_GLSL
#define DROPPED_TILES_SB_GLSL

const int k_maxDroppedTilesCount = 2047;

struct VkDrawIndirectCommand {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

struct VkDispatchIndirectCommand {
    uint x;
    uint y;
    uint z;
};

struct DroppedTile {
    vec2 botLeftPx;
    vec2 velPx;
    uint lifetimeSteps;
    uint layerTypeVar; // bits: 0..7 = variant, 8..15 = type, 16 = layer (block/wall)
};

layout (binding = k_droppedTilesBinding, std430)
restrict buffer DroppedTilesSB {
    VkDrawIndirectCommand     drawCommand;
    VkDispatchIndirectCommand dispatchCommand;
    uint                      padding;
    DroppedTile tiles[k_maxDroppedTilesCount];
} b_dropped;

const int k_moveTilesGroupSize    = 128;
const int k_moveTilesGroupSizeBit = 7;

uint roundUpBitshift(uint x, uint yShift) {
    return (x + (1 << yShift) - 1) >> yShift;
}

void adjustGroupCount(uint prevCount, uint added){
    uint before = roundUpBitshift(prevCount, k_moveTilesGroupSizeBit);
    uint after = roundUpBitshift(prevCount + added, k_moveTilesGroupSizeBit);
    uint diff = after - before;
    if (diff != 0) {
        atomicAdd(b_dropped.dispatchCommand.x, diff);
    }
}

/**
 * @brief Returns index where tiles should be put or -1 if space not available
 */
uint reserveSpaceForDroppedTiles(uint count){
    uint index = atomicAdd(b_dropped.drawCommand.vertexCount, count);
    if (index + count <= k_maxDroppedTilesCount) {
        adjustGroupCount(index, count);
        return index;
    } else {
        atomicAdd(b_dropped.drawCommand.vertexCount, -count);
        return -1;
    }
}

/**
 * @brief Returns index of the first tiles that was removed
 */
uint removeSpaceForDroppedTiles(uint count){
    uint index = atomicAdd(b_dropped.drawCommand.vertexCount, -count);
    adjustGroupCount(index, -count);
    return index - count;
}

#endif // !DROPPED_TILES_SB_GLSL