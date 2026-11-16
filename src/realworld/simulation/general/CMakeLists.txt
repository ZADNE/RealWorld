add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        ActionCmdBuf.cppm           
        ShaderMessageBroker.cppm    ShaderMessageBroker.cpp
)