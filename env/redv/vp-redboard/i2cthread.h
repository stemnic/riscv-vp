#ifndef I2CTHREAD_H
#define I2CTHREAD_H

#include <QThread>
#include <QObject>
#include <QtNetwork>
#include <QMutex>

class I2CThread : public QThread
{
    Q_OBJECT
public:
    I2CThread(QObject *parent = nullptr);
    ~I2CThread();

    //! set server, port...
    void setConnection(const QString& f_host, quint32 f_port)
    {
        m_host = f_host;
        m_port = f_port;
    }

    //! start thread
    void run() override;

public slots:
    //! stop thread
    void stop();

    //! connect for getting data from other (main) thread
    void i2cReadReply(char f_val);

signals:
    //! signal for read request
    void i2cReadReq(quint8 f_id, quint8 f_reg);
    //! signal for write request
    void i2cWriteReq(quint8 f_id, quint8 f_reg, quint8 f_val);

private:
    quint8 processTransmission(quint8 f_dataReceived[2]);

    enum State
    {
        IDLE,
        ADDRESS,
        REGISTER,
        READ,
        WRITE
    };

    //! for state machine
    State m_state = IDLE;
    //! internal register pointer (ToDo: works only with one i2c slave)
    qint32 m_regPointer = 0x0;
    //! current i2cId
    quint8 m_i2cId;

    //! pointer used here, so that construction is done in internal run method
    //! and, thus, this thread.
    QTcpSocket* m_socket;
    QMutex m_mutex;

    // data received by slot (not socket)
    char m_dataFromSlot;
    // flag before writing data by socket
    volatile bool m_waitForResponse;

    //! server data
    QString m_host;
    quint32 m_port;

    // connection flag
    bool m_connected;
};


#endif // I2CTHREAD_H
