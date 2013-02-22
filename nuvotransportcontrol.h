#ifndef NUVOTRANSPORTCONTROL_H
#define NUVOTRANSPORTCONTROL_H

#include <QObject>

class NuvoTransportControl : public QObject
{
    Q_OBJECT
public:
    explicit NuvoTransportControl(QObject *parent = 0);
    explicit NuvoTransportControl(QString _name, QString _url, bool _active = false);

signals:
    
public slots:

private:
    QString name;
    QString url;
    bool active;
};

#endif // NUVOTRANSPORTCONTROL_H
