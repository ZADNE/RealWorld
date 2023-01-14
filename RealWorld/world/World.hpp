/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <string>
#include <optional>

#include <RealEngine/rendering/batches/SpriteBatch.hpp>
#include <RealEngine/rendering/textures/Texture.hpp>

#include <RealWorld/constants/tile_properties/blockTransformationProperties.hpp>
#include <RealWorld/constants/tile_properties/blockTransformationRules.hpp>
#include <RealWorld/constants/tile_properties/wallTransformationProperties.hpp>
#include <RealWorld/constants/tile_properties/wallTransformationRules.hpp>
#include <RealWorld/world/ChunkManager.hpp>
#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/world/shaders/AllShaders.hpp>
#include <RealWorld/reserved_units/buffers.hpp>

enum class MODIFY_SHAPE : unsigned int {
    SQUARE,
    DISC,
    FILL
};

/**
 * @brief Represents the world as an endless grid of tiles.
 *
 * Also runs tile transformation and fluid dynamics simulation.
 */
class World {
public:

    /**
     * @brief Initializes the world
     * @param The generator that will be used to generate new chunks
    */
    World(ChunkGenerator& chunkGen);

    /**
     * @copydoc ChunkHandler::getNumberOfInactiveChunks
    */
    size_t getNumberOfInactiveChunks();

    /**
     * @brief Modifies tiles in the world
    */
    void modify(LAYER layer, MODIFY_SHAPE shape, float diameter, const glm::ivec2& posTi, const glm::uvec2& tile);

    /**
     * @brief Performs a simulation step of the world.
     * @param botLeftTi The most bottom-left tile that has to be active
     * @param topRightTi The most top-right tile that has to be active
     * @return The number of chunks that had to be activated this step
    */
    int step(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

    /**
     * @brief Sets this world class to simulate the world inside the given save
     * @param save Save of the world to run
     * @param activeChunksArea Size of the main texture that holds active chunks. Measured in chunks, must be multiples of 8
     * @returns The world texture
    */
    const RE::Texture& adoptSave(const MetadataSave& save, const glm::ivec2& activeChunksArea);

    void gatherSave(MetadataSave& save) const;

    bool saveChunks() const;

    void shouldPermuteOrder(bool should) { m_permuteOrder = should; }

private:

    struct DynamicsUniforms {
        glm::ivec2 globalPosTi;
        glm::uint modifyTarget;
        glm::uint modifyShape;
        glm::uvec2 modifySetValue;
        float modifyDiameter;
        glm::uint timeHash;
        glm::ivec2 updateOrder[16];
    };

    void fluidDynamicsStep(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

    RE::CommandBuffer m_commandBuffer{vk::CommandBufferLevel::ePrimary};
    std::optional<RE::Texture> m_worldTex;
    int m_seed = 0;

    std::string m_worldName;
    //RE::Buffer m_worldDynamicsBuf{UNIF_BUF_WORLDDYNAMICS, sizeof(WorldDynamicsUniforms), RE::BufferUsageFlags::MAP_WRITE};

    RE::Buffer m_tilePropertiesBuf;

    //RE::Pipeline m_simulateFluidsShd{simulateFluids_comp};
    //RE::Pipeline m_transformTilesShd{transformTiles_comp};
    //RE::Pipeline m_modifyTilesShd{modifyTiles_comp};

    std::array<glm::ivec2, 4> m_dynamicsUpdateOrder = {
        glm::ivec2{0, 0},
        glm::ivec2{1, 0},
        glm::ivec2{0, 1},
        glm::ivec2{1, 1}
    };
    uint32_t m_rngState;

    ChunkManager m_chunkManager;

    bool m_permuteOrder = true;
};