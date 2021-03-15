#include "i2cdevicedialog.h"
#include "ui_i2cdevicedialog.h"

I2CDeviceDialog::I2CDeviceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::I2CDeviceDialog)
{
    ui->setupUi(this);
}

I2CDeviceDialog::~I2CDeviceDialog()
{
    delete ui;
}

void I2CDeviceDialog::openForI2C(I2C* f_i2c)
{
    m_i2c = f_i2c;
    ui->label->setText("I2C device id: " + QStringLiteral("%1").arg(m_i2c->getId(), 2, 16, QLatin1Char('0')));
    showRegisters();

    open();
}

void I2CDeviceDialog::showRegisters()
{
    QMap<quint8, quint8>& reg = m_i2c->getRegisters();

    const quint16 nbRows = reg.size();
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(nbRows);

    QTableWidgetItem *item = new QTableWidgetItem("Register");
    ui->tableWidget->setHorizontalHeaderItem(0, item);

    item = new QTableWidgetItem("Value");
    ui->tableWidget->setHorizontalHeaderItem(1, item);

    quint8 i = 0;
    for (QMap<quint8, quint8>::iterator it = reg.begin(); it != reg.end(); ++it, i++)
    {
        QString regString = QStringLiteral("%1").arg(it.key(), 2, 16, QLatin1Char('0'));
        item = new QTableWidgetItem(regString);
        ui->tableWidget->setItem(i, 0, item);

        QString valString = QStringLiteral("%1").arg(it.value(), 2, 16, QLatin1Char('0'));
        item = new QTableWidgetItem(valString);
        ui->tableWidget->setItem(i, 1, item);

    }
    QHeaderView* header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    QObject::connect(ui->tableWidget, &QTableWidget::cellChanged, this, &I2CDeviceDialog::valueChanged, Qt::UniqueConnection);
}

void I2CDeviceDialog::done(int r){
    QObject::disconnect(ui->tableWidget, &QTableWidget::cellChanged, this, &I2CDeviceDialog::valueChanged);

    QDialog::done(r);
}



void I2CDeviceDialog::valueChanged(int row, int column)
{
    QMap<quint8, quint8>& reg = m_i2c->getRegisters();

    bool status = false;
    QString string = ui->tableWidget->item(row, 0)->text();
    quint8 key = string.toUInt(&status, 16);
    string = ui->tableWidget->item(row, 1)->text();
    quint8 val = string.toInt(&status, 16);

    if (column == 0)
    {
        QString regString = QStringLiteral("%1").arg(reg[val], 2, 16, QLatin1Char('0'));
        ui->tableWidget->item(row, 0)->setText(regString);
    }
    else if (column == 1)
    {
        reg[key] = val;
    }
    m_i2c->update();
}
