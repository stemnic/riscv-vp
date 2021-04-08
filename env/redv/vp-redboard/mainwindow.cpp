#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "uartthread.h"
#include "i2cthread.h"

#include "led.h"
#include "sevensegment.h"
#include "button.h"
#include "oled.h"
#include "i2c.h"


#include <QFileDialog>
#include <QMessageBox>

#include <iostream>

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
    m_i2cDeviceDialog = new I2CDeviceDialog(this);

    m_graphicsScene = new QGraphicsScene(this);
    QObject::connect(&m_timer, &QTimer::timeout, m_graphicsScene, &QGraphicsScene::advance);

    ui->statusbar->showMessage("not connected");

    m_gpio = nullptr;
    m_uartThread = nullptr;
    m_i2cThread = nullptr;
    m_i2cEnable = false;
}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_preferencesDialog;
    delete m_i2cDeviceDialog;
    delete m_graphicsScene;

    disconnectGPIO();
    disconnectUart();
    disconnectI2C();
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


void MainWindow::newBoard()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Image File"), m_preferencesDialog->getDataPath(), tr("PNG (*.png);;All Files (*)"));
    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        // delete current scene
        m_graphicsScene->clear();

        m_backgroundFileName = fileName;
        QPixmap backgroundPixmap(fileName);

        // ToDo: Ask for size
        QSize size(800, 600);
        backgroundPixmap = backgroundPixmap.scaled(size, Qt::IgnoreAspectRatio);

        m_graphicsScene->setSceneRect(0, 0, size.width(), size.height());

        ui->graphicsView->setFixedSize(backgroundPixmap.size());
        ui->graphicsView->setBackgroundBrush(backgroundPixmap);
        ui->graphicsView->setCacheMode(QGraphicsView::CacheBackground);
        ui->graphicsView->setScene(m_graphicsScene);
    }
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
        ui->actionEditor->setChecked(false);
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
    m_backgroundFileName = config["background"].toString("");
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
                19,
                &m_gpio);
        // graphics scene should take care now of item and delete it...
        m_graphicsScene->addItem(rgbLed);
    }

    if(config.contains("leds"))
    {
        QJsonArray leds = config["leds"].toArray();
        for(qint32 i = 0; i < leds.size(); i++)
        {
            QJsonObject ledJO = leds[i].toObject();
            LED* led = new LED(
                QPoint(ledJO["offs"].toArray().at(0).toInt(100),
                       ledJO["offs"].toArray().at(1).toInt(100)),
                       ledJO["pin"].toInt(13),
                    &m_gpio);
                quint8 colorMask = ledJO["color"].toInt(1);
            led->setColorMask(colorMask);
            // graphics scene should take care now of item and delete it...
            m_graphicsScene->addItem(led);
        }
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
    if(config.contains("oled"))
    {
        QJsonObject obj = config["oled"].toObject();
        OLED* oled = new OLED(
            QPoint(obj["offs"].toArray().at(0).toInt(450),
                   obj["offs"].toArray().at(1).toInt(343)),
            obj["margin"].toInt(15),
            obj["scale"].toDouble(1.0));
        // graphics scene should take care now of item and delete it...
        m_graphicsScene->addItem(oled);
    }

    if(config.contains("i2c"))
    {
        QJsonObject obj = config["i2c"].toObject();
        quint8 id = static_cast<quint8>(obj["id"].toInt(1));

        int sz = obj["reg"].toArray().size();
        QMap<quint8, quint8> regs;
        // no check... attention
        for (int i = 0; i < sz; i+=2)
        {
            quint8 regItem = static_cast<quint8>(obj["reg"].toArray().at(i).toInt(0));
            quint8 valItem = static_cast<quint8>(obj["reg"].toArray().at(i+1).toInt(0));
            regs.insert(regItem, valItem);
        }

        I2C* i2c = new I2C(QRect(
                               QPoint(obj["pos"].toArray().at(0).toInt(0),
                                      obj["pos"].toArray().at(1).toInt(0)),
                               QSize(obj["dim"].toArray().at(0).toInt(100),
                                     obj["dim"].toArray().at(1).toInt(100))),
                               id,
                               regs);
        i2c->setName(obj["name"].toString(QString("I2C")));


        m_graphicsScene->addItem(i2c);
        m_i2cEnable = true;
        QObject::connect(i2c, &I2C::openI2CDeviceSettings, m_i2cDeviceDialog, &I2CDeviceDialog::openForI2C, Qt::UniqueConnection);
    }
    else
    {
        m_i2cEnable = false;
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

// ToDo: extract json write
void MainWindow::saveBoard()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Board Config"), m_preferencesDialog->getDataPath(), tr("JSON (*.json);;All Files (*)"));
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);

    QJsonObject root;
    root["background"] = m_backgroundFileName;

    QJsonArray buttons, leds;

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
            led["color"] = dynamic_cast<LED*>(item)->getColorMask();
            led["pin"] =  dynamic_cast<LED*>(item)->getPin();
            leds.push_back(led);
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
        else if (dynamic_cast<OLED*>(item))
        {
            QJsonObject oled;
            oled["margin"] = dynamic_cast<OLED*>(item)->getMargin();
            oled["scale"] = dynamic_cast<OLED*>(item)->getScale();
            QJsonArray pos;
            pos.push_back(item->pos().x());
            pos.push_back(item->pos().y());
            oled["offs"] = pos;
            root["oled"] = oled;
        }
        else if (dynamic_cast<I2C*>(item))
        {
            I2C *i2c = dynamic_cast<I2C*>(item);
            QJsonObject i2cJson;
            i2cJson["id"] = i2c->getId();
            QJsonArray pos;
            pos.push_back(item->pos().x());
            pos.push_back(item->pos().y());
            i2cJson["pos"] = pos;
            QJsonArray dim;
            dim.push_back(i2c->getSize().width());
            dim.push_back(i2c->getSize().height());
            i2cJson["dim"] = dim;
            QJsonArray jsonReg;
            const QMap<quint8, quint8>& reg = i2c->getRegisters();
            // need typedef
            for (QMap<quint8, quint8>::const_iterator it = reg.begin(); it != reg.end(); ++it)
            {
                jsonReg.push_back(it.key());
                jsonReg.push_back(it.value());
            }
            i2cJson["reg"] = jsonReg;
            i2cJson["name"] = dynamic_cast<I2C*>(item)->getName();
            root["i2c"] = i2cJson;
        }
    }

    if (buttons.size() > 0)
    {
        root["buttons"] = buttons;
    }
    if (leds.size() > 0)
    {
        root["leds"] = leds;
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
    disconnectI2C();

    ui->statusbar->showMessage("disconnected", 1000);

}


// connect to gpio and uart, uart works only with redv-vp.
void MainWindow::connect()
{
    connectGPIO();
    connectUart();
    if (m_i2cEnable)
    {
        connectI2C();
    }
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
        ui->statusbar->showMessage("gpio connected", 1000);
    }
    else
    {
        disconnectGPIO();
    }
}

void MainWindow::disconnectGPIO()
{
    if (m_gpio)
    {
        delete m_gpio;
        m_gpio = nullptr;
    }
    ui->statusbar->showMessage("gpio disconnected", 1000);
}

//// I2C
void MainWindow::connectI2C()
{

    QString host;
    quint32 port;
    m_preferencesDialog->getI2CConnection(host, port);

    if (m_i2cThread)
    {
        disconnectI2C();
    }
    m_i2cThread = new I2CThread;

    m_i2cThread->setConnection(host, port);

    // read and writereq could be direct to graphics item, however I pass them through mainwindow which controls the animation
    QObject::connect(m_i2cThread, &I2CThread::i2cReadReq, this, &MainWindow::i2cReadReq, Qt::UniqueConnection);
    QObject::connect(m_i2cThread, &I2CThread::i2cWriteReq, this, &MainWindow::i2cWriteReq, Qt::UniqueConnection);
    QObject::connect(this, &MainWindow::i2cReadReply, m_i2cThread, &I2CThread::i2cReadReply, Qt::UniqueConnection);
    QObject::connect(this, &MainWindow::startI2CThread, m_i2cThread, &I2CThread::start, Qt::UniqueConnection);
    emit startI2CThread(QThread::LowPriority);
}

void MainWindow::disconnectI2C()
{
    if (m_i2cThread)
    {
        QObject::connect(this, &MainWindow::stopI2CThread, m_i2cThread, &I2CThread::stop, Qt::UniqueConnection);
        emit stopI2CThread();

        delete m_i2cThread;
    }
    m_i2cThread = nullptr;
}

void MainWindow::i2cReadReq(quint8 f_id, quint8 f_reg)
{
    foreach (QGraphicsItem* item, m_graphicsScene->items())
    {
        I2C* i2cGraphicsItem = dynamic_cast<I2C*>(item);
        if (i2cGraphicsItem && i2cGraphicsItem->getId() == f_id)
        {
            f_reg = i2cGraphicsItem->readReq(f_reg);
        }
    }

    std::cout << "i2cRead " << "from id " << std::hex << int(f_id) << " of reg " << int(f_reg) << "\n";
    emit i2cReadReply(f_reg);
}

void MainWindow::i2cWriteReq(quint8 f_id, quint8 f_reg, quint8 f_val)
{
    foreach (QGraphicsItem* item, m_graphicsScene->items())
    {
        I2C* i2cGraphicsItem = dynamic_cast<I2C*>(item);
        if (i2cGraphicsItem && i2cGraphicsItem->getId() == f_id)
        {
            i2cGraphicsItem->writeReq(f_reg, f_val);
        }
    }
    std::cout << "i2cWrite " << "from id " << std::hex << int(f_id) << " of reg " << int(f_reg) << " with value " << int(f_val) << "\n";
}


//// Uart
void MainWindow::connectUart()
{
    QString host;
    quint32 port;
    m_preferencesDialog->getUartConnection(host, port);

    if (m_uartThread)
    {
        disconnectUart();
    }
    m_uartThread = new UartThread;

    m_uartThread->setConnection(host, port);
    QObject::connect(m_uartThread, &UartThread::dataRead, this, &MainWindow::updateUartTextEdit, Qt::UniqueConnection);
    QObject::connect(this, &MainWindow::sendUart, m_uartThread, &UartThread::sendInput, Qt::UniqueConnection);
    QObject::connect(this, &MainWindow::startUartThread, m_uartThread, &UartThread::start, Qt::UniqueConnection);
    emit startUartThread(QThread::LowPriority);
}

void MainWindow::disconnectUart()
{
    if (m_uartThread)
    {
        QObject::connect(this, &MainWindow::stopUartThread, m_uartThread, &UartThread::stop);
        emit stopUartThread();

        delete m_uartThread;
    }
    m_uartThread = nullptr;
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




/*!
  aboutbox
*/
void MainWindow::about()
{
    QMessageBox::about(this, "About vp-redboard",
                       "Version 04/2021 <br>"
                       "<br>"
                       "THE SOFTWARE IS PROVIDED «AS IS», WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED. <br><br>"
                       "See LICENSE file for further information.");
}

/*!
  about Qt
*/
void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, "About Qt");
}
