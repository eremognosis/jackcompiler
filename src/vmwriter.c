#include <stdio.h>
#include <stdlib.h>
#include "vmwriter.h"


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
    if (!get_segment_name(seg))
    {
        fprintf(stderr, "ERRVM001: INVALID OPERATION");
        return;
    }
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }

    fprintf(out, "push %s %d\n", get_segment_name(seg), index);


}

void vm_write_pop(FILE* out, Segment seg, int index)
{
    if (!get_segment_name(seg))
    {
        fprintf(stderr, "ERRVM001: INVALID OPERATION");
        return;
    }
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }

    if (seg==SEG_CONST)
    {
        fprintf(stderr, "ERRVM003: POP FROM CONSTANT MAKES NO SENSE");
        return;
    }

    fprintf(out, "pop %s %d\n", get_segment_name(seg), index);
}

void vm_write_arithmetic(FILE* out, Command cmd)
{
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }

    const char *command = getcommandname(cmd);
    if (!command)
    {
        fprintf(stderr, "ERRVM003: INVALID COMMAND");
        return;
    }
    fprintf(out, "%s\n", command);
}

void vm_write_label(FILE* out, const char* label)
{
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }
    fprintf(out, "label %s\n", label);
}

void vm_write_goto(FILE* out, const char* label)
{
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }
    fprintf(out, "goto %s\n", label);
}

void vm_write_if(FILE* out, const char* label)
{
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }
    fprintf(out, "if-goto %s\n", label);
}

void vm_write_call(FILE* out, const char* name, int n_args)
{
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }
    fprintf(out, "call %s %d\n", name, n_args);
}

void vm_write_function(FILE* out, const char* name, int n_locals)
{
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }
    fprintf(out, "function %s %d\n", name, n_locals);
}

void vm_write_return(FILE* out)
{
    if (!out)
    {
        fprintf(stderr, "ERRVM002: NO OUTPUT FILE");
        return;
    }
    fprintf(out, "return\n");
}
