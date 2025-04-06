/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <span>

namespace rw {

struct CLIArguments {
    bool createDebugWorld = false; ///< Debug only: create new world and load it
};

CLIArguments parseArguments(int argc, char* argv[]);

} // namespace rw
