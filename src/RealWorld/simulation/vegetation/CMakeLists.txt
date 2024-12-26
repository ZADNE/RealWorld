add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        BodySimulator.hpp           BodySimulator.cpp
        BranchAllocRegSB.hpp        
        BranchRange.hpp             
        BranchSerialized.hpp        
        VegSimulator.hpp            VegSimulator.cpp
)
