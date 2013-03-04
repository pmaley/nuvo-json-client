#ifndef NUVOACTIONITEM_H
#define NUVOACTIONITEM_H

#include <QObject>

class NuvoActionItem : public QObject
{
    Q_OBJECT
public:
    explicit NuvoActionItem(QObject *parent = 0);
    explicit NuvoActionItem(QString _name, QString _url, bool _active = false);
signals:
    
public slots:

private:
    QString name;
    QString url;
    bool active;
};

#endif // NUVOACTIONITEM_H
