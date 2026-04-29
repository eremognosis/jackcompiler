//
// Created by raj on 4/29/26.
//
#include "tokenizer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "common.h"

typedef struct {
    char *name;
    KwdType type;
} KeywordMap;

static KeywordMap keywords[] = {
    {"class", K_CLASS}, {"method", K_METHOD}, {"function", K_FCN},
    {"constructor", K_CONS}, {"int", K_INT}, {"boolean", K_BOOL},
    {"char", K_CHAR}, {"void", K_VOID}, {"var", K_VAR},
    {"static", K_STATIC}, {"field", K_FIELD}, {"let", K_LET},
    {"do", K_DO}, {"if", K_IF}, {"else", K_ELSE},
    {"while", K_WHILE}, {"return", K_RETURN}, {"true", K_TRUE},
    {"false", K_FALSE}, {"null", K_NULL}, {"this", K_THIS},
    {NULL, 0}
};

static const char *symbols = "{}()[].,;+-*/&|<>=~";
void tokenizer_init(Tokenizer *tokenizer, FILE *stream)
{
    if (!stream)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_NO_FILES, "","");
        return;
    }
    tokenizer->stream = stream;
    memset(tokenizer->curr, 0, 256);
}

static void skipnonsense(Tokenizer *t) {
    int c;
    while ((c = fgetc(t->stream)) != EOF) {
        if (isspace(c)) continue;
        if (c == '/') {
            int next = fgetc(t->stream);
            if (next == '/') { // Line comment
                while ((c = fgetc(t->stream)) != EOF && c != '\n');
                continue;
            } else if (next == '*') { // Block comment
                while ((c = fgetc(t->stream)) != EOF) {
                    if (c == '*') {
                        if ((next = fgetc(t->stream)) == '/') break;
                        ungetc(next, t->stream);
                    }
                }
                continue;
            } else {
                ungetc(next, t->stream);
            }
        }
        ungetc(c, t->stream);
        break;
    }
}

void tokenizer_next(Tokenizer *t)
{
    skipnonsense(t);
    memset(t->curr, 0, 256);

    int c = fgetc(t->stream);
    if (c == EOF)
    {
        t->type = T_NONE;
        return;
    }

    if (strchr(symbols, c))
    {
        t->type = T_SYM;
        t->curr[0] = (char)c;
        return;
    }

    if (c=='"')
    {
        t->type = T_STRC;
        int i = 0;
        while ((c = fgetc(t->stream)) != EOF && c != '"'  && c!='\n' && i<255)
        {
            t->curr[i++] = (char)c;
        }
        return;
    }

    if (isdigit(c))
    {
        t->type = T_INTC;
        int i=0;
        t->curr[i++] = (char)c;
        while (isdigit(c=fgetc(t->stream))&&i<255)
        {
            t->curr[i++] = (char)c;
        }
        ungetc(c, t->stream);
        return;
    }

    if (isalpha(c)|| c=='_')
    {
        int i = 0 ;
        t->curr[i++] = (char)c;
        while ((isalnum(c = fgetc(t->stream)) || c == '_') && i < 255) {
            t->curr[i++] = (char)c;
        }
        ungetc(c, t->stream);
        for (int k = 0; keywords[k].name != NULL; k++) {
            if (strcmp(t->curr, keywords[k].name) == 0) {
                t->type = T_KWD;
                return;
            }
        }
        t->type = T_ID;
        return;
    }

}



void tokenizer_has_more(Tokenizer *t)
{
    long pos = ftell(t->stream);
    skipnonsense(t);
    int c = fgetc(t->stream);
    ungetc(c, t->stream);
    fseek(t->stream, pos, SEEK_SET);
}

TokenType tokenizer_get_type(Tokenizer *t)
{
    return t->type;
}

KwdType tokenizer_get_keyword(Tokenizer *t)
{
    for (int i = 0; keywords[i].name !=NULL; i++)
    {
        if (strcmp(t->curr, keywords[i].name) == 0) return keywords[i].type;
    }
    return K_NONE;
}

char* tokenizer_identifier(Tokenizer *t) { return t->curr; }
int   tokenizer_int(Tokenizer *t)        { return atoi(t->curr); }
char* tokenizer_strc(Tokenizer *t)       { return t->curr; }