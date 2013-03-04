#ifndef NUVOCONTAINERITEM_H
#define NUVOCONTAINERITEM_H

#include <QObject>
#include <QAbstractItemModel>

class NuvoContainerItem : public QObject
{
    Q_OBJECT
public:
    explicit NuvoContainerItem(QObject *parent = 0);
    NuvoContainerItem(QString _title, QString _url,
                               QString _icon, QString _type,
                               QString _id, QString _sortKey,
                                QObject *parent = 0);
    QString title;
    QString url;
    QString icon;
    QString itemType;
    QString name;
    QString sortKey;
    
signals:
    
public slots:

private:


    
};

#endif // NUVOCONTAINERITEM_H
