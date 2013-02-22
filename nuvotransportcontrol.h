#ifndef NUVOTRANSPORTCONTROL_H
#define NUVOTRANSPORTCONTROL_H

#include <QObject>

class NuvoTransportControl : public QObject
{
    Q_OBJECT
public:
    explicit NuvoTransportControl(QObject *parent = 0);
    explicit NuvoTransportControl(QString _name, QString _url);

signals:
    
public slots:

private:
    QString name;
    QString url;
};

#endif // NUVOTRANSPORTCONTROL_H
