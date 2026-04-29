//
// Created by raj on 4/29/26.
//

#ifndef CCOMP_VMWRITER_H
#define CCOMP_VMWRITER_H

#include <stdio.h>

/*
 * ====================================================================================
 */
typedef enum {
    SEG_CONST, SEG_ARG, SEG_LOCAL, SEG_STATIC,
    SEG_THIS, SEG_THAT, SEG_POINTER, SEG_TEMP
} Segment;

typedef enum {
    CMD_ADD, CMD_SUB, CMD_NEG, CMD_EQ,
    CMD_GT, CMD_LT, CMD_AND, CMD_OR, CMD_NOT
} Command;
/*
 * ====================================================================================
 */
void vm_write_push(FILE *out, Segment seg, int index);
void vm_write_pop(FILE *out, Segment seg, int index);
void vm_write_arithmetic(FILE *out, Command cmd);
void vm_write_label(FILE *out, const char *label);
void vm_write_goto(FILE *out, const char *label);
void vm_write_if(FILE *out, const char *label);
void vm_write_call(FILE *out, const char *name, int n_args);
void vm_write_function(FILE *out, const char *name, int n_locals);
void vm_write_return(FILE *out);

/*
 * ====================================================================================
 */

#endif //CCOMP_VMWRITER_H
