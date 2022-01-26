add_subdirectory(furniture)
add_subdirectory(items)
add_subdirectory(main)
add_subdirectory(rendering)
add_subdirectory(shaders)
add_subdirectory(world)

target_sources(${PROJECT_NAME} PRIVATE 
	div.hpp					
	KeyBinder.hpp			
	LanguageManager.hpp		
	metadata.hpp			
)
