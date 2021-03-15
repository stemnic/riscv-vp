#ifndef OLED_H
#define OLED_H

#include "oled/common.hpp"

#include <QGraphicsItem>
#include <QPainter>

class OLED : public QGraphicsItem
{
public:
    OLED(QPoint f_point, qint16 f_margin, double f_scale = 1.0)
        : m_margin(f_margin, f_margin)
        , m_scale(f_scale)
        , m_image(ss1106::width - 2*ss1106::padding_lr, ss1106::height, QImage::Format_Grayscale8)
    {
        setPos(f_point);
        m_state = reinterpret_cast<ss1106::State*>(ss1106::getSharedState());
        m_state->changed = 1;
    }

    qint16 getMargin() const
    {
        return m_margin.x();
    }

    double getScale() const
    {
        return m_scale;
    }

    QRectF boundingRect() const override
    {
        return QRectF(0, 0, (ss1106::width - 2 * ss1106::padding_lr) * m_scale + m_margin.x()*2,
                            ss1106::height * m_scale + m_margin.y()*2);
    }

    void paint(QPainter *f_painter, const QStyleOptionGraphicsItem *f_option, QWidget *f_widget) override
    {
        if (!m_state)
            return;

        f_painter->fillRect(
                    QRect(0, 0, (ss1106::width - 2 * ss1106::padding_lr) * m_scale + m_margin.x()*2,
                                ss1106::height * m_scale + m_margin.y()*2),
                    Qt::SolidPattern);

        if(m_state->display_on && m_state->changed)
        {
            m_state->changed = 0;	//We ignore this small race-condition
            uchar* map = m_image.bits();
            for(unsigned page = 0; page < ss1106::height/8; page++)
            {
                for(unsigned column = 0; column < (ss1106::width - 2 * ss1106::padding_lr); column++)
                {
                    for(unsigned page_pixel = 0; page_pixel < 8; page_pixel++)
                    {
                        map[((page * 8 + page_pixel) * (ss1106::width - 2 * ss1106::padding_lr)) + column] = m_state->frame[page][column+ss1106::padding_lr] & (1 << page_pixel) ? m_state->contrast : 0;
                    }
                }
            }


        }
        if(m_state->display_on)
        {
            f_painter->drawImage(m_margin, m_image.scaled((ss1106::width - 2 * ss1106::padding_lr) * m_scale, ss1106::height * m_scale));
        }
}

    void advance(int step) override
    {
        if (! step)
            return;
        if (!m_state)
            return;

        update();
    }

private:
    QPoint m_margin;
    double m_scale;
    QImage m_image;

    ss1106::State* m_state;
};
#endif // OLED_H
