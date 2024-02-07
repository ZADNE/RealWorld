/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>

namespace rw {

/**
 * @brief Ensures that vegetation is activated when needed
 */
class VegManager {
public:
    explicit VegManager(const re::PipelineLayout& pipelineLayout);

private:
    re::Pipeline m_saveVegetationPl;
};

} // namespace rw
