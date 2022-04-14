cmake_minimum_required(VERSION 3.14)
project(RealWorld)

add_executable(RealWorld)
target_include_directories(RealWorld PRIVATE src )

add_executable(CSVToDAT)
target_include_directories(CSVToDAT PRIVATE src )

add_subdirectory(src)

set_target_properties(RealWorld CSVToDAT PROPERTIES
	CXX_STANDARD 20
)

add_dependencies(RealWorld RealEngine)

target_link_libraries(RealWorld
	PUBLIC
		RealEngine
)

add_custom_command(TARGET RealWorld POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/data/ .
)
