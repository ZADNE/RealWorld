/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <string>

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
template<RE::Renderer R>
class World {
public:

    /**
     * @brief Initializes the world
     * @param The generator that will be used to generate new chunks
    */
    World(ChunkGenerator<R>& chunkGen);

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
     * @param activeChunksArea Size of the main texture that holds active chunks. Measured in chunks, must be multiples of 8.
    */
    void adoptSave(const MetadataSave& save, const glm::ivec2& activeChunksArea);

    void gatherSave(MetadataSave& save) const;

    bool saveChunks() const;

    void shouldPermuteOrder(bool should) { m_permuteOrder = should; }

private:

    void fluidDynamicsStep(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

    RE::Texture<R> m_worldTex;
    int m_seed = 0;

    std::string m_worldName;

    struct WorldDynamicsUBO {
        glm::ivec2 globalPosTi;
        glm::uint modifyTarget;
        glm::uint modifyShape;
        glm::uvec2 modifySetValue;
        float modifyDiameter;
        glm::uint timeHash;
        glm::ivec4 updateOrder[16];//Only the first two components are valid, the other two are padding required for std140 layout
    };
    RE::BufferTyped<R> m_worldDynamicsBuf{UNIF_BUF_WORLDDYNAMICS, sizeof(WorldDynamicsUBO), RE::BufferUsageFlags::MAP_WRITE};

    struct TilePropertiesUIB {
        //x = properties
        //yz = indices of first and last transformation rule
        std::array<glm::uvec4, 256> blockTransformationProperties;
        std::array<glm::uvec4, 256> wallTransformationProperties;

        //x = The properties that neighbors MUST have to transform
        //y = The properties that neighbors MUST NOT have to transform
        //z = Properties of the transformation
        //w = The wall that it will be transformed into
        std::array<glm::uvec4, 16> blockTransformationRules;
        std::array<glm::uvec4, 16> wallTransformationRules;
    };
    RE::BufferTyped<R> m_tilePropertiesBuf{UNIF_BUF_TILEPROPERTIES, RE::BufferUsageFlags::NO_FLAGS, TilePropertiesUIB{
        .blockTransformationProperties = BLOCK_TRANSFORMATION_PROPERTIES,
        .wallTransformationProperties = WALL_TRANSFORMATION_PROPERTIES,
        .blockTransformationRules = BLOCK_TRANSFORMATION_RULES,
        .wallTransformationRules = WALL_TRANSFORMATION_RULES
    }};

    RE::ShaderProgram<R> m_simulateFluidsShd{{.comp = simulateFluids_comp}};
    RE::ShaderProgram<R> m_transformTilesShd{{.comp = transformTiles_comp}};
    RE::ShaderProgram<R> m_modifyTilesShd{{.comp = modifyTiles_comp}};

    std::array<glm::ivec4, 4> m_dynamicsUpdateOrder = {glm::ivec4{0, 0, 0, 0}, glm::ivec4{1, 0, 1, 0}, glm::ivec4{0, 1, 0, 1}, glm::ivec4{1, 1, 1, 1}};
    uint32_t m_rngState;

    ChunkManager<R> m_chunkManager;

    bool m_permuteOrder = true;
};