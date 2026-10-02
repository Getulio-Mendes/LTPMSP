#include "ArgParser.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <getopt.h>

AppArgs parseArgs(int argc, char** argv) {
    AppArgs args;
    
    const char* const short_opts = "";
    const option long_opts[] = {
        {"algorithm", required_argument, nullptr, 'a'},
        {"descida", required_argument, nullptr, 'd'},
        {"itermax", required_argument, nullptr, 'i'},
        {"nivel", required_argument, nullptr, 'n'},
        {"ilsmax", required_argument, nullptr, 'm'},
        {"input", required_argument, nullptr, 'f'},
        {"runs", required_argument, nullptr, 'r'},
        {"seed", required_argument, nullptr, 's'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };

    while (true) {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt)
            break;

        switch (opt) {
        case 'a':
            args.algorithm = std::string(optarg);
            break;
        case 'd':
            args.descida = std::string(optarg);
            break;
        case 'i':
            args.itermax = std::stoi(optarg);
            break;
        case 'n':
            args.nivel = std::stoi(optarg);
            break;
        case 'm':
            args.ilsmax = std::stoi(optarg);
            break;
        case 'f':
            args.input_file = std::string(optarg);
            break;
        case 'r':
            args.runs = std::stoi(optarg);
            break;
        case 's':
            args.seed = std::stoi(optarg);
            break;
        case 'h': // --help
            std::cout << "Usage: " << argv[0] << " --input <file> [options]\n"
                      << "Note: use only long options in the form --arg value.\n"
                      << "Options:\n"
                      << "  --algorithm <name>     LateAcceptanceILS (default), SmartILS or SmartLateAcceptanceILS\n"
                      << "  --descida <type>       VND_randomico (default)\n"
                      << "  --itermax <int>        Values: 5, 10, 15 (default: 5)\n"
                      << "  --nivel <int>          Values: 7, 10, 15 (default: 7)\n"
                      << "  --ilsmax <int>         Values: 70, 100, 150 (default: 70)\n"
                      << "  --runs <int>           Default: 1\n"
                      << "  --seed <int>           Default: 1\n";
            std::exit(0);
            break;
        case '?': // Unrecognized option
        default:
            std::cerr << "Error: use only long options in the form --arg value.\n";
            std::exit(1);
            break;
        }
    }

    if (optind < argc) {
        std::cerr << "Error: positional arguments are not allowed.\n";
        std::exit(1);
    }

    if (args.input_file.empty()) {
        std::cerr << "Error: Input file is required.\n";
        std::exit(1);
    }

    return args;
}
