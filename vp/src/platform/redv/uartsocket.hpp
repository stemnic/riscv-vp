#ifndef RISCV_VP_UARTSOCKET_H
#define RISCV_VP_UARTSOCKET_H

#include "abstract_uart.h"

#include <atomic>
#include <boost/asio.hpp>
#include <thread>

class UARTSocket : public AbstractUART {
public:
	UARTSocket(const sc_core::sc_module_name&, uint32_t);
	~UARTSocket(void);

private:
	void handle_input(int fd) override;
	void write_data(uint8_t) override;

	boost::asio::io_service m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::asio::ip::tcp::socket m_socket;
	std::unique_ptr<std::thread> m_thread;
	std::atomic<bool> m_stop;
};

#endif  // RISCV_VP_UARTSOCKET_H
