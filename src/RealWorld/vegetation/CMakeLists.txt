add_subdirectory(shaders)

target_sources(RealWorld PRIVATE
    BodySimulator.hpp           BodySimulator.cpp
    BranchAllocRegSB.hpp        
    BranchAllocReqUB.hpp        
    BranchRange.hpp             
    BranchSB.hpp                
    ParticleSimulator.hpp       ParticleSimulator.cpp
    VegSimulator.hpp            VegSimulator.cpp
)
