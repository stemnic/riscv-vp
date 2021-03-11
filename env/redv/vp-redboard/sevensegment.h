#ifndef SEVENSEGMENT_H
#define SEVENSEGMENT_H

#include "gpioutils.h"

#include <QGraphicsItem>
#include <QPainter>

using namespace gpioutil;

class Sevensegment : public QGraphicsItem
{
public:
    Sevensegment(QPoint f_point, QSize f_size, quint8 f_linewidth, GpioClient** f_gpio)
        : m_size(f_size)
        , m_linewidth(f_linewidth)
        , m_gpio(f_gpio)
    {
        setPos(f_point);
    }

    QRectF boundingRect() const override
    {
        return QRectF(-m_linewidth, -m_linewidth, m_size.width()+2*m_linewidth, m_size.height()+2*m_linewidth);
    }

    void paint(QPainter *f_painter, const QStyleOptionGraphicsItem *f_option, QWidget *f_widget) override
    {
        if (!(*m_gpio))
            return;

        uint8_t map = translatePinNumberToRGBLed(translateGpioToExtPin((*m_gpio)->state));

        QPen segment(QColor("#f72727"), m_linewidth, Qt::PenStyle::SolidLine, Qt::PenCapStyle::RoundCap, Qt::RoundJoin);
        f_painter->setPen(segment);

        //  0
        // 5   1
        //  6
        // 4   2
        //  3   7

        int xcol1 = 0;
        int xcol2 = 3 * (m_size.width() / 4);
        int yrow1 = 0;
        int xrow1 = m_size.width()  / 4 - 2;
        int yrow2 = m_size.height() / 2;
        int xrow2 = m_size.width()  / 8 - 1;
        int yrow3 = m_size.height();
        int xrow3 = 0;

        if (map & 0b00000001)  // 0
            f_painter->drawLine(QPoint(xcol1 + xrow1, yrow1), QPoint(xcol2 + xrow1, yrow1));
        if (map & 0b00000010)  // 1
            f_painter->drawLine( QPoint(xcol2 + xrow1, yrow1),  QPoint(xcol2 + xrow2, yrow2));
        if (map & 0b00000100)  // 2
            f_painter->drawLine(QPoint(xcol2 + xrow2, yrow2), QPoint(xcol2 + xrow3, yrow3));
        if (map & 0b00001000)  // 3
            f_painter->drawLine(QPoint(xcol2 + xrow3, yrow3), QPoint(xcol1 + xrow3, yrow3));
        if (map & 0b00010000)  // 4
            f_painter->drawLine(QPoint(xcol1 + xrow3, yrow3), QPoint(xcol1 + xrow2, yrow2));
        if (map & 0b00100000)  // 5
            f_painter->drawLine(QPoint(xcol1 + xrow2, yrow2), QPoint(xcol1 + xrow1, yrow1));
        if (map & 0b01000000)  // 6
            f_painter->drawLine(QPoint(xcol1 + xrow2, yrow2), QPoint(xcol2 + xrow2, yrow2));
        if (map & 0b10000000)  // 7
            f_painter->drawPoint(m_size.width()-1, m_size.height()-1);

    }

    void advance(int step) override
    {
        if (! step)
            return;
        if (!(*m_gpio))
            return;

        if ((*m_gpio)->update())
        {
           update();
        }
    }

    QSize getSize() const
    {
        return m_size;
    }


private:
    QSize m_size;
    QColor m_color;
    quint8 m_linewidth;

    GpioClient** m_gpio;
};

#endif // SEVENSEGMENT_H
