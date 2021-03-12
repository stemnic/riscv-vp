#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "uartthread.h"
#include "led.h"
#include "sevensegment.h"
#include "button.h"

#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    ui->plainTextEditUart->setPalette(p);
    ui->lineEditUart->setPalette(p);

    m_preferencesDialog = new PreferencesDialog(this);

    m_graphicsScene = new QGraphicsScene(this);
    QObject::connect(&m_timer, &QTimer::timeout, m_graphicsScene, &QGraphicsScene::advance);

    ui->statusbar->showMessage("not connected");

    m_gpio = nullptr;
    m_uartThread = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_preferencesDialog;
    delete m_graphicsScene;

    disconnectGPIO();
    disconnectUart();
}

void MainWindow::changeMode()
{
    if (ui->actionEditor->isChecked())
    {
        //editor mode
        foreach (QGraphicsItem* item, m_graphicsScene->items())
        {
            item->setFlag(QGraphicsItem::ItemIsMovable );
        }
    }
    else
    {
        // run mode
        foreach (QGraphicsItem* item, m_graphicsScene->items())
        {
            item->setFlag(QGraphicsItem::ItemIsMovable, false);
        }
    }
}


////
void MainWindow::openPreferencesDialog()
{
    m_preferencesDialog->show();
}

void MainWindow::openBoard()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Board Config"), m_preferencesDialog->getDataPath(), tr("JSON (*.json);;All Files (*)"));
    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        loadBoardConfig(fileName);
    }
}

void MainWindow::loadBoardConfig(const QString& f_fileName)
{
    QFile confFile(f_fileName);
    if (!confFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Warning", "Could not open file");
        return;
    }

    // delete current scene
    m_graphicsScene->clear();

    QByteArray jsonText = confFile.readAll();
    confFile.close();

    QJsonParseError jsonError;
    m_jsonDoc = QJsonDocument::fromJson(jsonText, &jsonError);
    if(m_jsonDoc.isNull())
    {
        QMessageBox::warning(this, "Warning", "Could not read json file: \n" + jsonError.errorString());
        return;
    }
    QJsonObject config = m_jsonDoc.object();

    QPixmap backgroundPixmap(config["background"].toString(""));
    if(backgroundPixmap.isNull())
    {
        // trying relative to data path
        QString filename = m_preferencesDialog->getDataPath()+config["background"].toString("");
        m_backgroundFileName = filename;
        backgroundPixmap.load(filename);
    }
    if(backgroundPixmap.isNull())
    {
        QMessageBox::warning(this, "Warning", "Could not read json file: \n" + config["background"].toString());
        return;
    }

    if(config.contains("rgb"))
    {
        QJsonObject obj = config["rgb"].toObject();
        LED* rgbLed = new LED(
            QPoint(obj["offs"].toArray().at(0).toInt(89),
                   obj["offs"].toArray().at(1).toInt(161)),
                &m_gpio);
        // graphics scene should take care now of item and delete it...
        m_graphicsScene->addItem(rgbLed);
    }

    if(config.contains("sevensegment"))
    {
        QJsonObject obj = config["sevensegment"].toObject();
        Sevensegment* sevensegment = new Sevensegment(
            QPoint(obj["offs"].toArray().at(0).toInt(312),
                   obj["offs"].toArray().at(1).toInt(353)),
            QSize(obj["extent"].toArray().at(0).toInt(36),
                   obj["extent"].toArray().at(1).toInt(50)),
            obj["linewidth"].toInt(7),
            &m_gpio);
        // graphics scene should take care now of item and delete it...
        m_graphicsScene->addItem(sevensegment);
    }

    if(config.contains("buttons"))
    {
        QJsonArray buttons = config["buttons"].toArray();
        for(qint32 i = 0; i < buttons.size(); i++)
        {
            QJsonObject buttonJO = buttons[i].toObject();
            Button* button = new Button(
                QRect{
                    QPoint{buttonJO["pos"].toArray().at(0).toInt(), buttonJO["pos"].toArray().at(1).toInt()},
                    QSize{buttonJO["dim"].toArray().at(0).toInt(), buttonJO["dim"].toArray().at(1).toInt()}
                },
                static_cast<uint8_t>(buttonJO["pin"].toInt()),
                &m_gpio,
                buttonJO["name"].toString(QString("undef"))
            );
            // graphics scene should take care now of item and delete it...
            m_graphicsScene->addItem(button);
        }
    }


    QSize size(config["windowsize"].toArray().at(0).toInt(800),
               config["windowsize"].toArray().at(1).toInt(600));

    backgroundPixmap = backgroundPixmap.scaled(size, Qt::IgnoreAspectRatio);

    ///// graphics view
    m_graphicsScene->setSceneRect(0, 0, size.width(), size.height());

    ui->graphicsView->setFixedSize(size);
    ui->graphicsView->setBackgroundBrush(backgroundPixmap);
    ui->graphicsView->setCacheMode(QGraphicsView::CacheBackground);
    ui->graphicsView->setScene(m_graphicsScene);
}


void MainWindow::saveBoard()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Board Config"), m_preferencesDialog->getDataPath(), tr("JSON (*.json);;All Files (*)"));
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);

    QJsonObject root;
    root["background"] = m_backgroundFileName;

    QJsonArray buttons;

    // run mode
    foreach (QGraphicsItem* item, m_graphicsScene->items())
    {
        if (dynamic_cast<LED*>(item))
        {
            QJsonObject led;
            led["linewidth"] = 15;
            QJsonArray pos;
            pos.push_back(item->pos().x());
            pos.push_back(item->pos().y());
            led["offs"] = pos;
            root["rgb"] = led;
        }
        else if (dynamic_cast<Sevensegment*>(item))
        {
            QJsonObject sevensegment;
            sevensegment["linewidth"] = 7;
            QJsonArray pos;
            pos.push_back(item->pos().x());
            pos.push_back(item->pos().y());
            sevensegment["offs"] = pos;
            QJsonArray extent;
            extent.push_back(dynamic_cast<Sevensegment*>(item)->getSize().width());
            extent.push_back(dynamic_cast<Sevensegment*>(item)->getSize().height());
            sevensegment["extent"] = extent;
            root["sevensegment"] = sevensegment;
        }
        else if (dynamic_cast<Button*>(item))
        {
            QJsonObject button;
            button["pin"] = dynamic_cast<Button*>(item)->getPin();
            QJsonArray pos;
            pos.push_back(item->pos().x());
            pos.push_back(item->pos().y());
            button["pos"] = pos;
            QJsonArray dim;
            dim.push_back(dynamic_cast<Button*>(item)->getSize().width());
            dim.push_back(dynamic_cast<Button*>(item)->getSize().height());
            button["dim"] = dim;
            buttons.push_back(button);
        }
    }

    if (buttons.size() > 0)
    {
        root["buttons"] = buttons;
    }

    QJsonArray winsize;
    winsize.push_back(m_graphicsScene->width());
    winsize.push_back(m_graphicsScene->height());
    root["windowsize"] = winsize;


    QByteArray byteArray = QJsonDocument(root).toJson();
    jsonFile.write(byteArray);
    jsonFile.close();
}


// start gui / breadboard refreshing
void MainWindow::startGraphicsUpdates()
{
    m_timer.start(FRAME_UPDATE_DELAY);
}

// stop gui / breadboard refreshing
void MainWindow::stopGraphicsUpdates()
{
    m_timer.stop();
}

void MainWindow::disconnect()
{
    stopGraphicsUpdates();

    disconnectGPIO();
    disconnectUart();

    ui->statusbar->showMessage("gpio disconnected");

}


// connect to gpio and uart, uart works only with redv-vp.
void MainWindow::connect()
{
    connectGPIO();
    connectUart();

    startGraphicsUpdates();
}

//// GPIO
void MainWindow::connectGPIO()
{
    QString host;
    quint32 port;
    m_preferencesDialog->getGPIOConnection(host, port);
    QString portString;
    portString.setNum(port);
    if (!m_gpio)
    {
        m_gpio = new GpioClient;
    }
    if (m_gpio->setupConnection(host.toLocal8Bit(), portString.toLocal8Bit()))
    {
        ui->statusbar->showMessage("gpio connected");
    }
}

void MainWindow::disconnectGPIO()
{
    if (m_gpio)
    {
        delete m_gpio;
        m_gpio = nullptr;
    }
}


//// Uart
void MainWindow::connectUart()
{
    QString host;
    quint32 port;
    m_preferencesDialog->getUartConnection(host, port);

    if (!m_uartThread)
    {
        m_uartThread = new UartThread;
    }

    m_uartThread->setConnection(host, port);
    QObject::connect(m_uartThread, &UartThread::dataRead, this, &MainWindow::updateUartTextEdit);
    QObject::connect(this, &MainWindow::sendUart, m_uartThread, &UartThread::sendInput);
    QObject::connect(this, &MainWindow::startUartThread, m_uartThread, &UartThread::start);
    emit startUartThread(QThread::LowPriority);
}

void MainWindow::disconnectUart()
{
    if (m_uartThread)
    {
        QObject::connect(this, &MainWindow::stopUartThread, m_uartThread, &UartThread::stop);
        emit stopUartThread();

        delete m_uartThread;
        m_uartThread = nullptr;
    }
}

void MainWindow::updateUartTextEdit(QString f_string)
{
    ui->plainTextEditUart->appendPlainText(f_string.trimmed());
}

void MainWindow::inputUart()
{
    QString text = ui->lineEditUart->text();
    emit sendUart(text);
    ui->lineEditUart->clear();
}
