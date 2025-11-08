#include "asm/desc_defs.h"
#include <asm/desc.h>
#include <asm/idt.h>
#include <asm/traps.h>
#include <nyx/utils.h>
#include <nyx/linkage.h>

#define IDT_ENTRIES  256
#define IDT_TAB_SIZE IDT_ENTRIES * sizeof(gate_desc)

#define DPL0 0x0
#define DPL3 0x3

#define DEFAULT_STACK 0

#define GATE(_vector, _addr, _ist, _type, _dpl, _segment)                                                              \
    {                                                                                                                  \
            .vector    = _vector,                                                                                      \
            .segment   = _segment,                                                                                     \
            .addr      = _addr,                                                                                        \
            .bits.ist  = _ist,                                                                                         \
            .bits.zero = 0,                                                                                            \
            .bits.type = _type,                                                                                        \
            .bits.dpl  = _dpl,                                                                                         \
            .bits.p    = 1,                                                                                            \
    }

// FIXME: kernel CS should be a definition
#define INTG(_vector, _addr) GATE(_vector, _addr, 0, GATE_INTERRUPT, DPL0, 0x08)
#define TRPG(_vcetor, _addr) GATE(_vector, _addr, 0, GATE_TRAP, DPL0, 0x08)

gate_desc idt[IDT_ENTRIES] __page_aligned_bss;

static const struct idt_data early_idt[] = {
        INTG(3, int3),
};

static void idt_setup_from_table(gate_desc *idt, const struct idt_data *table, int size) {
    gate_desc desc;

    for (; size > 0; table++, size--) {
        idt_init_desc(&desc, table);
        write_idt_entry(idt, table->vector, &desc);
    }
}

void idt_setup_early_traps() { idt_setup_from_table(idt, early_idt, ARRAY_SIZE(early_idt)); }