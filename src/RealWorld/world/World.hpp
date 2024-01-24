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
#include <RealWorld/vegetation/BodySimulator.hpp>
#include <RealWorld/vegetation/VegSimulator.hpp>
#include <RealWorld/world/ChunkManager.hpp>

namespace rw {

/**
 * @brief Represents the world as an endless grid of tiles.
 *
 * Also runs tile transformation and fluid dynamics simulation.
 */
class World {
public:
    enum class ModificationShape : uint32_t { Square, Disk, Fill };

    /**
     * @brief Initializes the world
     */
    World();

    /**
     * @copydoc ChunkHandler::numberOfInactiveChunks
     */
    size_t numberOfInactiveChunks();

    /**
     * @brief                   Performs a simulation step of the world.
     * @param cmdBuf     Command buffer that will be used to record
     * computation commands
     * @param botLeftTi         The most bottom-left tile that has to be active
     * @param topRightTi        The most top-right tile that has to be active
     * @return                  The number of chunks that had to be activated
     * this step
     */
    int step(const re::CommandBuffer& cmdBuf, glm::ivec2 botLeftTi, glm::ivec2 topRightTi);

    /**
     * @brief Modifies tiles in the world
     */
    void modify(
        const re::CommandBuffer& cmdBuf,
        TileLayer                layer,
        ModificationShape        shape,
        float                    radius,
        glm::ivec2               posTi,
        glm::uvec2               tile
    );

    /**
     * @brief Performs layout transitions necessary to draw the world
     */
    void prepareWorldForDrawing(const re::CommandBuffer& cmdBuf);

    /**
     * @brief   Sets this world class to simulate the world inside the given save
     * @param save          Save of the world to run
     * @param worldTexSize  Must be multiples of 8
     * @returns             The new world texture
     */
    const re::Texture& adoptSave(const MetadataSave& save, glm::ivec2 worldTexSizeCh);

    void gatherSave(MetadataSave& save) const;

    bool saveChunks();

    void shouldPermuteOrder(bool should) { m_permuteOrder = should; }

private:
    void fluidDynamicsStep(
        const re::CommandBuffer& cmdBuf, glm::ivec2 botLeftTi, glm::ivec2 topRightTi
    );
    void tileTransformationsStep(const re::CommandBuffer& cmdBuf, int activatedChunkCount);

    re::Texture m_worldTex;
    int         m_seed = 0;

    std::string m_worldName;

    re::Buffer m_tilePropertiesBuf;

    struct WorldDynamicsPC {
        glm::ivec2 globalPosTi;
        glm::uint  modifyTarget;
        glm::uint  modifyShape;
        glm::uvec2 modifySetValue;
        float      modifyRadius;
        glm::uint  timeHash;
        glm::uint  updateOrder = 0b00011011'00011011'00011011'00011011;
    } m_worldDynamicsPC;

    re::PipelineLayout m_simulationPL;
    re::DescriptorSet  m_simulationDS{m_simulationPL.descriptorSetLayout(0)};
    re::Pipeline       m_simulateFluidsPl{
              {.pipelineLayout = *m_simulationPL,
               .debugName      = "rw::World::simulateFluids"},
              {.comp = simulateFluids_comp}};
    re::Pipeline m_transformTilesPl{
        {.pipelineLayout = *m_simulationPL,
         .debugName      = "rw::World::transformTiles"},
        {.comp = transformTiles_comp}};
    re::Pipeline m_modifyTilesPl{
        {.pipelineLayout = *m_simulationPL, .debugName = "rw::World::modifyTiles"},
        {.comp = modifyTiles_comp}};

    ChunkManager      m_chunkManager{m_simulationPL};
    const re::Buffer* m_activeChunksBuf = nullptr;

    BodySimulator m_bodySimulator{m_simulationPL};
    VegSimulator  m_vegSimulator{};

    bool m_permuteOrder = true;
};

} // namespace rw
