add_subdirectory(FX)

target_sources(${PROJECT_NAME} PRIVATE 
			FDB.hpp					FDB.cpp
			FIndex.hpp				FIndex.cpp
			FurnitureCollection.hpp	
			FurnitureRegister.hpp	FurnitureRegister.cpp
			Quadtree.hpp)