add_subdirectory(furniture)
add_subdirectory(items)
add_subdirectory(menu)
add_subdirectory(shaders)
add_subdirectory(world)

target_sources(${PROJECT_NAME} PRIVATE 
	div.hpp					
	KeyBinder.hpp			
	LanguageManager.hpp		
							main.cpp
	metadata.hpp
	RealWorld.hpp			RealWorld.cpp
)