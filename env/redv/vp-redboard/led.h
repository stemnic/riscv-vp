#ifndef LED_H
#define LED_H

#include "gpioutils.h"

#include <QGraphicsItem>
#include <QPainter>

using namespace gpioutil;

class LED : public QGraphicsItem
{
public:
    //! default colorMask is for rgb
    LED(QPoint f_point, quint8 f_pin, GpioClient** f_gpio)
        : m_size(20, 20)
        , m_colorMask(0xff)
        , m_pin(f_pin)
        , m_gpio(f_gpio)
    {
        setPos(f_point);
    }

    quint8 getPin() const
    {
        return m_pin;
    }

    //! set color mask for led (lsbs for rgb).
    void setColorMask(quint8 f_mask)
    {
        m_colorMask = f_mask;
    }

    //! return color mask
    quint8 getColorMask() const
    {
        return m_colorMask;
    }

    QRectF boundingRect() const override
    {
        return QRectF(-m_size.width()/2, -m_size.height()/2, m_size.width(), m_size.height());
    }

    void paint(QPainter *f_painter, const QStyleOptionGraphicsItem *f_option, QWidget *f_widget) override
    {
        uint8_t map = 0;
        if (*m_gpio)
        {
            if ((*m_gpio)->state & (1 << translatePinToGpioOffs(m_pin)))
            {
                map = m_colorMask;
            }
        }
        m_color = QColor(map & 1 ? 255 : 0, map & (1 << 1) ? 255 : 0, map & (1 << 2) ? 255 : 0, 0xC0);

        f_painter->setBrush(m_color);
        f_painter->drawEllipse(-m_size.width()/2, -m_size.height()/2, m_size.width(), m_size.height());
    }

    void advance(int step) override
    {
        if (! step)
            return;
        if (*m_gpio)
        {
            (*m_gpio)->update();
        }
        update();
    }


private:
    QSize m_size;
    QColor m_color;
    quint8 m_colorMask;
    quint8 m_pin;

    GpioClient** m_gpio;
};

#endif // LED_H
