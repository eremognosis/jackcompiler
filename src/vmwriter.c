#include <stdio.h>
#include <stdlib.h>
#include "vmwriter.h"
#include "common.h"


static const char* get_segment_name(Segment seg) {
    switch (seg) {
    case SEG_CONST:   return "constant";
    case SEG_ARG:     return "argument";
    case SEG_LOCAL:   return "local";
    case SEG_THIS:    return "this";
    case SEG_THAT:    return "that";
    case SEG_POINTER: return "pointer";
    case SEG_TEMP:    return "temp";
    case SEG_STATIC:  return "static";
    default:          return NULL;
    }
}


static const char* getcommandname(Command cmd)
{
    switch (cmd)
    {
    case CMD_ADD: return "add";
        case CMD_SUB: return "sub";
        case CMD_GT: return "gt";
        case CMD_LT: return "lt";
        case CMD_EQ: return "eq";
        case CMD_AND: return "and";
        case CMD_OR: return "or";
        case CMD_NEG: return "neg";
        case CMD_NOT: return "not";
        default:       return NULL;
    }
}



void vm_write_push(FILE* out, Segment seg, int index)
{
    const char *segment = get_segment_name(seg);
    if (!segment)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_INVALID_SEGMENT, "vmwriter", "vm_write_push received invalid segment");
        return;
    }
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_push output stream is NULL");
        return;
    }

    fprintf(out, "push %s %d\n", segment, index);


}

void vm_write_pop(FILE* out, Segment seg, int index)
{
    const char *segment = get_segment_name(seg);
    if (!segment)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_INVALID_SEGMENT, "vmwriter", "vm_write_pop received invalid segment");
        return;
    }
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_pop output stream is NULL");
        return;
    }

    if (seg==SEG_CONST)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_POP_CONST, "vmwriter", "vm_write_pop cannot pop to constant segment");
        return;
    }

    fprintf(out, "pop %s %d\n", segment, index);
}

void vm_write_arithmetic(FILE* out, Command cmd)
{
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_arithmetic output stream is NULL");
        return;
    }

    const char *command = getcommandname(cmd);
    if (!command)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_INVALID_COMMAND, "vmwriter", "vm_write_arithmetic received invalid command");
        return;
    }
    fprintf(out, "%s\n", command);
}

void vm_write_label(FILE* out, const char* label)
{
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_label output stream is NULL");
        return;
    }
    if (!label)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_INVALID_LABEL, "vmwriter", "vm_write_label label is NULL");
        return;
    }
    fprintf(out, "label %s\n", label);
}

void vm_write_goto(FILE* out, const char* label)
{
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_goto output stream is NULL");
        return;
    }
    if (!label)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_INVALID_LABEL, "vmwriter", "vm_write_goto label is NULL");
        return;
    }
    fprintf(out, "goto %s\n", label);
}

void vm_write_if(FILE* out, const char* label)
{
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_if output stream is NULL");
        return;
    }
    if (!label)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_INVALID_LABEL, "vmwriter", "vm_write_if label is NULL");
        return;
    }
    fprintf(out, "if-goto %s\n", label);
}

void vm_write_call(FILE* out, const char* name, int n_args)
{
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_call output stream is NULL");
        return;
    }
    if (!name)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_INVALID_FUNCTION, "vmwriter", "vm_write_call function name is NULL");
        return;
    }
    fprintf(out, "call %s %d\n", name, n_args);
}

void vm_write_function(FILE* out, const char* name, int n_locals)
{
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_function output stream is NULL");
        return;
    }
    if (!name)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_INVALID_FUNCTION, "vmwriter", "vm_write_function function name is NULL");
        return;
    }
    fprintf(out, "function %s %d\n", name, n_locals);
}

void vm_write_return(FILE* out)
{
    if (!out)
    {
        CCOMP_LOG_ERROR(CCOMP_ERR_VMW_OUTPUT_NULL, "vmwriter", "vm_write_return output stream is NULL");
        return;
    }
    fprintf(out, "return\n");
}
