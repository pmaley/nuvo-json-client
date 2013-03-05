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
    QScriptValue sc;

    //qDebug() << result;
    QByteArray utf8;
    utf8.append(result);
    qDebug() << QJsonDocument::fromJson(utf8).toJson();
    QJsonObject j = QJsonDocument::fromJson(utf8).object();
    qDebug() << j.keys();

    QJsonValue channel = j.value("channel");
    QJsonValue id = j.value("id");
    QJsonValue res = j.value("result");
    QJsonValue type1 = j.value("type");

    qDebug() << channel << id << res << type1;

    qDebug() << res;
    qDebug() << res.isObject();
    QJsonObject children = res.toObject();
    qDebug() << children.value("children");





    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(result) + ")");
    qDebug() << "Recv. on channel " << sc.property("channel").toString();
    qDebug() << result;
    QString type = sc.property("type").toString();

    if (type == "reply") { parseReplyMessage(sc); }
    else if ( type == "event"){  parseEventMessage(sc); }
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
void NuvoApiClient::invokeAction(QString url){
    qDebug() << "ENTERING" << __func__;
    QString reqId(tr("\"req-%1\"").arg(requestNum));
    QString request(tr(" { \"id\":%1, \"url\":\"%2\", \"method\":\"invoke\" }").arg(reqId,url));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::loadAv(NuvoContainerItem* item){
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

void NuvoApiClient::loadAv2(NuvoContainerItem* item){
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

void NuvoApiClient::parseReplyMessage(QScriptValue sc)
{
    qDebug() << "ENTERING" << __func__;

    if (sc.property("result").property("children").isArray() ){
        QScriptValueIterator it(sc.property("result").property("children"));
        while (it.hasNext()) {
            it.next();
            QScriptValue current = it.value();
            QString id(current.property("id").toString());
            QString type(current.property("type").toString());
            QString av(current.property("av").toString());

            if (id == "info"){  parseTrackMetadata(current);  }
            else if ( type == "action"){  parseActionItem(current); }
            else if ( type == "value"){ parseValueItem(current); }
            else if ( type == "container"){ parseContainerItem(current, sc.property("result").property("item")); }
            else if (av == "true") { parseContainerItem(current, sc.property("result").property("item")); }
            else { qDebug() << "ITEM NOT PROCESSED:" << id << current.toString(); }
        }
    }
    emit browseDataChanged();
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseContainerItem(QScriptValue value, QScriptValue parent){
    qDebug() << "ENTERING" << __func__;
//    QScriptValue current(value);
//    QString title(current.property("title").toString());
//    QString url(current.property("url").toString());
//    QString icon(current.property("icon").toString());
//    QString type(current.property("type").toString());
//    QString id(current.property("id").toString());
//    QString sortKey(current.property("sortKey").toString());
//    bool av = current.property("av").toBool();
//    QString index(current.property("index").toString());
//    qDebug() << title << id << type << index;
    NuvoContainerItem* item = new NuvoContainerItem(parent,value);
//    NuvoContainerItem* item = new NuvoContainerItem(title,url,icon,type,id,sortKey,av,index.toInt());
    qDebug() << item->title;
    browseList.append(item);

    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseEventMessage(QScriptValue value)
{
    qDebug() << "ENTERING" << __func__;
    if (value.property("event").isArray() ){
        QScriptValueIterator it(value.property("event"));
        while (it.hasNext()) {
            it.next();
            QScriptValue current = it.value();
            QString type = QString(current.property("type").toString());
            QString id = QString(current.property("id").toString());
            if ( id == "info"){  parseTrackMetadata(current.property("item"));}
            else if (type == "childValueChanged"){ parseChildValueChangedMessage(current);}
            else if (type == "childItemChanged"){ parseChildItemChangedMessage(current);}
            else if (type == "childInserted"){ parseChildInsertedMessage(current);}
            else if (type == "childRemoved"){  parseChildRemovedMessage(current); }
            else { qDebug() << "ITEM NOT PROCESSED:" << id << current.toString(); }
        }
    }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseValueItem(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QString id = QString(value.property("id").toString());
    qDebug() << id;

    NuvoActionItem *actionItem = findActionItem(id);
    if (actionItem != NULL)
        actionItem->setProperty("url",value.property("url").toString());

    if ( id == "volume"){
        qDebug() << volume << "/" << volumeMax;
        volumeMax = value.property("maxInt").toInt32();
        volume = value.property("value").property("int").toInt32();
        qDebug() << volume << "/" << volumeMax;
        emit volumeChanged();
    } else if (id == "time") {
        qDebug() << progressPos << "/" << progressMax;
        progressMax = value.property("maxDouble").toInt32();
        progressPos = value.property("value").property("double").toInt32();
        qDebug() << progressPos << "/" << progressMax;
        emit progressBarChanged();
    } else if (id == "state") {
        avState = QString(value.property("value").property("avState").toString());
        emit avChanged();
    } else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildValueChangedMessage(QScriptValue value)
{
    qDebug() << "ENTERING" << __func__;
    QString id = QString(value.property("id").toString());
    qDebug() << id;
    if ( id == "volume"){
        qDebug() << volume << "/" << volumeMax;
        volume = value.property("value").property("int").toInt32();
        qDebug() << volume << "/" << volumeMax;
        emit volumeChanged();
    } else if (id == "time") {
        qDebug() << progressPos << "/" << progressMax;
        progressPos = value.property("value").property("double").toInt32();
        qDebug() << progressPos << "/" << progressMax;
        emit progressBarChanged();
    } else if (id == "state") {
        avState = QString(value.property("value").property("avState").toString());
        emit avStateChanged();
    } else {
        qDebug() << "ITEM NOT PROCESSED:" << id;
    }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildItemChangedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QString id = QString(value.property("id").toString());
    qDebug() << id;
    if ( id == "volume"){
        qDebug() << volume << "/" << volumeMax;
        volume = value.property("value").property("int").toInt32();
        qDebug() << volume << "/" << volumeMax;
        emit volumeChanged();
    } else if (id == "time") {
        qDebug() << progressPos << "/" << progressMax;
        progressMax = value.property("item").property("maxDouble").toInt32();
        progressPos = value.property("item").property("value").property("double").toInt32();
        qDebug() << progressPos << "/" << progressMax;
        emit progressBarChanged();
    } else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildInsertedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    parseActionItem(value.property("item"));
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseChildRemovedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QString id(value.property("id").toString());
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

void NuvoApiClient::parseActionItem(QScriptValue value)
{
    qDebug() << "ENTERING" << __func__;
    QString url(value.property("url").toString());
    QString id(value.property("id").toString());
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

void NuvoApiClient::parseTrackMetadata(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    metadata1 = QString(value.property("title").toString());
    metadata2 = QString(tr("<b>%1</b>").arg(value.property("description").toString()));
    metadata3 = QString(value.property("longDescription").toString());
    emit metadataChanged();
    QUrl url(value.property("icon").toString());
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
