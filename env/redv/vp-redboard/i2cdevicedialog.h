#ifndef I2CDEVICEDIALOG_H
#define I2CDEVICEDIALOG_H

#include <QDialog>

#include "i2c.h"

namespace Ui {
class I2CDeviceDialog;
}

class I2CDeviceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit I2CDeviceDialog(QWidget *parent = nullptr);
    ~I2CDeviceDialog();

public slots:
    void openForI2C(I2C* f_i2c);
    void valueChanged(int row, int column);

    void done(int r) override;
private:
    void showRegisters();

    I2C* m_i2c;
    Ui::I2CDeviceDialog *ui;
};

#endif // I2CDEVICEDIALOG_H
