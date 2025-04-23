add_subdirectory(settings)

real_target_sources(RealWorld
    PUBLIC
        Arguments.hpp               Arguments.cpp
        MainMenuRoom.hpp            MainMenuRoom.cpp
        Room.hpp                    
        WorldRoom.hpp               WorldRoom.cpp
)
real_target_sources(RealWorld_Game
    PRIVATE
                                    Main.cpp
                                    WorldRoom.Game.cpp
)
