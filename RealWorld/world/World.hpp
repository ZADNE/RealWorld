﻿/*!
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

/**
 * @brief Represents the world as an endless grid of tiles.
 *
 * Also runs tile transformation and fluid dynamics simulation.
 */
class World {
public:

    enum class ModificationShape: uint32_t {
        Square,
        Disk,
        Fill
    };

    /**
     * @brief Initializes the world
     * @param The generator that will be used to generate new chunks
    */
    World(ChunkGenerator& chunkGen);

    /**
     * @copydoc ChunkHandler::numberOfInactiveChunks
    */
    size_t numberOfInactiveChunks();

    /**
     * @brief Performs layout transitions necessary to simulate the world
    */
    void beginStep(const vk::CommandBuffer& commandBuffer);

    /**
     * @brief Performs a simulation step of the world.
     * @param commandBuffer Command buffer that will be used to record computation commands
     * @param botLeftTi The most bottom-left tile that has to be active
     * @param topRightTi The most top-right tile that has to be active
     * @return The number of chunks that had to be activated this step
    */
    int step(const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

    /**
     * @brief Modifies tiles in the world
    */
    void modify(const vk::CommandBuffer& commandBuffer, TileLayer layer, ModificationShape shape, float radius, const glm::ivec2& posTi, const glm::uvec2& tile);

    /**
     * @brief Performs layout transitions necessary to draw the world
    */
    void endStep(const vk::CommandBuffer& commandBuffer);

    /**
     * @brief Sets this world class to simulate the world inside the given save
     * @param save Save of the world to run
     * @param activeChunksArea Size of the main texture that holds active chunks. Measured in chunks, must be multiples of 8
     * @returns The world texture
    */
    const RE::Texture& adoptSave(const MetadataSave& save, const glm::ivec2& activeChunksArea);

    void gatherSave(MetadataSave& save) const;

    bool saveChunks();

    void shouldPermuteOrder(bool should) { m_permuteOrder = should; }

private:

    void fluidDynamicsStep(const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

    std::optional<RE::Texture> m_worldTex;
    int m_seed = 0;

    std::string m_worldName;

    RE::Buffer m_tilePropertiesBuf;

    struct WorldDynamicsPC {
        glm::ivec2 globalPosTi;
        glm::uint modifyTarget;
        glm::uint modifyShape;
        glm::uvec2 modifySetValue;
        float modifyRadius;
        glm::uint timeHash;
        glm::uint updateOrder = 0b00011011'00011011'00011011'00011011;
    };
    WorldDynamicsPC m_worldDynamicsPC;

    RE::PipelineLayout m_pipelineLayout{
        {}, RE::PipelineComputeSources{
            .comp = transformTiles_comp
        }
    };
    RE::DescriptorSet m_descriptorSet{m_pipelineLayout, 0u};
    RE::Pipeline m_simulateFluidsPl{
        RE::PipelineComputeCreateInfo{.pipelineLayout = *m_pipelineLayout},
        RE::PipelineComputeSources{.comp = simulateFluids_comp}
    };
    RE::Pipeline m_transformTilesPl{
        RE::PipelineComputeCreateInfo{.pipelineLayout = *m_pipelineLayout},
        RE::PipelineComputeSources{.comp = transformTiles_comp}
    };
    RE::Pipeline m_modifyTilesPl{
        RE::PipelineComputeCreateInfo{.pipelineLayout = *m_pipelineLayout},
        RE::PipelineComputeSources{.comp = modifyTiles_comp}
    };

    ChunkManager m_chunkManager;

    bool m_permuteOrder = true;
};