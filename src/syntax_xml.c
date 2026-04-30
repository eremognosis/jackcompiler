#include "syntax_xml.h"

#include "common.h"
#include "tokenizer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    Tokenizer tokenizer;
    FILE *out;
    int indent;
    int ok;
} XmlState;

static int is_sym(XmlState *s, char c) {
    return tokenizer_get_type(&s->tokenizer) == T_SYM &&
           s->tokenizer.curr[0] == c &&
           s->tokenizer.curr[1] == '\0';
}

static int is_kwd(XmlState *s, KwdType kwd) {
    return tokenizer_get_type(&s->tokenizer) == T_KWD &&
           tokenizer_get_keyword(&s->tokenizer) == kwd;
}

static int is_op(XmlState *s) {
    if (tokenizer_get_type(&s->tokenizer) != T_SYM) {
        return 0;
    }
    return strchr("+-*/&|<>=", s->tokenizer.curr[0]) != NULL;
}

static void next_tok(XmlState *s) {
    tokenizer_next(&s->tokenizer);
}

static void parse_error(XmlState *s, const char *fn, const char *msg) {
    ccomp_log_errorf(
        "CC-XML-001",
        "syntax_xml",
        __FILE__,
        __LINE__,
        fn,
        "%s near token '%s'",
        msg,
        s ? s->tokenizer.curr : ""
    );
    if (s) {
        s->ok = 0;
    }
}

static void write_indent(XmlState *s) {
    for (int i = 0; i < s->indent; i++) {
        fputs("  ", s->out);
    }
}

static void escape_xml(FILE *out, const char *text) {
    for (const char *p = text; *p != '\0'; p++) {
        switch (*p) {
        case '<':
            fputs("&lt;", out);
            break;
        case '>':
            fputs("&gt;", out);
            break;
        case '&':
            fputs("&amp;", out);
            break;
        default:
            fputc(*p, out);
            break;
        }
    }
}

static void write_terminal(XmlState *s) {
    const char *tag = NULL;
    TokenType ttype = tokenizer_get_type(&s->tokenizer);

    switch (ttype) {
    case T_KWD:
        tag = "keyword";
        break;
    case T_SYM:
        tag = "symbol";
        break;
    case T_ID:
        tag = "identifier";
        break;
    case T_INTC:
        tag = "integerConstant";
        break;
    case T_STRC:
        tag = "stringConstant";
        break;
    default:
        parse_error(s, __func__, "unexpected token while writing terminal");
        return;
    }

    write_indent(s);
    fprintf(s->out, "<%s> ", tag);
    escape_xml(s->out, s->tokenizer.curr);
    fprintf(s->out, " </%s>\n", tag);
}

static void open_tag(XmlState *s, const char *tag) {
    write_indent(s);
    fprintf(s->out, "<%s>\n", tag);
    s->indent++;
}

static void close_tag(XmlState *s, const char *tag) {
    s->indent--;
    write_indent(s);
    fprintf(s->out, "</%s>\n", tag);
}

static int expect_sym(XmlState *s, char c, const char *fn) {
    if (!is_sym(s, c)) {
        parse_error(s, fn, "expected symbol");
        return 0;
    }
    write_terminal(s);
    next_tok(s);
    return 1;
}

static int expect_kwd(XmlState *s, KwdType kwd, const char *fn) {
    if (!is_kwd(s, kwd)) {
        parse_error(s, fn, "expected keyword");
        return 0;
    }
    write_terminal(s);
    next_tok(s);
    return 1;
}

static int expect_id(XmlState *s, const char *fn) {
    if (tokenizer_get_type(&s->tokenizer) != T_ID) {
        parse_error(s, fn, "expected identifier");
        return 0;
    }
    write_terminal(s);
    next_tok(s);
    return 1;
}

static int expect_type(XmlState *s, const char *fn, int allow_void) {
    if (tokenizer_get_type(&s->tokenizer) == T_ID) {
        write_terminal(s);
        next_tok(s);
        return 1;
    }
    if (tokenizer_get_type(&s->tokenizer) == T_KWD) {
        KwdType k = tokenizer_get_keyword(&s->tokenizer);
        if (k == K_INT || k == K_CHAR || k == K_BOOL || (allow_void && k == K_VOID)) {
            write_terminal(s);
            next_tok(s);
            return 1;
        }
    }
    parse_error(s, fn, "expected type");
    return 0;
}

static void compile_expression(XmlState *s);

static void compile_expression_list(XmlState *s) {
    open_tag(s, "expressionList");
    if (!is_sym(s, ')')) {
        compile_expression(s);
        while (s->ok && is_sym(s, ',')) {
            expect_sym(s, ',', __func__);
            compile_expression(s);
        }
    }
    close_tag(s, "expressionList");
}

static void compile_term(XmlState *s) {
    open_tag(s, "term");

    if (tokenizer_get_type(&s->tokenizer) == T_INTC ||
        tokenizer_get_type(&s->tokenizer) == T_STRC) {
        write_terminal(s);
        next_tok(s);
        close_tag(s, "term");
        return;
    }

    if (tokenizer_get_type(&s->tokenizer) == T_KWD) {
        KwdType k = tokenizer_get_keyword(&s->tokenizer);
        if (k == K_TRUE || k == K_FALSE || k == K_NULL || k == K_THIS) {
            write_terminal(s);
            next_tok(s);
            close_tag(s, "term");
            return;
        }
    }

    if (is_sym(s, '(')) {
        expect_sym(s, '(', __func__);
        compile_expression(s);
        expect_sym(s, ')', __func__);
        close_tag(s, "term");
        return;
    }

    if (is_sym(s, '-') || is_sym(s, '~')) {
        if (is_sym(s, '-')) {
            expect_sym(s, '-', __func__);
        } else {
            expect_sym(s, '~', __func__);
        }
        compile_term(s);
        close_tag(s, "term");
        return;
    }

    if (tokenizer_get_type(&s->tokenizer) == T_ID) {
        expect_id(s, __func__);
        if (is_sym(s, '[')) {
            expect_sym(s, '[', __func__);
            compile_expression(s);
            expect_sym(s, ']', __func__);
            close_tag(s, "term");
            return;
        }
        if (is_sym(s, '.')) {
            expect_sym(s, '.', __func__);
            expect_id(s, __func__);
        }
        if (is_sym(s, '(')) {
            expect_sym(s, '(', __func__);
            compile_expression_list(s);
            expect_sym(s, ')', __func__);
        }
        close_tag(s, "term");
        return;
    }

    parse_error(s, __func__, "invalid term");
    close_tag(s, "term");
}

static void compile_expression(XmlState *s) {
    open_tag(s, "expression");
    compile_term(s);
    while (s->ok && is_op(s)) {
        write_terminal(s);
        next_tok(s);
        compile_term(s);
    }
    close_tag(s, "expression");
}

static void compile_statements(XmlState *s);

static void compile_let(XmlState *s) {
    open_tag(s, "letStatement");
    expect_kwd(s, K_LET, __func__);
    expect_id(s, __func__);
    if (is_sym(s, '[')) {
        expect_sym(s, '[', __func__);
        compile_expression(s);
        expect_sym(s, ']', __func__);
    }
    expect_sym(s, '=', __func__);
    compile_expression(s);
    expect_sym(s, ';', __func__);
    close_tag(s, "letStatement");
}

static void compile_if(XmlState *s) {
    open_tag(s, "ifStatement");
    expect_kwd(s, K_IF, __func__);
    expect_sym(s, '(', __func__);
    compile_expression(s);
    expect_sym(s, ')', __func__);
    expect_sym(s, '{', __func__);
    compile_statements(s);
    expect_sym(s, '}', __func__);
    if (is_kwd(s, K_ELSE)) {
        expect_kwd(s, K_ELSE, __func__);
        expect_sym(s, '{', __func__);
        compile_statements(s);
        expect_sym(s, '}', __func__);
    }
    close_tag(s, "ifStatement");
}

static void compile_while(XmlState *s) {
    open_tag(s, "whileStatement");
    expect_kwd(s, K_WHILE, __func__);
    expect_sym(s, '(', __func__);
    compile_expression(s);
    expect_sym(s, ')', __func__);
    expect_sym(s, '{', __func__);
    compile_statements(s);
    expect_sym(s, '}', __func__);
    close_tag(s, "whileStatement");
}

static void compile_do(XmlState *s) {
    open_tag(s, "doStatement");
    expect_kwd(s, K_DO, __func__);
    expect_id(s, __func__);
    if (is_sym(s, '.')) {
        expect_sym(s, '.', __func__);
        expect_id(s, __func__);
    }
    expect_sym(s, '(', __func__);
    compile_expression_list(s);
    expect_sym(s, ')', __func__);
    expect_sym(s, ';', __func__);
    close_tag(s, "doStatement");
}

static void compile_return(XmlState *s) {
    open_tag(s, "returnStatement");
    expect_kwd(s, K_RETURN, __func__);
    if (!is_sym(s, ';')) {
        compile_expression(s);
    }
    expect_sym(s, ';', __func__);
    close_tag(s, "returnStatement");
}

static void compile_statements(XmlState *s) {
    open_tag(s, "statements");
    while (tokenizer_get_type(&s->tokenizer) == T_KWD && s->ok) {
        switch (tokenizer_get_keyword(&s->tokenizer)) {
        case K_LET:
            compile_let(s);
            break;
        case K_IF:
            compile_if(s);
            break;
        case K_WHILE:
            compile_while(s);
            break;
        case K_DO:
            compile_do(s);
            break;
        case K_RETURN:
            compile_return(s);
            break;
        default:
            close_tag(s, "statements");
            return;
        }
    }
    close_tag(s, "statements");
}

static void compile_var_dec(XmlState *s) {
    open_tag(s, "varDec");
    expect_kwd(s, K_VAR, __func__);
    expect_type(s, __func__, 0);
    expect_id(s, __func__);
    while (is_sym(s, ',')) {
        expect_sym(s, ',', __func__);
        expect_id(s, __func__);
    }
    expect_sym(s, ';', __func__);
    close_tag(s, "varDec");
}

static void compile_parameter_list(XmlState *s) {
    open_tag(s, "parameterList");
    if (!is_sym(s, ')')) {
        expect_type(s, __func__, 0);
        expect_id(s, __func__);
        while (is_sym(s, ',')) {
            expect_sym(s, ',', __func__);
            expect_type(s, __func__, 0);
            expect_id(s, __func__);
        }
    }
    close_tag(s, "parameterList");
}

static void compile_subroutine(XmlState *s) {
    open_tag(s, "subroutineDec");
    if (is_kwd(s, K_CONS)) {
        expect_kwd(s, K_CONS, __func__);
    } else if (is_kwd(s, K_FCN)) {
        expect_kwd(s, K_FCN, __func__);
    } else {
        expect_kwd(s, K_METHOD, __func__);
    }
    expect_type(s, __func__, 1);
    expect_id(s, __func__);
    expect_sym(s, '(', __func__);
    compile_parameter_list(s);
    expect_sym(s, ')', __func__);

    open_tag(s, "subroutineBody");
    expect_sym(s, '{', __func__);
    while (is_kwd(s, K_VAR)) {
        compile_var_dec(s);
    }
    compile_statements(s);
    expect_sym(s, '}', __func__);
    close_tag(s, "subroutineBody");

    close_tag(s, "subroutineDec");
}

static void compile_class_var_dec(XmlState *s) {
    open_tag(s, "classVarDec");
    if (is_kwd(s, K_STATIC)) {
        expect_kwd(s, K_STATIC, __func__);
    } else {
        expect_kwd(s, K_FIELD, __func__);
    }
    expect_type(s, __func__, 0);
    expect_id(s, __func__);
    while (is_sym(s, ',')) {
        expect_sym(s, ',', __func__);
        expect_id(s, __func__);
    }
    expect_sym(s, ';', __func__);
    close_tag(s, "classVarDec");
}

static void compile_class(XmlState *s) {
    open_tag(s, "class");
    expect_kwd(s, K_CLASS, __func__);
    expect_id(s, __func__);
    expect_sym(s, '{', __func__);
    while (is_kwd(s, K_STATIC) || is_kwd(s, K_FIELD)) {
        compile_class_var_dec(s);
    }
    while (is_kwd(s, K_CONS) || is_kwd(s, K_FCN) || is_kwd(s, K_METHOD)) {
        compile_subroutine(s);
    }
    expect_sym(s, '}', __func__);
    close_tag(s, "class");
}

int write_syntax_tree_xml(FILE *in, FILE *out) {
    XmlState state;

    if (!in || !out) {
        CCOMP_LOG_ERROR("CC-XML-002", "syntax_xml", "input or output stream is NULL");
        return 0;
    }

    tokenizer_init(&state.tokenizer, in);
    state.out = out;
    state.indent = 0;
    state.ok = 1;

    next_tok(&state);
    if (tokenizer_get_type(&state.tokenizer) == T_NONE) {
        parse_error(&state, __func__, "empty input");
        return 0;
    }

    compile_class(&state);

    if (state.ok && tokenizer_get_type(&state.tokenizer) != T_NONE) {
        parse_error(&state, __func__, "unexpected trailing tokens");
    }
    return state.ok;
}
