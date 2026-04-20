#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_REGS 512

typedef struct {
    unsigned int addr;
    unsigned int value;
} RegEntry;

typedef struct {
    RegEntry regs[MAX_REGS];
    size_t count;
} RegTable;

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s <input1> [input2]\n"
            "  Parse sensor register tables and optionally compare two tables.\n"
            "  Supported line formats: 0x300A 0x12, 300A=12, 300A:12\n",
            prog);
}

static int parse_hex_token(const char *s, unsigned int *out) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) return 0;
    char *end = NULL;
    unsigned long v = strtoul(s, &end, 16);
    if (end == s) return 0;
    *out = (unsigned int)v;
    return 1;
}

static int parse_line(const char *line, unsigned int *addr, unsigned int *value) {
    char buf[MAX_LINE];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    for (char *p = buf; *p; ++p) {
        if (*p == '=' || *p == ':' || *p == ',' || *p == ';' || *p == '\t') {
            *p = ' ';
        }
    }

    char *tokens[4] = {0};
    int n = 0;
    char *save = NULL;
    for (char *tok = strtok_r(buf, " ", &save); tok && n < 4; tok = strtok_r(NULL, " ", &save)) {
        if (*tok == '#') break;
        tokens[n++] = tok;
    }

    if (n < 2) return 0;
    if (!parse_hex_token(tokens[0], addr)) return 0;
    if (!parse_hex_token(tokens[1], value)) return 0;
    return 1;
}

static int load_table(const char *path, RegTable *table) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        unsigned int addr = 0, value = 0;
        if (!parse_line(line, &addr, &value)) {
            continue;
        }
        if (table->count >= MAX_REGS) {
            fprintf(stderr, "Register table too large (max %d)\n", MAX_REGS);
            fclose(fp);
            return -1;
        }
        table->regs[table->count].addr = addr;
        table->regs[table->count].value = value;
        table->count++;
    }

    fclose(fp);
    return 0;
}

static const RegEntry *find_reg(const RegTable *table, unsigned int addr) {
    for (size_t i = 0; i < table->count; ++i) {
        if (table->regs[i].addr == addr) {
            return &table->regs[i];
        }
    }
    return NULL;
}

static void print_table(const RegTable *table, const char *name) {
    printf("%s: %zu registers\n", name, table->count);
    for (size_t i = 0; i < table->count; ++i) {
        printf("  0x%04X = 0x%02X\n", table->regs[i].addr, table->regs[i].value);
    }
}

static void compare_tables(const RegTable *a, const RegTable *b) {
    printf("\n=== Diff ===\n");

    size_t added = 0, removed = 0, changed = 0;

    for (size_t i = 0; i < a->count; ++i) {
        const RegEntry *other = find_reg(b, a->regs[i].addr);
        if (!other) {
            ++removed;
            printf("- 0x%04X = 0x%02X\n", a->regs[i].addr, a->regs[i].value);
        } else if (other->value != a->regs[i].value) {
            ++changed;
            printf("~ 0x%04X: 0x%02X -> 0x%02X\n", a->regs[i].addr, a->regs[i].value, other->value);
        }
    }

    for (size_t i = 0; i < b->count; ++i) {
        const RegEntry *other = find_reg(a, b->regs[i].addr);
        if (!other) {
            ++added;
            printf("+ 0x%04X = 0x%02X\n", b->regs[i].addr, b->regs[i].value);
        }
    }

    printf("\nSummary: added=%zu removed=%zu changed=%zu\n", added, removed, changed);
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    RegTable table1 = {0};
    RegTable table2 = {0};

    if (load_table(argv[1], &table1) < 0) {
        return EXIT_FAILURE;
    }

    print_table(&table1, argv[1]);

    if (argc == 3) {
        if (load_table(argv[2], &table2) < 0) {
            return EXIT_FAILURE;
        }
        print_table(&table2, argv[2]);
        compare_tables(&table1, &table2);
    }

    return EXIT_SUCCESS;
}
