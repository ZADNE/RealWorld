add_subdirectory(shaders)

real_target_sources(RealWorld
    PUBLIC
        Hitbox.hpp                  Hitbox.cpp
        Player.hpp                  Player.cpp
)
