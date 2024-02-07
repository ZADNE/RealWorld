add_subdirectory(shaders)

target_sources(RealWorld PRIVATE 
    ActivationManager.hpp       ActivationManager.cpp
    ActiveChunksSB.hpp          
    Chunk.hpp                   Chunk.cpp
    ChunkManager.hpp            ChunkManager.cpp
    VegManager.hpp              VegManager.cpp
    World.hpp                   World.cpp
)
