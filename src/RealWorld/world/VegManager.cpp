/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/VegManager.hpp>
#include <RealWorld/world/shaders/AllShaders.hpp>

namespace rw {

VegManager::VegManager(const re::PipelineLayout& pipelineLayout)
    : m_saveVegetationPl(
          {.pipelineLayout = *pipelineLayout,
           .debugName      = "rw::ChunkManager::saveVegetation"},
          {.comp = saveVegetation_comp}
      ) {
}

} // namespace rw
