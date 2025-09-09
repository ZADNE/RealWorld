/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/buffers/BufferMapped.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/pipelines/Vertex.hpp>
#include <RealEngine/graphics/textures/ImageView.hpp>
#include <RealEngine/graphics/textures/TextureShaped.hpp>

#include <RealWorld/constants/Light.glsl.gen.hpp>
#include <RealWorld/constants/ResourceIndex.hpp>
#include <RealWorld/drawing/shaders/AnalysisPC.glsl.gen.hpp>
#include <RealWorld/drawing/shaders/ExternalLightsSB.glsl.gen.hpp>
#include <RealWorld/drawing/shaders/LightSweepPC.glsl.gen.hpp>
#include <RealWorld/drawing/shaders/WorldDrawingPC.glsl.gen.hpp>

namespace rw {

/**
 * @brief Calcualtes and renders shadows of the world
 */
class ShadowDrawer {
public:
    ShadowDrawer(
        re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx,
        glm::ivec2 viewSizeTi, glm::uint maxNumberOfExternalLights,
        glsl::WorldDrawingPC& pc
    );

    void setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize);

    void resizeView(glm::vec2 viewSizePx, glm::ivec2 viewSizeTi);

    /**
     * @brief Analyzes the world texture
     */
    void analyze(const re::CommandBuffer& cb, glm::ivec2 botLeftTi, glm::vec3 skyLight);

    /**
     * @brief Adds an external light
     * @note External lights have to be added between analyze() and calculate()
     */
    void addExternalLight(glm::ivec2 posPx, glm::vec3 light);

    /**
     * @brief Calculates the shadows
     */
    void calculate(const re::CommandBuffer& cb, glm::ivec2 botLeftPx);

    /**
     * @brief Renders calculated shadow to the framebuffer
     */
    void draw(const re::CommandBuffer& cb, glm::vec2 botLeftPx);

private:
    re::TextureShaped m_blockLightAtlasTex =
        re::RM::textureUnmanaged(textureID<"blockLightAtlas">());
    re::TextureShaped m_wallLightAtlasTex =
        re::RM::textureUnmanaged(textureID<"wallLightAtlas">());

    glsl::WorldDrawingPC& m_pc;

    re::PipelineLayout m_analysisPll;
    re::Pipeline m_analyzeTilesPl;
    re::Pipeline m_addExternalLightsPl;

    re::PipelineLayout m_lightSweepPll;
    re::Pipeline m_sweepLightPl;

    re::PipelineLayout m_shadowDrawingPll;
    re::Pipeline m_drawShadowsPl;

    re::BufferMapped<glsl::ExternalLight> m_lightsBuf;

    struct ViewSizeDependent {
        ViewSizeDependent(
            glm::vec2 viewSizePx, glm::ivec2 viewSizeTi,
            const re::PipelineLayout& analysisPll,
            const re::PipelineLayout& lightSweepPll,
            const re::PipelineLayout& shadowDrawingPll,
            const re::Texture& blockLightAtlasTex,
            const re::Texture& wallLightAtlasTex, const re::Buffer& lightsBuf
        );

        glm::vec2 viewSizePx;
        glm::uvec3 analysisGroupCount;
        glm::uvec3 lightSweepGroupCount;
        /**
         * @brief   Is the input texture for light sweep
         * @details It has two float16 channels in a texel and there are two
         *          array layers:
         *              1) r = red light intensity, g = green light intensity
         *              2) r = blue light intensity, g =  translucency
         *          The reason why four channel texture is not used is that it
         *          also must be accessed atomically so it must have 32 bits per
         *          texel.
         *          The texture has the same number of mipmaps as there are
         *          light-sweep cascades.
         */
        re::Texture lightXluTex;
        /**
         * @brief Per-mip R32Uint views of lightXluTex
         */
        std::array<re::ImageView, glsl::k_lightCellSizeCount> lightXluImgViews32ui;
        /**
         * @brief   Texture holding the calculated shadows.
         * @details This texture is rendered stretched over the whole viewport.
         */
        re::Texture shadowsTex;
        glsl::AnalysisPC analysisPC{};
        glsl::LightSweepPC lightSweepPC{};
        re::DescriptorSet calcInputsDS;
        re::DescriptorSet calculationDS;
        re::DescriptorSet shadowDrawingDS;
        glm::vec2 shadowAreaPxInv; ///< 1 over the area where shadows are calculated
    };

    ViewSizeDependent m_;
};

} // namespace rw
