add_subdirectory(shaders)

real_target_sources(RealWorld PRIVATE 
    Hitbox.hpp                  Hitbox.cpp
    Player.hpp                  Player.cpp
)
