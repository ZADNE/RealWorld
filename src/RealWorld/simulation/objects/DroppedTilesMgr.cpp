/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/simulation/objects/DroppedTilesMgr.hpp>
#include <RealWorld/simulation/objects/shaders/moveDroppedTiles_comp.hpp>

namespace rw {

DroppedTilesMgr::DroppedTilesMgr(const re::PipelineLayout& pipelineLayout)
    : m_pipelineLayout(pipelineLayout)
    , m_moveDroppedTilesPl(
          {.pipelineLayout = *pipelineLayout,
           .debugName      = "rw::DroppedTilesMgr::moveDroppedTilesPl"},
          {.comp = moveDroppedTiles_comp}
      ) {
}

void DroppedTilesMgr::step(
    const ActionCmdBuf& acb, const Hitbox& player, glm::ivec2 worldTexMaskTi,
    float timeSec
) {
    auto dbg = acb->createDebugRegion("rw::DroppedTilesMgr::step");
    acb.action([&](const re::CommandBuffer& cb) {
        cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_moveDroppedTilesPl);
        SimulationPC::DroppedTilesSim pc{
            .playerBotLeftPx = player.botLeftPx(),
            .playerDimsPx    = player.dimsPx(),
            .worldTexMaskTi  = worldTexMaskTi,
            .timeSec         = timeSec
        };
        cb->pushConstants<SimulationPC::DroppedTilesSim>(
            *m_pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, pc
        );
        cb->dispatchIndirect(
            *m_tilesBuf, offsetof(DroppedTilesSBHeader, dispatchCommand)
        );
    });
}

} // namespace rw
