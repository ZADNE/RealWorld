add_subdirectory(settings)

target_sources(RealWorld PRIVATE 
	MainMenuRoom.hpp		MainMenuRoom.cpp
	RealWorld.hpp			RealWorld.cpp
	Room.hpp				
	WorldRoom.hpp			WorldRoom.cpp
)
target_sources(RealWorld_Game PRIVATE 
							main.cpp
)
