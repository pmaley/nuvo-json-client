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
    QJsonObject zone = findZone(zoneTitle);
    QString url(zone.value("url").toString());
    currentAvRequestNum = browseContainer(url);
    if (!avChannel.isEmpty())
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

int NuvoApiClient::sendRequest2(QString request)
{
    QByteArray byteArray = request.toUtf8();
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
    else {
        emit displayUnformattedText(QString(tr("%1\n").arg(QString(QJsonDocument(j).toJson()))));
    }
    QString type(j.value("type").toString());
    QString channel(j.value("channel").toString());

    if (type == "reply") { parseReplyMessage(j); }
    else if ( type == "event"){  parseEventMessage(j); }
    else if ( type == "closed"){ channelClosed(channel); }
    else { qDebug() << "RESPONSE NOT PROCESSED:" << type; }

    if (!channel.isEmpty() && channel == avChannel)
        emit refreshDisplay();

    if (channel == currentBrowseChannel){
        emit browseDataChanged();
    } else if (channel == zonesChannel){
        emit zoneListChanged();
    } else if (channel == currentBrowseChannel){
        emit browseDataChanged();
    }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::channelClosed(QString channel)
{
    channels.remove(channel);
}

void NuvoApiClient::parseReplyMessage(QJsonObject obj)
{
    qDebug() << "ENTERING" << __func__;
    qDebug() << "ID: " << obj.value("id").toString();

    QString channel = obj.value("channel").toString();
    int count = (int)(obj.value("result").toObject().value("count").toDouble());
    int length = obj.value("result").toObject().value("children").toArray().size();


    if (!channel.isEmpty() && count == length){
        channels[channel] = QJsonObject(obj.value("result").toObject());
    } else if (!channel.isEmpty() && count != length && channel != currentReBrowseChannel){
        channels[channel] = QJsonObject(obj.value("result").toObject());
        currentReBrowseChannel = channel;
        int objLen = channels[channel].value("children").toArray().size();
        continueBrowseContainer(channel,objLen);
    } else if (!channel.isEmpty() && count != length && channel == currentReBrowseChannel){
        QJsonArray insertChildren(obj.value("result").toObject().value("children").toArray());
        QJsonArray children(channels[channel].value("children").toArray());
        for (int i = 0; i < insertChildren.size(); i++){
            children.append(insertChildren.at(i).toObject());
        }
        QJsonObject::Iterator iterator;
        iterator = channels[channel].find("children");
        iterator.value() = children;

        int objLen = channels[channel].value("children").toArray().size();
        if (objLen == count) { currentReBrowseChannel = ""; }
        else { continueBrowseContainer(channel,objLen); }
    }

    QJsonArray it(channels[channel].value("children").toArray());

    if (!channel.isEmpty() && currentReBrowseChannel.isEmpty())
        sendKeepAlive(channel);

    if ( QString(obj.value("id").toString()) == QString(tr("%1").arg(currentAvRequestNum)) ){
        avChannel = channel;
    } else if ( QString(obj.value("id").toString()) == QString(tr("%1").arg(currentZonesRequestNum)) ){
        zonesChannel = channel;
    } else if ( QString(obj.value("id").toString()) == QString(tr("%1").arg(currentNowPlayingContextMenuRequestNum)) ){
        currentNowPlayingContextMenuChannel = channel;
    }

    for (int i = 0; i < it.size(); i++ ){;
        QJsonObject current(it.at(i).toObject());
        QString id(current.value("id").toString());
        QString type(current.value("type").toString());

        if (id == "info"){
            parseTrackMetadata();
        } else if ( type == "value"){
            updateDisplay(channel,i);
        } else if ( id == "trackItem"){
            currentNowPlayingContextMenuRequestNum = browseContainer(current.value("context").toObject().value("url").toString());
        }
    }

    if ( QString(obj.value("id").toString()) == QString(tr("%1").arg(currentBrowseRequestNum)) ){
        if (!currentBrowseChannel.isEmpty()){
            browseChannelStack.push(currentBrowseChannel);
        }
        currentBrowseChannel = channel;
        emit metadataChanged();
    }

    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseEventMessage(QJsonObject obj)
{
    qDebug() << "ENTERING" << __func__;
    QString channel = obj.value("channel").toString();
    QJsonValue value = obj.value("event");
    qDebug() << channel;

    if ( value.isArray() ){
        QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); i++) {
            QJsonObject currentObj = array.at(i).toObject();
            QString type = QString(currentObj.value("type").toString());
            QString id = QString(currentObj.value("id").toString());
            if (type == "childValueChanged"){ parseChildValueChangedMessage(channel,currentObj); }
            else if (type == "childItemChanged"){ parseChildItemChangedMessage(channel,currentObj); }
            else if (type == "childInserted"){ parseChildInsertedMessage(channel,currentObj); }
            else if (type == "childRemoved"){ parseChildRemovedMessage(channel,currentObj); }
            else { qDebug() << "PARSE_EVENT_MESSAGE: EVENT NOT PROCESSED:" << type; }
        }
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
    QString url(obj.value("url").toString());
    QString request(tr(" { \"url\":\"%2\", \"method\":\"invoke\" }").arg(url));
    sendRequest(request);
}

void NuvoApiClient::browseClick(int index)
{
    bool av = channels[currentBrowseChannel].value("children").toArray().at(index).toObject().value("av").toBool();
    if (av == true) { loadAv(index);  }
    else { browseContainer(index); }
}

void NuvoApiClient::loadAv(int index)
{
    qDebug() << "ENTERING" << __func__;
    QString url( tr("\"%1load\"").arg(channels[avChannel].value("item").toObject().value("url").toString()) );
    QString reqItem(tr("{ \"item\" : %1 }").arg(QString(QJsonDocument(channels[currentBrowseChannel].value("children").toArray().at(index).toObject()).toJson())));
    QString parentItem( QJsonDocument(channels[browseChannelStack.top()].value("item").toObject()).toJson());
    QString context( tr("{ \"item\": %1, \"index\" : %2, \"parentItem\" : %3 }").arg(reqItem, QString::number(index), parentItem) );
    QString params( tr("{\"context\": %1 }").arg(context) );
    QString request( tr("{ \"url\" : %2, \"method\" : \"invoke\", \"params\" : %3 }").arg(url, params) );
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildValueChangedMessage(QString channel, QJsonObject value)
{
    qDebug() << "ENTERING" << __func__;
    int index = (int)value.value("index").toDouble();

    QJsonObject newItem(channels[channel].value("children").toArray().at(index).toObject());

    QJsonObject::Iterator iterator2;
    iterator2 = newItem.find("value");
    iterator2.value() = QJsonValue(value.value("value"));
    QJsonArray children(channels[channel].value("children").toArray());
    children.replace(index,QJsonValue(newItem));
    QJsonObject::Iterator iterator;
    iterator = channels[channel].find("children");
    iterator.value() = children;

    updateDisplay(channel,index);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildItemChangedMessage(QString channel, QJsonObject value){
    qDebug() << "ENTERING" << __func__;

    int index = (int)value.value("index").toDouble();
    QString id = channels[channel].value("children").toArray().at(index).toObject().value("id").toString();

    QJsonArray children(channels[channel].value("children").toArray());
    children.replace(index,QJsonValue(value.value("item")));
    QJsonObject::Iterator iterator;
    iterator = channels[channel].find("children");
    iterator.value() = children;

    updateDisplay(channel,index);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildRemovedMessage(QString channel, QJsonObject value){
    qDebug() << "ENTERING" << __func__;
    int index = (int)value.value("index").toDouble();
    QString id = channels[channel].value("children").toArray().at(index).toObject().value("id").toString();

    QJsonArray children(channels[channel].value("children").toArray());
    children.removeAt(index);
    QJsonObject::Iterator iterator;
    iterator = channels[channel].find("children");
    iterator.value() = children;

    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildInsertedMessage(QString channel, QJsonObject value)
{
    qDebug() << "ENTERING" << __func__;
    int index = (int)value.value("index").toDouble();
    QString url = value.value("item").toObject().value("url").toString();
    QString id = value.value("item").toObject().value("id").toString();

    QJsonArray children(channels[channel].value("children").toArray());
    children.insert(index,value.value("item").toObject());
    QJsonObject::Iterator iterator;
    iterator = channels[channel].find("children");
    iterator.value() = children;

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

    QJsonArray array(channels[avChannel].value("children").toArray());
    int i;
    for (i=0; i < array.size(); i++){
        if (array.at(i).toObject().value("id").toString() == "info"){
            break;
        }
    }
    QJsonObject obj(array.at(i).toObject());

    metadata1 = QString(obj.value("title").toString());
    metadata2 = QString(tr("<b>%1</b>").arg(obj.value("description").toString()));
    metadata3 = QString(obj.value("longDescription").toString());

    emit albumArtCleared();
    QUrl url(obj.value("icon").toString());
    QNetworkRequest request(url);
    m_netwManager->get(request);

    qDebug() << "EXITING" << __func__;
}


void NuvoApiClient::slot_netwManagerFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
        return;
    }

    QByteArray jpegData = reply->readAll();
    albumArt.loadFromData(jpegData);
    emit albumArtChanged();
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
