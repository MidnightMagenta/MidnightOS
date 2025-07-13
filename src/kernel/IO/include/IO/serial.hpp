#ifndef SERIAL_H
#define SERIAL_H

#include <IO/hwio.hpp>
#include <k_utils/result.h>

#define COM1_PORT 0x3F8
#define COM1_REG(reg) uint16_t(COM1_PORT + reg)

#define COM_R_RX_BUFF 0
#define COM_W_TX_BUFF 0
#define COM_RW_INTERUPT_ENABLE_REG 1
#define COM_RW_LEAST_SIGNIFICANT_BYTE_BAUD 0
#define COM_RW_MOST_SIGNIFICANT_BYTE_BAUD 1
#define COM_R_INT_ID 2
#define COM_W_FIFO_CONTROL_REG 2
#define COM_RW_LINE_CONTROL_REG 3
#define COM_RW_MODEM_CONTROL_REG 4
#define COM_R_LINE_STATUS_REG 5
#define COM_R_MODEM_STATUS_REG 6
#define COM_RW_SCRATCH 7

namespace MdOS::IO {
class BasicSerial {
public:
	static Result init();
	inline static void write_serial(char c) {
		if (c == '\n') { write_serial('\r'); }
		if (!m_initialized) { init(); }
		while (!is_tx_empty());
		MdOS::IO::outb(uint8_t(c), COM1_REG(COM_W_TX_BUFF));
	}

private:
	inline static bool is_tx_empty() { return MdOS::IO::inb(COM1_REG(COM_R_LINE_STATUS_REG)) & 0x20; }
	static bool m_initialized;
};
}// namespace MdOS::IO

#endif