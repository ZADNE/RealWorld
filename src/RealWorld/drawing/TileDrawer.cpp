/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/drawing/MinimapLayout.hpp>
#include <RealWorld/drawing/TileDrawer.hpp>
#include <RealWorld/drawing/shaders/AllShaders.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;

namespace rw {

// NOLINTBEGIN : This is taken from a shader
float hash12(glm::vec2 p) {
    glm::vec3 p3 = fract(glm::vec3(p.x, p.y, p.x) * .1031f);
    p3 += glm::dot(p3, glm::vec3(p3.y, p3.z, p3.x) + 33.33f);
    return glm::fract((p3.x + p3.y) * p3.z);
}

// x = left val, y = right val, z = 0 to 1 interp. factor between the two
glm::vec3 columnValues(float x, float seed) {
    float columnX     = floor(x);
    float columnFract = glm::fract(x);
    float a           = hash12(glm::vec2(columnX, seed));
    float b           = hash12(glm::vec2(columnX + 1.0f, seed));
    return glm::vec3(a, b, columnFract);
}

float linColumnValue(float x, float seed) {
    glm::vec3 vals = columnValues(x, seed);
    return glm::mix(vals.x, vals.y, vals.z);
}

float biomeTemp(float xPx, float seed) {
    float res = 0.0f;
    float x   = xPx * (1.0f / 8192.0f);
    float amp = 0.5f;

    for (int i = 0; i < 3; ++i) {
        res += linColumnValue(x, seed + 11.0f) * amp;
        x *= 2.0f;
        amp *= 0.5f;
    }

    return res;
}

constexpr glm::vec3 k_cold{0.2625, 0.851, 0.952};
constexpr glm::vec3 k_normal{0.25411764705, 0.7025490196, 0.90470588235};
constexpr glm::vec3 k_hot{0.2895, 0.698, 0.7583};

constexpr glm::vec3 k_color[3] = {k_cold, k_normal, k_hot};

glm::vec3 bcColor(float biomeTemp) {
    // Calculate coords
    biomeTemp = glm::fract(biomeTemp);
    biomeTemp *= 2.0f;
    int ll     = biomeTemp;
    float frac = glm::fract(biomeTemp);

    // Gather
    glm::vec3 b00 = k_color[ll];
    glm::vec3 b01 = k_color[ll + 1];

    // Interpolate
    return glm::mix(b00, b01, frac);
}
// NOLINTEND

TileDrawer::TileDrawer(
    re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx, WorldDrawingPC& pc
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
                  eVertex | eFragment, 0u, sizeof(WorldDrawingPC)
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
          re::PipelineGraphicsSources{.vert = drawFullscreen_vert, .frag = drawTiles_frag}
      )
    , m_drawMinimapPl(
          re::PipelineGraphicsCreateInfo{
              .topology          = vk::PrimitiveTopology::eTriangleStrip,
              .pipelineLayout    = *m_pipelineLayout,
              .renderPassSubpass = renderPassSubpass,
              .debugName         = "rw::TileDrawer::drawMinimap"
          },
          re::PipelineGraphicsSources{.vert = drawMinimap_vert, .frag = drawMinimap_frag}
      ) {
    m_descriptorSet.write(
        eCombinedImageSampler, 1u, 0u, *m_blockAtlasTex, eShaderReadOnlyOptimal
    );
    m_descriptorSet.write(
        eCombinedImageSampler, 2u, 0u, *m_wallAtlasTex, eShaderReadOnlyOptimal
    );

    resizeView(viewSizePx);
}

void TileDrawer::setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize) {
    m_descriptorSet.write(
        eCombinedImageSampler, 0u, 0u, worldTexture, eShaderReadOnlyOptimal
    );
    m_pc.worldTexMask = worldTexSize - 1;
    glm::vec2 viewSizePx =
        glm::vec2(2.0f, 2.0f) /
        glm::vec2(m_pc.minimapViewMat[0][0], m_pc.minimapViewMat[1][1]);
    resizeView(viewSizePx);
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
        bcColor(biomeTemp(botLeftPx.x + m_viewSizePx.x * 0.5f, 1.0f)) * skyLight, 1.0
    );
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u,
        *m_descriptorSet, {}
    );
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawTilesPl);
    cb->pushConstants<WorldDrawingPC>(*m_pipelineLayout, eVertex | eFragment, 0u, m_pc);
    cb->draw(3u, 1u, 0u, 0u);
}

void TileDrawer::drawMinimap(const re::CommandBuffer& cb) {
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u,
        *m_descriptorSet, {}
    );
    cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawMinimapPl);
    cb->pushConstants<WorldDrawingPC>(*m_pipelineLayout, eVertex | eFragment, 0u, m_pc);
    cb->draw(4u, 1u, 0u, 0u);
}

} // namespace rw
