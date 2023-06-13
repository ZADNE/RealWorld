add_subdirectory(shaders)

target_sources(RealWorld PRIVATE 
    BodySimulator.hpp           BodySimulator.cpp
    Chunk.hpp                   Chunk.cpp
    ChunkManager.hpp            ChunkManager.cpp
    ParticleSimulator.hpp       ParticleSimulator.cpp
    World.hpp                   World.cpp
)
