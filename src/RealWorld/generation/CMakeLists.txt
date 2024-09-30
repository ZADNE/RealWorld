add_subdirectory(external_shaders)
add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        ChunkGenerator.hpp          ChunkGeneratorMain.cpp
        ChunkGeneratorTerrain.cpp   ChunkGeneratorVeg.cpp
        VegPrepSB.hpp               
        VegTemplatesUB.hpp          
)
