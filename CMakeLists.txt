cmake_minimum_required(VERSION 3.14)
project(RealWorld)

add_executable(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

add_subdirectory(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC . RealEngine )