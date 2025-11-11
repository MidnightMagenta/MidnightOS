#include "debug/dbg_serial.h"
#include <asm/cpu.h>
#include <asm/idtentry.h>
#include <debug/dbgio.h>
#include <nyx/utils.h>
#include <stddef.h>

static unsigned long dbg_strlen(const char *str) {
    size_t len = 0;
    if (str) {
        while (str[len] != '\0') { len++; }
    }
    return len;
}

int dbg_isspace(int c) { return c == ' ' || (unsigned) c - '\t' < 5; }

int dbg_isdigit(int c) { return (unsigned) c - '0' < 10; }

long dbg_atol(const char *s) {
    long n    = 0;
    int  neg  = 0;
    int  base = 10;

    while (dbg_isspace(*s)) s++;
    switch (*s) {
        case '-':
            neg = 1;
        case '+':
            s++;
    }

    // Check for hexadecimal prefix
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s += 2;
    }

    if (base == 16) {
        while (*s) {
            int digit;
            if (dbg_isdigit(*s)) {
                digit = *s - '0';
            } else if (*s >= 'a' && *s <= 'f') {
                digit = *s - 'a' + 10;
            } else if (*s >= 'A' && *s <= 'F') {
                digit = *s - 'A' + 10;
            } else {
                break;
            }
            n = n * 16 + digit;
            s++;
        }
        return neg ? -n : n;
    } else {
        /* Compute n as a negative number to avoid overflow on LONG_MIN */
        while (dbg_isdigit(*s)) n = 10 * n - (*s++ - '0');
        return neg ? n : -n;
    }
}

static int dbg_strcmp(const char *str1, const char *str2) {
    size_t i   = 0;
    size_t res = 0;
    while ((str1[i] == str2[i]) && (str1[i] != '\0') && (str2[i] != '\0')) { i++; }
    res = ((unsigned char) str1[i] - (unsigned char) str2[i]);
    return (int) res;
}

char *dbg_strcpy(char *dst, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return dst;
}

int dbg_strncmp(const char *str1, const char *str2, unsigned long num) {
    size_t i   = 0;
    size_t res = 0;
    while ((str1[i] == str2[i]) && (str1[i] != '\0') && (str2[i] != '\0') && (i < num)) { i++; }
    res = ((unsigned char) str1[i] - (unsigned char) str2[i]);
    return (int) res;
}

char *dbg_strncpy(char *dst, const char *src, unsigned long num) {
    char       *dstPtr = dst;
    const char *srcPtr = src;
    while (num--) { *dstPtr++ = *srcPtr++; }
    return dst;
}

char *dbg_strchr(const char *str, int c) {
    char *res = NULL;
    while ((*str != '\0') && (*str != c)) { str++; }
    if (*str == c) { res = (char *) str; }
    return res;
}

enum dbg_exec_res {
    DBG_SUCCESS,
    DBG_CONTINUE,
    DBG_STEP,
    DBG_UNKNOWN_CMD,
    DBG_INVALID_PARAMS,
};

void dbg_getcmd(char *cmd, unsigned long *size) {
    unsigned long i = 0;
    while (i < *size) {
        char c = dbg_serial_getc();

        if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                dbg_serial_putc('\b');
                dbg_serial_putc(' ');
                dbg_serial_putc('\b');
            }
            continue;
        }

        cmd[i] = c;
        dbg_serial_putc(cmd[i] == '\r' ? '\n' : cmd[i]);
        if (cmd[i] == '\n' || cmd[i] == '\r') {
            cmd[i++] = '\0';
            break;
        }
        i++;
    };

    *size = i;
}

int dbg_parsecmd(char *buff, char *argv[], int max_args) {
    int argc = 0;

    while (*buff && argc < max_args) {
        // find first non whitespace char
        while (dbg_isspace(*buff)) { buff++; }

        // if the non whitespace char is \0, don't continue parsing
        if (!*buff) { break; }

        argv[argc++] = buff;
        // find the end of the argument
        while (*buff && !dbg_isspace(*buff)) { buff++; }

        // and finally add a null terminator to the end of the argument
        if (*buff) { *buff++ = 0; }
    }

    return argc;
}

#define DBG_DEFINE_COMMAND(func) int dbg_do_##func(int argc, char **argv, const struct int_info *info)
#define DBG_DEFINE_COMMAND_PU(func)                                                                                    \
    int dbg_do_##func(int __unused argc, char __unused **argv, const struct int_info __unused *info)

#define DBG_REGS_ARRAY                                                                                                 \
    struct {                                                                                                           \
        const char   *name;                                                                                            \
        unsigned long value;                                                                                           \
    } regs[] =                                                                                                         \
            {                                                                                                          \
                    {"rax", info->regs.rax},  {"rbx", info->regs.rbx},        {"rcx", info->regs.rcx},                 \
                    {"rdx", info->regs.rdx},  {"rsi", info->regs.rsi},        {"rdi", info->regs.rdi},                 \
                    {"rsp", info->frame.rsp}, {"rbp", info->regs.rbp},        {"r8", info->regs.r8},                   \
                    {"r9", info->regs.r9},    {"r10", info->regs.r10},        {"r11", info->regs.r11},                 \
                    {"r12", info->regs.r12},  {"r13", info->regs.r13},        {"r14", info->regs.r14},                 \
                    {"r15", info->regs.r15},  {"rip", info->frame.rip},       {"cs", info->frame.cs},                  \
                    {"ss", info->frame.ss},   {"rflags", info->frame.rflags},                                          \
    };

DBG_DEFINE_COMMAND_PU(cntn) { return DBG_CONTINUE; }

/** @brief Prints the value of the specified register. If none are specified, prints all registers
    Command: reg [register]
*/
DBG_DEFINE_COMMAND(reg) {
    DBG_REGS_ARRAY
    if (argc == 0) {
        unsigned long count = ARRAY_SIZE(regs);
        for (size_t i = 0; i < count; i++) {
            dbg_msg("%s: 0x%lx ", regs[i].name, regs[i].value);
            if ((i + 1) % 4 == 0 || i == count - 1) dbg_msg("\n");
        }
    } else if (argc == 1) {
        for (size_t i = 0; i < ARRAY_SIZE(regs); i++) {
            if (dbg_strcmp(argv[0], regs[i].name) == 0) {
                dbg_msg("0x%lx\n", regs[i].value);
                return DBG_SUCCESS;
            }
        }

        dbg_msg("Unknown register %s\n", argv[0]);
        return DBG_INVALID_PARAMS;
    } else {
        return DBG_INVALID_PARAMS;
    }
    return DBG_SUCCESS;
}

/**
    @brief sets a hardware breakpoint in one of the debug registers
    Command: hbp [address] [reg] [type]
    Allowed values of the [reg] parameter are: dr0, dr1, dr2, and dr3
    Allowed values of the [type] parameter are:
     - ex - break on execute
     - ma - break on any 1 byte memory access
     - m2 - break on any 2 byte memory access
     - m4 - break on any 4 byte memory access
     - m8 - break on any 8 byte memory access
*/
DBG_DEFINE_COMMAND_PU(hardware_bp) {
    dbg_msg("Unimplemented\n");
    return DBG_SUCCESS;
}

/**
    @brief single steps the next instruction using the hardware single stepping capabilites
*/
DBG_DEFINE_COMMAND_PU(step) { return DBG_STEP; }

/**
    @brief Prints the [size] bytes of memory at [base]. Both size and base must be specified
    Command: x [base] [size]
*/
DBG_DEFINE_COMMAND_PU(dumphex) {
    DBG_REGS_ARRAY
    unsigned char *base = NULL;

    for (size_t i = 0; i < ARRAY_SIZE(regs); i++) {
        if (dbg_strcmp(argv[0], regs[i].name) == 0) {
            base = (unsigned char *) regs[i].value;
            break;
        }
    }

    if (base == NULL) { base = (unsigned char *) dbg_atol(argv[0]); }
    unsigned char *limit = (unsigned char *) ((unsigned long) base + dbg_atol(argv[1]));

    int c = 0;
    dbg_msg("0x%lx: ", base);
    while (base < limit) {
        dbg_msg("0x%b ", *base++);
        c++;
        if (c == 8) { dbg_msg(" "); }
        if (c == 16 && base < limit) {
            dbg_msg("\n0x%lx: ", base);
            c = 0;
        }
    }

    dbg_msg("\n");

    return DBG_SUCCESS;
}

struct dbg_cmd {
    const char *name;
    int (*func)(int argc, char **argv, const struct int_info *info);
};

#define DBG_CMD(name, func) {name, func}

// array containing literals used to invoke the command
// and a pointer to a function which executes the command
static const struct dbg_cmd cmd_table[] = {
        DBG_CMD("c", dbg_do_cntn), DBG_CMD("reg", dbg_do_reg),   DBG_CMD("hbp", dbg_do_hardware_bp),
        DBG_CMD("s", dbg_do_step), DBG_CMD("x", dbg_do_dumphex),
};

int dbg_exec(int argc, char **argv, const struct int_info *info) {
    if (argc == 0) { return DBG_UNKNOWN_CMD; }

    for (unsigned int i = 0; i < ARRAY_SIZE(cmd_table); i++) {
        if (dbg_strcmp(argv[0], cmd_table[i].name) == 0) { return cmd_table[i].func(--argc, ++argv, info); }
    }

    return DBG_UNKNOWN_CMD;
}

static char  cmd_buffer[256];
static char *argv_buffer[8];
static bool  stepping = false;

void dbg_main(struct int_info *info) {
    if (!stepping) {
        dbg_msg("\nDebug exception\n");
    } else {
        info->frame.rflags &= ~((u64) 1 << 8);
    }

    stepping = false;
    while (1) {
        dbg_msg("/> ");

        unsigned long size = 256;
        dbg_getcmd(cmd_buffer, &size);
        if (size == 0) { continue; }

        int argc = dbg_parsecmd(cmd_buffer, argv_buffer, 8);

        int res = dbg_exec(argc, argv_buffer, info);
        switch (res) {
            case DBG_UNKNOWN_CMD:
                dbg_msg("Error: Unknown command\n");
                break;
            case DBG_INVALID_PARAMS:
                dbg_msg("Error: Invalid parameter\n");
                break;
            case DBG_CONTINUE:
                return;
            case DBG_STEP:
                info->frame.rflags |= 1 << 8;
                stepping = true;
                return;
            default:
                continue;
        }
    }
}

DEFINE_IDTENTRY(dbg_entry) {
#ifdef _DEBUG
    dbg_main(info);
#endif
}