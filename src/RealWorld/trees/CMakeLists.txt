add_subdirectory(shaders)

target_sources(RealWorld PRIVATE 
    BodySimulator.hpp           BodySimulator.cpp
    ParticleSimulator.hpp       ParticleSimulator.cpp
    TreeSimulator.hpp           TreeSimulator.cpp
    TreesPoC.hpp                TreesPoC.cpp
)
