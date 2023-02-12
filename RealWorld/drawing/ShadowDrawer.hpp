/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <vector>

#include <glm/vec3.hpp>

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

    ShadowDrawer(const glm::uvec2& viewSizeTi, glm::uint maxNumberOfExternalLights);

    void resizeView(const glm::uvec2& viewSizeTi);

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
        const glm::ivec2& botLeftTi
    );

    void draw(const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi);

private:

    struct ViewSizeDependent {
        explicit ViewSizeDependent(const glm::uvec2& viewSizeTi);

        RE::Texture lightTex;//RGB = color of the light, A = intensity of the light
        RE::Texture transluTex;//R = translucency of the unit
        RE::Texture shadowsTex;
        glm::uvec3 analysisGroupCount;
        glm::uvec3 calculationGroupCount;

    private:
        ViewSizeDependent(const glm::uvec3& analysisGroupCount_, const glm::uvec3& calculationGroupCount_);
    };

    ViewSizeDependent m_;

    struct AnalysisPushConstants {
        glm::ivec2 worldTexMask;
        glm::ivec2 analysisOffsetTi;
        glm::ivec2 addLightOffsetPx;
        glm::uint lightCount;
    };

    RE::TextureShaped m_blockLightAtlasTex{{.file = "blockLightAtlas"}};
    RE::TextureShaped m_wallLightAtlasTex{{.file = "wallLightAtlas"}};

    RE::PipelineLayout m_analysisPll;
    RE::Pipeline m_analyzeTilesPl;
    RE::Pipeline m_addLightsPl;

    RE::PipelineLayout m_calculationPll;
    RE::Pipeline m_calculateShadowsPl;

    RE::PipelineLayout m_drawingPll;
    RE::Pipeline m_drawShadowsPl;

    std::vector<ExternalLight> m_lights;
    //RE::Buffer m_lightsBuf;
};
