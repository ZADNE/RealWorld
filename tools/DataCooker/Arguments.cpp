/*!
 *  @author    Dubsky Tomas
 */
#include <argparse/argparse.hpp>

#include <DataCooker/Arguments.hpp>

namespace rw::dc {

CLIArguments parseArguments(int argc, char* argv[]) {
    argparse::ArgumentParser parser("DataCooker", "0.1.0");

    parser.add_argument("-i")
        .metavar("input_dir")
        .required()
        .help("directory containing the input data to process");
    parser.add_argument("-o")
        .metavar("output_dir")
        .required()
        .help("directory where output data will be placed");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    return CLIArguments{
        .inputDirectory = parser.get<>("-i"), .outputDirectory = parser.get<>("-o")
    };
}

} // namespace rw::dc
