# author     Dubsky Tomas

# DataCooker
add_real_executable(DataCooker)
real_target_init_file_sets(DataCooker
    BASE_DIR "tools"
    SOURCES_AS_CXX_MODULE_IMPL_UNITS
)
set_target_properties(DataCooker PROPERTIES
    CXX_STANDARD 23
    INTERPROCEDURAL_OPTIMIZATION TRUE
)
target_include_directories(DataCooker PRIVATE src)
target_link_libraries(DataCooker
    PRIVATE
        argparse
        RealWorld_constants
)

# Data cooking target
set(cook_output_dirs
    "${CMAKE_CURRENT_BINARY_DIR}/fonts/"
    "${CMAKE_CURRENT_BINARY_DIR}/textures/"
)
add_custom_command(
    OUTPUT ${cook_output_dirs}
    # Copy game ready data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/data/game_ready/
            ${CMAKE_CURRENT_BINARY_DIR}
    # Cook additional data
    COMMAND DataCooker -i "${CMAKE_CURRENT_SOURCE_DIR}/data"
                       -o ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS DataCooker
    COMMENT "Cooking data..."
    VERBATIM
    COMMAND_EXPAND_LISTS
)
add_custom_target(DataCooker_Cook DEPENDS ${cook_output_dirs})
