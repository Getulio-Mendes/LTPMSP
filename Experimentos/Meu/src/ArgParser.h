#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <string>

struct AppArgs {
    std::string algorithm = "LateAcceptanceILS";
    std::string descida = "VND_randomico";
    int itermax = 5;
    int nivel = 7;
    int ilsmax = 70;
    int runs = 1;
    int seed = 1;
    std::string input_file = "";
};

AppArgs parseArgs(int argc, char** argv);

#endif
