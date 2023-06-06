add_subdirectory(external_shaders)
add_subdirectory(shaders)

target_sources(RealWorld PRIVATE 
    ChunkGenerator.hpp          ChunkGenerator.cpp
    GenerationPC.hpp            
    TerrainGenerator.hpp        TerrainGenerator.cpp
    TreeGenerator.hpp           TreeGenerator.cpp
)
