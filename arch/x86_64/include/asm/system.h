#ifndef _NYX_SYSTEM_H
#define _NYX_SYSTEM_H

#define hlt() __asm__ volatile("hlt" ::: "memory")
#define cli() __asm__ volatile("cli" ::: "memory")
#define sti() __asm__ volatile("sti" ::: "memory")

#define exit()                                                                                                         \
    cli();                                                                                                             \
    while (1) { hlt(); }

#endif