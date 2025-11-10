#include "debug/dbg_serial.h"
#include <asm/cpu.h>
#include <asm/idtentry.h>
#include <debug/dbgio.h>
#include <nyx/utils.h>

static unsigned long dbg_strlen(const char *str) {
    size_t len = 0;
    if (str) {
        while (str[len] != '\0') { len++; }
    }
    return len;
}

int dbg_isspace(int c) { return c == ' ' || (unsigned) c - '\t' < 5; }

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
    DBG_UNKNOWN_CMD,
    DBG_INVALID_PARAMS,
};

void dbg_getcmd(char *cmd, unsigned long *size) {
    unsigned long i = 0;
    while (i < *size) {
        cmd[i] = dbg_serial_getc();
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

DBG_DEFINE_COMMAND_PU(cntn) { return DBG_CONTINUE; }

DBG_DEFINE_COMMAND(reg) {
    struct {
        const char   *name;
        unsigned long value;
    } regs[] = {
            {"rax", info->regs.rax},  {"rbx", info->regs.rbx}, {"rcx", info->regs.rcx},  {"rdx", info->regs.rdx},
            {"rsi", info->regs.rsi},  {"rdi", info->regs.rdi}, {"rsp", info->frame.rsp}, {"rbp", info->regs.rbp},
            {"r8", info->regs.r8},    {"r9", info->regs.r9},   {"r10", info->regs.r10},  {"r11", info->regs.r11},
            {"r12", info->regs.r12},  {"r13", info->regs.r13}, {"r14", info->regs.r14},  {"r15", info->regs.r15},
            {"rip", info->frame.rip}, {"cs", info->frame.cs},  {"ss", info->frame.ss},   {"rflags", info->frame.rflags},
    };

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

DBG_DEFINE_COMMAND_PU(hardware_bp) {
    dbg_msg("Unimplemented\n");
    return DBG_SUCCESS;
}

DBG_DEFINE_COMMAND_PU(step) {
    dbg_msg("Unimplemented\n");
    return DBG_SUCCESS;
}

DBG_DEFINE_COMMAND_PU(dumphex) {
    dbg_msg("Unimplemented\n");
    return DBG_SUCCESS;
}


struct dbg_cmd {
    const char *name;
    int (*func)(int argc, char **argv, const struct int_info *info);
};

#define DBG_CMD(name, func) {name, func}

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

char  cmd_buffer[256];
char *argv_buffer[8];
void  dbg_main(const struct int_info *info) {
    dbg_msg("\nDebug exception\n");

    while (1) {
        dbg_msg("/> ");

        unsigned long size = 256;
        dbg_getcmd(cmd_buffer, &size);
        if (size == 0) { continue; }

        int argc = dbg_parsecmd(cmd_buffer, argv_buffer, 8);

        int res = dbg_exec(argc, argv_buffer, info);
        if (res == DBG_UNKNOWN_CMD) {
            dbg_msg("Error: Unknown command\n");
        } else if (res == DBG_INVALID_PARAMS) {
            dbg_msg("Error: Invalid parameter\n");
        } else if (res == DBG_CONTINUE) {
            return;
        }
    }
}

DEFINE_IDTENTRY_RAW(dbg_entry) {
#ifdef _DEBUG
    dbg_main(info);
#endif
}