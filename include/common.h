//
// Created by raj on 4/29/26.
//

#ifndef CCOMP_COMMON_H
#define CCOMP_COMMON_H

/*
 * Centralized error codes for consistent, traceable logging.
 */
#define CCOMP_ERR_SYMTAB_ALLOC "CC-SYM-001"
#define CCOMP_ERR_SYMTAB_NULL_TABLE "CC-SYM-002"
#define CCOMP_ERR_SYMTAB_INVALID_INPUT "CC-SYM-003"
#define CCOMP_ERR_VMW_INVALID_SEGMENT "CC-VMW-001"
#define CCOMP_ERR_VMW_OUTPUT_NULL "CC-VMW-002"
#define CCOMP_ERR_VMW_POP_CONST "CC-VMW-003"
#define CCOMP_ERR_VMW_INVALID_COMMAND "CC-VMW-004"
#define CCOMP_ERR_VMW_INVALID_LABEL "CC-VMW-005"
#define CCOMP_ERR_VMW_INVALID_FUNCTION "CC-VMW-006"
#define CCOMP_ERR_NO_FILES "CC-TKN-001"
void ccomp_log_errorf(
    const char *code,
    const char *component,
    const char *file,
    int line,
    const char *function,
    const char *fmt,
    ...
);

void ccomp_log_error(
    const char *code,
    const char *component,
    const char *file,
    int line,
    const char *function,
    const char *message
);

#define CCOMP_LOG_ERROR(code, component, message) \
    ccomp_log_error((code), (component), __FILE__, __LINE__, __func__, (message))

#endif //CCOMP_COMMON_H
