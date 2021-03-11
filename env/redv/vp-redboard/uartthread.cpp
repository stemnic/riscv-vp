#include "uartthread.h"

#include <QMutexLocker>

UartThread::UartThread(QObject *parent)
    : QThread(parent)
    , m_socket(nullptr)
    , m_connected(true)
    , m_alive(true)
{
}

UartThread::~UartThread()
{
    m_alive = false;
    stop();
    quit();
    wait();
}

void UartThread::stop()
{
    m_connected = false;
}

void UartThread::run()
{
    while (m_alive)
    {
        if (m_connected)
        {
            processUart();
        }
    }
}


void UartThread::processUart()
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

        QString inString = m_socket->readLine();
        emit dataRead(inString);


        if (m_sendData.size() > 0)
        {
            qDebug() << m_sendData;
            m_socket->write(m_sendData.toLocal8Bit());
            m_socket->write("\n "); //don't know why one byte is stolen by write? Thus, space added
            m_sendData.clear();
            if (!m_socket->waitForBytesWritten())
            {
                return;
            }
        }

        msleep(SOCKET_DELAY);
    } while (m_socket->isOpen() && m_connected);
    if (m_socket)
    {
        m_socket->disconnectFromHost();
        delete m_socket;
        m_socket = nullptr;
    }
}
