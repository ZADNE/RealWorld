/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/buffers/BufferMapped.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/textures/ImageView.hpp>
#include <RealEngine/graphics/textures/TextureShaped.hpp>

#include <RealWorld/drawing/ExternalLight.hpp>

namespace rw {

/**
 * @brief Renders shadows of the world
 */
class ShadowDrawer {
public:
    ShadowDrawer(
        const glm::vec2&  viewSizePx,
        const glm::ivec2& viewSizeTi,
        glm::uint         maxNumberOfExternalLights
    );

    void setTarget(const re::Texture& worldTexture, const glm::ivec2& worldTexSize);

    void resizeView(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi);

    /**
     * @brief Analyzes the world texture
     */
    void analyze(const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftTi);

    /**
     * @brief Adds external light
     * @note External lights have to be added between analyze() and calculate()
     */
    void addExternalLight(const glm::ivec2& posPx, re::Color col);

    /**
     * @brief Calculates the shadows
     */
    void calculate(const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftPx);

    /**
     * @brief Renders calculated shadow to the framebuffer
     */
    void draw(
        const vk::CommandBuffer& commandBuffer,
        const glm::vec2&         botLeftPx,
        const glm::uvec2&        viewSizeTi
    );

private:
    re::TextureShaped m_blockLightAtlasTex{re::TextureSeed{"blockLightAtlas"}};
    re::TextureShaped m_wallLightAtlasTex{re::TextureSeed{"wallLightAtlas"}};

    re::PipelineLayout m_analysisPll;
    re::Pipeline       m_analyzeTilesPl;
    re::Pipeline       m_addLightsPl;

    re::PipelineLayout m_calculationPll;
    re::Pipeline       m_calculateShadowsPl;

    re::PipelineLayout m_shadowDrawingPll;
    re::Pipeline       m_drawShadowsPl;

    re::BufferMapped<ExternalLight> m_lightsBuf;

    struct AnalysisPC {
        glm::ivec2 worldTexMask;
        glm::ivec2 analysisOffsetTi;
        glm::ivec2 addLightOffsetPx;
        glm::uint  lightCount;
    };

    struct ShadowDrawingPC {
        glm::mat4  viewMat;
        glm::ivec2 viewSizeTi;
        glm::vec2  botLeftPxModTilePx;
        glm::ivec2 readOffsetTi;
    };

    struct ViewSizeDependent {
        ViewSizeDependent(
            const glm::vec2&          viewSizePx,
            const glm::ivec2&         viewSizeTi,
            const re::PipelineLayout& analysisPll,
            const re::PipelineLayout& calculationPll,
            const re::PipelineLayout& shadowDrawingPll,
            const re::Texture&        blockLightAtlasTex,
            const re::Texture&        wallLightAtlasTex,
            const re::Buffer&         lightsBuf
        );

        glm::uvec3 analysisGroupCount;
        glm::uvec3 calculationGroupCount;
        re::Texture lightTex; /**< RGB = color of the light, A = intensity of the light */
        re::ImageView     lightTexR32ImageView;
        re::Texture       transluTex; /**< R = translucency of the unit */
        re::Texture       shadowsTex;
        AnalysisPC        analysisPC;
        ShadowDrawingPC   shadowDrawingPC;
        re::DescriptorSet analysisDS;
        re::DescriptorSet calculationDS;
        re::DescriptorSet shadowDrawingDS;
    };

    ViewSizeDependent m_;

    static vk::ImageMemoryBarrier2 imageMemoryBarrier(
        vk::PipelineStageFlags2 srcStageMask,
        vk::AccessFlags2        srcAccessMask,
        vk::PipelineStageFlags2 dstStageMask,
        vk::AccessFlags2        dstAccessMask,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::Image               image
    );
};

} // namespace rw
