#ifndef NUVOCONTAINERITEM_H
#define NUVOCONTAINERITEM_H

#include <QObject>

class NuvoContainerItem : public QObject
{
    Q_OBJECT
public:
    explicit NuvoContainerItem(QObject *parent = 0);
    NuvoContainerItem(NuvoContainerItem *item);
    explicit NuvoContainerItem(QString _title, QString _url,
                               QString _icon, QString _type,
                               QString _id, QString _sortKey);
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
