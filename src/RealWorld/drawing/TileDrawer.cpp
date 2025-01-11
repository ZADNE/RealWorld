/**
 *  @author    Dubsky Tomas
 */
#define GLM_FORCE_SWIZZLE // Used in shaders in generateStructure.glsl
#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/drawing/MinimapLayout.hpp>
#include <RealWorld/drawing/TileDrawer.hpp>
#include <RealWorld/drawing/shaders/AllShaders.hpp>
#include <RealWorld/generation/shaders/generateStructure_glsl.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;

namespace rw {

constexpr glm::vec3 k_cold{0.2625, 0.851, 0.952};
constexpr glm::vec3 k_normal{0.25411764705, 0.7025490196, 0.90470588235};
constexpr glm::vec3 k_hot{0.2895, 0.698, 0.7583};

constexpr std::array<glm::vec3, 3> k_skyColors{k_cold, k_normal, k_hot};

glm::vec3 skyColor(float biomeTemp) {
    // Calculate coords
    biomeTemp = glm::fract(biomeTemp);
    biomeTemp *= 2.0f;
    int ll     = static_cast<int>(biomeTemp);
    float frac = glm::fract(biomeTemp);

    // Gather
    glm::vec3 b00 = k_skyColors[ll];
    glm::vec3 b01 = k_skyColors[ll + 1];

    // Interpolate
    return glm::mix(b00, b01, frac);
}

TileDrawer::TileDrawer(
    re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx,
    glsl::WorldDrawingPC& pc
)
    : m_pc(pc)
    , m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0u, eCombinedImageSampler, 1u, eFragment}, // worldTexture
                  {1u, eCombinedImageSampler, 1u, eFragment}, // blockAtlas
                  {2u, eCombinedImageSampler, 1u, eFragment}  // wallAtlas
              }},
              .ranges   = {vk::PushConstantRange{
                  eVertex | eFragment, 0u, sizeof(glsl::WorldDrawingPC)
              }}
          }
      )
    , m_drawTilesPl(
          re::PipelineGraphicsCreateInfo{
              .topology          = vk::PrimitiveTopology::eTriangleStrip,
              .enableBlend       = false,
              .pipelineLayout    = *m_pipelineLayout,
              .renderPassSubpass = renderPassSubpass,
              .debugName         = "rw::TileDrawer::drawTiles"
          },
          re::PipelineGraphicsSources{
              .vert = glsl::drawFullscreen_vert, .frag = glsl::drawTiles_frag
          }
      )
    , m_drawMinimapPl(
          re::PipelineGraphicsCreateInfo{
              .topology          = vk::PrimitiveTopology::eTriangleStrip,
              .pipelineLayout    = *m_pipelineLayout,
              .renderPassSubpass = renderPassSubpass,
              .debugName         = "rw::TileDrawer::drawMinimap"
          },
          re::PipelineGraphicsSources{
              .vert = glsl::drawMinimap_vert, .frag = glsl::drawMinimap_frag
          }
      ) {
    m_descriptorSet.write(
        eCombinedImageSampler, 1u, 0u, m_blockAtlasTex, eShaderReadOnlyOptimal
    );
    m_descriptorSet.write(
        eCombinedImageSampler, 2u, 0u, m_wallAtlasTex, eShaderReadOnlyOptimal
    );

    resizeView(viewSizePx);
}

void TileDrawer::setTarget(
    const re::Texture& worldTexture, glm::ivec2 worldTexSize, float seed
) {
    m_descriptorSet.write(
        eCombinedImageSampler, 0u, 0u, worldTexture, eShaderReadOnlyOptimal
    );
    m_pc.worldTexMask = worldTexSize - 1;
    glm::vec2 viewSizePx =
        glm::vec2(2.0f, 2.0f) /
        glm::vec2(m_pc.minimapViewMat[0][0], m_pc.minimapViewMat[1][1]);
    resizeView(viewSizePx);
    m_seed = seed;
}

void TileDrawer::resizeView(glm::vec2 viewSizePx) {
    m_pc.minimapViewMat = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y);
    m_viewSizePx        = viewSizePx;

    auto layout        = minimapLayout(m_pc.worldTexMask + 1, viewSizePx);
    m_pc.minimapOffset = layout.offsetPx;
    m_pc.minimapSize   = layout.sizePx;
}

void TileDrawer::drawTiles(
    const re::CommandBuffer& cb, glm::vec2 botLeftPx, float skyLight
) {
    m_pc.uvRectSize   = m_viewSizePx;
    m_pc.uvRectOffset = glm::mod(botLeftPx, TilePx);
    m_pc.botLeftTi    = glm::ivec2(pxToTi(botLeftPx));
    m_pc.skyColor     = glm::vec4(
        skyColor(
            glsl::calcBiomeClimate(botLeftPx.x + m_viewSizePx.x * 0.5f, m_seed).x
        ) * skyLight,
        1.0
    );
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u,
        *m_descriptorSet, {}
    );
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawTilesPl);
    cb->pushConstants<glsl::WorldDrawingPC>(
        *m_pipelineLayout, eVertex | eFragment, 0u, m_pc
    );
    cb->draw(3u, 1u, 0u, 0u);
}

void TileDrawer::drawMinimap(const re::CommandBuffer& cb) {
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u,
        *m_descriptorSet, {}
    );
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawMinimapPl);
    cb->pushConstants<glsl::WorldDrawingPC>(
        *m_pipelineLayout, eVertex | eFragment, 0u, m_pc
    );
    cb->draw(4u, 1u, 0u, 0u);
}

} // namespace rw
