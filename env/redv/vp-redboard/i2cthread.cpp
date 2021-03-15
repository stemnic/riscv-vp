#include "i2cthread.h"

#include <iostream>

//#define DEBUG
#ifdef DEBUG
#define DEBUGOUT std::cout
#else
#define DEBUGOUT if(false) std::cout
#endif


#define I2C_CMD_STA 7
#define I2C_CMD_STO 6
#define I2C_CMD_ACK 3

#define I2C_STAT_RXACK 7
#define I2C_STAT_BUSY 6
#define I2C_STAT_ARLO 5
#define I2C_STAT_TIP 1
#define I2C_STAT_IF 0


I2CThread::I2CThread(QObject *parent)
    : QThread(parent)
    , m_socket(nullptr)
    , m_connected(true)
{
}

I2CThread::~I2CThread()
{
    quit();
    wait();
}

void I2CThread::stop()
{
    m_connected = false;
}

void I2CThread::run()
{
    QMutexLocker locker(&m_mutex);

    if (!m_socket)
    {
        m_socket = new QTcpSocket();
    }
    m_socket->connectToHost(m_host, m_port);

    if (!m_socket->waitForConnected())
    {
        return;
    }

    do {
        if (!m_socket->waitForReadyRead())
        {
            return;
        }

        char charDataReceived[2] = {0, 0};
        qint64 result = m_socket->read(charDataReceived, 2);
        if (result < 0)
        {
            m_connected = false;
            continue;
        }

        quint8 dataReceived[2];
        dataReceived[0] = static_cast<quint8>(charDataReceived[0]);
        dataReceived[1] = static_cast<quint8>(charDataReceived[1]);
        DEBUGOUT << "RECEIVED  " << std::hex << int(dataReceived[0]) << " " << int(dataReceived[1]) << std::endl;

        quint8 dataToSend[1] = {0};
        dataToSend[0] = processTransmission(dataReceived);

        char charDataToSend[1];
        charDataToSend[0] = static_cast<char>(dataToSend[0]);
        result = m_socket->write(charDataToSend, 1);
        if (result < 0)
        {
            m_connected = false;
            continue;
        }
        DEBUGOUT << "SEND " << int(dataToSend[0]) << std::endl;

        msleep(100);
    } while (m_socket->isOpen() && m_connected);
    if (m_socket)
    {
        m_socket->disconnectFromHost();
        delete m_socket;
        m_socket = nullptr;
    }
}


void I2CThread::i2cReadReply(char f_val)
{
    m_dataFromSlot = f_val;
    m_waitForResponse = false;
}

quint8 I2CThread::processTransmission(quint8 f_dataReceived[2])
{

    quint8 dataToSend[1] = {0};

    bool crsr = f_dataReceived[0] & 0x10 ? true : false;

    if (crsr && f_dataReceived[1] & (1 << I2C_CMD_STO))
    {
        DEBUGOUT << "GOT STO" << std::endl;
        DEBUGOUT << "RESET TO IDLE" << std::endl;
        m_state = IDLE;
    }

    switch (m_state)
    {
    default:
    case IDLE:
        if (crsr && f_dataReceived[1] & (1 << I2C_CMD_STA))
        {
            DEBUGOUT << "GOT STA" << std::endl;
            DEBUGOUT << "IDLE -> ADDRESS" << std::endl;
            m_state = ADDRESS;
        }
        break;
    case ADDRESS:
        if (crsr)
            break;
        m_i2cId = (quint8(f_dataReceived[1]) >> 1);
        if (f_dataReceived[1] & 0x1)
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
        break;
    case REGISTER:
        if (crsr)
            break;
        m_regPointer = f_dataReceived[1];
        DEBUGOUT << "SET POINTER " << std::hex << int(m_regPointer) << std::endl;
        DEBUGOUT << "ADDRESS -> WRITE" << std::endl;
        m_state = WRITE;
        dataToSend[0] = (1 << I2C_CMD_ACK);
        break;
    case WRITE:
        if (crsr && f_dataReceived[1] & (1 << I2C_CMD_STA))
        {
            m_state = ADDRESS;
            DEBUGOUT << "GOT STA" << std::endl;
            DEBUGOUT << "WRITE -> ADDRESS" << std::endl;
        }
        else
        {
            m_state = WRITE;
            emit i2cWriteReq(m_i2cId, m_regPointer, f_dataReceived[1]);

            DEBUGOUT << "SET " << m_regPointer << " TO " << int(f_dataReceived[1]) << std::endl;
            DEBUGOUT << "WRITE -> WRITE" << std::endl;
            dataToSend[0] = (1 << I2C_CMD_ACK);
            ++m_regPointer;
        }
        break;
    case READ:
        if (crsr && f_dataReceived[1] & (1 << I2C_CMD_ACK))
        {
            m_state = IDLE;
            DEBUGOUT << "GOT ACK" << std::endl;
            DEBUGOUT << "READ -> IDLE" << std::endl;
        }
        else
        {
            m_state = READ;
            DEBUGOUT << "READ -> READ" << std::endl;
            emit i2cReadReq(m_i2cId, m_regPointer);

            //                m_waitForResponse = true;
            //                while (m_waitForResponse);

            dataToSend[0] = m_dataFromSlot;
            ++m_regPointer;
            DEBUGOUT << "READ " << m_regPointer << " : " << int(dataToSend[0]) << std::endl;
        }
        break;
    }
    return dataToSend[0];
}

