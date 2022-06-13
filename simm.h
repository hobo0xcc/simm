#ifndef SIMM_H
#define SIMM_H

#include <stddef.h>

typedef enum token_kind token_kind;
enum token_kind {
    TOKEN_INT = 256,
};

typedef struct token token;
struct token {
    char *ptr;
    size_t len;
    int line;
    int kind;
    token *prev;
    token *next;
};

typedef enum node_kind node_kind;
enum node_kind {
    NODE_INTEGER,
    NODE_BINARY,
};

typedef struct node node;
struct node {
    int node_kind;
    struct {
        int num;
    } integer;
    struct {
        node *lhs;
        node *rhs;
        token *op;
    } binary;
};

typedef struct op_bp op_bp;
struct op_bp {
    int left;
    int right;
};

void error(const char *fmt, ...);
char *read_file(const char *filename);

#endif
