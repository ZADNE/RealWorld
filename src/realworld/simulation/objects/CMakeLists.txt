add_subdirectory(shaders)

re_target_sources(RealWorld
    PUBLIC
        Hitbox.cppm                 Hitbox.cpp
        Player.cppm                 Player.cpp
)
