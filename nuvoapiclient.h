#ifndef NUVOAPICLIENT_H
#define NUVOAPICLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QtNetwork>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QtWidgets>


#include <nuvoactionitem.h>
#include <nuvocontaineritem.h>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

class NuvoApiClient : public QObject
{
    Q_OBJECT
public:
    QTcpSocket *tcpSocket;
    QNetworkSession *networkSession;
    QNetworkAccessManager *m_netwManager;
    QPixmap albumArt;
    QList<NuvoContainerItem*> browseList;

    explicit NuvoApiClient(QObject *parent = 0);

    void connectToHost();
    void sendRequest(QString request);
    void invokeAction(NuvoActionItem *action);
    void invokeAction(QString url);
    void loadAv(NuvoContainerItem* item);
    void updateValue(NuvoActionItem *actionItem, int value);
    void toggleValue(NuvoActionItem *actionItem);
    void connectToHost(QString host, int port);
    void parseJsonResponse(QString json);
    void parseReplyMessage(QString channel, QJsonValue value);
    void parseEventMessage(QString channel, QJsonValue value);
    void parseTrackMetadata(QJsonObject obj);
    void parseActionItem(QJsonObject value);
    void parseValueItem(QJsonObject value);
    void parseContainerItem(QJsonObject parent, QJsonObject value);

    void parseChildValueChangedMessage(QString channel, QJsonObject value);
    void parseChildItemChangedMessage(QString channel, QJsonObject value);
    void parseChildInsertedMessage(QJsonObject value);
    void parseChildRemovedMessage(QString channel, QJsonObject value);
    NuvoActionItem* findActionItem(QString id);

    NuvoActionItem *nextActionItem, *playActionItem, *pauseActionItem,
                            *prevActionItem, *stopActionItem, *likeActionItem,
                            *dislikeActionItem, *volumeActionItem, *muteActionItem,
                            *shuffleActionItem, *repeatActionItem;
    int volumeMax, volume, progressMax, progressPos;
    QString avState;
    QString metadata1, metadata2, metadata3;
    void updateValue(QString id);
    
signals:
    void avChanged();
    void avStateChanged();
    void raiseError(const QString &err);
    void albumArtChanged();
    void progressBarChanged();
    void displayText(const QString &err);
    void transportChanged();
    void volumeChanged();
    void metadataChanged();
    void browseDataChanged();

public slots:
    void disconnectFromHost();
    void messageReceived();
    void tcpError(QAbstractSocket::SocketError socketError);
    void slot_netwManagerFinished(QNetworkReply *reply);
    void browseContainer(NuvoContainerItem *item);
    void browseContainer(QString url);
    void browseContainer();

private:
    QString currentMessage;
    NuvoContainerItem* musicContainer;
    int requestNum = 0;
    QString avChannel;
    QJsonObject avObject;
    QMap<QString, QJsonObject> channels;
    
};

#endif // NUVOAPICLIENT_H
