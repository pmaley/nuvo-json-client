#ifndef NUVOCONTAINERITEM_H
#define NUVOCONTAINERITEM_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptValueIterator>

class NuvoContainerItem : public QObject
{
    Q_OBJECT
public:
    explicit NuvoContainerItem(QObject *parent = 0);
    NuvoContainerItem(NuvoContainerItem *item);
    explicit NuvoContainerItem(QString _title, QString _url,
                               QString _icon, QString _type,
                               QString _id, QString _sortKey, bool _av, int _index);
    explicit NuvoContainerItem(QScriptValue _parent, QScriptValue item);
    QString title;
    QString url;
    QString icon;
    QString itemType;
    QString name;
    QString sortKey;
    QScriptValue parent;
    QScriptValue myItem;
    bool av;
    int index;
    
signals:
    
public slots:

private:


    
};

#endif // NUVOCONTAINERITEM_H
