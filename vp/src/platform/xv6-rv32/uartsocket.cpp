#include "uartsocket.hpp"

#include <iostream>
#include <string>
using namespace boost;

UARTSocket::UARTSocket(const sc_core::sc_module_name& name, uint32_t irqsrc)
		: AbstractUART(name, irqsrc),
    m_acceptor(m_io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 2654)),
    m_socket(m_io_service),
    m_stop(false)
    {

    m_thread.reset(new std::thread([this]() {
        for (;;) {
            if (!m_stop.load()) {
                std::cout << "You can connect via client (e.g. telnet) to port 2654" << std::endl;
                m_acceptor.accept(m_socket);
                std::cout << "CONNECTED TO UART CLIENT" << std::endl;
                start_threads(m_socket.native_handle());
                m_stop.store(true);
            }
        }
    })); 
}

UARTSocket::~UARTSocket(void) {
}

void UARTSocket::handle_input(int fd) {
    // error code
    system::error_code ec;  

    // read in to buf
    asio::streambuf l_buf; 

    // read until \n
    asio::read_until(m_socket, l_buf, "\n", ec); 

    if (ec)
    {
        std::cout << ec.message() << std::endl;
        if ((asio::error::eof == ec) || (asio::error::connection_reset == ec) || (asio::error::broken_pipe ==ec))
        {
            m_socket.close();
            m_stop.store(false);
        }
        return;
    }

    // transmit char by char
    std::string l_data = asio::buffer_cast<const char*>(l_buf.data()); 
    // remove \n
    l_data.pop_back(); 
    l_data.pop_back(); 

    for (uint32_t i = 0; i < l_data.size(); ++i)
    {
        rxpush(l_data[i]);
    }
    //note: UART_FIFO_DEPTH limits the rx buffer if data is not processed
    std::cout << "UART: RX " << l_data << std::endl;
}

void UARTSocket::write_data(uint8_t data) {
   // no client, return
    if (!m_stop.load()) {
        return;
    }

    // copy the byte
    uint8_t l_transmission[1] = { data };

    // error code
    system::error_code ec;  

    // client, send transmisstion and receive result
    asio::write(m_socket, asio::buffer(l_transmission), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
        if ((asio::error::eof == ec) || (asio::error::connection_reset == ec) || (asio::error::broken_pipe ==ec))
        {
            m_socket.close();
            m_stop.store(false);
        }
    }

}
