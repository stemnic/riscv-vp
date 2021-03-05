#include <boost/asio.hpp>
#include <iostream>

#include "i2cclient.hpp"

using namespace boost;

#ifdef DEBUG
    #define DEBUGOUT std::cout
#else
    #define DEBUGOUT if(false) std::cout
#endif

bool I2CClient::run()
{
  try
  {
    asio::io_service l_io_service;
    asio::ip::tcp::socket l_socket(l_io_service);

    l_socket.connect(asio::ip::tcp::endpoint(asio::ip::address_v6::any(), 4444));

    bool connected = true;
    while (connected)
    {
      system::error_code error;
      uint8_t dataReceived[2];
      uint8_t dataToSend[1];

      asio::read(l_socket, asio::buffer(dataReceived, 2), error);
      if (error)
      {
        std::cerr << error.message() << std::endl;
        connected = false;
      }
      else
      {
        DEBUGOUT << "RECEIVED  " << std::hex << int(dataReceived[0]) << " " << int(dataReceived[1]) << std::endl;
      }

      dataToSend[0] = 0;
      bool crsr = dataReceived[0] & 0x10 ? true : false;

      if (crsr && dataReceived[1] & (1 << I2C_CMD_STO))
      {
        DEBUGOUT << "GOT STO" << std::endl;
        DEBUGOUT << "RESET TO IDLE" << std::endl;
        m_state = IDLE;
      }

      switch (m_state)
      {
      default:
      case IDLE:
        if (crsr && dataReceived[1] & (1 << I2C_CMD_STA))
        {
          DEBUGOUT << "GOT STA" << std::endl;
          DEBUGOUT << "IDLE -> ADDRESS" << std::endl;
          m_state = ADDRESS;
        }
        break;
      case ADDRESS:
        if (crsr)
          break;
        if ((dataReceived[1] >> 1) == m_id)
        {
          if (dataReceived[1] & 0x1)
          {
            DEBUGOUT << "ADDRESS -> READ" << std::endl;
            m_state = READ;
          }
          else
          {
            DEBUGOUT << "ADDRESS -> REGISTER" << std::endl;
            m_state = REGISTER;
          }
          dataToSend[0] = (1 << I2C_CMD_ACK);
        }
        else
        {
          DEBUGOUT << "ADDRESS -> IDLE" << std::endl;
          m_state = IDLE;
        }
        break;
      case REGISTER:
        if (crsr)
          break;
        m_regPointer = dataReceived[1];
        DEBUGOUT << "SET POINTER " << std::hex << int(m_regPointer) << std::endl;
        DEBUGOUT << "ADDRESS -> WRITE" << std::endl;
        m_state = WRITE;
        dataToSend[0] = (1 << I2C_CMD_ACK);
        break;
      case WRITE:
        if (crsr && dataReceived[1] & (1 << I2C_CMD_STA))
        {
          m_state = ADDRESS;
          DEBUGOUT << "GOT STA" << std::endl;
          DEBUGOUT << "WRITE -> ADDRESS" << std::endl;
        }
        else
        {
          m_state = WRITE;
          writeRegister(m_regPointer, dataReceived[1]);
          DEBUGOUT << "SET " << m_regPointer << " TO " << int(dataReceived[1]) << std::endl;
          DEBUGOUT << "WRITE -> WRITE" << std::endl;
          dataToSend[0] = (1 << I2C_CMD_ACK);
          ++m_regPointer;
        }
        break;
      case READ:
        if (crsr && dataReceived[1] & (1 << I2C_CMD_ACK))
        {
          m_state = IDLE;
          DEBUGOUT << "GOT ACK" << std::endl;
          DEBUGOUT << "READ -> IDLE" << std::endl;
        }
        else
        {
          m_state = READ;
          DEBUGOUT << "READ -> READ" << std::endl;
          dataToSend[0] = readRegister(m_regPointer);
          ++m_regPointer;
          DEBUGOUT << "READ " << m_regPointer << " : " << int(dataToSend[0]) << std::endl;
        }
        break;
      }

      asio::write(l_socket, asio::buffer(dataToSend, 1), error);
      if (error)
      {
        connected = false;
        std::cerr << error.message() << std::endl;
      }
      else
      {
        DEBUGOUT << "SEND " << int(dataToSend[0]) << std::endl;
      }
    }
  }
  catch (system::system_error &e)
  {
    std::cerr << "Error occured! Error code = "
              << e.code() << ". Message: "
              << e.what() << std::endl;
  }
  return false;
}