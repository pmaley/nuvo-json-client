#include <QtWidgets>



#include "nuvoapiclient.h"
#include "nuvocontaineritem.h"

NuvoApiClient::NuvoApiClient(QObject *parent) :
    QObject(parent)
{
    avState = "";
    volume = volumeMax = 0;
    progressPos = progressMax = 0;
    requestNum = 0;

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

void NuvoApiClient::browseContainer(){
    browseContainer(musicContainer);
}

void NuvoApiClient::browseContainer(NuvoContainerItem *item)
{
    browseContainer(QString(item->url));
}

void NuvoApiClient::browseContainer(QString url){
    qDebug() << "ENTERING" << __func__;
    qDebug() << url;
    QString reqId(tr("\"req-%1\"").arg(requestNum));
    QString request(tr( "{ \"id\" : %1, \"url\" : \"%2\", \"method\" : \"browse\", \"params\" : { \"count\" : -1 } } ").arg(reqId,url));
    browseList.clear();
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::sendRequest(QString request)
{
    QByteArray byteArray = request.toUtf8();
    const char* cString = byteArray.constData();
    tcpSocket->write(cString);
    emit displayText(QString(tr("<font color=\"Red\">%1</font><br>").arg(cString)));
    qDebug() << cString << "written to socket";
    requestNum++;
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

    emit displayText(QString(data));
    currentMessage.append(QString(data));

    if (currentMessage.contains('\n')){
        QStringList query = currentMessage.split(QRegExp("\n"));
        for (int i = 0; i < query.length()-1; i++){
            qDebug() << "message" << i+1 << ":" << QString(query.at(i));
            //messageQueue.enqueue(QString(query.at(i)));
            parseJsonResponse(QString(query.at(i)));
        }
        currentMessage = QString(query.last());
    }
    delete(data);
    emit avChanged();
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseJsonResponse(QString result)
{
    qDebug() << "ENTERING" << __func__;

    QByteArray utf8;
    utf8.append(result);
    QJsonObject j = QJsonDocument::fromJson(utf8).object();

    QJsonValue res = j.value("result");
    QJsonValue event = j.value("event");
    QString channel(j.value("channel").toString());
    QString id(j.value("id").toString());
    QString type(j.value("type").toString());


    qDebug() << "KEYS:" << j.keys();
    qDebug() << "CHANNEL: " << channel;
    qDebug() << "ID: " << id;
   // qDebug() << "RESULT: " << res;
    qDebug() << "EVENT: " << event;
    qDebug() << "TYPE: " << type;
    qDebug() << "RESULT IS OBJECT? " << res.isObject();
    qDebug() << "RESULT IS ARRAY? " << res.isArray();
    qDebug() << "RESULT IS BOOL? " << res.isBool();
    qDebug() << "RESULT IS STRING? " << res.isString();


    QScriptEngine engine;
    QScriptValue sc = engine.evaluate("(" + QString(result) + ")");

    if (type == "reply") { parseReplyMessage(res); }
    else if ( type == "event"){  parseEventMessage(event); }
    else { qDebug() << "RESPONSE NOT PROCESSED:" << type; }

    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::updateValue(NuvoActionItem *actionItem, int value)
{
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    QString params( tr("{ \"value\" : { \"int\" : %1 } }").arg(value));
    QString reqId(tr("\"req-%1\"").arg(requestNum));
    qDebug() << requestNum;
    qDebug() << "String" << QString(requestNum);
    QString request(tr(" { \"id\" : %1, \"url\" : \"%2\", \"method\" : \"setValue\", \"params\" : %3 }").arg(reqId,url,params));
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
    qDebug() << QString(item->title);
    qDebug() << QString(item->url);
    QString reqId(tr("\"req-%1\"").arg(requestNum));
    QString parentItem("{ \"url\" : \"/tuneIn01/local\", \"type\" : \"container\", \"title\" : \"Local Radio\"}");
    QString context(tr("{ \"item\" : { \"url\" : \"/tuneIn01/radio2\", \"av\" : true, \"title\" : \"Radio 2\", \"description\" : \"Best Radio in Town\", \"creator\" : \"tunein\", \"resource\" : \"http:radio2.com/stream\" }, \"index\" : 1, \"parentItem\" : %1 }").arg(parentItem));
    //QString request(tr("{ \"id\" : %1, \"url\" : \"/stable/av/load/\", \"method\" : \"invoke\", \"context\" : %2 }").arg(reqId,context));
    QString request(tr("" ));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::loadAv2(NuvoContainerItem* item)
{
    qDebug() << "ENTERING" << __func__;
    QString index("1");
    //QString reqItem( item->myItem.toString() );
    //QString parentItem( item->parent.toString() );
    //qDebug() << "ITEM:" << reqItem;
    //qDebug() << "PARENT:" << parentItem;
    QString reqItem(tr("{\"item\":{\"station\":true,\"title\":\"QuickMix\",\"av\":true,\"creator\":\"pandora\",\"url\":\"pandora:StationList/station_983251158063407\",\"resourceUrl\":\"station?stationToken=983251158063407\",\"icon\":\"skin:iconPandoraQuickmix\",\"context\":{\"url\":\"player:player/context\",\"type\":\"container\",\"nsdk\":{\"containerType\":\"context\"}},\"nsdk\":{\"mediaData\":{\"metaData\":{\"playLogicPath\":\"pandora:PandoraPlayLogic\"},\"resources\":[{\"mimeType\":\"audio/x-pandora\"}]},\"type\":\"audio\"}}}"));
    QString parentItem(tr("{\"title\":\"Pandora\",\"activity\":true,\"search\":{\"title\":\"Artist, Track\",\"url\":\"pandora:searches\",\"type\":\"container\",\"nsdk\":{\"containerType\":\"search\"}},\"url\":\"pandora:StationList\",\"icon\":\"skin:iconPandora\",\"type\":\"container\"}"));
    QString reqId( tr("\"req-%1\"").arg(requestNum) );
    QString context( tr("{ \"item\": %1, \"index\" : %2, \"parentItem\" : %3 }").arg(reqItem, index, parentItem) );
    QString request( tr("{ \"id\" : %1, \"url\" : \"/stable/gav/load\", \"method\" : \"invoke\", \"context\" : %2 }").arg(reqId, context) );
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

void NuvoApiClient::parseReplyMessage(QJsonValue value)
{
    qDebug() << "ENTERING" << __func__;
    if (value.toObject().value("children").isArray()){
        QJsonArray it = value.toObject().value("children").toArray();
        for (int i = 0; i < it.size(); i++ ){;
            QJsonObject current = it.at(i).toObject();
            QString id(current.value("id").toString());
            QString type(current.value("type").toString());
            QString av(current.value("av").toString());

            if (id == "info"){  parseTrackMetadata(current);  }
            else if ( type == "action"){  parseActionItem(current); }
            else if ( type == "value"){ parseValueItem(current); }
            else if ( type == "container"){ parseContainerItem(current, current.value("result").toObject().value("item").toObject()); }
            else if (av == "true") { parseContainerItem(current, current.value("result").toObject().value("item").toObject()); }
            else { qDebug() << "ITEM NOT PROCESSED:" << id; }
        }
    }
    emit browseDataChanged();
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseContainerItem(QJsonObject value, QJsonObject parent){
    qDebug() << "ENTERING" << __func__;
    NuvoContainerItem* item = new NuvoContainerItem(parent,value);
    qDebug() << item->title;
    browseList.append(item);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseEventMessage(QJsonValue value)
{
    qDebug() << "ENTERING" << __func__;
    if ( value.isArray() ){
        QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); i++) {
            QJsonValue current = array.at(i);
            qDebug() << current.isObject();
            QJsonObject currentObj = current.toObject();
            qDebug() << currentObj.keys();
            QString type = QString(currentObj.value("type").toString());
            QString id = QString(currentObj.value("id").toString());
            if ( id == "info"){  parseTrackMetadata(currentObj); }
            else if (type == "childValueChanged"){ parseChildValueChangedMessage(currentObj);}
            else if (type == "childItemChanged"){ parseChildItemChangedMessage(currentObj);}
            else if (type == "childInserted"){ parseChildInsertedMessage(currentObj);}
            else if (type == "childRemoved"){  parseChildRemovedMessage(currentObj); }
            else { qDebug() << "ITEM NOT PROCESSED:" << id; }
        }
    }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseValueItem(QJsonObject value){
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.keys();
    QString id = QString(value.value("id").toString());
    qDebug() << id;

    NuvoActionItem *actionItem = findActionItem(id);
    if (actionItem != NULL)
        actionItem->setProperty("url",value.value("url").toString());

    if ( id == "volume"){
        qDebug() << volume << "/" << volumeMax;
        volumeMax = (int)value.value("maxInt").toDouble();
        volume = (int)value.value("value").toObject().value("int").toDouble();
        qDebug() << volume << "/" << volumeMax;
        emit volumeChanged();
    } else if (id == "time") {
        qDebug() << progressPos << "/" << progressMax;
        progressMax = (int)value.value("maxDouble").toDouble();
        progressPos = (int)value.value("value").toObject().value("double").toDouble();
        qDebug() << progressPos << "/" << progressMax;
        emit progressBarChanged();
    } else if (id == "state") {
        avState = QString(value.value("value").toObject().value("avState").toString());
        emit avChanged();
    } else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildValueChangedMessage(QJsonObject value)
{
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.keys();
    QString id = QString(value.value("id").toString());
    qDebug() << id;
    if ( id == "volume"){
        qDebug() << volume << "/" << volumeMax;
        volume = (int)value.value("value").toObject().value("int").toDouble();
        qDebug() << volume << "/" << volumeMax;
        emit volumeChanged();
    } else if (id == "time") {
        qDebug() << progressPos << "/" << progressMax;
        progressPos = value.value("value").toObject().value("double").toDouble();
        qDebug() << progressPos << "/" << progressMax;
        emit progressBarChanged();
    } else if (id == "state") {
        avState = QString(value.value("value").toObject().value("avState").toString());
        emit avStateChanged();
    } else {
        qDebug() << "ITEM NOT PROCESSED:" << id;
    }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildItemChangedMessage(QJsonObject value){
    qDebug() << "ENTERING" << __func__;
        qDebug() << value.keys();
    QString id = QString(value.value("id").toString());
    qDebug() << id;
    if ( id == "volume"){
        qDebug() << volume << "/" << volumeMax;
        volume = (int)value.value("value").toObject().value("int").toDouble();
        qDebug() << volume << "/" << volumeMax;
        emit volumeChanged();
    } else if (id == "time") {
        qDebug() << progressPos << "/" << progressMax;
        progressMax = (int)value.value("item").toObject().value("maxDouble").toDouble();
        progressPos = (int)value.value("item").toObject().value("value").toObject().value("double").toDouble();
        qDebug() << progressPos << "/" << progressMax;
        emit progressBarChanged();
    } else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildInsertedMessage(QJsonObject value){
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.keys();
    parseActionItem(value.value("item").toObject());
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildRemovedMessage(QJsonObject value){
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.keys();
    QString id(value.value("id").toString());
    qDebug() << id;
    NuvoActionItem *actionItem = findActionItem(id);

    if (actionItem) {
        actionItem->setProperty("url","");
        actionItem->setProperty("active",false);
        emit transportChanged();
    } else {
        qDebug() << "ITEM NOT PROCESSED:" << id;
    }

    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseActionItem(QJsonObject value)
{
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.keys();
    QString url(value.value("url").toString());
    QString id(value.value("id").toString());
    qDebug() << id << ":" << url;
    NuvoActionItem *actionItem = findActionItem(id);
    if (actionItem) {
        actionItem->setProperty("url",url);
        actionItem->setProperty("active",true);
        emit transportChanged();
    } else {
        qDebug() << "ITEM NOT PROCESSED:" << id;
    }

    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseTrackMetadata(QJsonObject obj){
    qDebug() << "ENTERING" << __func__;
    qDebug() << obj.keys();
    QJsonObject value = obj.value("item").toObject();

    metadata1 = QString(value.value("title").toString());
    metadata2 = QString(tr("<b>%1</b>").arg(value.value("description").toString()));
    metadata3 = QString(value.value("longDescription").toString());
    emit metadataChanged();
    QUrl url(value.value("icon").toString());
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
