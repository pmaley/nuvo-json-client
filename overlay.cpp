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
    painter.setOpacity(0.7);

    QBrush brush;
    brush.setColor(Qt::blue);
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);

    QRect r(this->rect());
    painter.fillRect(r,brush);



    if (!this->layout()){
        QLabel *waiting = new QLabel(tr("<font color=\"White\"><b>%1</b></font>").arg("Connecting to system..."));
        QGridLayout *l = new QGridLayout();
        l->addWidget(waiting,0,0,Qt::AlignCenter);
        this->setLayout(l);
    }

}
