#include "eval.hh"
#include "ast.h"
#include "parser_inc.h"
#include <filesystem>
#include <iostream>
#include <vector>

std::unordered_map<std::string, ListState> g_lists;

static int parse(std::string dir) {
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) { return EXIT_FAILURE; }
    yyin = fopen((dir + "mbuild").c_str(), "r");
    if (!yyin) {
        std::cerr << "Could not open file" << dir << "/mbuild\n";
        return EXIT_FAILURE;
    }
    if (yyparse() != 0) {
        fclose(yyin);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int eval_node(struct AstNode *node, std::string dir) {
    if (node == nullptr) {
        std::cerr << "AST node was NULL\n";
        return EXIT_FAILURE;
    }

    std::vector<std::string> listEntries;
    std::vector<std::string> qualEntries;
    for (size_t i = EXIT_SUCCESS; i < node->list.count; i++) { listEntries.push_back(node->list.values[i]); }
    for (size_t i = EXIT_SUCCESS; i < node->qualifiers.count; i++) {
        qualEntries.push_back(node->qualifiers.values[i]);
    }

    for (auto &e : listEntries) { e = std::filesystem::weakly_canonical(dir + e); }

    // for (auto &qualifier : qualEntries) {
    //     if (qualifier == "prepend_dir") {

    //     } else {
    //         std::cerr << "Unknwon qualifier: " << qualifier << "\n";
    //     }
    // }

    switch (node->op) {
        case OP_ADD:
            for (auto &e : listEntries) { g_lists[node->ident].data.insert(e); }
            break;
        case OP_SET:
            g_lists[node->ident].data.clear();
            for (auto &e : listEntries) { g_lists[node->ident].data.insert(e); }
            break;
        case OP_REMOVE:
            for (auto &e : listEntries) { g_lists[node->ident].data.erase(e); }
            break;
        default:
            std::cerr << "Invalid operation: " << node->op << "\n";
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static bool is_dir(std::string dir) { return (!dir.empty() && dir.back() == '/'); }

static bool mbuild_exists(std::string dir) {
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) { return false; }
    if (!std::filesystem::exists(dir + "mbuild")) { return false; }
    return true;
}

static int eval_internal(std::string dir);

static int resolve_include(std::string dir, ListState list) {
    static std::set<std::string> visitedDirs;
    auto dataCopy = list.data;
    for (auto &e : dataCopy) {
        if (!is_dir(e)) { continue; }
        if (!mbuild_exists(e)) { std::cerr << "Warning: " << e << " does not exist or does not contain mbuild\n"; }
        if (visitedDirs.find(e) == visitedDirs.end()) {
            visitedDirs.insert(e);
            if (eval_internal(e) != 0) { return EXIT_FAILURE; }
        }
    }

    return EXIT_SUCCESS;
}

static int eval_internal(std::string dir) {
    if (parse(dir) != EXIT_SUCCESS) { return EXIT_FAILURE; }

    for (AstNode *n = ast; n; n = n->next) {
        if (eval_node(n, dir) != EXIT_SUCCESS) { return EXIT_FAILURE; }
    }
    ast_free(ast);

    for (auto &e : g_lists) { resolve_include(dir, e.second); }

    return EXIT_SUCCESS;
}

int eval(std::string dir) {
    if (eval_internal(dir) != EXIT_SUCCESS) { return EXIT_FAILURE; }
    for (auto &i : g_lists) {
        for (auto &e : i.second.data) {
            if (is_dir(e)) { i.second.data.erase(e); }
        }
    }
    return EXIT_SUCCESS;
}