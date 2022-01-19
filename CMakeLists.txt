cmake_minimum_required(VERSION 3.14)
project(RealWorld)

add_executable(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/exec")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

add_subdirectory(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC . )

add_subdirectory(RealEngine)
add_dependencies(${PROJECT_NAME} RealEngine)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		RealEngine
)