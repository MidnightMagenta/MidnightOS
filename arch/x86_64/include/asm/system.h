#ifndef _NYX_SYSTEM_H
#define _NYX_SYSTEM_H

#define hlt() __asm__ volatile("hlt")
#define cli() __asm__ volatile("cli")
#define sti() __asm__ volatile("sti")


#define exit()                                                                                                         \
    cli();                                                                                                             \
    while (1) { hlt(); }

#endif