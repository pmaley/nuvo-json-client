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


NuvoContainerItem::NuvoContainerItem(QScriptValue _parent, QScriptValue item) :
                            QObject()
{
     parent = QScriptValue(_parent);
     myItem = QScriptValue(item);
     QScriptValue current(item);
     title = QString(current.property("title").toString());
     url = QString(current.property("url").toString());
     icon = QString(current.property("icon").toString());
     itemType = QString(current.property("type").toString());
     name = QString(current.property("id").toString());
     sortKey = QString(current.property("sortKey").toString());
     av = current.property("av").toBool();
     index = QString(current.property("index").toString()).toInt();
     qDebug() << title << name << itemType << index;
}
