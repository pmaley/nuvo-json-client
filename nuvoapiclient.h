#ifndef NUVOAPICLIENT_H
#define NUVOAPICLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QtNetwork>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QtWidgets>


#include <nuvoactionitem.h>

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
    QString errorMessage;

    explicit NuvoApiClient(QObject *parent = 0);

    void connectToHost();
    void sendRequest(QString request);
    void invokeAction(NuvoActionItem *action);
    void updateValue(NuvoActionItem *actionItem, int value);
    void toggleValue(NuvoActionItem *actionItem);
    void connectToHost(QString host, int port);
    void parseJsonResponse(QString json);
    void parseReplyMessage(QScriptValue sc);
    void parseEventMessage(QScriptValue sc);
    void parseTrackMetadata(QScriptValue value);
    void parseActionItem(QScriptValue value);
    void parseValueItem(QScriptValue value);
    void parseContainerItem(QScriptValue value);
    void parseChildValueChangedMessage(QScriptValue value);
    void parseChildItemChangedMessage(QScriptValue value);
    void parseChildInsertedMessage(QScriptValue value);
    void parseChildRemovedMessage(QScriptValue value);
    NuvoActionItem* findActionItem(QString id);

    NuvoActionItem *nextActionItem, *playActionItem, *pauseActionItem,
                            *prevActionItem, *stopActionItem, *likeActionItem,
                            *dislikeActionItem, *volumeActionItem, *muteActionItem,
                            *shuffleActionItem, *repeatActionItem;
    int volumeMax, volume, progressMax, progressPos;
    QString avState;
    QString metadata1, metadata2, metadata3;
    
signals:
    void avChanged();
    void avStateChanged();
    void raiseError();
    
public slots:
    void disconnectFromHost();
    void messageReceived();
    void tcpError(QAbstractSocket::SocketError socketError);
    void slot_netwManagerFinished(QNetworkReply *reply);



private:
    QString currentMessage;




    
};

#endif // NUVOAPICLIENT_H
