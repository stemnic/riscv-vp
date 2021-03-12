#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QFileDialog>
#include <QTextStream>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    QFile prefsFile(QDir::homePath()+"/"+m_prefsFileName);
    if (prefsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&prefsFile);
        QString inString;
        stream >> inString;
        setDataPath(inString);
        stream >> inString;
        ui->gpioHost->setText(inString);
        stream >> inString;
        ui->gpioPort->setText(inString);
        stream >> inString;
        ui->uartHost->setText(inString);
        stream >> inString;
        ui->uartPort->setText(inString);
        stream >> inString;
        ui->i2cHost->setText(inString);
        stream >> inString;
        ui->i2cPort->setText(inString);
        prefsFile.close();
    }
    else
    {
        setDataPath(QDir::homePath()+"/");
    }
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::setDataPath(const QString& f_path)
{
    ui->dataPath->setText(f_path);
}

QString PreferencesDialog::getDataPath() const
{
    return ui->dataPath->text();
}


void PreferencesDialog::getI2CConnection(QString& f_host, quint32& f_port) const
{
    f_host = ui->i2cHost->text();
    f_port = ui->i2cPort->text().toUInt();
}

void PreferencesDialog::getUartConnection(QString& f_host, quint32& f_port) const
{
    f_host = ui->uartHost->text();
    f_port = ui->uartPort->text().toUInt();
}

void PreferencesDialog::getGPIOConnection(QString& f_host, quint32& f_port) const
{
    f_host = ui->gpioHost->text();
    f_port = ui->gpioPort->text().toUInt();
}

void PreferencesDialog::choosePath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Set Path"), getDataPath());
    setDataPath(path+"/");
}

void PreferencesDialog::accept()
{
    QFile prefsFile(QDir::homePath()+"/"+m_prefsFileName);
    if (prefsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&prefsFile);
        stream << getDataPath() << "\n";
        stream << ui->gpioHost->text() << " " << ui->gpioPort->text() << "\n";
        stream << ui->uartHost->text() << " " << ui->uartPort->text() << "\n";
        stream << ui->i2cHost->text() << " " << ui->i2cPort->text() << "\n";
        prefsFile.close();
    }
    QDialog::accept();
}
