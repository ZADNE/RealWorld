add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        BodySimulator.hpp           BodySimulator.cpp
        BranchAllocRegSB.hpp        
        BranchAllocReqUB.hpp        
        BranchRange.hpp             
        BranchSB.hpp                
        VegSimulator.hpp            VegSimulator.cpp
)
