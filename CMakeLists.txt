cmake_minimum_required(VERSION 3.14)
project(RealWorld)

add_executable(RealWorld)
add_subdirectory(RealWorld)
target_include_directories(RealWorld PUBLIC . )

add_executable(RTICreator)
add_subdirectory(RTICreator)
target_include_directories(RTICreator PUBLIC . )

add_executable(CSVToDAT)
add_subdirectory(CSVToDAT)
target_include_directories(CSVToDAT PUBLIC . )

set_target_properties(RealWorld PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/exec")
set_target_properties(RTICreator PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/RTICreator")
set_target_properties(CSVToDAT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/CSVToDAT")
set_target_properties(RealWorld RTICreator CSVToDAT PROPERTIES CXX_STANDARD 20)



add_subdirectory(RealEngine)
add_dependencies(RealWorld RealEngine)
add_dependencies(RTICreator RealEngine)

target_link_libraries(RealWorld
	PUBLIC
		RealEngine
)

target_link_libraries(RTICreator
	PUBLIC
		RealEngine
)
