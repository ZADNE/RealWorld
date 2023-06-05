cmake_minimum_required(VERSION 3.24)
project(RealWorld)

#RealWorld library
add_library_with_shaders(RealWorld STATIC)
target_include_directories(RealWorld PRIVATE .)
set_target_properties(RealWorld PROPERTIES
    CXX_STANDARD 20
    SHADERS_CXX_NAMESPACE "rw"
)
target_link_libraries(RealWorld PUBLIC RealEngine)

#RealWorld Game
add_executable(RealWorld_Game)
target_include_directories(RealWorld_Game PRIVATE .)
set_target_properties(RealWorld_Game PROPERTIES CXX_STANDARD 20)
target_link_libraries(RealWorld_Game PUBLIC RealWorld)
add_custom_command(TARGET RealWorld_Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data/ .
)

#Add subdirectories
add_subdirectory(RealWorld)
