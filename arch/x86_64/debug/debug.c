#include "debug/dbg_serial.h"
#include <asm/cpu.h>
#include <debug/dbgio.h>
#include <asm/idtentry.h>

static int dbg_strcmp(char *str1, char *str2) {
    size_t i   = 0;
    size_t res = 0;
    while ((str1[i] == str2[i]) && (str1[i] != '\0') && (str2[i] != '\0')) { i++; }
    res = ((unsigned char) str1[i] - (unsigned char) str2[i]);
    return (int) res;
}

void dbg_start(const struct int_info *info) {
    dbg_msg("\nDebug exception\n");
    char cmd[64];
    while (1) {
        dbg_msg("/> ");
        int i = 0;
        while (i < 64) {
            cmd[i] = dbg_serial_getc();
            dbg_serial_putc(cmd[i] == '\r' ? '\n' : cmd[i]);
            if (cmd[i] == '\n' || cmd[i] == '\r') {
                cmd[i++] = '\0';
                break;
            }
            i++;
        };

        if (dbg_strcmp(cmd, "c") == 0) {
            return;
        } else {
            dbg_msg("Error: Unknown command\n");
        }
    }
}

DEFINE_IDTENTRY_RAW(dbg_entry) {
#ifdef _DEBUG
    dbg_start(info);
#endif
}