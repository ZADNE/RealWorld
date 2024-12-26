add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        ActionCmdBuf.hpp            
        ShaderMessageBroker.hpp     ShaderMessageBroker.cpp
)