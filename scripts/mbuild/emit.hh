#ifndef _MBUILD_EMIT_H
#define _MBUILD_EMIT_H

#include <string>

enum EmitType {
    EMIT_MAKEFILE,
};

int emit(EmitType type, std::string path);

#endif