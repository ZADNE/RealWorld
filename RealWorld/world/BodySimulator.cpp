/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/world/BodySimulator.hpp>

namespace rw {

constexpr int k_bodiesPerChunk = 16;

const re::Buffer& BodySimulator::adoptSave(
    const MetadataSave& save, const glm::ivec2& worldTexSizeCh
) {
    const glm::ivec2 maxChunksInRow = worldTexSizeCh - 1;
    auto maxBodyCount = k_bodiesPerChunk * maxChunksInRow.x * maxChunksInRow.y;

    m_bodiesBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(Body) * maxBodyCount,
        .usage       = vk::BufferUsageFlagBits::eStorageBuffer,
        .initData    = re::objectToByteSpan(BodiesSBHeader{
               .currentBodyCount = 0, .maxBodyCount = maxBodyCount})}};

    return *m_bodiesBuf;
}

} // namespace rw
