add_subdirectory(settings)

target_sources(RealWorld PRIVATE 
    ActionCmdBuf.hpp            
    MainMenuRoom.hpp            MainMenuRoom.cpp
    Room.hpp                    
    WorldRoom.hpp               WorldRoom.cpp
)
target_sources(RealWorld_Game PRIVATE 
                                main.cpp
                                WorldRoom.Game.cpp
)
