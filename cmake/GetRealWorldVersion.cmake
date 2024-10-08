# author     Dubsky Tomas

function(get_realworld_version)
    file(READ "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../src/RealWorld/utility/Version.hpp" version_file)
    string(REGEX MATCH "constexpr int k_versionMajor = ([0-9]+)" _ ${version_file})
    set(version_major "${CMAKE_MATCH_1}")
    string(REGEX MATCH "constexpr int k_versionMinor = ([0-9]+)" _ ${version_file})
    set(version_minor "${CMAKE_MATCH_1}")
    string(REGEX MATCH "constexpr int k_versionPatch = ([0-9]+)" _ ${version_file})
    set(version_patch "${CMAKE_MATCH_1}")

    set(realworld_version "${version_major}.${version_minor}.${version_patch}" PARENT_SCOPE)
endfunction()

# Print the version to stdout if running in script mode
if(DEFINED CMAKE_ARGV0)
    get_realworld_version()
    execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${realworld_version}")
endif()