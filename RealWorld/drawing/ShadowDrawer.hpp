/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>

#include <RealEngine/rendering/descriptors/DescriptorSet.hpp>
#include <RealEngine/rendering/pipelines/PipelineLayout.hpp>
#include <RealEngine/rendering/textures/TextureShaped.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>

#include <RealWorld/drawing/ExternalLight.hpp>

 /**
  * @brief Renders shadows of the world
 */
class ShadowDrawer {
public:

    ShadowDrawer(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi, glm::uint maxNumberOfExternalLights);

    void setTarget(const RE::Texture& worldTexture, const glm::ivec2& worldTexSize);

    void resizeView(const glm::vec2& viewSizePx, const glm::ivec2& viewSizeTi);

    /**
     * @brief Analyzes the world texture
    */
    void analyze(
        const vk::CommandBuffer& commandBuffer,
        const glm::ivec2& botLeftTi
    );

    /**
     * @brief Adds external light
     * @note External lights have to be added between analyze() and calculate()
    */
    void addExternalLight(const glm::ivec2& posPx, RE::Color col);

    /**
     * @brief Calculates the shadows
    */
    void calculate(
        const vk::CommandBuffer& commandBuffer,
        const glm::ivec2& botLeftPx
    );

    /**
     * @brief Renders calculated shadow to the framebuffer
    */
    void draw(
        const vk::CommandBuffer& commandBuffer,
        const glm::vec2& botLeftPx,
        const glm::uvec2& viewSizeTi
    );

private:

    RE::TextureShaped m_blockLightAtlasTex{{.file = "blockLightAtlas"}};
    RE::TextureShaped m_wallLightAtlasTex{{.file = "wallLightAtlas"}};

    RE::PipelineLayout m_analysisPll;
    RE::Pipeline m_analyzeTilesPl;
    RE::Pipeline m_addLightsPl;

    RE::PipelineLayout m_calculationPll;
    RE::Pipeline m_calculateShadowsPl;

    RE::PipelineLayout m_shadowDrawingPll;
    RE::Pipeline m_drawShadowsPl;

    RE::Buffer m_lightsBuf;
    ExternalLight* m_lightsMapped;

    struct AnalysisPC {
        glm::ivec2  worldTexMask;
        glm::ivec2  analysisOffsetTi;
        glm::ivec2  addLightOffsetPx;
        glm::uint   lightCount;
    };

    struct ShadowDrawingPC {
        glm::mat4    viewMat;
        glm::ivec2   viewSizeTi;
        glm::vec2    botLeftPxModTilePx;
        glm::ivec2   readOffsetTi;
    };

    struct ViewSizeDependent {
        ViewSizeDependent(
            const glm::vec2& viewSizePx,
            const glm::ivec2& viewSizeTi,
            const RE::PipelineLayout& analysisPll,
            const RE::PipelineLayout& calculationPll,
            const RE::PipelineLayout& shadowDrawingPll,
            const RE::Texture& blockLightAtlasTex,
            const RE::Texture& wallLightAtlasTex,
            const RE::Buffer& lightsBuf
        );

        glm::uvec3 analysisGroupCount;
        glm::uvec3 calculationGroupCount;
        RE::Texture lightTex;//RGB = color of the light, A = intensity of the light
        RE::Texture transluTex;//R = translucency of the unit
        RE::Texture shadowsTex;
        AnalysisPC analysisPC;
        ShadowDrawingPC shadowDrawingPC;
        RE::DescriptorSet analysisDS;
        RE::DescriptorSet calculationDS;
        RE::DescriptorSet shadowDrawingDS;

    };

    ViewSizeDependent m_;
};
