/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <string>

#include <RealEngine/graphics/batches/SpriteBatch.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/constants/tile_properties/blockTransformationProperties.hpp>
#include <RealWorld/constants/tile_properties/blockTransformationRules.hpp>
#include <RealWorld/constants/tile_properties/wallTransformationProperties.hpp>
#include <RealWorld/constants/tile_properties/wallTransformationRules.hpp>
#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/simulation/objects/Hitbox.hpp>
#include <RealWorld/simulation/tiles/ChunkActivationMgr.hpp>
#include <RealWorld/simulation/tiles/shaders/AllShaders.hpp>
#include <RealWorld/simulation/tiles/shaders/WorldDynamicsPC_glsl.hpp>
#include <RealWorld/simulation/vegetation/BodySimulator.hpp>
#include <RealWorld/simulation/vegetation/VegSimulator.hpp>

namespace rw {

/**
 * @brief Represents the world as an endless grid of tiles.
 * @details Also runs tile transformation and fluid dynamics simulation.
 */
class World {
public:
    enum class ModificationShape : uint32_t {
        Square,
        Disk,
        Fill
    };

    /**
     * @brief Initializes the world
     */
    explicit World(const re::Buffer& shaderMessageBuf);

    /**
     * @copydoc ChunkActivationMgr::numberOfInactiveChunks
     */
    size_t numberOfInactiveChunks();

    /**
     * @brief Performs a simulation step of the world
     * @param acb        Command buffer that will be used to record the commands
     * @param botLeftTi  The most bottom-left tile that has to be active
     * @param topRightTi The most top-right tile that has to be active
     * @param player     Items home to this hitbox's position
     */
    void step(
        const ActionCmdBuf& acb, glm::ivec2 botLeftTi, glm::ivec2 topRightTi,
        const Hitbox& player
    );

    /**
     * @brief Places tiles into the world
     */
    void placeTiles(
        const ActionCmdBuf& acb, TileLayer layer, ModificationShape shape,
        float radius, glm::ivec2 posTi, glm::uvec2 tile, int maxCount
    );

    /**
     * @brief Mines tiles of the world
     */
    void mineTiles(
        const ActionCmdBuf& acb, TileLayer layer, ModificationShape shape,
        float radius, glm::ivec2 posTi
    );

    /**
     * @brief Performs layout transitions necessary to draw the world
     */
    void prepareWorldForDrawing(const ActionCmdBuf& acb);

    /**
     * @brief Sets this world class to simulate the world inside the given save
     * @param acb               Command buffer
     * @param save              A save of the world to run
     * @param worldTexSizeCh    Must be multiples of 8
     * @returns                 The new world texture
     */
    const re::Texture& adoptSave(
        ActionCmdBuf& acb, const MetadataSave& save, glm::ivec2 worldTexSizeCh
    );

    void gatherSave(MetadataSave& save) const;

    bool saveChunks(const ActionCmdBuf& acb);

private:
    void modifyTiles(
        const ActionCmdBuf& acb, TileLayer layer, ModificationShape shape,
        float radius, glm::ivec2 posTi, glm::uvec2 tile, int maxCount
    );

    void tileTransformationsStep(const ActionCmdBuf& acb);

    void fluidDynamicsStep(const ActionCmdBuf& acb);

    re::Texture m_worldTex;
    int m_seed = 0;

    std::string m_worldName;

    re::Buffer m_tilePropertiesBuf;

    glsl::WorldDynamicsPC m_worldDynamicsPC{
        .timeHash = static_cast<uint32_t>(time(nullptr)),
        .timeSec  = static_cast<float>(time(nullptr) & 0xFFFF)
    };

    re::PipelineLayout m_simulationPL;
    re::DescriptorSet m_simulationDS{re::DescriptorSetCreateInfo{
        .layout = m_simulationPL.descriptorSetLayout(0), .debugName = "rw::World::simulation"
    }};
    re::Pipeline m_simulateMovementPl{
        {.pipelineLayout = *m_simulationPL,
         .debugName      = "rw::World::simulateMovement"},
        {.comp = glsl::simulateMovement_comp}
    };
    re::Pipeline m_transformTilesPl{
        {.pipelineLayout = *m_simulationPL,
         .debugName      = "rw::World::transformTiles"},
        {.comp = glsl::transformTiles_comp}
    };
    re::Pipeline m_modifyTilesPl{
        {.pipelineLayout = *m_simulationPL, .debugName = "rw::World::modifyTiles"},
        {.comp = glsl::modifyTiles_comp}
    };

    ChunkActivationMgr m_chunkActivationMgr{m_simulationPL};
    const re::Buffer* m_activeChunksBuf = nullptr;
    BodySimulator m_bodySimulator{m_simulationPL};
    VegSimulator m_vegSimulator{};
};

} // namespace rw
