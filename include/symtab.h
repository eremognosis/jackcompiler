//
// Created by raj on 4/29/26.
//

#ifndef CCOMP_SYMTAB_H
#define CCOMP_SYMTAB_H

/*
 * =================================================================================
 * Defoinitions (again)
 * ===================================================================================
 */
typedef enum
{
    VK_STATIC, VK_FIELD, VK_ARG, VK_VAR, VK_NONE
} VarKind_t;

typedef  struct
{
    char name[64];
    char type[64];
    VarKind_t kind;
    int index;
}Symbol;


/*
 * =========================================================================================
 * The APIs
 * =========================================================================================
 */
typedef struct SymbolTable SymbolTable; ///i think linked list will work

SymbolTable *new_SymbolTable(void);
void symtab_destroy(SymbolTable *st);
void symtab_start(SymbolTable *st);
void symtab_define(SymbolTable *st, const char *name, const char *type, VarKind_t kind);

int symtab_var_count(SymbolTable *st, VarKind_t kind);
VarKind_t symtab_var_kind(SymbolTable *st, const char *name);

char* symtab_tyoe(SymbolTable *st, const char *name);
int symtab_index(SymbolTable *st, const char *name);


#endif //CCOMP_SYMTAB_H
