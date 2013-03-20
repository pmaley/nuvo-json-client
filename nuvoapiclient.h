#ifndef NUVOAPICLIENT_H
#define NUVOAPICLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QtNetwork>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QtWidgets>

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

    explicit NuvoApiClient(QObject *parent = 0);

    int getAvValue(QString id);
    QString getBrowseHeader();
    QList<QString> getChildTitles(QString channel);
    QList<QString> getBrowseItems();
    QList<QString> getZonesList();
    QList<QString> getNowPlayingContextItems();
    bool getMuteState();
    bool getShuffleState();
    bool getRepeatState();
    QString getStatusString();

    void connectToHost();
    void connectToHost(QString host, int port);

    int sendRequest(QString request);

    void invokeAction(QJsonObject obj);
    void invokeAction(QString id);

    void loadAv(int index);
    void loadAv2(int index);

    void toggleValue(QString id);
    void updateVolume(QString id, int value);

    void browseClick(int index);

    bool getItemActive(QString id);

    int volumeMax, progressMax, progressPos;
    QString avState;
    QString metadata1, metadata2, metadata3;
    
signals:
    void avChanged();
    void avStateChanged();
    void raiseError(const QString &err);
    void albumArtChanged();
    void albumArtCleared();
    void progressBarChanged();
    void displayUnformattedText(const QString &err);
    void displayFormattedText(const QString &err);
    void transportChanged();
    void metadataChanged();
    void browseDataChanged();
    void refreshDisplay();
    void zoneListChanged();
    void browseListCleared();

public slots:
    void disconnectFromHost();
    void messageReceived();
    void tcpError(QAbstractSocket::SocketError socketError);
    void socketConnected(QAbstractSocket::SocketState);
    void slot_netwManagerFinished(QNetworkReply *reply);
    int browseContainer(QString url, int startIndex = 0);
    int browseContainer(int index);
    int continueBrowseContainer(QString channel, int startIndex = 0);
    void browseUpOne();
    void changeCurrentZone(QString zoneId);

private:
    int requestNum;
    QString currentMessage;
    int currentBrowseRequestNum, currentAvRequestNum, currentZonesRequestNum, currentNowPlayingContextMenuRequestNum;
    QString avChannel, currentBrowseChannel, zonesChannel, currentReBrowseChannel, currentNowPlayingContextMenuChannel;

    QMap<QString, QJsonObject> channels;
    QStack<QString> browseChannelStack;

    void subscribeToChannelList();
    void unsubscribe(QString channel);
    void channelClosed(QString channel);
    int sendKeepAlive(QString channel);

    void updateDisplay(QString channel, int index);

    void parseJsonMessage(QString json);
    void parseReplyMessage(QJsonObject obj);
    void parseEventMessage(QJsonObject obj);

    void parseTrackMetadata();

    void parseChildValueChangedMessage(QString channel, QJsonObject value);
    void parseChildItemChangedMessage(QString channel, QJsonObject value);
    void parseChildRemovedMessage(QString channel, QJsonObject value);
    void parseChildInsertedMessage(QString channel, QJsonObject value);

    QJsonObject findZone(QString zoneTitle);
    QJsonObject findAvItem(QString id);

    
};

#endif // NUVOAPICLIENT_H
