#include "i2cserver.hpp"

#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

// set up socket, currently one to one, should be one to many 
I2CServer::I2CServer() : 
    I2C("I2C"), 
    m_acceptor(m_io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 4444)),
    m_socket(m_io_service),
    m_stop(false)
    {

    m_thread.reset(new std::thread([this]() {
        for (;;) {
            if (!m_stop.load()) {
                std::cout << "WAIT FOR I2C CLIENT" << std::endl;
                m_acceptor.accept(m_socket);
                std::cout << "CONNECTED TO I2C CLIENT" << std::endl;
                m_stop.store(true);
            }
        }
    })); 
}

I2CServer::~I2CServer() {
}

uint8_t I2CServer::process_i2c(uint8_t f_reg, uint8_t f_in) {

    uint8_t l_transmission[2] {f_reg, f_in};
    uint8_t l_return[1];
    system::error_code ec;  

    // send signal using crsr
    if (f_reg == I2C::CRSR_REG_ADDR)
    {
        if (f_in & (1 << I2C_CMD_STA))
        {
            l_transmission[1] &= (1 << I2C_CMD_STA);
        }
        else if (f_in & (1 << I2C_CMD_ACK))
        {
            l_transmission[1] &= (1 << I2C_CMD_ACK);
        }
        else if (f_in & (1 << I2C_CMD_STO))
        {
            l_transmission[1] &= (1 << I2C_CMD_STO);
        }
    }

    // data value, init for case of no connection
    if (f_reg == I2C::TXRX_REG_ADDR)
    {
        l_return[0] = f_in;
    }
    else
    {
        l_return[0] = 0;
    }

    // no client, return
    if (!m_stop.load()) {
        return l_return[0];
    }

    // client, send transmisstion and receive result
    asio::write(m_socket, asio::buffer(l_transmission), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
        if ((asio::error::eof == ec) || (asio::error::connection_reset == ec))
        {
            m_socket.close();
            m_stop.store(false);
        }
    }
    asio::read(m_socket, asio::buffer(l_return, 1), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
        if ((asio::error::eof == ec) || (asio::error::connection_reset == ec))
        {
            m_socket.close();
            m_stop.store(false);
        }
    }

    // if nothing worked, nothing is happend.
    return l_return[0];
}