/**
 *  @author    Dubsky Tomas
 */
#include <iostream>

#include <DataCooker/Arguments.hpp>
#include <DataCooker/TileItemTextures.hpp>

int main(int argc, char* argv[]) {
    using namespace rw::dc;
    try {
        CLIArguments args = parseArguments(argc, argv);
        composeTileItemTextures(args.inputDirectory, args.outputDirectory);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}
