#include <QPainter>
#include <QPen>
#include <QPalette>
#include "overlay.h"

Overlay::Overlay(QWidget *parent)
    : QWidget(parent)
{

}

void Overlay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect r(this->rect());
    painter.fillRect(r,Qt::Dense5Pattern);

}
