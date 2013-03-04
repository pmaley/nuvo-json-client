#include "nuvoactionitem.h"

NuvoActionItem::NuvoActionItem(QObject *parent) :
    QObject(parent)
{

}

NuvoActionItem::NuvoActionItem(QString _name, QString _url, bool _active) : QObject()
{
    name  = _name;
    url = _url;
    active = _active;
}


