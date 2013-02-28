#include <QtWidgets>



#include "nuvoapiclient.h"

NuvoApiClient::NuvoApiClient(QObject *parent) :
    QObject(parent)
{
    avState = "";
    volume = volumeMax = 0;


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
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(messageReceived()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));
    //connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onConnectionStateChange()));
}

void NuvoApiClient::sendRequest(QString request)
{
    QString alertHtml  = tr("<font color=\"Red\">%1</font><br>").arg(request);
    //consoleTextEdit->append(alertHtml);
    QByteArray byteArray = request.toUtf8();
    const char* cString = byteArray.constData();
    tcpSocket->write(cString);
    qDebug() << cString << "written to socket";
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

//    consoleTextEdit->append(data);
//    consoleTextEdit->verticalScrollBar()->setSliderPosition(consoleTextEdit->verticalScrollBar()->maximum());

    currentMessage.append(QString(data));

    if (currentMessage.contains('\n')){
        QStringList query = currentMessage.split(QRegExp("\n"));
        for (int i = 0; i < query.length()-1; i++){
            qDebug() << "message" << i+1 << ":" << QString(query.at(i));
            //messageQueue.enqueue(QString(query.at(i)));
            parseJsonResponse(QString(query.at(i)));
        }
        currentMessage = QString(query.last());
        //sendButton->setEnabled(true);
    }
    delete(data);
    emit avChanged();
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseJsonResponse(QString result)
{
    qDebug() << "ENTERING" << __func__;
    QScriptValue sc;
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
    QString params( tr("{ \"value\" : { \"int\" : %1 } } }").arg(value));
    QString request(tr(" { \"id\" : \"req-3\", \"url\" : \"%1\", \"method\" : \"setValue\", \"params\" : %2 ").arg(url,params));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::toggleValue(NuvoActionItem *actionItem)
{
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    QString request(tr("{ \"id\" : \"req-7\", \"url\" : \"%1\", \"method\" : \"toggleValue\" }").arg(url));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::invokeAction(NuvoActionItem *actionItem)
{
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    QString name(actionItem->property("name").toString());
    QString request(tr(" { \"id\":\"%1\", \"url\":\"%2\", \"method\":\"invoke\" }").arg(name,url));
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

            if (id == "info"){  parseTrackMetadata(current);  }
            else if ( type == "action"){  parseActionItem(current); }
            else if ( type == "value"){ parseValueItem(current); }
            else if ( type == "container"){ parseContainerItem(current); }
            else { qDebug() << "ITEM NOT PROCESSED:" << id << current.toString(); }
        }
    }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseContainerItem(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QScriptValue current(value);
    QString type(current.property("type").toString());
    QString id(current.property("id").toString());
    QString title(current.property("title").toString());
    qDebug() << title << id << type;

//    int row = browseModel->rowCount();
//    browseModel->insertRow(row);
//    browseModel->setData(browseModel->index(row, 0), title);

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
    } else if (id == "time") {
        qDebug() << "Progress" << progressPos << "/" << progressMax;
        progressMax = value.property("maxDouble").toInt32();
        progressPos = value.property("value").property("double").toInt32();
        qDebug() << "Progress" << progressPos << "/" << progressMax;
    } else if (id == "state") {
        avState = QString(value.property("value").property("avState").toString());
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
    } else if (id == "time") {
        progressPos = value.property("value").property("double").toInt32();
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
    } else if (id == "time") {
        qDebug() << "Progress" << progressPos << "/" << progressMax;
        progressMax = value.property("item").property("maxDouble").toInt32();
        progressPos = value.property("item").property("value").property("int").toInt32();
        qDebug() << "Progress" << progressPos << "/" << progressMax;
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

    if (actionItem)
        actionItem->setProperty("url","");

//    if ( id == "next"){ nextButton->setEnabled(false); }
//    else if ( id == "play"){  playButton->setEnabled(false); }
//    else if ( id == "pause"){ pauseButton->setEnabled(false); }
//    else if ( id == "previous"){ prevButton->setEnabled(false); }
//    else if ( id == "stop"){ stopButton->setEnabled(false); }
//    else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseActionItem(QScriptValue value)
{
    qDebug() << "ENTERING" << __func__;
    QString url(value.property("url").toString());
    QString id(value.property("id").toString());
    qDebug() << id << ":" << url;
    NuvoActionItem *actionItem = findActionItem(id);
    if (actionItem)
        actionItem->setProperty("url",url);
//    if ( id == "next"){ nextButton->setEnabled(true);  }
//    else if ( id == "play"){  playButton->setEnabled(true); }
//    else if ( id == "pause"){ pauseButton->setEnabled(true); }
//    else if ( id == "previous"){ prevButton->setEnabled(true); }
//    else if ( id == "stop"){ stopButton->setEnabled(true); }
//    else if ( id == "like"){ likeButton->setEnabled(true); }
//    else if ( id == "dislike"){ dislikeButton->setEnabled(true); }
//    else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void NuvoApiClient::parseTrackMetadata(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    metadata1 = QString(value.property("title").toString());
    metadata2 = QString(tr("<b>%1</b>").arg(value.property("description").toString()));
    metadata3 = QString(value.property("longDescription").toString());
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
        errorMessage = tr("The host was not found. Please check the "
                          "host name and port settings.");
        emit raiseError();
        break;
    case QAbstractSocket::ConnectionRefusedError:
        errorMessage = tr("The connection was refused by the peer. "
                          "Make sure the fortune server is running, "
                          "and check that the host name and port "
                          "settings are correct.");
        emit raiseError();
        break;
    default:
        errorMessage = tr("The following error occurred: %1.")
                .arg(tcpSocket->errorString());
        emit raiseError();
    }

//    sendButton->setEnabled(true);
}
