#ifndef SIMM_H
#define SIMM_H

#include <stddef.h>
#include <stdlib.h>

typedef enum token_kind token_kind;
enum token_kind {
    TOKEN_INT = 256,
    TOKEN_IDENT = 257,
    TOKEN_RETURN = 258,
    TOKEN_INT_TYPE = 259,
    TOKEN_ADD_ASSIGN = 260,
    TOKEN_SUB_ASSIGN = 261,
    TOKEN_MUL_ASSIGN = 262,
    TOKEN_DIV_ASSIGN = 263,
    TOKEN_EQ = 264,
    TOKEN_NEQ = 265,
    TOKEN_IF = 266,
    TOKEN_ELSE = 267,
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
    NODE_UNARY,
    NODE_VARIABLE,
    NODE_DEFVAR,
    NODE_RETURN,
    NODE_IF,
};

typedef struct node node;
struct node {
    int node_kind;
    node *prev;
    node *next;
    struct {
        int num;
    } integer;
    struct {
        node *lhs;
        node *rhs;
        token *op;
    } binary;
    struct {
        node *expr;
        token *op;
    } unary;
    struct {
        char *name;
        int index;
    } variable;
    struct {
        char *name;
        int index;
        node *expr;
    } defvar;
    struct {
        node *expr;
    } return_stmt;
    struct {
        node *cond;
        node *then_stmt;
        node *else_stmt;
    } if_stmt;
};

typedef struct binding_power binding_power;
struct binding_power {
    int left;
    int right;
};

typedef struct local_var local_var;
struct local_var {
    char *name;
    int sp;
    local_var *prev;
    local_var *next;
};

node *stmt();
void error(const char *fmt, ...);
char *read_file(const char *filename);

#endif
