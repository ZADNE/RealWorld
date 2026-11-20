/**
 *  @author    Dubsky Tomas
 */
export module realworld.tools.data_cooker:Arguments;
import std;

export namespace rw::datac {

struct CLIArguments {
    std::string inputDirectory;
    std::string outputDirectory;
};

CLIArguments parseArguments(int argc, char* argv[]);

} // namespace rw::datac
