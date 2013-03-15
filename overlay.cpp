#include <QPainter>
#include <QPen>
#include <QPalette>
#include "overlay.h"

Overlay::Overlay(QWidget *parent)
    : QWidget(parent)
{
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::blue);
    setPalette(pal);
}

void Overlay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect r(this->rect());
    painter.fillRect(r,Qt::Dense5Pattern);

}
