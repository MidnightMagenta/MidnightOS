#ifndef _MBUILD_EVAL_H
#define _MBUILD_EVAL_H

#include <string>
#include <unordered_map>
#include <set>

struct ListState{
    std::set<std::string> data;
};

extern std::unordered_map<std::string, ListState> g_lists;

int eval(std::string dir);

#endif