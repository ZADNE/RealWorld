add_subdirectory(settings)

re_target_sources(RealWorld
    PUBLIC
        Arguments.cppm              Arguments.cpp
        MainMenuRoom.cppm           MainMenuRoom.cpp
        Room.cppm                   
        WorldRoom.cppm              WorldRoom.cpp               WorldRoom.ImGui.cpp
)
re_target_sources(RealWorld_Game
    PRIVATE
                                    Main.cpp
                                    WorldRoom.Game.cpp
)
