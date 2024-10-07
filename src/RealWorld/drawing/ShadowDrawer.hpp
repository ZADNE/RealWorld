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
#include <RealWorld/drawing/WorldDrawingPC.hpp>

namespace rw {

/**
 * @brief Calcualtes and renders shadows of the world
 */
class ShadowDrawer {
public:
    ShadowDrawer(
        re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx,
        glm::ivec2 viewSizeTi, glm::uint maxNumberOfExternalLights,
        WorldDrawingPC& pc
    );

    void setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize);

    void resizeView(glm::vec2 viewSizePx, glm::ivec2 viewSizeTi);

    /**
     * @brief Analyzes the world texture
     */
    void analyze(
        const re::CommandBuffer& cb, glm::ivec2 botLeftTi, const glm::vec4& skyLight
    );

    /**
     * @brief Adds external light
     * @note External lights have to be added between analyze() and calculate()
     */
    void addExternalLight(glm::ivec2 posPx, re::Color col);

    /**
     * @brief Calculates the shadows
     */
    void calculate(const re::CommandBuffer& cb, glm::ivec2 botLeftPx);

    /**
     * @brief Renders calculated shadow to the framebuffer
     */
    void draw(const re::CommandBuffer& cb, glm::vec2 botLeftPx);

private:
    re::TextureShaped m_blockLightAtlasTex{re::TextureSeed{"blockLightAtlas"}};
    re::TextureShaped m_wallLightAtlasTex{re::TextureSeed{"wallLightAtlas"}};

    WorldDrawingPC& m_pc;

    re::PipelineLayout m_calcInputsPll;
    re::Pipeline m_analyzeTilesPl;
    re::Pipeline m_addLightsPl;

    re::PipelineLayout m_calculationPll;
    re::Pipeline m_calculateShadowsPl;

    re::PipelineLayout m_shadowDrawingPll;
    re::Pipeline m_drawShadowsPl;

    re::BufferMapped<ExternalLight> m_lightsBuf;

    struct AnalysisPC {
        glm::vec4 skyLight;
        glm::ivec2 worldTexMask;
        glm::ivec2 analysisOffsetTi;
        glm::ivec2 addLightOffsetPx;
        glm::uint lightCount;
    };

    struct ViewSizeDependent {
        ViewSizeDependent(
            glm::vec2 viewSizePx, glm::ivec2 viewSizeTi,
            const re::PipelineLayout& shadowInputsPll,
            const re::PipelineLayout& calculationPll,
            const re::PipelineLayout& shadowDrawingPll,
            const re::Texture& blockLightAtlasTex,
            const re::Texture& wallLightAtlasTex, const re::Buffer& lightsBuf
        );

        glm::vec2 viewSizePx;
        glm::uvec3 analysisGroupCount;
        glm::uvec3 calculationGroupCount;
        re::Texture lightTex; ///< RGB = color of the light, A = intensity of the light
        re::ImageView lightTexR32ImageView;
        re::Texture transluTex; ///< R = translucency of the unit
        re::Texture shadowsTex;
        AnalysisPC analysisPC{};
        re::DescriptorSet calcInputsDS;
        re::DescriptorSet calculationDS;
        re::DescriptorSet shadowDrawingDS;
        glm::vec2 shadowAreaPxInv; ///< 1 over the area where shadows are calculated
    };

    ViewSizeDependent m_;
};

} // namespace rw
