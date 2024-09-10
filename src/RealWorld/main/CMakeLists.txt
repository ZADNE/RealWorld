add_subdirectory(settings)

real_target_sources(RealWorld PRIVATE
    MainMenuRoom.hpp            MainMenuRoom.cpp
    Room.hpp                    
    WorldRoom.hpp               WorldRoom.cpp
)
real_target_sources(RealWorld_Game PRIVATE
                                main.cpp
                                WorldRoom.Game.cpp
)
