#ifndef RISCV_VP_I2CSERVER_H
#define RISCV_VP_I2CSERVER_H

#include <atomic>
#include <boost/asio.hpp>
#include <thread>

#include "i2c.hpp"

class I2CServer : public I2C {
   public:
	I2CServer();
	~I2CServer();

	virtual uint8_t process_i2c(uint8_t f_reg, uint8_t f_in);

   private:
	boost::asio::io_service m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::asio::ip::tcp::socket m_socket;
	std::unique_ptr<std::thread> m_thread;
	std::atomic<bool> m_stop;
};

#endif  // RISCV_VP_I2CSERVER_H