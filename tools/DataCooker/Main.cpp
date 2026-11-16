/**
 *  @author    Dubsky Tomas
 */
module realworld.tools.datacooker:Main_impl;
import :Arguments;
import :TileItemTextures;

extern "C++" int main(int argc, char* argv[]) {
    using namespace rw::dc;
    try {
        CLIArguments args = parseArguments(argc, argv);
        composeTileItemTextures(args.inputDirectory, args.outputDirectory);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}
