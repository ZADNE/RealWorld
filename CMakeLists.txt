cmake_minimum_required(VERSION 3.14)
project(RealWorld)

add_executable(RealWorld)
add_subdirectory(RealWorld)

add_executable(CSVToDAT)
add_subdirectory(CSVToDAT)

set_property(TARGET RealWorld PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/exec")
set_property(TARGET CSVToDAT PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/CSVToDAT")
set_property(TARGET RealWorld CSVToDAT PROPERTY CXX_STANDARD 20)


target_include_directories(RealWorld PUBLIC . )
target_include_directories(CSVToDAT PUBLIC . )

add_subdirectory(RealEngine)
add_dependencies(RealWorld RealEngine)

target_link_libraries(RealWorld
	PUBLIC
		RealEngine
)