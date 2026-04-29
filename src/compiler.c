#include "compile.h"
#include "common.h"
#include "vmwriter.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char cls_name[128];
static int lbl_id = 0;

static void parse_error(const char *fn, Tokenizer *t, const char *msg) {
    ccomp_log_errorf("CC-CMP-001", "compiler", __FILE__, __LINE__, fn, "%s near token '%s'", msg, t ? t->curr : "");
}

static int is_sym(Tokenizer *t, char c) {
    return tokenizer_get_type(t) == T_SYM && t->curr[0] == c && t->curr[1] == '\0';
}

static int is_kwd(Tokenizer *t, KwdType kwd) {
    return tokenizer_get_type(t) == T_KWD && tokenizer_get_keyword(t) == kwd;
}

static int is_op(Tokenizer *t) {
    if (tokenizer_get_type(t) != T_SYM) {
        return 0;
    }
    return strchr("+-*/&|<>=", t->curr[0]) != NULL;
}

static void next_tok(Tokenizer *t) {
    tokenizer_next(t);
}

static int expect_sym(Tokenizer *t, char c, const char *fn) {
    if (!is_sym(t, c)) {
        parse_error(fn, t, "expected symbol");
        return 0;
    }
    next_tok(t);
    return 1;
}

static int expect_kwd(Tokenizer *t, KwdType kwd, const char *fn) {
    if (!is_kwd(t, kwd)) {
        parse_error(fn, t, "expected keyword");
        return 0;
    }
    next_tok(t);
    return 1;
}

static int expect_id(Tokenizer *t, char *out, size_t out_len, const char *fn) {
    if (tokenizer_get_type(t) != T_ID) {
        parse_error(fn, t, "expected identifier");
        return 0;
    }
    strncpy(out, tokenizer_identifier(t), out_len - 1);
    out[out_len - 1] = '\0';
    next_tok(t);
    return 1;
}

static int read_type(Tokenizer *t, char *out, size_t out_len, const char *fn, int allow_void) {
    if (tokenizer_get_type(t) == T_ID) {
        strncpy(out, tokenizer_identifier(t), out_len - 1);
        out[out_len - 1] = '\0';
        next_tok(t);
        return 1;
    }
    if (tokenizer_get_type(t) == T_KWD) {
        KwdType k = tokenizer_get_keyword(t);
        if (k == K_INT || k == K_CHAR || k == K_BOOL || (allow_void && k == K_VOID)) {
            strncpy(out, tokenizer_identifier(t), out_len - 1);
            out[out_len - 1] = '\0';
            next_tok(t);
            return 1;
        }
    }
    parse_error(fn, t, "expected type");
    return 0;
}

static Segment kind_to_segment(VarKind_t kind) {
    switch (kind) {
    case VK_STATIC: return SEG_STATIC;
    case VK_FIELD:  return SEG_THIS;
    case VK_ARG:    return SEG_ARG;
    case VK_VAR:    return SEG_LOCAL;
    default:        return SEG_TEMP;
    }
}

static int push_var(SymbolTable *st, FILE *out, const char *name) {
    VarKind_t kind = symtab_var_kind(st, name);
    int idx = symtab_index(st, name);
    if (kind == VK_NONE || idx < 0) {
        ccomp_log_errorf("CC-CMP-002", "compiler", __FILE__, __LINE__, __func__, "unknown variable '%s'", name);
        return 0;
    }
    vm_write_push(out, kind_to_segment(kind), idx);
    return 1;
}

static int pop_var(SymbolTable *st, FILE *out, const char *name) {
    VarKind_t kind = symtab_var_kind(st, name);
    int idx = symtab_index(st, name);
    if (kind == VK_NONE || idx < 0) {
        ccomp_log_errorf("CC-CMP-003", "compiler", __FILE__, __LINE__, __func__, "unknown variable '%s'", name);
        return 0;
    }
    vm_write_pop(out, kind_to_segment(kind), idx);
    return 1;
}

static void write_label(char *out, size_t out_len, const char *prefix, int id) {
    snprintf(out, out_len, "%s%d", prefix, id);
}

static int compile_subroutine_call(Tokenizer *t, SymbolTable *st, FILE *out, const char *first_name) {
    char left[128];
    char right[128];
    int n_args = 0;

    strncpy(left, first_name, sizeof(left) - 1);
    left[sizeof(left) - 1] = '\0';

    if (is_sym(t, '.')) {
        next_tok(t);
        if (!expect_id(t, right, sizeof(right), __func__)) {
            return 0;
        }

        if (symtab_var_kind(st, left) != VK_NONE) {
            char call_name[256];
            char *type_name = symtab_tyoe(st, left);
            if (!type_name) {
                return 0;
            }
            push_var(st, out, left);
            snprintf(call_name, sizeof(call_name), "%s.%s", type_name, right);
            if (!expect_sym(t, '(', __func__)) {
                return 0;
            }
            n_args = 1 + compile_expression_list(t, st, out);
            if (!expect_sym(t, ')', __func__)) {
                return 0;
            }
            vm_write_call(out, call_name, n_args);
            return 1;
        } else {
            char call_name[256];
            snprintf(call_name, sizeof(call_name), "%s.%s", left, right);
            if (!expect_sym(t, '(', __func__)) {
                return 0;
            }
            n_args = compile_expression_list(t, st, out);
            if (!expect_sym(t, ')', __func__)) {
                return 0;
            }
            vm_write_call(out, call_name, n_args);
            return 1;
        }
    }

    if (!expect_sym(t, '(', __func__)) {
        return 0;
    }
    vm_write_push(out, SEG_POINTER, 0);
    n_args = 1 + compile_expression_list(t, st, out);
    if (!expect_sym(t, ')', __func__)) {
        return 0;
    }

    {
        char call_name[256];
        snprintf(call_name, sizeof(call_name), "%s.%s", cls_name, left);
        vm_write_call(out, call_name, n_args);
    }
    return 1;
}

void compile_class(Tokenizer *t, SymbolTable *st, FILE *out) {
    char name[128];

    if (!t || !st || !out) {
        CCOMP_LOG_ERROR("CC-CMP-004", "compiler", "compile_class got NULL input");
        return;
    }

    next_tok(t);
    if (!expect_kwd(t, K_CLASS, __func__)) {
        return;
    }
    if (!expect_id(t, name, sizeof(name), __func__)) {
        return;
    }
    strncpy(cls_name, name, sizeof(cls_name) - 1);
    cls_name[sizeof(cls_name) - 1] = '\0';

    if (!expect_sym(t, '{', __func__)) {
        return;
    }

    while (is_kwd(t, K_STATIC) || is_kwd(t, K_FIELD)) {
        compile_class_var_dec(t, st);
    }
    while (is_kwd(t, K_CONS) || is_kwd(t, K_FCN) || is_kwd(t, K_METHOD)) {
        compile_subroutine(t, st, out);
    }

    expect_sym(t, '}', __func__);
}

void compile_class_var_dec(Tokenizer *t, SymbolTable *st) {
    VarKind_t kind;
    char type[128];
    char name[128];

    if (is_kwd(t, K_STATIC)) {
        kind = VK_STATIC;
    } else if (is_kwd(t, K_FIELD)) {
        kind = VK_FIELD;
    } else {
        parse_error(__func__, t, "expected static or field");
        return;
    }
    next_tok(t);

    if (!read_type(t, type, sizeof(type), __func__, 0)) {
        return;
    }

    if (!expect_id(t, name, sizeof(name), __func__)) {
        return;
    }
    symtab_define(st, name, type, kind);

    while (is_sym(t, ',')) {
        next_tok(t);
        if (!expect_id(t, name, sizeof(name), __func__)) {
            return;
        }
        symtab_define(st, name, type, kind);
    }

    expect_sym(t, ';', __func__);
}

void compile_subroutine(Tokenizer *t, SymbolTable *st, FILE *out) {
    KwdType sk;
    char ret_type[128];
    char sub_name[128];
    char fn_name[256];

    if (!t || !st || !out) {
        return;
    }

    sk = tokenizer_get_keyword(t);
    if (!(sk == K_CONS || sk == K_FCN || sk == K_METHOD)) {
        parse_error(__func__, t, "expected subroutine keyword");
        return;
    }
    next_tok(t);

    symtab_start(st);
    if (sk == K_METHOD) {
        symtab_define(st, "this", cls_name, VK_ARG);
    }

    if (!read_type(t, ret_type, sizeof(ret_type), __func__, 1)) {
        return;
    }
    if (!expect_id(t, sub_name, sizeof(sub_name), __func__)) {
        return;
    }
    snprintf(fn_name, sizeof(fn_name), "%s.%s", cls_name, sub_name);

    if (!expect_sym(t, '(', __func__)) {
        return;
    }
    compile_parameter_list(t, st);
    if (!expect_sym(t, ')', __func__)) {
        return;
    }

    if (!expect_sym(t, '{', __func__)) {
        return;
    }

    while (is_kwd(t, K_VAR)) {
        compile_var_dec(t, st);
    }

    vm_write_function(out, fn_name, symtab_var_count(st, VK_VAR));
    if (sk == K_CONS) {
        vm_write_push(out, SEG_CONST, symtab_var_count(st, VK_FIELD));
        vm_write_call(out, "Memory.alloc", 1);
        vm_write_pop(out, SEG_POINTER, 0);
    } else if (sk == K_METHOD) {
        vm_write_push(out, SEG_ARG, 0);
        vm_write_pop(out, SEG_POINTER, 0);
    }

    compile_statements(t, st, out);
    expect_sym(t, '}', __func__);
}

void compile_parameter_list(Tokenizer *t, SymbolTable *st) {
    char type[128];
    char name[128];

    if (is_sym(t, ')')) {
        return;
    }

    if (!read_type(t, type, sizeof(type), __func__, 0)) {
        return;
    }
    if (!expect_id(t, name, sizeof(name), __func__)) {
        return;
    }
    symtab_define(st, name, type, VK_ARG);

    while (is_sym(t, ',')) {
        next_tok(t);
        if (!read_type(t, type, sizeof(type), __func__, 0)) {
            return;
        }
        if (!expect_id(t, name, sizeof(name), __func__)) {
            return;
        }
        symtab_define(st, name, type, VK_ARG);
    }
}

void compile_var_dec(Tokenizer *t, SymbolTable *st) {
    char type[128];
    char name[128];

    if (!expect_kwd(t, K_VAR, __func__)) {
        return;
    }
    if (!read_type(t, type, sizeof(type), __func__, 0)) {
        return;
    }

    if (!expect_id(t, name, sizeof(name), __func__)) {
        return;
    }
    symtab_define(st, name, type, VK_VAR);

    while (is_sym(t, ',')) {
        next_tok(t);
        if (!expect_id(t, name, sizeof(name), __func__)) {
            return;
        }
        symtab_define(st, name, type, VK_VAR);
    }

    expect_sym(t, ';', __func__);
}

void compile_statements(Tokenizer *t, SymbolTable *st, FILE *out) {
    while (tokenizer_get_type(t) == T_KWD) {
        switch (tokenizer_get_keyword(t)) {
        case K_LET:    compile_let(t, st, out); break;
        case K_IF:     compile_if(t, st, out); break;
        case K_WHILE:  compile_while(t, st, out); break;
        case K_DO:     compile_do(t, st, out); break;
        case K_RETURN: compile_return(t, st, out); break;
        default:       return;
        }
    }
}

void compile_let(Tokenizer *t, SymbolTable *st, FILE *out) {
    char name[128];
    int is_array = 0;

    if (!expect_kwd(t, K_LET, __func__)) {
        return;
    }
    if (!expect_id(t, name, sizeof(name), __func__)) {
        return;
    }

    if (is_sym(t, '[')) {
        is_array = 1;
        push_var(st, out, name);
        next_tok(t);
        compile_expression(t, st, out);
        if (!expect_sym(t, ']', __func__)) {
            return;
        }
        vm_write_arithmetic(out, CMD_ADD);
    }

    if (!expect_sym(t, '=', __func__)) {
        return;
    }
    compile_expression(t, st, out);
    if (!expect_sym(t, ';', __func__)) {
        return;
    }

    if (is_array) {
        vm_write_pop(out, SEG_TEMP, 0);
        vm_write_pop(out, SEG_POINTER, 1);
        vm_write_push(out, SEG_TEMP, 0);
        vm_write_pop(out, SEG_THAT, 0);
    } else {
        pop_var(st, out, name);
    }
}

void compile_if(Tokenizer *t, SymbolTable *st, FILE *out) {
    int id = lbl_id++;
    char l_true[64];
    char l_false[64];
    char l_end[64];

    write_label(l_true, sizeof(l_true), "IF_TRUE", id);
    write_label(l_false, sizeof(l_false), "IF_FALSE", id);
    write_label(l_end, sizeof(l_end), "IF_END", id);

    if (!expect_kwd(t, K_IF, __func__)) {
        return;
    }
    if (!expect_sym(t, '(', __func__)) {
        return;
    }
    compile_expression(t, st, out);
    if (!expect_sym(t, ')', __func__)) {
        return;
    }

    vm_write_if(out, l_true);
    vm_write_goto(out, l_false);
    vm_write_label(out, l_true);

    if (!expect_sym(t, '{', __func__)) {
        return;
    }
    compile_statements(t, st, out);
    if (!expect_sym(t, '}', __func__)) {
        return;
    }

    if (is_kwd(t, K_ELSE)) {
        vm_write_goto(out, l_end);
        vm_write_label(out, l_false);
        next_tok(t);
        if (!expect_sym(t, '{', __func__)) {
            return;
        }
        compile_statements(t, st, out);
        if (!expect_sym(t, '}', __func__)) {
            return;
        }
        vm_write_label(out, l_end);
    } else {
        vm_write_label(out, l_false);
    }
}

void compile_while(Tokenizer *t, SymbolTable *st, FILE *out) {
    int id = lbl_id++;
    char l_exp[64];
    char l_end[64];

    write_label(l_exp, sizeof(l_exp), "WHILE_EXP", id);
    write_label(l_end, sizeof(l_end), "WHILE_END", id);

    if (!expect_kwd(t, K_WHILE, __func__)) {
        return;
    }
    vm_write_label(out, l_exp);

    if (!expect_sym(t, '(', __func__)) {
        return;
    }
    compile_expression(t, st, out);
    if (!expect_sym(t, ')', __func__)) {
        return;
    }
    vm_write_arithmetic(out, CMD_NOT);
    vm_write_if(out, l_end);

    if (!expect_sym(t, '{', __func__)) {
        return;
    }
    compile_statements(t, st, out);
    if (!expect_sym(t, '}', __func__)) {
        return;
    }

    vm_write_goto(out, l_exp);
    vm_write_label(out, l_end);
}

void compile_do(Tokenizer *t, SymbolTable *st, FILE *out) {
    char name[128];

    if (!expect_kwd(t, K_DO, __func__)) {
        return;
    }
    if (!expect_id(t, name, sizeof(name), __func__)) {
        return;
    }
    if (!compile_subroutine_call(t, st, out, name)) {
        return;
    }
    if (!expect_sym(t, ';', __func__)) {
        return;
    }
    vm_write_pop(out, SEG_TEMP, 0);
}

void compile_return(Tokenizer *t, SymbolTable *st, FILE *out) {
    (void)st;
    if (!expect_kwd(t, K_RETURN, __func__)) {
        return;
    }
    if (!is_sym(t, ';')) {
        compile_expression(t, st, out);
    } else {
        vm_write_push(out, SEG_CONST, 0);
    }
    if (!expect_sym(t, ';', __func__)) {
        return;
    }
    vm_write_return(out);
}

void compile_expression(Tokenizer *t, SymbolTable *st, FILE *out) {
    compile_term(t, st, out);
    while (is_op(t)) {
        char op = t->curr[0];
        next_tok(t);
        compile_term(t, st, out);
        switch (op) {
        case '+': vm_write_arithmetic(out, CMD_ADD); break;
        case '-': vm_write_arithmetic(out, CMD_SUB); break;
        case '*': vm_write_call(out, "Math.multiply", 2); break;
        case '/': vm_write_call(out, "Math.divide", 2); break;
        case '&': vm_write_arithmetic(out, CMD_AND); break;
        case '|': vm_write_arithmetic(out, CMD_OR); break;
        case '<': vm_write_arithmetic(out, CMD_LT); break;
        case '>': vm_write_arithmetic(out, CMD_GT); break;
        case '=': vm_write_arithmetic(out, CMD_EQ); break;
        default:  parse_error(__func__, t, "unknown operator"); return;
        }
    }
}

void compile_term(Tokenizer *t, SymbolTable *st, FILE *out) {
    if (tokenizer_get_type(t) == T_INTC) {
        vm_write_push(out, SEG_CONST, tokenizer_int(t));
        next_tok(t);
        return;
    }

    if (tokenizer_get_type(t) == T_STRC) {
        const char *s = tokenizer_strc(t);
        size_t n = strlen(s);
        vm_write_push(out, SEG_CONST, (int)n);
        vm_write_call(out, "String.new", 1);
        for (size_t i = 0; i < n; i++) {
            vm_write_push(out, SEG_CONST, (unsigned char)s[i]);
            vm_write_call(out, "String.appendChar", 2);
        }
        next_tok(t);
        return;
    }

    if (tokenizer_get_type(t) == T_KWD) {
        KwdType k = tokenizer_get_keyword(t);
        if (k == K_TRUE) {
            vm_write_push(out, SEG_CONST, 0);
            vm_write_arithmetic(out, CMD_NOT);
            next_tok(t);
            return;
        }
        if (k == K_FALSE || k == K_NULL) {
            vm_write_push(out, SEG_CONST, 0);
            next_tok(t);
            return;
        }
        if (k == K_THIS) {
            vm_write_push(out, SEG_POINTER, 0);
            next_tok(t);
            return;
        }
    }

    if (is_sym(t, '(')) {
        next_tok(t);
        compile_expression(t, st, out);
        expect_sym(t, ')', __func__);
        return;
    }

    if (is_sym(t, '-') || is_sym(t, '~')) {
        char op = t->curr[0];
        next_tok(t);
        compile_term(t, st, out);
        if (op == '-') {
            vm_write_arithmetic(out, CMD_NEG);
        } else {
            vm_write_arithmetic(out, CMD_NOT);
        }
        return;
    }

    if (tokenizer_get_type(t) == T_ID) {
        char name[128];
        strncpy(name, tokenizer_identifier(t), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        next_tok(t);

        if (is_sym(t, '[')) {
            push_var(st, out, name);
            next_tok(t);
            compile_expression(t, st, out);
            if (!expect_sym(t, ']', __func__)) {
                return;
            }
            vm_write_arithmetic(out, CMD_ADD);
            vm_write_pop(out, SEG_POINTER, 1);
            vm_write_push(out, SEG_THAT, 0);
            return;
        }

        if (is_sym(t, '(') || is_sym(t, '.')) {
            compile_subroutine_call(t, st, out, name);
            return;
        }

        push_var(st, out, name);
        return;
    }

    parse_error(__func__, t, "invalid term");
}

int compile_expression_list(Tokenizer *t, SymbolTable *st, FILE *out) {
    int n = 0;
    if (is_sym(t, ')')) {
        return 0;
    }
    compile_expression(t, st, out);
    n++;
    while (is_sym(t, ',')) {
        next_tok(t);
        compile_expression(t, st, out);
        n++;
    }
    return n;
}
