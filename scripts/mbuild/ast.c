#include "ast.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define DEFINE_OP(op, str) [op] = str
#define ARRAY_SIZE(array)  (sizeof(array) / sizeof(*array))

const char *const opStrTab[] = {
        DEFINE_OP(OP_ADD, ".add"),
        DEFINE_OP(OP_SET, ".set"),
        DEFINE_OP(OP_REMOVE, ".remove"),
};

int ast_decode_op(const char *const opStr) {
    for (size_t i = 0; i < ARRAY_SIZE(opStrTab); i++) {
        if (opStrTab[i] && strcmp(opStr, opStrTab[i]) == 0) { return i; }
    }
    return OP_INVALID;
}

struct AstNode *ast;

static void build_ast_node(struct AstNode *node,
                           char           *ident,
                           int             op,
                           struct AstList *list,
                           struct AstList *qualifiers) {
    node->ident      = ident;
    node->op         = op;
    node->list       = *list;
    node->qualifiers = *qualifiers;
    node->next       = NULL;
}

void ast_insert_node(char *ident, int op, struct AstList *list, struct AstList *qualifiers) {
    if (!ast) {
        ast = calloc(1, sizeof(struct AstNode));
        build_ast_node(ast, ident, op, list, qualifiers);
        return;
    }

    struct AstNode *currentNode = ast;
    while (currentNode->next) { currentNode = currentNode->next; }
    currentNode->next = calloc(1, sizeof(struct AstNode));
    build_ast_node(currentNode->next, ident, op, list, qualifiers);
}

void ast_free_list(struct AstList *list) {
    for (int i = 0; i < list->count; i++) { free(list->values[i]); }
    free(list->values);
}

void ast_free(struct AstNode *node) {
    while (node) {
        struct AstNode *next = node->next;
        free(node->ident);
        ast_free_list(&node->list);
        ast_free_list(&node->qualifiers);
        free(node);
        node = next;
    }
}