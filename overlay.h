#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>

class Overlay : public QWidget
{
    Q_OBJECT

public:
    Overlay(QWidget *parent);
    void setText(QString txt);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent ( QMouseEvent * e );
    void mousePressEvent ( QMouseEvent * e );

private:
    QString text;
    QPoint m_lastPoint;
    bool m_mouseClick;

signals:
    void mouseClickEvent();
};

#endif // OVERLAY_H
