#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QJsonDocument>

#include <gpio/gpio-client.hpp>

#include "preferencesdialog.h"
#include "i2cdevicedialog.h"

#include "uartthread.h"
#include "i2cthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadBoardConfig(const QString& f_fileName);

public slots:
    void newBoard();
    void openBoard();
    void saveBoard();

    void changeMode();

    void openPreferencesDialog();

    void connect();
    void disconnect();

    void updateUartTextEdit(QString f_string);
    void inputUart();

    void i2cReadReq(quint8 f_id, quint8 f_reg);
    void i2cWriteReq(quint8 f_id, quint8 f_reg, quint8 f_val);

signals:
    void sendUart(QString f_string);
    void startUartThread(QThread::Priority f_priority);
    void stopUartThread();

    void startI2CThread(QThread::Priority f_priority);
    void stopI2CThread();
    void i2cReadReply(quint8 f_val);

private slots:
    void about();
    void aboutQt();

private:
    void connectGPIO();
    void disconnectGPIO();

    void connectUart();
    void disconnectUart();

    void connectI2C();
    void disconnectI2C();
    bool m_i2cEnable;

    Ui::MainWindow *ui;
    QGraphicsScene *m_graphicsScene;

    PreferencesDialog *m_preferencesDialog;
    I2CDeviceDialog *m_i2cDeviceDialog;

    UartThread* m_uartThread;
    GpioClient* m_gpio;
    I2CThread* m_i2cThread;

    //! delay between update of gui
    const uint32_t FRAME_UPDATE_DELAY = 500;

    void startGraphicsUpdates();
    void stopGraphicsUpdates();
    QString m_backgroundFileName;

    QTimer m_timer;

    QJsonDocument m_jsonDoc;
};
#endif // MAINWINDOW_H
