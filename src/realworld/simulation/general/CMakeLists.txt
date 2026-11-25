add_subdirectory(shaders)

re_target_sources(RealWorld
    PUBLIC
        ActionCmdBuf.cppm           
        ShaderMessageBroker.cppm    ShaderMessageBroker.cpp
)