add_subdirectory(shaders)

target_sources(RealWorld PRIVATE 
    DroppedTilesMgr.hpp         DroppedTilesMgr.cpp
    DroppedTilesSB.hpp          
    Hitbox.hpp                  Hitbox.cpp
    Player.hpp                  Player.cpp
)
