/**
 *  @author    Dubsky Tomas
 */
module realworld.tools.data_cooker:Arguments_impl;
import argparse;
import :Arguments;

namespace rw::datac {

CLIArguments parseArguments(int argc, char* argv[]) { // NOLINT(*-avoid-c-arrays)
    argparse::ArgumentParser parser("DataCooker", "0.1.1");

    parser.add_argument("-i")
        .metavar("input_dir")
        .required()
        .help("directory containing the input data to process");
    parser.add_argument("-o")
        .metavar("output_dir")
        .required()
        .help("directory where output data will be placed");

    try {
        parser.parse_args({&argv[0], &argv[argc]});
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    return CLIArguments{
        .inputDirectory = parser.get<>("-i"), .outputDirectory = parser.get<>("-o")
    };
}

} // namespace rw::datac
