/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/DroppedTilesDrawer.hpp>

using enum vk::DescriptorType;
using enum vk::ImageLayout;

namespace rw {

DroppedTilesDrawer::DroppedTilesDrawer(
    re::RenderPassSubpass renderPassSubpass, const re::Buffer& droppedTilesBuf
)
    : m_droppedTilesBuf(droppedTilesBuf)
    , m_drawDroppedTilesPl([this, &renderPassSubpass]() {
        constexpr static std::array k_bindings =
            std::to_array<vk::VertexInputBindingDescription>({{
                0u,                          // Binding index
                sizeof(DroppedTile),         // Stride
                vk::VertexInputRate::eVertex // Input rate
            }});
        constexpr static std::array k_attributes =
            std::to_array<vk::VertexInputAttributeDescription>(
                {{
                     0u,                              // Location
                     0u,                              // Binding index
                     vk::Format::eR32G32Sfloat,       // Format
                     offsetof(DroppedTile, botLeftPx) // Relative offset
                 },
                 {
                     1u,                          // Location
                     0u,                          // Binding index
                     vk::Format::eR32G32Sfloat,   // Format
                     offsetof(DroppedTile, velPx) // Relative offset
                 },
                 {
                     2u,                                 // Location
                     0u,                                 // Binding index
                     vk::Format::eR32Uint,               // Format
                     offsetof(DroppedTile, layerTypeVar) // Relative offset
                 }}
            );
        vk::PipelineVertexInputStateCreateInfo vertexInput{{}, k_bindings, k_attributes};
        return re::Pipeline{
            re::PipelineGraphicsCreateInfo{
                .vertexInput       = &vertexInput,
                .topology          = vk::PrimitiveTopology::ePointList,
                .pipelineLayout    = *m_pipelineLayout,
                .renderPassSubpass = renderPassSubpass,
                .debugName         = "rw::DroppedTilesDrawer::drawDroppedTiles"
            },
            re::PipelineGraphicsSources{
                .vert = drawDroppedTiles_vert,
                .geom = drawDroppedTiles_geom,
                .frag = drawColor_frag
            }
        };
    }()) {
    m_descriptorSet.write(eCombinedImageSampler, 0, 0, *m_blockAtlasTex, eReadOnlyOptimal);
    m_descriptorSet.write(eCombinedImageSampler, 0, 1, *m_wallAtlasTex, eReadOnlyOptimal);
}

void DroppedTilesDrawer::draw(
    const re::CommandBuffer& cb, const glm::mat4& mvpMat, float timeSec,
    float interpFactor
) {
    DroppedTilesDrawingPC pc{
        .mvpMat       = mvpMat,
        .blinkState   = std::sin(timeSec * 6.0f) * 0.125f + 0.25f,
        .interpFactor = interpFactor
    };
    cb->pushConstants<DroppedTilesDrawingPC>(
        *m_pipelineLayout, vk::ShaderStageFlagBits::eGeometry, 0, {pc}
    );
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0,
        {*m_descriptorSet}, {}
    );
    cb->bindVertexBuffers(0, *m_droppedTilesBuf, {offsetof(DroppedTilesSB, tiles)});
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawDroppedTilesPl);
    cb->drawIndirect(
        *m_droppedTilesBuf, offsetof(DroppedTilesSBHeader, drawCommand), 1, 0
    );
}

} // namespace rw
