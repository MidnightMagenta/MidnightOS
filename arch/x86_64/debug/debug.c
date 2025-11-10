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
    DBG_SUCCESS     = 0,
    DBG_CONTINUE    = 1,
    DBG_UNKNOWN_CMD = 2,
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

int do_continue(int __unused argc, char __unused **argv, const struct int_info __unused *info) { return DBG_CONTINUE; }

int do_placeholder(int __unused argc, char __unused **argv, const struct int_info __unused *info) {
    return DBG_SUCCESS;
}

struct dbg_cmd {
    const char *name;
    int (*func)(int argc, char **argv, const struct int_info *info);
};

#define DBG_CMD(name, func) {name, func}

static const struct dbg_cmd cmd_table[] = {
        DBG_CMD("c", do_continue),    DBG_CMD("reg", do_placeholder), DBG_CMD("hbp", do_placeholder),
        DBG_CMD("s", do_placeholder), DBG_CMD("x", do_placeholder),
};

int dbg_exec(int argc, char **argv, const struct int_info *info) {
    if (argc == 0) { return DBG_UNKNOWN_CMD; }

    for (unsigned int i = 0; i < ARRAY_SIZE(cmd_table); i++) {
        if (dbg_strcmp(argv[0], cmd_table[i].name) == 0) { return cmd_table[i].func(argc, argv, info); }
    }

    return DBG_UNKNOWN_CMD;
}

void dbg_main(const struct int_info *info) {
    dbg_msg("\nDebug exception\n");
    char cmd[256];
    while (1) {
        dbg_msg("/> ");

        unsigned long size = 256;
        dbg_getcmd(cmd, &size);
        if (size == 0) { continue; }

        char *argv[8];
        int   argc = dbg_parsecmd(cmd, argv, 8);

        int res = dbg_exec(argc, argv, info);
        if (res == DBG_UNKNOWN_CMD) {
            dbg_msg("Error: Unknown command\n");
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