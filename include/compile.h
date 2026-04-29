//
// Created by raj on 4/29/26.
//

#ifndef CCOMP_COMPILE_H
#define CCOMP_COMPILE_H

#include "tokenizer.h";
#include "symtab.h";

void compile_class(Tokenizer *t, SymbolTable *st, FILE *out);

void compile_class_var_dec(Tokenizer *t, SymbolTable *st);
void compile_subroutine(Tokenizer *t, SymbolTable *st, FILE *out);
void compile_parameter_list(Tokenizer *t, SymbolTable *st);
void compile_var_dec(Tokenizer *t, SymbolTable *st);
void compile_statements(Tokenizer *t, SymbolTable *st, FILE *out);
void compile_let(Tokenizer *t, SymbolTable *st, FILE *out);
void compile_if(Tokenizer *t, SymbolTable *st, FILE *out);
void compile_while(Tokenizer *t, SymbolTable *st, FILE *out);
void compile_do(Tokenizer *t, SymbolTable *st, FILE *out);
void compile_return(Tokenizer *t, SymbolTable *st, FILE *out);
void compile_expression(Tokenizer *t, SymbolTable *st, FILE *out);
void compile_term(Tokenizer *t, SymbolTable *st, FILE *out);
int  compile_expression_list(Tokenizer *t, SymbolTable *st, FILE *out);

#endif //CCOMP_COMPILE_H
