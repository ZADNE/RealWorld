/**
 *  @author    Dubsky Tomas
 */
#include <argparse/argparse.hpp>

#include <RealEngine/utility/BuildType.hpp>

#include <RealWorld/main/Arguments.hpp>
#include <RealWorld/utility/Version.hpp>

namespace rw {

CLIArguments parseArguments(int argc, char* argv[]) { // NOLINT(*-avoid-c-arrays)
    argparse::ArgumentParser parser("RealWorld", versionString());

    if constexpr (re::k_buildType == re::BuildType::Debug) {
        parser.add_argument("--create_debug_world")
            .flag()
            .help("create new world and load it [debug only]");
    }

    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    CLIArguments rval{};
    if constexpr (re::k_buildType == re::BuildType::Debug) {
        rval.createDebugWorld = parser.get<bool>("--create_debug_world");
    }
    return rval;
}

} // namespace rw
