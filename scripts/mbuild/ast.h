#ifndef _MDBUILD_AST_H
#define _MDBUILD_AST_H

#ifdef __cplusplus
extern "C" {
#endif

enum AstOperations {
    OP_INVALID,
    OP_ADD,
    OP_SET,
    OP_REMOVE,
};

int ast_decode_op(const char *const opStr);

struct AstList {
    int    count;
    char **values;
};

struct AstNode {
    char           *ident;
    int             op;
    struct AstList  list;
    struct AstList  qualifiers;
    struct AstNode *next;
};

extern struct AstNode *ast;

void ast_insert_node(char *ident, int op, struct AstList *list, struct AstList *qualifiers);

void ast_free_list(struct AstList *list);
void ast_free(struct AstNode *node);

#ifdef __cplusplus
}
#endif

#endif