//
// Created by raj on 4/29/26.
//

#ifndef CCOMP_TOKENIZER_H
#define CCOMP_TOKENIZER_H

#include <stdio.h>

/*
========================================================================
So that we dont have pain in ass while writing case statemwent
========================================================================
 */

typedef enum
{
    T_KWD,  T_SYM, T_ID, T_INTC, T_STRC, T_NONE
} TokenType;

typedef enum
{
    K_CLASS,
    K_METHOD,
    K_FCN,
    K_CONS,
    K_INT,
    K_BOOL,
    K_CHAR,
    K_VOID,
    K_VAR,
    K_STATIC,
    K_FIELD,
    K_LET, K_DO, K_IF, K_ELSE, K_WHILE,
    K_RETURN,
    K_TRUE, K_FALSE, K_NULL,
    K_THIS, K_THAT,
    K_NONE
}KwdType;

///=======================================================================================///

///=======================================================================================///
/// THE MAIN STATE
///=======================================================================================///
typedef struct
{
    FILE *stream;
    char curr[256];
    TokenType type;
} Tokenizer;

///=======================================================================================///

/*
========================================================================
Thr functions
========================================================================
 */


/// core vitals

void tokenizer_init(Tokenizer *tokenizer, FILE *stream);
void tokenizer_has_more(Tokenizer *t);
void tokenizer_next(Tokenizer *t);


/// getters

TokenType tokenizer_get_type(Tokenizer *t);
KwdType tokenizer_get_keyword(Tokenizer *t);
char* tokenizer_identifier(Tokenizer *t);
int tokenizer_int(Tokenizer *t);
char* tokenizer_strc(Tokenizer *t);

#endif //CCOMP_TOKENIZER_H
