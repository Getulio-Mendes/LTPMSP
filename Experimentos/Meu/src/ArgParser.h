#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <string>

struct AppArgs {
    std::string algorithm = "LateAcceptanceILS";
    std::string descida = "segmento_randomica";
    int itermax = 40;
    int nivel = 15;
    int ilsmax = 70;
    int runs = 1;
    int seed = 1;
    std::string input_file = "";
};

AppArgs parseArgs(int argc, char** argv);

#endif
