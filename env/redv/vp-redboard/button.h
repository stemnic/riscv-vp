#ifndef BUTTON_H
#define BUTTON_H

#include "gpioutils.h"

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

using namespace gpioutil;

class Button : public QGraphicsItem
{
public:
    Button(QRect f_rect, quint8 f_pin, GpioClient** f_gpio, QString f_name = "")
        : m_rect(f_rect)
        , m_pin(f_pin)
        , m_gpio(f_gpio)
        , m_name(f_name)
        , m_pressed(false)
    {
        setPos(m_rect.x(), m_rect.y());
        m_color = QColor(0, 0, 255, 0xC0);
    }

    QRectF boundingRect() const override
    {
        return QRectF(0, 0, m_rect.width(), m_rect.height());
    }

    void paint(QPainter *f_painter, const QStyleOptionGraphicsItem *f_option, QWidget *f_widget) override
    {
        if (!(*m_gpio))
            return;
        f_painter->setBrush(m_color);
        f_painter->drawRect(0, 0, m_rect.width(), m_rect.height());
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* f_event) override
    {
        if (f_event->button() == Qt::LeftButton)
        {
            (*m_gpio)->setBit(translatePinToGpioOffs(m_pin), 0);  // Active low
            m_pressed = true;
        }
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* f_event) override
    {
        if (f_event->button() == Qt::LeftButton)
        {
            (*m_gpio)->setBit(translatePinToGpioOffs(m_pin), 1);  // Active low
            m_pressed = false;
        }
    }

    void advance(int step) override
    {
        if (! step)
            return;
        if (!m_gpio)
            return;

        if (m_pressed)
        {
            (*m_gpio)->setBit(translatePinToGpioOffs(m_pin), 0);  // Active low
        }


        if ((*m_gpio)->update())
        {
           update();
        }
    }

    qint8 getPin() const
    {
        return m_pin;
    }

    QSize getSize() const
    {
        return m_rect.size();
    }

private:
    QRect m_rect;
    quint8 m_pin;
    QColor m_color;

    GpioClient** m_gpio;

    QString m_name;
    bool m_pressed;

};

#endif // BUTTON_H
