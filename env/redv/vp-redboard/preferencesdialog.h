#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public slots:
    void choosePath();

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    void setDataPath(const QString& f_path);
    QString getDataPath() const;

    void getUartConnection(QString& f_host, quint32& f_port) const;
    void getI2CConnection(QString& f_host, quint32& f_port) const;
    void getGPIOConnection(QString& f_host, quint32& f_port) const;

    void accept() override;

private:
    Ui::PreferencesDialog *ui;

    const QString m_prefsFileName = ".redboard";
};

#endif // PREFERENCESDIALOG_H
