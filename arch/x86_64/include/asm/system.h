#ifndef _MDOS_SYSTEM_H
#define _MDOS_SYSTEM_H

#define hlt() __asm__ volatile("hlt")
#define cli() __asm__ volatile("cli")
#define sti() __asm__ volatile("sti")


#define halt_forever()                                                                                                 \
    while (1) { hlt(); }

#endif