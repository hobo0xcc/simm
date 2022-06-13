#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

#include "simm.h"

char *p;
int line = 1;
token *curr = NULL;
local_var *local = NULL;
int stack = 0;
int block = 1;

char *copy_string(char *src, int len) {
    char *dest = calloc(1, sizeof(char) * (len + 1));
    strncpy(dest, src, len);
    return dest;
}

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

token *new_token(int kind) {
    token *tok = calloc(1, sizeof(token));
    tok->kind = kind;
    return tok;
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
    if (isalpha(*p)) {
        char *ptr = p;
        while (isalnum(*p) || *p == '_') {
            p++;
        }
        token *tok = malloc(sizeof(token));
        tok->ptr = ptr;
        tok->len = (size_t)(p - ptr);
        if (strncmp(tok->ptr, "return", tok->len) == 0) {
            tok->kind = TOKEN_RETURN;
        } else if (strncmp(tok->ptr, "int", tok->len) == 0) {
            tok->kind = TOKEN_INT_TYPE;
        } else if (strncmp(tok->ptr, "if", tok->len) == 0) {
            tok->kind = TOKEN_IF;
        } else if (strncmp(tok->ptr, "else", tok->len) == 0) {
            tok->kind = TOKEN_ELSE;
        } else {
            tok->kind = TOKEN_IDENT;
        }
        tok->next = tok->prev = NULL;
        tok->line = line;
        return tok;
    }
    if (ispunct(*p)) {
        if (strncmp(p, "+=", 2) == 0) {
            token *tok = malloc(sizeof(token));
            tok->ptr = p;
            tok->kind = TOKEN_ADD_ASSIGN;
            p += 2;
            tok->len = 2;
            tok->next = tok->prev = NULL;
            tok->line = line;
            return tok;
        }
        if (strncmp(p, "-=", 2) == 0) {
            token *tok = malloc(sizeof(token));
            tok->ptr = p;
            tok->kind = TOKEN_SUB_ASSIGN;
            p += 2;
            tok->len = 2;
            tok->next = tok->prev = NULL;
            tok->line = line;
            return tok;
        }
        if (strncmp(p, "*=", 2) == 0) {
            token *tok = malloc(sizeof(token));
            tok->ptr = p;
            tok->kind = TOKEN_MUL_ASSIGN;
            p += 2;
            tok->len = 2;
            tok->next = tok->prev = NULL;
            tok->line = line;
            return tok;
        }
        if (strncmp(p, "/=", 2) == 0) {
            token *tok = malloc(sizeof(token));
            tok->ptr = p;
            tok->kind = TOKEN_DIV_ASSIGN;
            p += 2;
            tok->len = 2;
            tok->next = tok->prev = NULL;
            tok->line = line;
            return tok;
        }
        if (strncmp(p, "==", 2) == 0) {
            token *tok = malloc(sizeof(token));
            tok->ptr = p;
            tok->kind = TOKEN_EQ;
            p += 2;
            tok->len = 2;
            tok->next = tok->prev = NULL;
            tok->line = line;
            return tok;
        }
        if (strncmp(p, "!=", 2) == 0) {
            token *tok = malloc(sizeof(token));
            tok->ptr = p;
            tok->kind = TOKEN_NEQ;
            p += 2;
            tok->len = 2;
            tok->next = tok->prev = NULL;
            tok->line = line;
            return tok;
        }
        switch (*p) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '=':
            case ';':
            case '!':
            case '(':
            case ')':
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

int insert_local_var(char *name) {
    if (local == NULL) {
        local = calloc(1, sizeof(local_var));
        stack += 8;
        local->sp = stack;
        local->name = name;
        return local->sp;
    }
    local_var *l = local;
    for (;;) {
        if (strcmp(l->name, name) == 0) {
            error("'%s' already defined", name);
        }
        if (l->next == NULL) {
            break;
        }
        l = l->next;
    }
    local_var *new = calloc(1, sizeof(local_var));
    new->name = name;
    stack += 8;
    new->sp = stack;
    l->next = new;
    new->prev = l;
    return new->sp;
}

int get_local_var(char *name) {
    if (local == NULL) {
        error("local variable '%s' not found", name);
    }
    local_var *l = local;
    for (;;) {
        if (strcmp(l->name, name) == 0) {
            return l->sp;
        }
        if (l->next == NULL) {
            break;
        }
        l = l->next;
    }
    error("local variable '%s' not found", name);
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

bool equal(int kind, int n) {
    token *tok = peek(n);
    return tok->kind == kind;
}

token *expect(int kind) {
    token *tok = eat();
    if (tok->kind != kind) {
        char *buf = copy_string(tok->ptr, tok->len);
        error("expected %d but found %s", kind, buf);
    }
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
    } else if (curr->kind == TOKEN_IDENT) {
        node *n = new_node(NODE_VARIABLE);
        n->variable.name = copy_string(curr->ptr, curr->len);
        n->variable.index = get_local_var(n->variable.name);
        eat();
        return n;
    } else {
        error("I can't find primary expr at line %d...", curr->line);
    }
}

binding_power new_bp(int left, int right) {
    binding_power bp = {
        .left = left,
        .right = right,
    };
    return bp;
}

binding_power prefix_binding_power(token *op) {
    switch (op->kind) {
        case '!':
            return new_bp(-1, 101);
        default:
            return new_bp(-1, -1);
    }
}

binding_power infix_binding_power(token *op) {
    switch (op->kind) {
        case '=':
        case TOKEN_ADD_ASSIGN:
        case TOKEN_SUB_ASSIGN:
        case TOKEN_MUL_ASSIGN:
        case TOKEN_DIV_ASSIGN:
            return new_bp(2, 1);
        case TOKEN_EQ:
        case TOKEN_NEQ:
            return new_bp(3, 4);
        case '+':
        case '-':
            return new_bp(5, 6);
        case '*':
        case '/':
            return new_bp(7, 8);
        default:
            return new_bp(-1, -1);
    }
}

node *expr_bp(int min_bp) {
    node *lhs;
    binding_power bp = prefix_binding_power(curr);
    if (bp.left == -1 && bp.right == -1) {
        lhs = primary_expr();
    } else {
        node *rhs = expr_bp(bp.right);
        lhs = new_node(NODE_UNARY);
        lhs->unary.expr = rhs;
        lhs->unary.op = eat();
    }
    for (;;) {
        token *op = curr;
        binding_power bp = infix_binding_power(op);
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
        bool assign = false;
        bool not = false;
        switch (op->kind) {
            case TOKEN_ADD_ASSIGN:
                {
                    lhs_tmp->binary.op = new_token('+');
                    assign = true;
                    break;
                }
            case TOKEN_SUB_ASSIGN:
                {
                    lhs_tmp->binary.op = new_token('-');
                    assign = true;
                    break;
                }
            case TOKEN_MUL_ASSIGN:
                {
                    lhs_tmp->binary.op = new_token('*');
                    assign = true;
                    break;
                }
            case TOKEN_DIV_ASSIGN:
                {
                    lhs_tmp->binary.op = new_token('/');
                    assign = true;
                    break;
                }
            case TOKEN_NEQ:
                {
                    lhs_tmp->binary.op = new_token(TOKEN_EQ);
                    not = true;
                    break;
                }
        }
        if (assign) {
            node *lhs_tmp2 = new_node(NODE_BINARY);
            lhs_tmp2->binary.rhs = lhs_tmp;
            lhs_tmp2->binary.lhs = lhs;
            lhs_tmp2->binary.op = new_token('=');
            lhs_tmp = lhs_tmp2;
        } else if (not) {
            node *lhs_tmp2 = new_node(NODE_UNARY);
            lhs_tmp2->unary.expr = lhs_tmp;
            lhs_tmp2->unary.op = new_token('!');
            lhs_tmp = lhs_tmp2;
        } else {
            lhs_tmp->binary.op = op;
        }
        lhs = lhs_tmp;
    }

    return lhs;
}

node *expr() {
    return expr_bp(0);
}

node *define_variable() {
    token *ty = expect(TOKEN_INT_TYPE);
    if (strncmp(ty->ptr, "int", 3) != 0) {
        char *buf = copy_string(ty->ptr, ty->len);
        error("expect 'int' but found %s", buf);
    }
    token *name_tok = expect(TOKEN_IDENT);
    char *name = copy_string(name_tok->ptr, name_tok->len);
    expect('=');
    node *exp = expr();
    expect(';');
    node *n = new_node(NODE_DEFVAR);
    n->defvar.expr = exp;
    n->defvar.index = insert_local_var(name);
    n->defvar.name = name;
    return n;
}

node *return_stmt() {
    expect(TOKEN_RETURN);
    node *exp = expr();
    expect(';');
    node *n = new_node(NODE_RETURN);
    n->return_stmt.expr = exp;
    return n;
}

node *if_stmt() {
    node *n = new_node(NODE_IF);
    expect(TOKEN_IF);
    expect('(');
    node *cond = expr();
    expect(')');
    node *then_stmt = stmt();
    n->if_stmt.cond = cond;
    n->if_stmt.then_stmt = then_stmt;
    if (equal(TOKEN_ELSE, 0)) {
        eat();
        node *else_stmt = stmt();
        n->if_stmt.else_stmt = else_stmt;
    }
    return n;
}

node *stmt() {
    if (equal(TOKEN_INT_TYPE, 0)) {
        return define_variable();
    } else if (equal(TOKEN_RETURN, 0)) {
        return return_stmt();
    } else if (equal(TOKEN_IF, 0)) {
        return if_stmt();
    } else {
        node *n = expr();
        expect(';');
        return n;
    }
}

node *parse() {
    node *n = stmt();
    node *t = n;
    while (curr->kind != 0) {
        node *new = stmt();
        t->next = new;
        new->prev = t;
        t = new;
    }
    return n;
}

void emit(const char *fmt, ...) {
    va_list ap;
    FILE *fp = stdout;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    fprintf(fp, "\n");
    va_end(ap);
}

void codegen_lval(node *n) {
    if (n->node_kind != NODE_VARIABLE) {
        error("lval must be variable");
    }
    emit("  lea rax, [rbp - %d]", n->variable.index);
    emit("  push rax");
}

void codegen_expr(node *n) {
    if (n->node_kind == NODE_BINARY) {
        if (n->binary.op->kind == '=') {
            codegen_lval(n->binary.lhs);
            codegen_expr(n->binary.rhs);
            emit("  pop rdi");
            emit("  pop rax");
            emit("  mov [rax], rdi");
            emit("  push rdi");
            return;
        }
        codegen_expr(n->binary.lhs);
        codegen_expr(n->binary.rhs);
        emit("  pop rdi");
        emit("  pop rax");
        switch (n->binary.op->kind) {
            case '+':
                emit("  add rax, rdi");
                break;
            case '-':
                emit("  sub rax, rdi");
                break;
            case '*':
                emit("  imul rax, rdi");
                break;
            case '/':
                emit("  cqo");
                emit("  idiv rdi");
                break;
            case TOKEN_EQ:
                emit("  cmp rax, rdi");
                emit("  sete al");
                emit("  movzx rax, al");
                break;
            default:
                error("unimplemented op");
        }
        emit("  push rax");
    } else if (n->node_kind == NODE_UNARY) {
        codegen_expr(n->unary.expr);
        emit("  pop rax");
        switch (n->unary.op->kind) {
            case '!':
                emit("  cmp rax, 0");
                emit("  sete al");
                emit("  movzx rax, al");
                emit("  push rax");
                break;
            default:
                error("unimplemented unary op: %d", n->unary.op->kind);
        }
    } else if (n->node_kind == NODE_INTEGER) {
        emit("  push %d", n->integer.num);
    } else if (n->node_kind == NODE_VARIABLE) {
        emit("  mov rax, [rbp - %d]", n->variable.index);
        emit("  push rax");
    } else {
        error("unimplemented expr");
    }
}

void codegen_stmt(node *n) {
    if (n->node_kind == NODE_DEFVAR) {
        codegen_expr(n->defvar.expr);
        emit("  pop rax");
        emit("  mov [rbp - %d], rax", n->defvar.index);
    } else if (n->node_kind == NODE_RETURN) {
        codegen_expr(n->return_stmt.expr);
        emit("  pop rax");
        emit("  mov rsp, rbp");
        emit("  pop rbp");
        emit("  ret");
    } else if (n->node_kind == NODE_IF) {
        codegen_expr(n->if_stmt.cond);
        emit("  pop rax");
        if (n->if_stmt.else_stmt == NULL) {
            int after = block++;
            emit("  cmp rax, 0");
            emit("  je .L%d", after);
            codegen_stmt(n->if_stmt.then_stmt);
            emit(".L%d:", after);
        } else {
            int else_block = block++;
            int after = block++;
            emit("  cmp rax, 0");
            emit("  je .L%d", else_block);
            codegen_stmt(n->if_stmt.then_stmt);
            emit("  jmp .L%d", after);
            emit(".L%d:", else_block);
            codegen_stmt(n->if_stmt.else_stmt);
            emit(".L%d:", after);
        }
    } else {
        codegen_expr(n);
    }
    if (n->next != NULL) {
        codegen_stmt(n->next);
    }
}

void codegen(node *n) {
    emit(".intel_syntax noprefix");
    emit(".globl main");
    emit("main:");
    emit("  push rbp");
    emit("  mov rbp, rsp");
    emit("  sub rsp, %d", stack);
    codegen_stmt(n);
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
    codegen(n);
    // printf("eval: %d\n", eval(n));
    return 0;
}
