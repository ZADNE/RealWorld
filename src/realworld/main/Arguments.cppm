/**
 *  @author    Dubsky Tomas
 */
export module realworld:main.Arguments;
import std;

export namespace rw {

struct CLIArguments {
    bool createDebugWorld = false; ///< Debug only: create new world and load it
};

CLIArguments parseArguments(int argc, char* argv[]); // NOLINT(*-avoid-c-arrays)

} // namespace rw
