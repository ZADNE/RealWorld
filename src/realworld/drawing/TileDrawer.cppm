/**
 *  @author    Dubsky Tomas
 */
export module realworld:drawing.TileDrawer;
import glm;
import realengine;
import realworld_constants;
import :drawing.shaders.WorldDrawingPC_glsl;

export namespace rw {

/**
 * @brief Renders tiles and minimap of the world
 */
class TileDrawer {
public:
    TileDrawer(
        re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx,
        glsl::WorldDrawingPC& pc
    );

    void setTarget(const re::Texture& worldTexture, glm::ivec2 worldTexSize, float seed);

    void resizeView(glm::vec2 viewSizePx);

    void drawTiles(const re::CommandBuffer& cb, glm::vec2 botLeftPx, float timeD);

    void drawMinimap(const re::CommandBuffer& cb);

private:
    re::TextureShaped m_blockAtlasTex =
        re::RM::textureUnmanaged(textureID<"blockAtlas">());
    re::TextureShaped m_wallAtlasTex =
        re::RM::textureUnmanaged(textureID<"wallAtlas">());

    glm::vec2 m_viewSizePx{};

    glsl::WorldDrawingPC& m_pc;

    re::PipelineLayout m_pipelineLayout;
    re::DescriptorSet m_descriptorSet{re::DescriptorSetCreateInfo{
        .layout    = m_pipelineLayout.descriptorSetLayout(0),
        .debugName = "rw::TileDrawer::descriptorSet"
    }};
    re::Pipeline m_drawTilesPl;

    // Minimap
    re::Pipeline m_drawMinimapPl;
};

} // namespace rw
