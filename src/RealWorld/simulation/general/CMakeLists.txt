add_subdirectory(shaders)

target_sources(RealWorld PRIVATE
    ActionCmdBuf.hpp            
    ShaderMessageBroker.hpp     ShaderMessageBroker.cpp
)