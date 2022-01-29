add_subdirectory(FX)

target_sources(${PROJECT_NAME} PRIVATE 
	FDB.hpp					FDB.cpp
	FIndex.hpp				FIndex.cpp
	FurnitureCollection.hpp	
	FurnitureManager.hpp	FurnitureManager.cpp
	Quadtree.hpp
)
