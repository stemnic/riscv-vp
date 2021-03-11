#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QJsonDocument>

#include <gpio/gpio-client.hpp>

#include "preferencesdialog.h"

#include "uartthread.h"

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
    void openBoard();
    void saveBoard();

    void changeMode();

    void openPreferencesDialog();

    void connect();
    void disconnect();

    void updateUartTextEdit(QString f_string);
    void inputUart();

signals:
    void sendUart(QString f_string);
    void startUartThread(QThread::Priority f_priority);
    void stopUartThread();

private:
    void connectGPIO();
    void disconnectGPIO();

    void connectUart();
    void disconnectUart();

    Ui::MainWindow *ui;
    QGraphicsScene *m_graphicsScene;

    PreferencesDialog *m_preferencesDialog;

    UartThread* m_uartThread;
    GpioClient* m_gpio;

    //! delay between update of gui
    const uint32_t FRAME_UPDATE_DELAY = 500;

    void startGraphicsUpdates();
    void stopGraphicsUpdates();
    QString m_backgroundFileName;

    QTimer m_timer;

    QJsonDocument m_jsonDoc;
};
#endif // MAINWINDOW_H
