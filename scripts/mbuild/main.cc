#include "emit.hh"
#include "eval.hh"
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
    int opt;

    std::string outPath = "./out.auto";
    std::string inPath  = "./";

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                outPath = std::string(optarg);
                break;
            default:
                return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        std::cerr << "Missing input file\n";
        return EXIT_FAILURE;
    }
    if (optind != argc - 1) {
        std::cerr << "Too many input files\n";
        return EXIT_FAILURE;
    }

    inPath = argv[optind];

    int r = eval(inPath);
    if (r != 0) { return EXIT_FAILURE; }

    r = emit(EMIT_MAKEFILE, outPath);
    if (r != 0) { return EXIT_FAILURE; }

    std::cout << "Built " << outPath << "\n";

    return EXIT_SUCCESS;
}