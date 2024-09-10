cmake_minimum_required(VERSION 3.24)
project(RealWorld)

#RealWorld library
    add_real_library(RealWorld STATIC)
    real_target_init_file_sets(RealWorld BASE_DIR src)
    target_include_directories(RealWorld PRIVATE src)
    set_target_properties(RealWorld PROPERTIES
        CXX_STANDARD 23
        realproject_shader_cxx_namespace "rw"
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
    target_link_libraries(RealWorld PUBLIC RealEngine)

#RealWorld Game
    add_real_executable(RealWorld_Game)
    target_include_directories(RealWorld_Game PRIVATE src)
    set_target_properties(RealWorld_Game PROPERTIES
        CXX_STANDARD 23
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
    target_link_libraries(RealWorld_Game PUBLIC RealWorld)
    add_custom_command(TARGET RealWorld_Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data/ .
    )

#RealWorld PerfTest
    add_real_executable(RealWorld_PerfTest)
    target_include_directories(RealWorld_PerfTest PRIVATE src perf_test)
    set_target_properties(RealWorld_PerfTest PROPERTIES
        CXX_STANDARD 23
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
    target_link_libraries(RealWorld_PerfTest PUBLIC RealWorld)
    add_custom_command(TARGET RealWorld_PerfTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data/ .
    )

#Doxygen documentation
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target(RealWorld_Documentation ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )
    endif (DOXYGEN_FOUND)

#Add subdirectories
    add_subdirectory(src)
    add_subdirectory(perf_test)
