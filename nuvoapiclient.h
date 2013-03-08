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

    int getAvValue(QString id);
    QList<QString> getBrowseItems();

    void connectToHost();
    void connectToHost(QString host, int port);

    int sendRequest(QString request);

    void invokeAction(NuvoActionItem *action);
    void invokeAction(QString url);
    void loadAv(NuvoContainerItem* item);
    void updateValue(NuvoActionItem *actionItem, int value);
    void toggleValue(NuvoActionItem *actionItem);



    void updateActionUrl(QString id, QString url, bool active);

    NuvoActionItem* findActionItem(QString id);

    NuvoActionItem *nextActionItem, *playActionItem, *pauseActionItem,
                            *prevActionItem, *stopActionItem, *likeActionItem,
                            *dislikeActionItem, *volumeActionItem, *muteActionItem,
                            *shuffleActionItem, *repeatActionItem;
    int volumeMax, volume, progressMax, progressPos;
    QString avState;
    QString metadata1, metadata2, metadata3;

    int currentBrowseRequestNum;
    
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
    void refreshDisplay();

public slots:
    void disconnectFromHost();
    void messageReceived();
    void tcpError(QAbstractSocket::SocketError socketError);
    void slot_netwManagerFinished(QNetworkReply *reply);
    void browseContainer(NuvoContainerItem *item);
    void browseContainer(QString url);
    void browseContainer(int index);
    void browseContainer();
    void browseUpOne();

private:
    int requestNum;
    QString currentMessage;
    NuvoContainerItem* musicContainer;

    QString avChannel, currentBrowseChannel;

    QMap<QString, QJsonObject> channels;
    QStack<QString> browseChannelStack;

    void unsubscribe(QString channel);

    void updateDisplay(QString channel, int index);

    void parseJsonResponse(QString json);
    void parseReplyMessage(QJsonObject obj);
    void parseEventMessage(QJsonObject obj);

    void parseTrackMetadata();

    void parseChildValueChangedMessage(QString channel, QJsonObject value);
    void parseChildItemChangedMessage(QString channel, QJsonObject value);
    void parseChildRemovedMessage(QString channel, QJsonObject value);
    void parseChildInsertedMessage(QString channel, QJsonObject value);

    
};

#endif // NUVOAPICLIENT_H
