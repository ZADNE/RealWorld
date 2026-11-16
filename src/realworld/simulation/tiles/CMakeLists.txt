add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        Chunk.cppm                  Chunk.cpp
        ChunkActivationMgr.cppm     ChunkActivationMgr.cpp
        ChunkTransferMgr.cppm       ChunkTransferMgr.cpp
        ChunkTransferStage.cppm     
        World.cppm                  World.cpp
)
