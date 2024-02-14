add_subdirectory(shaders)

target_sources(RealWorld PRIVATE
    BodySimulator.hpp           BodySimulator.cpp
    Branch.hpp                  
    BranchAllocRegSB.hpp        
    BranchAllocReqUB.hpp        
    BranchRange.hpp             
    BranchSB.hpp                
    ParticleSimulator.hpp       ParticleSimulator.cpp
    VegSimulator.hpp            VegSimulator.cpp
)
