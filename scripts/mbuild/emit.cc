#include "emit.hh"
#include "eval.hh"
#include <fstream>

static int emit_makefile(std::string path) {
    std::ofstream f(path);
    if (!f.is_open()) { return EXIT_FAILURE; }
    for (const auto &e : g_lists) {
        f << e.first << " := ";
        for (const auto &i : e.second.data) { f << i << " "; }
        f << "\n";
    }

    return EXIT_SUCCESS;
}

int emit(EmitType type, std::string path) {
    switch (type) {
        case EMIT_MAKEFILE:
            return emit_makefile(path);
        default:
            return EXIT_FAILURE;
    }
}