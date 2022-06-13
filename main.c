#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "simm.h"

char *p;
int line = 1;
token *curr = NULL;

__attribute__((noreturn)) void error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buf = malloc(sizeof(char) * (fsize + 1));
    fread(buf, fsize, 1, file);
    fclose(file);
    buf[fsize] = 0;
    return buf;
}

token *next_token() {
    while (isspace(*p)) {
        if (*p == '\n') {
            line++;
        }
        p++;
    }
    if (isdigit(*p)) {
        char *ptr = p;
        while (isdigit(*p)) {
            p++;
        }
        token *tok = malloc(sizeof(token));
        tok->ptr = ptr;
        tok->len = (size_t)(p - ptr);
        tok->kind = TOKEN_INT;
        tok->next = tok->prev = NULL;
        tok->line = line;
        return tok;
    }
    if (ispunct(*p)) {
        switch (*p) {
            case '+':
            case '-':
            case '*':
            case '/':
            {
                token *tok = malloc(sizeof(token));
                tok->ptr = p;
                tok->kind = *p;
                p++;
                tok->len = 1;
                tok->next = tok->prev = NULL;
                tok->line = line;
                return tok;
            }
            default:
                error("I don't know '%c' at line %d", *p, line);
        }
    }
    if (*p == 0) {
        token *tok = malloc(sizeof(token));
        tok->ptr = p;
        tok->kind = 0;
        tok->len = 1;
        tok->next = tok->prev = NULL;
        tok->line = line;
        return tok;
    } else {
        error("I don't know '%c' at line %d", *p, line);
    }
}

void tokenize() {
    while (*p != 0) {
        token *tok = next_token();
        if (curr == NULL) {
            curr = tok;
        } else {
            tok->prev = curr;
            curr->next = tok;
            curr = tok;
        }
    }
    if (curr->kind != 0) {
        token *tok = malloc(sizeof(token));
        tok->ptr = p;
        tok->kind = 0;
        tok->len = 1;
        tok->next = NULL;
        tok->prev = curr;
        tok->line = line;
        curr->next = tok;
        curr = tok;
    }
    while (curr->prev != NULL) {
        curr = curr->prev;
    }
}

token *peek(int n) {
    token *tok = curr;
    for (int i = 0; i < n; i++) {
        if (tok->next == NULL) {
            error("too much peeking");
        }
        tok = tok->next;
    }

    return tok;
}

token *eat() {
    assert(curr != NULL);
    token *tok = curr;
    curr = curr->next;
    return tok;
}

node *new_node(int kind) {
    node *n = malloc(sizeof(node));
    n->node_kind = kind;
    return n;
}

node *primary_expr() {
    if (curr->kind == TOKEN_INT) {
        node *n = new_node(NODE_INTEGER);
        char *slice = calloc(1, curr->len + 1);
        strncpy(slice, curr->ptr, curr->len);
        int num = atoi(slice);
        n->integer.num = num;
        free(slice);
        eat();
        return n;
    } else {
        error("I can't find primary expr at line %d...", curr->line);
    }
}

op_bp new_bp(int left, int right) {
    op_bp bp = {
        .left = left,
        .right = right,
    };
    return bp;
}

op_bp infix_binding_power(token *op) {
    switch (op->kind) {
        case '+':
        case '-':
            return new_bp(1, 2);
        case '*':
        case '/':
            return new_bp(3, 4);
        default:
            return new_bp(-1, -1);
    }
}

node *expr_bp(int min_bp) {
    node *lhs = primary_expr();
    for (;;) {
        token *op = curr;
        op_bp bp = infix_binding_power(op);
        if (bp.left == -1 && bp.right == -1) {
            break;
        }
        if (bp.left < min_bp) {
            break;
        }
        eat();
        node *rhs = expr_bp(bp.right);
        node *lhs_tmp = new_node(NODE_BINARY);
        lhs_tmp->binary.lhs = lhs;
        lhs_tmp->binary.rhs = rhs;
        lhs_tmp->binary.op = op;
        lhs = lhs_tmp;
    }

    return lhs;
}

node *expr() {
    return expr_bp(0);
}

node *parse() {
    return expr();
}

int eval(node *n) {
    if (n->node_kind == NODE_BINARY) {
        int lhs = eval(n->binary.lhs);
        int rhs = eval(n->binary.rhs);
        switch (n->binary.op->kind) {
            case '+':
                return lhs + rhs;
            case '-':
                return lhs - rhs;
            case '*':
                return lhs * rhs;
            case '/':
                return lhs / rhs;
            default:
                error("unknwon op: %d", n->binary.op->kind);
        }
    } else if (n->node_kind == NODE_INTEGER) {
        return n->integer.num;
    }

    error("eval failed");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        error("no args");
    }
    char *buf;
    if (strcmp(argv[1], "-e") == 0) {
        if (argc < 3) {
            error("require string");
        }
        buf = calloc(1, sizeof(char) * strlen(argv[2]));
        strncpy(buf, argv[2], strlen(argv[2]));
    } else {
        buf = read_file(argv[1]);
    }
    p = buf;
    tokenize();
    node *n = parse();
    printf("eval: %d\n", eval(n));
    return 0;
}
