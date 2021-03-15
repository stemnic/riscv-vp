#ifndef I2C_H
#define I2C_H

#include <QGraphicsItem>
#include <QPainter>
#include <QMap>

class I2C : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    I2C(QRect f_rect, quint8 f_id, const QMap<quint8, quint8>& f_registers)
       : m_rect(f_rect)
       , m_id(f_id)
       , m_registers(f_registers)
       , m_accessState(IDLE)
    {
        setPos(f_rect.x(), f_rect.y());
    }

    QSize getSize() const
    {
        return m_rect.size();
    }

    virtual ~I2C()
    {
    }

    qint8 getId() const
    {
        return m_id;
    }

    QRectF boundingRect() const override
    {
        return QRectF(0, 0, m_rect.width(), m_rect.height());
    }

    void setName(const QString& f_name)
    {
        m_name = f_name;
    }

    QString getName() const
    {
        return m_name;
    }


    void paint(QPainter *f_painter, const QStyleOptionGraphicsItem *f_option, QWidget *f_widget) override
    {
        f_painter->setBrush(QColor(100, 0, 100, 100));
        f_painter->drawRect(0, 0, m_rect.width(), m_rect.height());
        f_painter->setPen(Qt::white);
        QString text = m_name;
        f_painter->drawText(0, 10, text);

        if (m_accessState == IDLE)
        {
            return;
        }
        else if (m_accessState == READ)
        {
            f_painter->setPen(Qt::green);
            text = "<R " +  QStringLiteral("%1").arg(m_reg, 2, 16, QLatin1Char('0')) +
                    ": " + QStringLiteral("%1").arg(m_val, 2, 16, QLatin1Char('0'));
            f_painter->drawText(0, 20, text);
        }
        else if (m_accessState == WRITE)
        {
            f_painter->setPen(Qt::red);
            text = ">W " +  QStringLiteral("%1").arg(m_reg, 2, 16, QLatin1Char('0')) +
                    ": " + QStringLiteral("%1").arg(m_val, 2, 16, QLatin1Char('0'));
            f_painter->drawText(0, 30, text);
        }
    }

    void advance(int step) override
    {
        if (!step)
            return;
        update();
    }


    virtual quint8 readReq(quint8 f_reg)
    {
        m_accessState = READ;

        m_reg = f_reg;
        m_val = m_registers[m_reg];
        return m_val;
    }

    virtual void writeReq(quint8 f_reg, quint8 f_val)
    {
        m_accessState = WRITE;

        m_reg = f_reg;
        m_val = f_val;
        m_registers[m_reg] = m_val;
    }

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override
    {
        emit openI2CDeviceSettings(this);
    }

    QMap<quint8, quint8>& getRegisters()
    {
        return m_registers;
    }


signals:
    void openI2CDeviceSettings(I2C*);


protected:
    enum AccessState
    {
        IDLE,
        READ,
        WRITE
    };

private:
    QRect m_rect;
    quint8 m_id;

    QMap<quint8, quint8> m_registers;

    AccessState m_accessState;
    quint8 m_reg, m_val;

    QString m_name;
};

#endif // I2C_H
