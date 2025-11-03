#include <asm/io.h>
#include <debug/dbg_serial.h>
#include <mdos/types.h>

#define COM1_PORT     0x3F8
#define COM1_REG(reg) (u16)(COM1_PORT + reg)

#define COM_R_RX_BUFF                      0
#define COM_W_TX_BUFF                      0
#define COM_RW_INTERUPT_ENABLE_REG         1
#define COM_RW_LEAST_SIGNIFICANT_BYTE_BAUD 0
#define COM_RW_MOST_SIGNIFICANT_BYTE_BAUD  1
#define COM_R_INT_ID                       2
#define COM_W_FIFO_CONTROL_REG             2
#define COM_RW_LINE_CONTROL_REG            3
#define COM_RW_MODEM_CONTROL_REG           4
#define COM_R_LINE_STATUS_REG              5
#define COM_R_MODEM_STATUS_REG             6
#define COM_RW_SCRATCH                     7

static bool dbg_serial_initialized = false;

mdos_result_t dbg_serial_init() {
    outb(COM1_REG(COM_RW_INTERUPT_ENABLE_REG), 0x00);
    outb(COM1_REG(COM_RW_LINE_CONTROL_REG), 0x80);
    outb(COM1_REG(COM_RW_LEAST_SIGNIFICANT_BYTE_BAUD), 0x03);
    outb(COM1_REG(COM_RW_MOST_SIGNIFICANT_BYTE_BAUD), 0x00);
    outb(COM1_REG(COM_RW_LINE_CONTROL_REG), 0x03);
    outb(COM1_REG(COM_W_FIFO_CONTROL_REG), 0xC7);
    outb(COM1_REG(COM_RW_MODEM_CONTROL_REG), 0x1E);

    outb(COM1_REG(COM_W_TX_BUFF), 0xAE);
    if (inb(COM1_REG(COM_R_RX_BUFF)) != 0xAE) { return MDOS_RES_INIT_FAIL; }

    outb(COM1_REG(COM_RW_MODEM_CONTROL_REG), 0x0F);
    dbg_serial_initialized = true;
    return MDOS_RES_SUCCESS;
}

inline static bool dbg_serial_is_tx_empty() { return inb(COM1_REG(COM_R_LINE_STATUS_REG)) & 0x20; }

inline void dbg_serial_putc(char c) {
    if (!dbg_serial_initialized) {
        if (MDOS_ERROR(dbg_serial_init())) { return; }
    }

    if (c == '\n') { dbg_serial_putc('\r'); }
    while (!dbg_serial_is_tx_empty());
    outb(COM1_REG(COM_W_TX_BUFF), (u8) c);
}