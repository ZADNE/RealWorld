/**
 *  @author    Dubsky Tomas
 */
export module realworld.tools.datacooker:Arguments;
import std;

export namespace rw::dc {

struct CLIArguments {
    std::string inputDirectory;
    std::string outputDirectory;
};

CLIArguments parseArguments(int argc, char* argv[]);

} // namespace rw::dc
