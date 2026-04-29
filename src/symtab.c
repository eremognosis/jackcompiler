#include "symtab.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>

typedef struct Node {
    Symbol data;
    struct Node *next;
} Node; /// here we go again, implementing linked list for 3838th time

struct SymbolTable {
    Node *class_scope;     // for STATIC and FIELD
    Node *sub_scope;       // ARG and VAR
    int counts[4];         // Indexed by VarKind_t
};

SymbolTable *new_SymbolTable(void) {
    SymbolTable *st = malloc(sizeof(SymbolTable));
    if (!st)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_SYMTAB_ALLOC, "symtab", "failed to allocate symbol table");
        return NULL;
    }
    st->class_scope = NULL;
    st->sub_scope = NULL;
    for (int i = 0; i < 4; i++) st->counts[i] = 0;
    return st;
}

static void free_list(Node *head) {
    while (head) {
        Node *temp = head;
        head = head->next;
        free(temp); /// garbage
    }
}

void symtab_destroy(SymbolTable *st) {
    if (!st)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_SYMTAB_NULL_TABLE, "symtab", "symtab_destroy called with NULL table");
        return;
    }
    free_list(st->class_scope);
    free_list(st->sub_scope);
    free(st);
}

void symtab_start(SymbolTable *st) {
    if (!st)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_SYMTAB_NULL_TABLE, "symtab", "symtab_start called with NULL table");
        return;
    }
    free_list(st->sub_scope);
    st->sub_scope = NULL;
    st->counts[K_ARG] = 0;
    st->counts[K_VAR] = 0;
}

void symtab_define(SymbolTable *st, const char *name, const char *type, VarKind_t kind) {
    if (!st) {
        CCOMP_LOG_ERROR(CCOMP_ERR_SYMTAB_NULL_TABLE, "symtab", "symtab_define called with NULL table");
        return;
    }
    if (!name || !type || kind == K_NONE) {
        CCOMP_LOG_ERROR(CCOMP_ERR_SYMTAB_INVALID_INPUT, "symtab", "symtab_define got invalid input");
        return;
    }

    Node *new_node = malloc(sizeof(Node));
    if (!new_node) {
        CCOMP_LOG_ERROR(CCOMP_ERR_SYMTAB_ALLOC, "symtab", "failed to allocate symbol node");
        return;
    }

    strncpy(new_node->data.name, name, 63);
    new_node->data.name[63] = '\0';
    strncpy(new_node->data.type, type, 63);
    new_node->data.type[63] = '\0';
    
    new_node->data.kind = kind;
    new_node->data.index = st->counts[kind]++;

    if (kind == K_STATIC || kind == K_FIELD) {
        new_node->next = st->class_scope;
        st->class_scope = new_node;
    } else {
        new_node->next = st->sub_scope;
        st->sub_scope = new_node;
    }
}

int symtab_var_count(SymbolTable *st, VarKind_t kind) {
    return (st && kind != K_NONE) ? st->counts[kind] : 0;
}

static Symbol* find_symbol(SymbolTable *st, const char *name) {
    if (!st || !name) {
        return NULL;
    }
    Node *curr = st->sub_scope;
    while (curr) {
        if (strcmp(curr->data.name, name) == 0) return &curr->data;
        curr = curr->next;
    }
    curr = st->class_scope;
    while (curr) {
        if (strcmp(curr->data.name, name) == 0) return &curr->data;
        curr = curr->next;
    }
    return NULL;
}

VarKind_t symtab_var_kind(SymbolTable *st, const char *name) {
    Symbol *s = find_symbol(st, name);
    return s ? s->kind : K_NONE;
}

char* symtab_tyoe(SymbolTable *st, const char *name) {
    Symbol *s = find_symbol(st, name);
    return s ? s->type : NULL;
}

int symtab_index(SymbolTable *st, const char *name) {
    Symbol *s = find_symbol(st, name);
    return s ? s->index : -1;
}
