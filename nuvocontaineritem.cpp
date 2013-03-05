#include <QtWidgets>

#include "nuvocontaineritem.h"

NuvoContainerItem::NuvoContainerItem(QObject *parent) :
    QObject(parent)
{
}


NuvoContainerItem::NuvoContainerItem(QString _title, QString _url,
                           QString _icon, QString _type,
                           QString _id, QString _sortKey, bool _av, int _index) :
                            QObject()
{
    qDebug() << _title << _url << _icon << _type << _id << _sortKey;
    title = _title;
    url = _url;
    icon = _icon;
    itemType = _type;
    name = _id;
    sortKey = _sortKey;
    av = _av;
    index = _index;
    qDebug() << title << url << icon << itemType << name << sortKey << index;

}
