add_subdirectory(external_shaders)
add_subdirectory(shaders)

re_target_sources(RealWorld
    PUBLIC
        ChunkGenerator.cppm         ChunkGenerator.Main.cpp
        ChunkGenerator.Terrain.cpp  ChunkGenerator.Veg.cpp
        VegTemplatesUB.cppm         
)
