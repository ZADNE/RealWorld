/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <string>

namespace rw::dc {

struct CLIArguments {
    std::string inputDirectory;
    std::string outputDirectory;
};

CLIArguments parseArguments(int argc, char* argv[]);

} // namespace rw::dc
