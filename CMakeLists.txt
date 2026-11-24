cmake_minimum_required(VERSION 4.5)
project(RealWorld)

include("cmake/GetRealWorldVersion.cmake")

# If not building RealEngine from source
if(NOT TARGET RealEngine)
    # Find installed RealEngine
    find_package(RealEngine 3.0.0 CONFIG REQUIRED)
endif()

# RealWorld library
function(_add_realworld_sublib target_name)
    add_real_library(${target_name} STATIC)
    string(TOLOWER ${target_name} target_name_lower)
    real_target_init_file_sets(${target_name}
        BASE_DIR "src"
        GENERATE_CXX_MODULE_INTERFACE ${target_name_lower}
    )
    set_target_properties(${target_name} PROPERTIES
        RE_SHADER_NAMESPACE "rw::glsl"
    )
    target_link_libraries(${target_name}
        PUBLIC
            RealEngine
            argparse
    )
endfunction()
_add_realworld_sublib(RealWorld_constants)
_add_realworld_sublib(RealWorld)
set_target_properties(RealWorld PROPERTIES
    RE_SETUP_FILEPATH_REL  "realworld/main/Setup.gen.cppm"
    RE_SETUP_MODULE        "realworld:main.Setup_gen"
)
target_link_libraries(RealWorld PUBLIC RealWorld_constants)

# Data cooking and packaging for RealWorld
include("cmake/DataCooking.cmake")
add_dependencies(RealWorld DataCooker_Cook)
real_target_package_resources(
    TARGET
        RealWorld
    PACKAGE_DIR
        ${CMAKE_CURRENT_BINARY_DIR}
    INDEX_FILE
        "${CMAKE_CURRENT_BINARY_DIR}/src/realworld/resources/ResourceIndex.gen.cppm"
    MODULE_NAME
        "realworld:resources.ResourceIndex_gen"
    INPUT_DIRS
        "${CMAKE_CURRENT_BINARY_DIR}/fonts"
        "${CMAKE_CURRENT_BINARY_DIR}/textures"
)
add_dependencies(RealWorld_PackageResources DataCooker_Cook)

# RealWorld Game
add_real_executable(RealWorld_Game)
real_target_init_file_sets(RealWorld_Game
    BASE_DIR "src"
)
target_link_libraries(RealWorld_Game PUBLIC RealWorld)

# RealWorld PerfTest
add_real_executable(RealWorld_PerfTest)
real_target_init_file_sets(RealWorld_PerfTest
    BASE_DIR "perf_test"
)
set_target_properties(RealWorld_PerfTest PROPERTIES
    RE_SOURCES_AS_CXX_IMPL_UNITS    ON
    COMPILE_WARNING_AS_ERROR        OFF
    EXPORT_COMPILE_COMMANDS         OFF
    CXX_CLANG_TIDY                  ""
)
target_link_libraries(RealWorld_PerfTest PUBLIC RealWorld)

# Doxygen documentation
get_realworld_version()
find_package(Doxygen)
if(Doxygen_FOUND)
    set(doxyfile_template "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    set(doxyfile "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
    configure_file(
        "${doxyfile_template}"
        "${doxyfile}"
        @ONLY
    )

    add_custom_target(RealWorld_Documentation
        COMMAND "${DOXYGEN_EXECUTABLE}" "${doxyfile}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        VERBATIM
    )
endif()

# Add subdirectories
add_subdirectory("src")
add_subdirectory("tools")
add_subdirectory("perf_test")

# Installation
install(
    TARGETS RealWorld_Game
    RUNTIME
        DESTINATION .
        COMPONENT RealWorld_Runtime
)
install(
    IMPORTED_RUNTIME_ARTIFACTS ${REALENGINE_RUNTIME_ARTIFACTS}
    RUNTIME
        DESTINATION .
        COMPONENT RealWorld_Runtime
)
install( # Install unpackaged data (in Debug builds)
    DIRECTORY
        "$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/textures>"
        "$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/fonts>"
    DESTINATION .
    COMPONENT RealWorld_Runtime
)
install( # Install packaged data (in non-Debug builds)
    FILES
        "$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_BINARY_DIR}/package.dat>"
    DESTINATION .
    COMPONENT RealWorld_Runtime
)
if(MSVC)
    install( # Install PDB file
        FILES
            "$<TARGET_PDB_FILE:RealWorld_Game>"
        DESTINATION .
        COMPONENT RealWorld_Runtime
        OPTIONAL
    )
endif()
