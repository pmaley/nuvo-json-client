#include <QtWidgets>

#include "nuvoapiclient.h"

NuvoApiClient::NuvoApiClient(QObject *parent) : QObject(parent)
{
    avState = "";
    volumeMax = 100;
    progressPos = progressMax = 0;
    requestNum = 0;
    avChannel = zonesChannel = currentBrowseChannel = "";
    currentBrowseRequestNum = -1;
    m_netwManager = new QNetworkAccessManager(this);
    tcpSocket = new QTcpSocket(this);

    connect(m_netwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished(QNetworkReply*)));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(messageReceived()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketConnected(QAbstractSocket::SocketState)));
}

void NuvoApiClient::changeCurrentZone(QString zoneTitle)
{
    qDebug() << "CHANGE CURRENT ZONE";
    QJsonObject zone = findZone(zoneTitle);
    QString url(zone.value("url").toString());
    currentAvRequestNum = browseContainer(url);
    if (!avChannel.isEmpty() && !channels[avChannel].isEmpty())
        unsubscribe(avChannel);
}

QJsonObject NuvoApiClient::findZone(QString zoneTitle)
{
    QJsonArray children = channels[zonesChannel].value("children").toArray();
    for (int i = 0; i < children.size(); i++){
        if (QString(children.at(i).toObject().value("title").toString()) == QString(zoneTitle)){
            return children.at(i).toObject();
        }
    }
    return QJsonObject(QJsonValue(QString("error")).toObject());
}

void NuvoApiClient::setChildren(QString channel, QJsonArray children)
{
    QJsonObject::Iterator iterator;
    iterator = channels[channel].find("children");
    iterator.value() = children;
}

QString NuvoApiClient::getStatusString()
{
    QJsonObject obj = findAvItem("status");
    return obj.value("value").toObject().value("string").toString();
}

bool NuvoApiClient::getMuteState(){
    QJsonObject obj = findAvItem("volume");
    return obj.value("value").toObject().value("volume").toObject().keys().contains("mute");
}

bool NuvoApiClient::getShuffleState(){
    QJsonObject obj = findAvItem("shuffle");
    return obj.value("value").toObject().value("bool").toBool();
}

bool NuvoApiClient::getRepeatState(){
    QJsonObject obj = findAvItem("repeatMode");
    return QString(obj.value("value").toObject().value("avRepeatMode").toString()) != QString("normal");
}

QJsonObject NuvoApiClient::findAvItem(QString id)
{
    QJsonArray children = channels[avChannel].value("children").toArray();
    for (int i = 0; i < children.size(); i++){
        if (QString(children.at(i).toObject().value("id").toString()) == QString(id)){
            return children.at(i).toObject();
        }
    }
    return QJsonObject(QJsonValue(QString("error")).toObject());
}

bool NuvoApiClient::getItemActive(QString id)
{
    QJsonArray children = channels[avChannel].value("children").toArray();
    for (int i = 0; i < children.size(); i++){
        if (QString(children.at(i).toObject().value("id").toString()) == QString(id)){
            return true;
        }
    }
    return false;
}

void NuvoApiClient::socketConnected(QAbstractSocket::SocketState state)
{
    if (state == QAbstractSocket::ConnectedState){
        subscribeToChannelList();
    }
}

int NuvoApiClient::getAvValue(QString id){
    QJsonArray children = channels[avChannel].value("children").toArray();
    for (int i = 0; i < children.size(); i++){
        if (QString(children.at(i).toObject().value("id").toString()) == QString(id)){
            return (int) children.at(i).toObject().value("value").toObject().value("volume").toObject().value("level").toDouble();
        }
    }
    return -1;
}


QList<QString> NuvoApiClient::getNowPlayingContextItems()
{
    return getChildTitles(currentNowPlayingContextMenuChannel);
}

QString NuvoApiClient::getBrowseHeader()
{
    return channels[currentBrowseChannel].value("item").toObject().value("title").toString();
}

QList<QString> NuvoApiClient::getZonesList()
{
    return getChildTitles(zonesChannel);
}

QList<QString> NuvoApiClient::getBrowseItems()
{
    return getChildTitles(currentBrowseChannel);
}

QList<QString> NuvoApiClient::getChildTitles(QString channel)
{
    QList<QString> list;
    QJsonArray children = channels[channel].value("children").toArray();
    for (int i = 0; i < children.size(); i++){
        list.append(QString(children.at(i).toObject().value("title").toString()));
    }
    return list;
}

void NuvoApiClient::browseUpOne()
{
    if (!browseChannelStack.empty()){
        unsubscribe(currentBrowseChannel);
        currentBrowseChannel = browseChannelStack.pop();
        emit browseDataChanged();
    }
}

void NuvoApiClient::unsubscribe(QString channel)
{
    QString channelString( tr("{ \"channels\" : [\"%1\"] }").arg(channel) );
    QString closeRequest( tr("{ \"method\" : \"close\", \"params\" : %2 }\n").arg(channelString) );
    channelsToClose.append(channel);
    sendRequest(closeRequest);
}

void NuvoApiClient::unsafe_unsubscribe(QString channel)
{
    QString channelString( tr("{ \"channels\" : [\"%1\"] }").arg(channel) );
    QString closeRequest( tr("{ \"method\" : \"close\", \"params\" : %2 }\n").arg(channelString) );
    sendRequest(closeRequest);
}


void NuvoApiClient::subscribeToChannelList()
{
    currentAvRequestNum = browseContainer("/stable/av/");
    currentBrowseRequestNum = browseContainer("/stable/music/");
    currentZonesRequestNum = browseContainer("/stable/avs/");
}

int NuvoApiClient::browseContainer(int index)
{
    QString url(channels[currentBrowseChannel].value("children").toArray().at(index).toObject().value("url").toString());
    currentBrowseRequestNum = browseContainer(url);
    return currentBrowseRequestNum;
}

int NuvoApiClient::browseContainer(QString url, int startIndex){
    QString count(tr("{ \"from\": %1, \"count\" : -1 }").arg(startIndex));
    QString request(tr( "{ \"url\" : \"%2\", \"method\" : \"browse\", \"params\" : %3 }\n").arg(url,count));
    return sendRequest(request);
}

int NuvoApiClient::continueBrowseContainer(QString channel, int startIndex){
    QString count(tr("{ \"from\": %1, \"count\" : -1 }").arg(startIndex));
    QString request(tr( "{ \"channel\" : \"%2\", \"method\" : \"browse\", \"params\" : %3 }\n").arg(channel,count));
    return sendRequest(request);
}

int NuvoApiClient::sendKeepAlive(QString channel)
{
    QString channels(tr("{ \"channels\" : [\"%1\"] }").arg(channel));
    QString request( tr("{ \"method\" : \"keepAlive\", \"params\" : %2 }\n").arg(channels) );
    return sendRequest(request);
}

int NuvoApiClient::sendRequest(QString request)
{
    QByteArray utf8;
    utf8.append(request);
    QJsonObject obj = QJsonDocument::fromJson(utf8).object();
    obj.insert("id",QString(tr("%1").arg(requestNum)));
    QByteArray byteArray = QString(QJsonDocument(obj).toJson()).toUtf8();
    const char* cString = byteArray.constData();
    tcpSocket->write(cString);

    emit displayFormattedText(QString(tr("<font color=\"Blue\">%1</font><br>").arg(cString)));
    qDebug() << cString << "written to socket. Request #" << requestNum;
    requestNum++;
    return requestNum-1;
}

void NuvoApiClient::connectToHost(QString host, int port)
{
    if (!tcpSocket->isOpen())
    {
        tcpSocket->abort();
        tcpSocket->connectToHost(host, port);
    }
}

void NuvoApiClient::disconnectFromHost()
{
    tcpSocket->abort();
}

void NuvoApiClient::messageReceived()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    int blockSize = tcpSocket->bytesAvailable()/sizeof(char);
    char * data = new char[blockSize+1];
    in.readRawData(data,blockSize);
    data[blockSize] = '\0';

    currentMessage.append(QString(data));
    if (currentMessage.contains('\n')){
        QStringList query = currentMessage.split(QRegExp("\n"));
        for (int i = 0; i < query.length()-1; i++){
            parseJsonMessage(QString(query.at(i)));
        }
        currentMessage = QString(query.last());
    }
    delete(data);
}

void NuvoApiClient::parseJsonMessage(QString result)
{
    qDebug() << "ENTERING" << __func__;
    QByteArray utf8;
    utf8.append(result);
    QJsonObject j = QJsonDocument::fromJson(utf8).object();

    if (j.keys().contains("error"))
        emit displayFormattedText(QString(tr("<font color=\"Red\">%1</font><br>").arg(QString(QJsonDocument(j).toJson()))));
    else
        emit displayUnformattedText(QString(tr("%1\n").arg(QString(QJsonDocument(j).toJson()))));

    QString type(j.value("type").toString());
    QString channel(j.value("channel").toString());

    if (type == "reply" && !channel.isEmpty() ) { parseReplyMessage(j); }
    else if ( type == "event"){  parseEventMessage(j); }
    else if ( type == "closed"){ channelClosed(channel); }
    else if ( type == "removed"){ channelRemoved(channel); }

    if (channel == avChannel)
    {
        emit refreshDisplay();
    }
    else if (channel == currentBrowseChannel)
    {
        emit browseDataChanged();
    }
    else if (channel == zonesChannel)
    {
        emit zoneListChanged();
    }
    else if (channel == currentNowPlayingContextMenuChannel)
    {
        emit contextMenuLoaded();
    }

    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::replaceChannel(int reqId, QString newChannel)
{
    qDebug() << "ENTERING" << __func__;
    QString oldChannel(channelsToReopen[reqId]);

    qDebug() << currentBrowseChannel << avChannel << zonesChannel << currentNowPlayingContextMenuChannel;
    qDebug() << newChannel << oldChannel;

    if (browseChannelStack.contains(oldChannel)){
        int index = browseChannelStack.indexOf(oldChannel);
        browseChannelStack.replace(index,newChannel);
    }
    if (currentBrowseChannel == oldChannel)
        currentBrowseChannel = newChannel;
    if (avChannel == oldChannel)
        avChannel = newChannel;
    if (zonesChannel == oldChannel)
        zonesChannel = newChannel;
    if (currentNowPlayingContextMenuChannel == oldChannel)
        currentNowPlayingContextMenuChannel = newChannel;

    channelsToReopen.remove(reqId);
    channels.remove(oldChannel);
    qDebug() << "CHANNELS:" << channels.keys();
    printChannels();
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::channelClosed(QString channel)
{

    qDebug() << "CHANNEL CLOSED" << channel;
    qDebug() << "CHANNELS TO CLOSE:" << channelsToClose;
    qDebug() << "CHANNELS BEFORE:" << channels.keys();

    if ( channelsToClose.contains(channel) )
    {
        channelsToClose.removeAll(channel);
        channels.remove(channel);
    }
    else if ( channels.contains(channel) && !channels[channel].isEmpty())
    {
        QString url = channels[channel].value("item").toObject().value("url").toString();
        qDebug() << "Re-opening channel" << channel << "at:" << url;
        int reqId = browseContainer(url);

        channelsToReopen[reqId] = QString(channel);
    }

    qDebug() << "CHANNELS AFTER:" << channels.keys();
    printChannels();
}

void NuvoApiClient::printChannels()
{

    qDebug() << QTime::currentTime() << QTime::currentTime().msec();
    QMapIterator<QString, QJsonObject> i(channels);
     while (i.hasNext()) {
         i.next();
         if (!i.value().isEmpty()){
            qDebug() << i.key() << ": " << i.value().value("item").toObject().value("url").toString();
         }
     }

}

void NuvoApiClient::channelRemoved(QString channel)
{
    qDebug() << "CHANNEL REMOVED" << channel;
    channels.remove(channel);
    qDebug() << "CHANNELS:" << channels.keys();
    printChannels();
}

void NuvoApiClient::parseReplyMessage(QJsonObject obj)
{
    qDebug() << "ENTERING" << __func__;
    qDebug() << "ID: " << obj.value("id").toString();
    printChannels();

    int id = obj.value("id").toString().toInt();
    QString channel = obj.value("channel").toString();
    int count = (int)(obj.value("result").toObject().value("count").toDouble());
    int length = obj.value("result").toObject().value("children").toArray().size();


    // Handle incremental browsing
    // REFACTOR

    // If the channel exists, and the browse result is complete,
    // then set the container in the hash
    if (count == length)
    {
        qDebug() << "SETTING CHANNEL" << channel;
        channels[channel] = QJsonObject(obj.value("result").toObject());
    }
    // If channel exists, browse result is incomplete, and
    // we aren't already mid-incremental browse (first response),
    // then set the container in the hash, and continue browsing container
    else if (count != length && channel != currentReBrowseChannel)
    {
        qDebug() << "SETTING CHANNEL" << channel;
        channels[channel] = QJsonObject(obj.value("result").toObject());
        currentReBrowseChannel = channel;
        int objLen = channels[channel].value("children").toArray().size();
        continueBrowseContainer(channel,objLen);
    }
    // If channel exists, browse result is incomplete,
    // and we are incrementally browsing,
    // then append children to browse container,
    // and either continue browsing, or clear browse channel variable
    else if (count != length && channel == currentReBrowseChannel)
    {
        QJsonArray insertChildren(obj.value("result").toObject().value("children").toArray());
        QJsonArray children(channels[channel].value("children").toArray());
        for (int i = 0; i < insertChildren.size(); i++){
            children.append(insertChildren.at(i).toObject());
        }
        setChildren(channel,children);

        int objLen = channels[channel].value("children").toArray().size();
        if (objLen == count){
            currentReBrowseChannel = "";
        }
        else
        {
            continueBrowseContainer(channel,objLen);
        }
    }


    // Keep channel alive if finished browsing it
//    if (currentReBrowseChannel.isEmpty())
//        sendKeepAlive(channel);

    // Match requests and their channel with stored values
    if ( id == currentAvRequestNum )
    {
        avChannel = channel;
        parseTrackMetadata();
    }
    else if ( id == currentZonesRequestNum )
    {
        zonesChannel = channel;
    }
    else if ( id == currentNowPlayingContextMenuRequestNum )
    {
        currentNowPlayingContextMenuChannel = channel;
    }
    else if ( id == currentBrowseRequestNum ){
        if (!currentBrowseChannel.isEmpty()){
            browseChannelStack.push(currentBrowseChannel);
        }
        currentBrowseChannel = channel;
    }

    if (channelsToReopen.contains(id))
        replaceChannel(id,channel);

    // Inspect individual elements, process accordingly
    // REFACTOR
    QJsonArray it(channels[channel].value("children").toArray());
    for (int i = 0; i < it.size(); i++ ){;
        QJsonObject current(it.at(i).toObject());
        QString id(current.value("id").toString());
        QString type(current.value("type").toString());

        if (id == "info"){
            parseTrackMetadata();
        } else if ( type == "value"){
            updateDisplay(channel,i);
        }
    }

    qDebug() << "CHANNELS:" << channels.keys();
    printChannels();
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::browseNowPlayingContextMenu()
{
    QJsonObject obj = findAvItem("context");
    QString contextUrl(obj.value("url").toString());
    if (!contextUrl.isEmpty()) {
        currentNowPlayingContextMenuRequestNum = browseContainer(contextUrl);
    }
}
void NuvoApiClient::closeNowPlayingContextMenu()
{
    if (!currentNowPlayingContextMenuChannel.isEmpty()){
        unsubscribe(currentNowPlayingContextMenuChannel);
    }
}

void NuvoApiClient::parseEventMessage(QJsonObject obj)
{
    qDebug() << "ENTERING" << __func__;
    QString channel = obj.value("channel").toString();
    QJsonArray array = obj.value("event").toArray();
    for (int i = 0; i < array.size(); i++) {
        QJsonObject currentObj = array.at(i).toObject();
        QString type = QString(currentObj.value("type").toString());
        if (type == "childValueChanged"){ parseChildValueChangedMessage(channel,currentObj); }
        else if (type == "childItemChanged"){ parseChildItemChangedMessage(channel,currentObj); }
        else if (type == "childInserted"){ parseChildInsertedMessage(channel,currentObj); }
        else if (type == "childRemoved"){ parseChildRemovedMessage(channel,currentObj); }
        else { qDebug() << "PARSE_EVENT_MESSAGE: EVENT NOT PROCESSED:" << type; }
    }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::updateVolume(QString id, int value){
    QJsonObject obj = findAvItem(id);
    QString url(obj.value("url").toString());
    QString params( tr("{ \"value\" : { \"int\" : %1 } }").arg(value));
    QString request(tr(" { \"url\" : \"%2\", \"method\" : \"updateValue\", \"params\" : %3 }").arg(url,params));
    sendRequest(request);
}

void NuvoApiClient::toggleValue(QString id)
{
    QJsonObject obj = findAvItem(id);
    QString url(obj.value("url").toString());
    QString params(tr("{ \"operation\" : \"toggle\" }"));
    QString request(tr("{\"url\" : \"%2\", \"method\" : \"updateValue\", \"params\" : %3 }").arg(url,params));
    sendRequest(request);
}

void NuvoApiClient::invokeAction(QString id)
{
    QJsonObject obj = findAvItem(id);
    invokeAction(obj);
}

void NuvoApiClient::invokeAction(QJsonObject obj)
{
    QString request(tr(" { \"url\":\"%2\", \"method\":\"invoke\" }").arg(obj.value("url").toString()));
    sendRequest(request);
}

void NuvoApiClient::browseClick(int index)
{
    bool av = channels[currentBrowseChannel].value("children").toArray().at(index).toObject().value("av").toBool();

    if (av == true)
    {
        loadAv(index);
    }
    else
    {
        browseContainer(index);
    }
}

void NuvoApiClient::loadAv(int index)
{
    QString url( tr("\"%1load\"").arg(channels[avChannel].value("item").toObject().value("url").toString()) );
    QString reqItem(QJsonDocument(channels[currentBrowseChannel].value("children").toArray().at(index).toObject()).toJson());
    QString parentItem( QJsonDocument(channels[currentBrowseChannel].value("item").toObject()).toJson());
    QString context ( tr( "{ \"item\": %1, \"parent\": %2, \"index\": %3 }").arg(reqItem,parentItem,QString::number(index)) );
    QString params( tr("{\"context\": %1 }").arg(context));
    QString request( tr("{ \"url\" : %1, \"method\" : \"invoke\", \"params\" : %2 }").arg(url, params) );
    sendRequest(request);
}

void NuvoApiClient::parseChildValueChangedMessage(QString channel, QJsonObject value)
{
    int index = (int)value.value("index").toDouble();
    QJsonObject newItem(channels[channel].value("children").toArray().at(index).toObject());
    QJsonObject::Iterator iterator2;
    iterator2 = newItem.find("value");
    iterator2.value() = QJsonValue(value.value("value"));
    QJsonArray children(channels[channel].value("children").toArray());
    children.replace(index,QJsonValue(newItem));
    setChildren(channel,children);
    updateDisplay(channel,index);
}

void NuvoApiClient::parseChildItemChangedMessage(QString channel, QJsonObject value){
    int index = (int)value.value("index").toDouble();
    QJsonArray children(channels[channel].value("children").toArray());
    children.replace(index,QJsonValue(value.value("item")));
    setChildren(channel,children);
    updateDisplay(channel,index);
}

void NuvoApiClient::parseChildRemovedMessage(QString channel, QJsonObject value){
    int index = (int)value.value("index").toDouble();
    QJsonArray children(channels[channel].value("children").toArray());
    children.removeAt(index);
    setChildren(channel,children);
    updateDisplay(channel,index);
}

void NuvoApiClient::parseChildInsertedMessage(QString channel, QJsonObject value)
{
    qDebug() << "ENTERING" << __func__;
    int index = (int)value.value("index").toDouble();
    QJsonArray children(channels[channel].value("children").toArray());
    children.insert(index,value.value("item").toObject());
    setChildren(channel,children);
    updateDisplay(channel,index);

    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::updateDisplay(QString channel, int index)
{
    QJsonObject item(channels[channel].value("children").toArray().at(index).toObject());
    QString id = item.value("id").toString();
    if (id == "time") {
        progressMax = (int)item.value("maxDouble").toDouble();
        progressPos = (int)item.value("value").toObject().value("double").toDouble();
        emit progressBarChanged();
    } else if (id == "state") {
        avState = QString(item.value("value").toObject().value("avState").toString());
    } else if (id == "info") {
        parseTrackMetadata();
    } else {
        qDebug() << "UPDATE_DISPLAY: ITEM NOT PROCESSED:" << id;
    }
}

void NuvoApiClient::parseTrackMetadata(){
    qDebug() << "ENTERING" << __func__;
    QJsonObject obj = findAvItem("info");

    QJsonObject timeObj = findAvItem("time");
    progressMax = (int)timeObj.value("maxDouble").toDouble();
    progressPos = (int)timeObj.value("value").toObject().value("double").toDouble();
    emit progressBarChanged();

    metadata1 = QString(obj.value("title").toString());
    metadata2 = QString(obj.value("description").toString());
    metadata3 = QString(obj.value("longDescription").toString());

    emit albumArtCleared();
    currentArtUrl = "";
    QUrl url(obj.value("icon").toString());
    if ( !url.isEmpty() ){
        QNetworkRequest request(url);
        currentArtUrl = url.toString();
        m_netwManager->get(request);
    }
    qDebug() << "EXITING" << __func__;
}


void NuvoApiClient::slot_netwManagerFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
        return;
    }

    if (reply->url().toString() == currentArtUrl)
    {
        QByteArray jpegData = reply->readAll();
        albumArt.loadFromData(jpegData);
        emit albumArtChanged();
    }
}

void NuvoApiClient::tcpError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        emit raiseError(tr("The host was not found. Please check the "
                           "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        emit raiseError(tr("The connection was refused by the peer. "
                           "Make sure the fortune server is running, "
                           "and check that the host name and port "
                           "settings are correct."));
        break;
    default:
        emit raiseError(tr("The following error occurred: %1.")
                        .arg(tcpSocket->errorString()));
    }
}
