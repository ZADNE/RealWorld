add_subdirectory(shaders)

real_target_sources(RealWorld PRIVATE
    ActionCmdBuf.hpp            
    ShaderMessageBroker.hpp     ShaderMessageBroker.cpp
    SimulationPC.hpp            
)