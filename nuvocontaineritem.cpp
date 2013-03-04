#include <QtWidgets>

#include "nuvocontaineritem.h"

NuvoContainerItem::NuvoContainerItem(QObject *parent) :
     QObject(parent)
{
}


NuvoContainerItem::NuvoContainerItem(QString _title, QString _url,
                           QString _icon, QString _type,
                           QString _id, QString _sortKey, QObject *parent) :
                             QObject(parent)
{
    qDebug() << _title << _url << _icon << _type << _id << _sortKey;
    title = _title;
    url = _url;
    icon = _icon;
    itemType = _type;
    name = _id;
    sortKey = _sortKey;
    qDebug() << title << url << icon << itemType << name << sortKey;

}
