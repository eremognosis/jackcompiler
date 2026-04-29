#include "compile.h"
#include "common.h"
#include "symtab.h"

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int has_jack_ext(const char *path) {
    size_t n = strlen(path);
    return n >= 5 && strcmp(path + n - 5, ".jack") == 0;
}

static int make_vm_path(const char *in_path, char *out_path, size_t out_len) {
    size_t n = strlen(in_path);
    if (n + 1 > out_len) {
        return 0;
    }
    strncpy(out_path, in_path, out_len - 1);
    out_path[out_len - 1] = '\0';
    if (n >= 5 && strcmp(out_path + n - 5, ".jack") == 0) {
        strcpy(out_path + n - 5, ".vm");
        return 1;
    }
    return 0;
}

static int compile_one(const char *in_path) {
    char out_path[PATH_MAX];
    FILE *in = NULL;
    FILE *out = NULL;
    Tokenizer t;
    SymbolTable *st = NULL;
    int ok = 0;

    if (!make_vm_path(in_path, out_path, sizeof(out_path))) {
        CCOMP_LOG_ERROR("CC-MAIN-001", "main", "cannot derive .vm output path");
        return 0;
    }

    in = fopen(in_path, "r");
    if (!in) {
        ccomp_log_errorf("CC-MAIN-002", "main", __FILE__, __LINE__, __func__, "failed to open input '%s'", in_path);
        return 0;
    }

    out = fopen(out_path, "w");
    if (!out) {
        ccomp_log_errorf("CC-MAIN-003", "main", __FILE__, __LINE__, __func__, "failed to open output '%s'", out_path);
        fclose(in);
        return 0;
    }

    st = new_SymbolTable();
    if (!st) {
        fclose(in);
        fclose(out);
        return 0;
    }

    tokenizer_init(&t, in);
    compile_class(&t, st, out);
    ok = 1;

    symtab_destroy(st);
    fclose(out);
    fclose(in);
    return ok;
}

static int compile_dir(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    struct dirent *ent;
    int count = 0;
    int all_ok = 1;

    if (!dir) {
        ccomp_log_errorf("CC-MAIN-004", "main", __FILE__, __LINE__, __func__, "failed to open directory '%s'", dir_path);
        return 0;
    }

    while ((ent = readdir(dir)) != NULL) {
        char in_path[PATH_MAX];
        struct stat st;

        if (ent->d_name[0] == '.') {
            continue;
        }
        if (!has_jack_ext(ent->d_name)) {
            continue;
        }
        if (snprintf(in_path, sizeof(in_path), "%s/%s", dir_path, ent->d_name) >= (int)sizeof(in_path)) {
            all_ok = 0;
            continue;
        }
        if (stat(in_path, &st) != 0 || !S_ISREG(st.st_mode)) {
            continue;
        }
        count++;
        if (!compile_one(in_path)) {
            all_ok = 0;
        }
    }

    closedir(dir);
    if (count == 0) {
        CCOMP_LOG_ERROR(CCOMP_ERR_NO_FILES, "main", "no .jack files found");
        return 0;
    }
    return all_ok;
}

int main(int argc, char **argv) {
    struct stat st;

    if (argc != 2) {
        fprintf(stderr, "Usage: jack <file.jack | directory>\n");
        return 1;
    }

    if (stat(argv[1], &st) != 0) {
        ccomp_log_errorf("CC-MAIN-005", "main", __FILE__, __LINE__, __func__, "path not found '%s'", argv[1]);
        return 1;
    }

    if (S_ISREG(st.st_mode)) {
        if (!has_jack_ext(argv[1])) {
            CCOMP_LOG_ERROR("CC-MAIN-006", "main", "input file must have .jack extension");
            return 1;
        }
        return compile_one(argv[1]) ? 0 : 1;
    }

    if (S_ISDIR(st.st_mode)) {
        return compile_dir(argv[1]) ? 0 : 1;
    }

    CCOMP_LOG_ERROR("CC-MAIN-007", "main", "input path must be file or directory");
    return 1;
}
