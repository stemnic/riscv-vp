#ifndef UARTTHREAD_H
#define UARTTHREAD_H

#include <QThread>
#include <QtNetwork>
#include <QMutex>

class UartThread : public QThread
{
    Q_OBJECT
public:
    UartThread(QObject *parent = nullptr);
    ~UartThread();

    //! set server, port...
    void setConnection(const QString& f_host, quint32 f_port)
    {
        m_host = f_host;
        m_port = f_port;
    }

    //! start thread, connects to server
    void run() override;

public slots:
    //! sends \p f_string over the uart connection
    void sendInput(QString f_string)
    {
        m_sendData = f_string;
    }

    //! stop thread
    void stop();

signals:
    //! signal about input read
    void dataRead(QString f_string);

private:
    //! read/write uart
    void processUart();

    //! pointer used here, so that construction is done in internal run method
    //! and, thus, this thread.
    QTcpSocket* m_socket;
    QMutex m_mutex;

    // data to be send,
    QString m_sendData;

    //! server data
    QString m_host;
    quint32 m_port;

    // connection flag
    bool m_connected;
    // flag for quitting
    bool m_alive;

    //! delay in read and write by socket
    const quint32 SOCKET_DELAY = 100;
};

#endif // UARTTHREAD_H
