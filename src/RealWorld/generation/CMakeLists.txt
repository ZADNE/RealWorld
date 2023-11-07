add_subdirectory(external_shaders)
add_subdirectory(shaders)

target_sources(RealWorld PRIVATE 
    ChunkGenerator.hpp          ChunkGeneratorMain.cpp
    ChunkGeneratorTerrain.cpp   ChunkGeneratorVeg.cpp
    VegPreparationSB.hpp        
)
