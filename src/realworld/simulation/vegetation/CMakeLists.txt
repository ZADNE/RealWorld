add_subdirectory(shaders)

re_target_sources(RealWorld
    PUBLIC
        BodySimulator.cppm          BodySimulator.cpp
        BranchAllocRegSB.cppm       
        BranchRange.cppm            
        BranchSerialized.cppm       
        VegSimulator.cppm           VegSimulator.cpp
)
