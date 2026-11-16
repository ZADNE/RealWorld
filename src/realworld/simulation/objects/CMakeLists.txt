add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        Hitbox.cppm                 Hitbox.cpp
        Player.cppm                 Player.cpp
)
