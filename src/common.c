#include "common.h"

#include <stdatomic.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static _Atomic unsigned long long log_sequence = 0;

static void iso8601_now(char *out, size_t out_size) {
    struct timespec ts;
    struct tm *tm_utc;

    timespec_get(&ts, TIME_UTC);
    tm_utc = gmtime(&ts.tv_sec);
    if (!tm_utc) {
        snprintf(out, out_size, "1970-01-01T00:00:00.000Z");
        return;
    }

    strftime(out, out_size, "%Y-%m-%dT%H:%M:%S", tm_utc);

    {
        size_t len = strlen(out);
        if (len + 6 < out_size) {
            snprintf(out + len, out_size - len, ".%03ldZ", ts.tv_nsec / 1000000L);
        }
    }
}

void ccomp_log_errorf(
    const char *code,
    const char *component,
    const char *file,
    int line,
    const char *function,
    const char *fmt,
    ...
) {
    char message[512];
    va_list args;

    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    ccomp_log_error(code, component, file, line, function, message);
}

void ccomp_log_error(
    const char *code,
    const char *component,
    const char *file,
    int line,
    const char *function,
    const char *message
) {
    char timestamp[32];
    unsigned long long seq;

    iso8601_now(timestamp, sizeof(timestamp));
    seq = atomic_fetch_add(&log_sequence, 1) + 1;

    fprintf(
        stderr,
        "{\"timestamp\":\"%s\",\"severity\":\"ERROR\",\"trace_id\":\"%ld-%llu\","
        "\"code\":\"%s\",\"component\":\"%s\",\"message\":\"%s\","
        "\"location\":{\"file\":\"%s\",\"line\":%d,\"function\":\"%s\"}}\n",
        timestamp,
        (long)getpid(),
        seq,
        code ? code : "CC-UNKNOWN",
        component ? component : "unknown",
        message ? message : "",
        file ? file : "unknown",
        line,
        function ? function : "unknown"
    );
}
