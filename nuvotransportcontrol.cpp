#include "nuvotransportcontrol.h"

NuvoTransportControl::NuvoTransportControl(QObject *parent) :
    QObject(parent)
{

}

NuvoTransportControl::NuvoTransportControl(QString _name, QString _url) : QObject()
{
    name  = _name;
    url = _url;

}
