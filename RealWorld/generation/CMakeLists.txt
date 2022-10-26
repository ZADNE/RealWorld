add_subdirectory(external_shaders)
add_subdirectory(shaders)

target_sources(RealWorld PRIVATE 
    ChunkGenerator.hpp          ChunkGenerator.cpp
    ChunkGeneratorCS.hpp        ChunkGeneratorCS.cpp
    ChunkGeneratorFBO.hpp       ChunkGeneratorFBO.cpp
)
