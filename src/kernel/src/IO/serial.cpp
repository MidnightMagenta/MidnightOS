#include "../../include/IO/serial.hpp"

bool MdOS::IO::BasicSerial::m_initialized = false;

MdOS::Result MdOS::IO::BasicSerial::init() {
	if (m_initialized) { return MdOS::Result::SUCCESS; }

	MdOS::IO::outb(0x00, COM1_REG(COM_RW_INTERUPT_ENABLE_REG));
	MdOS::IO::outb(0x80, COM1_REG(COM_RW_LINE_CONTROL_REG));
	MdOS::IO::outb(0x03, COM1_REG(COM_RW_LEAST_SIGNIFICANT_BYTE_BAUD));
	MdOS::IO::outb(0x00, COM1_REG(COM_RW_MOST_SIGNIFICANT_BYTE_BAUD));
	MdOS::IO::outb(0x03, COM1_REG(COM_RW_LINE_CONTROL_REG));
	MdOS::IO::outb(0xC7, COM1_REG(COM_W_FIFO_CONTROL_REG));
	MdOS::IO::outb(0x1E, COM1_REG(COM_RW_MODEM_CONTROL_REG));

	MdOS::IO::outb(0xAE, COM1_REG(COM_W_TX_BUFF));
	if (MdOS::IO::inb(COM1_REG(COM_R_RX_BUFF)) != 0xAE) { return MdOS::Result::INIT_FAILURE; }

	MdOS::IO::outb(0x0F, COM1_REG(COM_RW_MODEM_CONTROL_REG));
	m_initialized = true;
	return MdOS::Result::SUCCESS;
}