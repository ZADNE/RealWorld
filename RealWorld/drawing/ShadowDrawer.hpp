/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <vector>

#include <RealEngine/rendering/vertices/ShaderProgram.hpp>
#include <RealEngine/rendering/vertices/VertexArray.hpp>

#include <RealWorld/drawing/ExternalLight.hpp>
#include <RealWorld/shaders/drawing.hpp>
#include <RealWorld/reserved_units/buffers.hpp>

/**
 * @brief Renders shadows of the world
*/
template<RE::Renderer R>
class ShadowDrawer {
public:

    ShadowDrawer(const glm::uvec2& viewSizeTi);

    void resizeView(const glm::uvec2& viewSizeTi);

    void analyze(const glm::ivec2& botLeftTi);

    void addExternalLight(const glm::ivec2& posPx, RE::Color col);

    void calculate(const glm::ivec2& botLeftPx);

    void draw(const RE::VertexArray<R>& va, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi);

private:

    struct ViewSizeDependent {
        ViewSizeDependent(const glm::uvec2& viewSizeTi);

        RE::Texture<R> lightTex;//RGB = color of the light, A = intensity of the light
        RE::Texture<R> transluTex;//R = translucency of the unit
        RE::Texture<R> shadowsTex;
        glm::uvec3 analysisGroupCount;
        glm::uvec3 calcShadowsGroupCount;
    };

    ViewSizeDependent m_;

    RE::Texture<R> m_blockLightAtlasTex{{.file = "blockLightAtlas"}};
    RE::Texture<R> m_wallLightAtlasTex{{.file = "wallLightAtlas"}};

    RE::ShaderProgram<R> m_analysisShd{{.comp = analysis_comp}};
    RE::ShaderProgram<R> m_addLightsShd{{.comp = addDynamicLights_comp}};
    RE::ShaderProgram<R> m_calcShadowsShd{{.comp = calcShadows_comp}};
    RE::ShaderProgram<R> m_drawShadowsShd{{.vert = drawShadows_vert, .frag = colorDraw_frag}};

    std::vector<ExternalLight> m_lights;
    RE::BufferTyped<R> m_lightsBuf{ STRG_BUF_EXTERNALLIGHTS, sizeof(ExternalLight) * 8, RE::BufferAccessFrequency::DYNAMIC, RE::BufferAccessNature::DRAW, nullptr };
};
