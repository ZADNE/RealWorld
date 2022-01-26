add_subdirectory(physics)

target_sources(${PROJECT_NAME} PRIVATE 
	DestroyTicket.hpp		
	DynamicLight.hpp		
	Chunk.hpp				Chunk.cpp
	ChunkGenerator.hpp		ChunkGenerator.cpp
	ChunkLoader.hpp			ChunkLoader.cpp
	ChunkManager.hpp		ChunkManager.cpp
	LightManipulator.hpp	LightManipulator.cpp
	StaticLight.hpp			
	TDB.hpp					TDB.cpp
	World.hpp				World.cpp
	WorldCreator.hpp		WorldCreator.cpp
	WorldData.hpp			
	WorldDataLoader.hpp		WorldDataLoader.cpp
	WorldDrawer.hpp			WorldDrawer.cpp
	WorldInfo.hpp			
)
