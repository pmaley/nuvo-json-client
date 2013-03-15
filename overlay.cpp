#include <QPainter>
#include <QtWidgets>
#include <QPen>
#include <QPalette>
#include <QLabel>
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

    QLabel *waiting = new QLabel("Connecting to system...");

    QGridLayout *l = new QGridLayout();

    l->addWidget(waiting,0,0,Qt::AlignCenter);

    this->setLayout(l);

}
