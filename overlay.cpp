#include <QPainter>
#include <QtWidgets>
#include <QPen>
#include <QPalette>
#include <QLabel>
#include "overlay.h"

Overlay::Overlay(QWidget *parent)
    : QWidget(parent)
{
    m_mouseClick = false;
    text = "";
}

void Overlay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(0.7);

    QBrush brush;
    brush.setColor(Qt::blue);
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);

    QRect r(this->rect());
    painter.fillRect(r,brush);



    if (!this->layout()){
        QLabel *waiting = new QLabel(tr("<font color=\"White\"><b>%1</b></font>").arg(text));
        waiting->setWordWrap(true);
        QGridLayout *l = new QGridLayout();
        l->addWidget(waiting,0,0,Qt::AlignCenter);
        this->setLayout(l);
    }

    event->accept();

}

void Overlay::setText(QString txt)
{
    text = txt;
}

void Overlay::mousePressEvent ( QMouseEvent * e )
{
    // store click position
    m_lastPoint = e->pos();
    // set the flag meaning "click begin"
    m_mouseClick = true;
}

void Overlay::mouseReleaseEvent ( QMouseEvent * e )
{
    // check if cursor not moved since click beginning
    if ((m_mouseClick) && (e->pos() == m_lastPoint))
    {
        emit mouseClickEvent();
    }
}
