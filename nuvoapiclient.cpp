#include <QtWidgets>

#include "nuvoapiclient.h"
#include "nuvocontaineritem.h"

NuvoApiClient::NuvoApiClient(QObject *parent) : QObject(parent)
{
    avState = "";
    volume = 0;
    volumeMax = 100;
    progressPos = progressMax = 0;
    requestNum = 0;
    avChannel = "ch0";
    currentBrowseChannel = "";
    currentBrowseRequestNum = -1;

    m_netwManager = new QNetworkAccessManager(this);
    connect(m_netwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished(QNetworkReply*)));

    volumeActionItem = new NuvoActionItem("volume","");
    muteActionItem = new NuvoActionItem("mute","");
    prevActionItem = new NuvoActionItem("prev","");
    stopActionItem = new NuvoActionItem("stop","");
    pauseActionItem = new NuvoActionItem("pause","");
    playActionItem = new NuvoActionItem("play","");
    nextActionItem = new NuvoActionItem("next","");
    likeActionItem = new NuvoActionItem("like","");
    dislikeActionItem = new NuvoActionItem("dislike","");
    shuffleActionItem = new NuvoActionItem("shuffle","");
    repeatActionItem = new NuvoActionItem("repeat","");

    musicContainer = new NuvoContainerItem("Music","/stable/music/","","","Music","",false,0);
    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(messageReceived()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));
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

QList<QString> NuvoApiClient::getBrowseItems()
{
    qDebug() << "ENTERING" << __func__;
    qDebug() << "Current browse channel:" << currentBrowseChannel;
    qDebug() << channels[currentBrowseChannel].keys();

    QList<QString> list;
    QJsonArray children = channels[currentBrowseChannel].value("children").toArray();
    for (int i = 0; i < children.size(); i++){
        qDebug() << children.at(i).toObject().value("title").toString();
        list.append(QString(children.at(i).toObject().value("title").toString()));
    }

    qDebug() << "EXITING" << __func__;
    return list;
}



void NuvoApiClient::browseContainer(){
    browseContainer(musicContainer);
}

void NuvoApiClient::browseContainer(NuvoContainerItem *item)
{
    browseContainer(QString(item->url));
}

void NuvoApiClient::browseContainer(QString url){
    qDebug() << "ENTERING" << __func__;
    QString reqId(tr("\"req-%1\"").arg(requestNum));
    QString request(tr( "{ \"id\" : %1, \"url\" : \"%2\", \"method\" : \"browse\", \"params\" : { \"count\" : -1 } } ").arg(reqId,url));
    browseList.clear();
    currentBrowseRequestNum = sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

int NuvoApiClient::sendRequest(QString request)
{
    QByteArray byteArray = request.toUtf8();
    const char* cString = byteArray.constData();
    tcpSocket->write(cString);
    emit displayText(QString(tr("<font color=\"Blue\">%1</font><br>").arg(cString)));
    qDebug() << cString << "written to socket";
    requestNum++;
    return requestNum-1;
}

void NuvoApiClient::connectToHost(QString host, int port)
{
    qDebug() << "ENTERING" << __func__;
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
    qDebug() << "ENTERING" << __func__;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    int blockSize = tcpSocket->bytesAvailable()/sizeof(char);
    char * data = new char[blockSize];
    in.readRawData(data,blockSize);
    data[blockSize] = '\0';

    currentMessage.append(QString(data));
    if (currentMessage.contains('\n')){
        QStringList query = currentMessage.split(QRegExp("\n"));
        for (int i = 0; i < query.length()-1; i++){
            //qDebug() << "message" << i+1 << ":" << QString(query.at(i));
            //messageQueue.enqueue(QString(query.at(i)));
            parseJsonResponse(QString(query.at(i)));
        }
        currentMessage = QString(query.last());
    }
    delete(data);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseJsonResponse(QString result)
{
    qDebug() << "ENTERING" << __func__;
    QByteArray utf8;
    utf8.append(result);
    QJsonObject j = QJsonDocument::fromJson(utf8).object();
    if (j.keys().contains("error"))
        emit displayText(QString(tr("<font color=\"Red\">%1</font><br>").arg(result)));
    else
        emit displayText(QString(tr("<font color=\"Black\">%1</font><br>").arg(result)));

    QString type(j.value("type").toString());

    if (type == "reply") { parseReplyMessage(j); }
    else if ( type == "event"){  parseEventMessage(j); }
    else { qDebug() << "RESPONSE NOT PROCESSED:" << type; }

    emit refreshDisplay();
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseReplyMessage(QJsonObject obj)
{
    qDebug() << "ENTERING" << __func__;
    qDebug() << "ID: " << obj.value("id").toString();
    QString channel = obj.value("channel").toString();
    channels[channel] = QJsonObject(obj.value("result").toObject());
    QJsonArray it(channels[channel].value("children").toArray());



    for (int i = 0; i < it.size(); i++ ){;
        QJsonObject current(it.at(i).toObject());
        QString id(current.value("id").toString());
        QString url(current.value("url").toString());
        QString type(current.value("type").toString());
        bool av(current.value("av").toBool());

        if (id == "info"){  parseTrackMetadata();  }
        else if ( type == "action"){
            updateActionUrl(id,url,true);
        }
        else if ( type == "value"){
            updateActionUrl(id,url,true);
            updateDisplay(channel,i);
        }
        else if ( type == "container" || av == true){
            NuvoContainerItem* item = new NuvoContainerItem(obj.value("result").toObject().value("item").toObject(),current);
            browseList.append(item);
        }
        else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    }

    if ( QString(obj.value("id").toString()) == QString(tr("req-%1").arg(currentBrowseRequestNum)) ){
        currentBrowseChannel = channel;
        emit browseDataChanged();
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
            else { qDebug() << "ITEM NOT PROCESSED:" << id; }
        }
    }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::updateValue(NuvoActionItem *actionItem, int value)
{
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    QString params( tr("{ \"value\" : { \"int\" : %1 } }").arg(value));
    QString reqId(tr("\"req-%1\"").arg(requestNum));
    QString request(tr(" { \"id\" : %1, \"url\" : \"%2\", \"method\" : \"updateValue\", \"params\" : %3 }").arg(reqId,url,params));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::toggleValue(NuvoActionItem *actionItem)
{
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    QString request(tr("{ \"id\" : \"req-%1\", \"url\" : \"%1\", \"method\" : \"toggleValue\" }").arg(QString(requestNum),url));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::invokeAction(NuvoActionItem *actionItem)
{
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    invokeAction(url);
    qDebug() << "EXITING" << __func__;
}
void NuvoApiClient::invokeAction(QString url)
{
    qDebug() << "ENTERING" << __func__;
    QString reqId(tr("\"req-%1\"").arg(requestNum));
    QString request(tr(" { \"id\":%1, \"url\":\"%2\", \"method\":\"invoke\" }").arg(reqId,url));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::loadAv(NuvoContainerItem* item)
{
    qDebug() << "ENTERING" << __func__;
    QString index("1");
    qDebug() << item->myItem.keys();
    QString reqItem(tr("{ \"item\" : %1 }").arg(QString(QJsonDocument(item->myItem).toJson())));
    QString parentItem( QJsonDocument(item->parent).toJson() );
    QString reqId( tr("\"req-%1\"").arg(requestNum) );
    QString context( tr("{ \"item\": %1, \"index\" : %2, \"parentItem\" : %3 }").arg(reqItem, index, parentItem) );
    QString params( tr("{\"context\": %1 }").arg(context) );
    QString request( tr("{ \"id\" : %1, \"url\" : \"/stable/gav/load\", \"method\" : \"invoke\", \"params\" : %2 }").arg(reqId, params) );
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}


NuvoActionItem* NuvoApiClient::findActionItem(QString id)
{
    if ( id == "next"){ return nextActionItem;  }
    else if ( id == "play"){  return playActionItem; }
    else if ( id == "pause"){ return pauseActionItem; }
    else if ( id == "previous"){ return prevActionItem; }
    else if ( id == "stop"){ return stopActionItem; }
    else if ( id == "like"){ return likeActionItem; }
    else if ( id == "dislike"){ return dislikeActionItem; }
    else if ( id == "volume"){ return volumeActionItem; }
    else if ( id == "mute"){ return muteActionItem; }
    else if ( id == "repeat"){ return repeatActionItem; }
    else if ( id == "shuffle"){ return shuffleActionItem; }
    else { return NULL; }
}

void NuvoApiClient::parseChildValueChangedMessage(QString channel, QJsonObject value)
{
    qDebug() << "ENTERING" << __func__;
    int index = (int)value.value("index").toDouble();

    QString id = channels[channel].value("children").toArray().at(index).toObject().value("id").toString();
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
    qDebug() << "SIZE BEFORE:" << children.size();
    children.removeAt(index);
    QJsonObject::Iterator iterator;
    iterator = channels[channel].find("children");
    iterator.value() = children;
    qDebug() << "SIZE AFTER:" << channels[channel].value("children").toArray().size();

    updateActionUrl(id,"",false);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildInsertedMessage(QString channel, QJsonObject value)
{
    qDebug() << "ENTERING" << __func__;
    int index = (int)value.value("index").toDouble();
    QString url = value.value("item").toObject().value("url").toString();
    QString id = value.value("item").toObject().value("id").toString();

    QJsonArray children(channels[channel].value("children").toArray());
    qDebug() << "SIZE BEFORE:" << children.size();
    children.insert(index,value.value("item").toObject());
    QJsonObject::Iterator iterator;
    iterator = channels[channel].find("children");
    iterator.value() = children;
    qDebug() << "SIZE AFTER:" << channels[channel].value("children").toArray().size();

    updateActionUrl(id,url,true);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::updateActionUrl(QString id, QString url, bool active){
    NuvoActionItem *actionItem = findActionItem(id);
    if (actionItem) {
        actionItem->setProperty("url",url);
        actionItem->setProperty("active",active);
    } else {
        qDebug() << "ITEM NOT PROCESSED:" << id;
    }
}

void NuvoApiClient::updateDisplay(QString channel, int index)
{
    qDebug() << "ENTERING" << __func__;
    QJsonObject item(channels[channel].value("children").toArray().at(index).toObject());
    QString id = item.value("id").toString();
    if ( id == "volume"){
        volume = (int)item.value("value").toObject().value("volume").toObject().value("level").toDouble();
    } else if (id == "time") {
        progressMax = (int)item.value("maxDouble").toDouble();
        progressPos = (int)item.value("value").toObject().value("double").toDouble();
        emit progressBarChanged();
    } else if (id == "state") {
        avState = QString(item.value("value").toObject().value("avState").toString());
    } else if (id == "info") {
        parseTrackMetadata();;
    } else {
        qDebug() << "ITEM NOT PROCESSED:" << id;
    }
    qDebug() << "EXITING" << __func__;
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
