#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 2048

typedef enum {
    ISSUE_I2C_READ_FAIL,
    ISSUE_PROBE_FAIL,
    ISSUE_CSI_TIMEOUT,
    ISSUE_STREAM_TIMEOUT,
    ISSUE_CLOCK_POWER,
    ISSUE_FORMAT_MISMATCH,
    ISSUE_SENSOR_ID,
    ISSUE_UNKNOWN
} IssueType;

typedef struct {
    IssueType type;
    const char *name;
    const char *patterns[6];
    const char *suggestion;
} Rule;

static const Rule rules[] = {
    {ISSUE_I2C_READ_FAIL, "i2c read failure",
     {"i2c", "read", "fail", NULL},
     "Check I2C address, pull-ups, power rail, and sensor reset/pwdn sequence."},
    {ISSUE_PROBE_FAIL, "probe failure",
     {"probe", "fail", NULL},
     "Check device tree, regulator/clock setup, and sensor ID readback."},
    {ISSUE_CSI_TIMEOUT, "csi timeout",
     {"csi", "timeout", NULL},
     "Check MIPI lane mapping, clock, sensor streaming state, and receiver configuration."},
    {ISSUE_STREAM_TIMEOUT, "stream timeout",
     {"stream", "timeout", NULL},
     "Check whether stream-on succeeded and whether buffers are queued properly."},
    {ISSUE_CLOCK_POWER, "clock or power issue",
     {"clock", "fail", NULL},
     "Check power rails, mclk/xclk, and reset timing before probe starts."},
    {ISSUE_FORMAT_MISMATCH, "format mismatch",
     {"format", "mismatch", NULL},
     "Check pixel format, resolution, and CSI/ISP configuration."},
    {ISSUE_SENSOR_ID, "sensor id issue",
     {"sensor", "id", NULL},
     "Check I2C communication, chip ID register, and correct sensor driver binding."}
};

static void lower_string(char *s) {
    for (; *s; ++s) {
        *s = (char)tolower((unsigned char)*s);
    }
}

static int contains_all(const char *line, const char *const *patterns) {
    for (int i = 0; patterns[i]; ++i) {
        if (!strstr(line, patterns[i])) {
            return 0;
        }
    }
    return 1;
}

static const Rule *match_rule(const char *line) {
    for (size_t i = 0; i < sizeof(rules) / sizeof(rules[0]); ++i) {
        if (contains_all(line, rules[i].patterns)) {
            return &rules[i];
        }
    }
    return NULL;
}

static const char *issue_name(IssueType type) {
    switch (type) {
        case ISSUE_I2C_READ_FAIL: return "I2C Read Failure";
        case ISSUE_PROBE_FAIL: return "Probe Failure";
        case ISSUE_CSI_TIMEOUT: return "CSI Timeout";
        case ISSUE_STREAM_TIMEOUT: return "Stream Timeout";
        case ISSUE_CLOCK_POWER: return "Clock/Power Issue";
        case ISSUE_FORMAT_MISMATCH: return "Format Mismatch";
        case ISSUE_SENSOR_ID: return "Sensor ID Issue";
        default: return "Unknown";
    }
}

int main(int argc, char **argv) {
    FILE *fp = stdin;
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [logfile]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (!fp) {
            perror("fopen");
            return EXIT_FAILURE;
        }
    }

    char line[MAX_LINE];
    int total_lines = 0;
    int matched_lines = 0;
    int counts[ISSUE_UNKNOWN + 1] = {0};

    while (fgets(line, sizeof(line), fp)) {
        ++total_lines;
        char normalized[MAX_LINE];
        strncpy(normalized, line, sizeof(normalized) - 1);
        normalized[sizeof(normalized) - 1] = '\0';
        lower_string(normalized);

        const Rule *rule = match_rule(normalized);
        if (rule) {
            ++matched_lines;
            ++counts[rule->type];
            printf("[%s] %s", issue_name(rule->type), line);
        }
    }

    if (fp != stdin) {
        fclose(fp);
    }

    printf("\n=== Summary ===\n");
    printf("Total lines: %d\n", total_lines);
    printf("Matched lines: %d\n", matched_lines);

    for (size_t i = 0; i < sizeof(rules) / sizeof(rules[0]); ++i) {
        if (counts[rules[i].type] > 0) {
            printf("- %s: %d\n", rules[i].name, counts[rules[i].type]);
            printf("  Suggestion: %s\n", rules[i].suggestion);
        }
    }

    if (matched_lines == 0) {
        printf("No common camera issues were detected in this log.\n");
    }

    return EXIT_SUCCESS;
}
