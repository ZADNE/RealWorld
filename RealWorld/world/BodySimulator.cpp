/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/world/BodySimulator.hpp>

namespace rw {

constexpr vk::DeviceSize k_bodiesPerChunk = 16;

const re::Buffer& BodySimulator::adoptSave(
    const MetadataSave& save, const glm::ivec2& worldTexSizeCh
) {
    const glm::ivec2 maxChunksInRow = worldTexSizeCh - 1;
    vk::DeviceSize   bufSize        = k_bodiesPerChunk * maxChunksInRow.x *
                             maxChunksInRow.y * sizeof(Body);

    m_bodiesBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = bufSize,
        .usage       = vk::BufferUsageFlagBits::eStorageBuffer}};

    return *m_bodiesBuf;
}

} // namespace rw
