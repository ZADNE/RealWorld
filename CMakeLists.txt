cmake_minimum_required(VERSION 3.14)
project(RealWorld)

# RealWorld library
add_library(RealWorld STATIC)
target_include_directories(RealWorld PRIVATE . )
set_target_properties(RealWorld PROPERTIES
	CXX_STANDARD 20
)
add_dependencies(RealWorld RealEngine)
target_link_libraries(RealWorld
	PUBLIC
		RealEngine
)

# RealWorld Game
add_executable(RealWorld_Game)
target_include_directories(RealWorld_Game PRIVATE . )
set_target_properties(RealWorld_Game PROPERTIES
	CXX_STANDARD 20
)
add_dependencies(RealWorld_Game RealWorld)
target_link_libraries(RealWorld_Game
	PUBLIC
		RealWorld
)
add_custom_command(TARGET RealWorld_Game POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/data/ .
)

# RealWorld Generator performance test
add_executable(RealWorld_GeneratorTest EXCLUDE_FROM_ALL)
target_include_directories(RealWorld_GeneratorTest PRIVATE .)
set_target_properties(RealWorld_GeneratorTest PROPERTIES
	CXX_STANDARD 20
)
add_dependencies(RealWorld_GeneratorTest RealWorld)
target_link_libraries(RealWorld_GeneratorTest
	PUBLIC
		RealWorld
)

add_subdirectory(RealWorld)
add_subdirectory(tests)
