add_subdirectory(shaders)

real_target_sources(RealWorld PRIVATE
    ActiveChunksSB.hpp          
    Chunk.hpp                   Chunk.cpp
    ChunkActivationMgr.hpp      ChunkActivationMgr.cpp
    ChunkTransferMgr.hpp        ChunkTransferMgr.cpp
    ChunkTransferStage.hpp      
    World.hpp                   World.cpp
)
