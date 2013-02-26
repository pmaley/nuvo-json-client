#include <QtWidgets>
#include <QtNetwork>
#include <QScriptValueIterator>
#include <QQueue>

#include "dialog.h"
#include "NuvoActionItem.h"

Dialog::Dialog()
{
    createMenu();
    createTransportControlsBox();
    createNowPlayingBox();
    createConsoleBox();

    browseView = new QTreeView;
    browseView->setRootIsDecorated(false);
    browseView->setAlternatingRowColors(true);

    m_netwManager = new QNetworkAccessManager(this);
    connect(m_netwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished(QNetworkReply*)));



    avState = "";
    connect(this, SIGNAL(avStateChanged()), this, SLOT(onAvStateChange()));

    progressBarTimer = new QTimer(this);
    connect(progressBarTimer, SIGNAL(timeout()), this, SLOT(updateProgressBar()));
    progressBarTimer->start(1000);


    connect(volumeSlider, SIGNAL(sliderReleased()), this, SLOT(volumeSliderAdjusted()));

    //QVBoxLayout *mainLayout = new QVBoxLayout;
    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(nowPlayingBox,0,0);



    mainLayout->addWidget(consoleBox,1,0,1,2);
    mainLayout->addWidget(browseView,0,1,1,1);
    mainLayout->setRowStretch(1,1000);
    setLayout(mainLayout);

    setWindowTitle(tr("NWAS API Controller"));
    resize(1000,1000);

    volumeActionItem = new NuvoActionItem("volume","");
    muteActionItem = new NuvoActionItem("mute","");
    prevActionItem = new NuvoActionItem("prev","");
    stopActionItem = new NuvoActionItem("stop","");
    pauseActionItem = new NuvoActionItem("pause","");
    playActionItem = new NuvoActionItem("play","");
    nextActionItem = new NuvoActionItem("next","");
    likeActionItem = new NuvoActionItem("like","");
    dislikeActionItem = new NuvoActionItem("dislike","");
    muteActionItem = new NuvoActionItem("mute","");
    shuffleActionItem = new NuvoActionItem("shuffle","");
    repeatActionItem = new NuvoActionItem("repeat","");
}

void Dialog::createMenu()
{
    menuBar = new QMenuBar;
    fileMenu = new QMenu(tr("&File"), this);
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}

void Dialog::createTransportControlsBox()
{
    transportControlsBox = new QGroupBox();
    QHBoxLayout *layout = new QHBoxLayout;

    // Create next button
    nextButton = new QPushButton();
    QPixmap pixmap1(":/images/player_icons/next_normal.png");
    QIcon buttonIcon1(pixmap1);
    nextButton->setIcon(buttonIcon1);
    nextButton->setIconSize(pixmap1.rect().size());
    nextButton->setEnabled(false);
    connect(nextButton,SIGNAL(clicked()),this,SLOT(nextButtonPressed()));

    // Create play button
    playButton = new QPushButton();
    QPixmap pixmap2(":/images/player_icons/play_normal.png");
    QIcon buttonIcon2(pixmap2);
    playButton->setIcon(buttonIcon2);
    playButton->setIconSize(pixmap2.rect().size());
    playButton->setEnabled(false);
    connect(playButton,SIGNAL(clicked()),this,SLOT(playButtonPressed()));

    // Create pause button
    pauseButton = new QPushButton();
    QPixmap pixmap3(":/images/player_icons/pause_normal.png");
    QIcon buttonIcon3(pixmap3);
    pauseButton->setIcon(buttonIcon3);
    pauseButton->setIconSize(pixmap3.rect().size());
    pauseButton->setEnabled(false);
    connect(pauseButton,SIGNAL(clicked()),this,SLOT(pauseButtonPressed()));

    // Create prev button
    prevButton = new QPushButton();
    QPixmap pixmap4(":/images/player_icons/prev_normal.png");
    QIcon buttonIcon4(pixmap4);
    prevButton->setIcon(buttonIcon4);
    prevButton->setIconSize(pixmap4.rect().size());
    prevButton->setEnabled(false);
    connect(prevButton,SIGNAL(clicked()),this,SLOT(prevButtonPressed()));

    // Create stop button
    stopButton = new QPushButton();
    QPixmap pixmap5(":/images/player_icons/stop_normal.png");
    QIcon buttonIcon5(pixmap5);
    stopButton->setIcon(buttonIcon5);
    stopButton->setIconSize(pixmap5.rect().size());
    stopButton->setEnabled(false);
    connect(stopButton,SIGNAL(clicked()),this,SLOT(stopButtonPressed()));

    // Create thumbs up button
    likeButton = new QPushButton();
    QPixmap pixmap6(":/images/player_icons/like_normal.png");
    QIcon buttonIcon6(pixmap6);
    likeButton->setIcon(buttonIcon6);
    likeButton->setIconSize(pixmap6.rect().size());
    likeButton->setEnabled(false);
    connect(likeButton,SIGNAL(clicked()),this,SLOT(likeButtonPressed()));

    // Create thumbsdown button
    dislikeButton = new QPushButton();
    QPixmap pixmap7(":/images/player_icons/dislike_normal.png");
    QIcon buttonIcon7(pixmap7);
    dislikeButton->setIcon(buttonIcon7);
    dislikeButton->setIconSize(pixmap7.rect().size());
    dislikeButton->setEnabled(false);
    connect(dislikeButton,SIGNAL(clicked()),this,SLOT(dislikeButtonPressed()));

    // Create mute button
    muteButton = new QPushButton();
    QPixmap pixmap8(":/images/player_icons/player_seeker.png");
    QIcon buttonIcon8(pixmap8);
    muteButton->setIcon(buttonIcon8);
    muteButton->setEnabled(false);
    connect(muteButton,SIGNAL(clicked()),this,SLOT(muteButtonPressed()));

    // Create shuffle button
    shuffleButton = new QPushButton();
    QPixmap pixmap9(":/images/player_icons/player_icon_shuffle_off_normal.png");
    QIcon buttonIcon9(pixmap9);
    shuffleButton->setIcon(buttonIcon9);
    shuffleButton->setEnabled(false);
    connect(shuffleButton,SIGNAL(clicked()),this,SLOT(shuffleButtonPressed()));

    // Create repeat button
    repeatButton = new QPushButton();
    QPixmap pixmap10(":/images/player_icons/player_icon_repeat_off_normal.png");
    QIcon buttonIcon10(pixmap10);
    repeatButton->setIcon(buttonIcon10);
    repeatButton->setEnabled(false);
    connect(repeatButton,SIGNAL(clicked()),this,SLOT(repeatButtonPressed()));

    layout->addWidget(likeButton);
    layout->addWidget(dislikeButton);
    layout->addWidget(prevButton);
    layout->addWidget(stopButton);
    layout->addWidget(pauseButton);
    layout->addWidget(playButton);
    layout->addWidget(nextButton);
//    layout->addWidget(muteButton);
//    layout->addWidget(shuffleButton);
//    layout->addWidget(repeatButton);
    transportControlsBox->setLayout(layout);
}

void Dialog::createMetadataBox(QStringList trackMetadata)
{
    metadataBox = new QGroupBox();
    QVBoxLayout *layout = new QVBoxLayout;

    for (int i = 0; i < trackMetadata.length(); ++i) {
        labels[i] = new QLabel(trackMetadata[i]);
        labels[i]->setTextFormat(Qt::RichText);
        layout->addWidget(labels[i]);
    }
    metadataBox->setLayout(layout);
}

//void Dialog::createNowPlayingBox2()
//{
//    nowPlayingBox = new QGroupBox();
//    QGridLayout *layout = new QGridLayout;
//    QPixmap *image = new QPixmap(":/images/aom.jpg");
//    image = new QPixmap(image->scaledToHeight(100));
//    imageLabel = new QLabel();
//    imageLabel->setPixmap(*image);
//    layout->addWidget(imageLabel,0,0);
//    QStringList trackMetadata = QStringList() << "Fear Before the March of Flames Radio" << "<b>Dog Sized Bird</b>"
//                                                  << "Fear Before the March of Flames - The Always Open Mouth";
//    createMetadataBox(trackMetadata);
//    layout->addWidget(metadataBox,0,1);
//    layout->setColumnStretch(1, 10);
//    layout->setColumnStretch(2, 10);
//    nowPlayingBox->setLayout(layout);
//}

void Dialog::createNowPlayingBox()
{
    nowPlayingBox = new QGroupBox();
    QGridLayout *layout = new QGridLayout;
    QPixmap *image = new QPixmap(":/images/aom.jpg");
    image = new QPixmap(image->scaledToHeight(100));
    imageLabel = new QLabel();
    imageLabel->setPixmap(*image);
    QStringList trackMetadata = QStringList() << "Fear Before the March of Flames Radio" << "<b>Dog Sized Bird</b>"
                                                  << "Fear Before the March of Flames - The Always Open Mouth";
    createMetadataBox(trackMetadata);

    trackProgressBar = new QProgressBar();
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setMaximum(100);


    layout->addWidget(imageLabel,0,0);
    layout->addWidget(metadataBox,0,1);
    layout->addWidget(trackProgressBar,1,0,1,2);
    layout->addWidget(volumeSlider,2,0,1,2);
    layout->addWidget(transportControlsBox,4,0,1,2);

    layout->setColumnStretch(1, 10);
    layout->setColumnStretch(2, 10);
    nowPlayingBox->setLayout(layout);
}

void Dialog::prevButtonPressed(){ invokeAction(prevActionItem); }
void Dialog::nextButtonPressed(){ invokeAction(nextActionItem); }
void Dialog::playButtonPressed(){  invokeAction(playActionItem); }
void Dialog::pauseButtonPressed(){ invokeAction(pauseActionItem); }
void Dialog::stopButtonPressed(){ invokeAction(stopActionItem); }
void Dialog::likeButtonPressed(){ invokeAction(likeActionItem); }
void Dialog::dislikeButtonPressed(){ invokeAction(dislikeActionItem); }
void Dialog::volumeSliderAdjusted(){ updateValue(volumeActionItem, volumeSlider->value()); }
void Dialog::muteButtonPressed(){ toggleValue(muteActionItem); }
void Dialog::shuffleButtonPressed(){ toggleValue(shuffleActionItem); }
void Dialog::repeatButtonPressed(){ toggleValue(repeatActionItem); }

void Dialog::updateValue(NuvoActionItem *actionItem, int value){
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    QString params( tr("{ \"value\" : { \"int\" : %1 } } }").arg(value));
    QString request(tr(" { \"id\" : \"req-3\", \"url\" : \"%1\", \"method\" : \"setValue\", \"params\" : %2 ").arg(url,params));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void Dialog::toggleValue(NuvoActionItem *actionItem){
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    QString request(tr("{ \"id\" : \"req-7\", \"url\" : \"%1\", \"method\" : \"toggleValue\" }").arg(url));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void Dialog::invokeAction(NuvoActionItem *actionItem){
    qDebug() << "ENTERING" << __func__;
    QString url(actionItem->property("url").toString());
    QString name(actionItem->property("name").toString());
    QString request(tr(" { \"id\":\"%1\", \"url\":\"%2\", \"method\":\"invoke\" }").arg(name,url));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}


void Dialog::createConsoleBox()
{
    consoleBox = new QGroupBox();
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));

    hostCombo = new QLineEdit("192.168.1.102");
    hostCombo->setFixedWidth(100);
    portLineEdit = new QLineEdit("4747");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    portLineEdit->setFixedWidth(50);
    commandTextEdit = new QTextEdit("{ \"id\" : \"req-1\", \"url\" : \"/stable/av/\", \"method\" : \"browse\", \"params\" : { \"count\" : -1 } }");
    commandTextEdit->setFixedHeight(50);
    consoleTextEdit = new QTextEdit;
    consoleTextEdit->setReadOnly(true);

    hostLabel->setBuddy(hostCombo);
    portLabel->setBuddy(portLineEdit);

    sendButton = new QPushButton(tr("Send"));
    sendButton->setDefault(true);
    quitButton = new QPushButton(tr("Quit"));
    connectButton = new QPushButton(tr("Connect"));
    disconnectButton = new QPushButton(tr("Disconnect"));

    buttonBox2 = new QDialogButtonBox;
    buttonBox2->addButton(sendButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(quitButton, QDialogButtonBox::RejectRole);

    tcpSocket = new QTcpSocket(this);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(messageReceived()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0, Qt::AlignLeft);
    mainLayout->addWidget(hostCombo, 0, 1, Qt::AlignLeft);
    mainLayout->addWidget(portLabel, 0, 2, Qt::AlignLeft);
    mainLayout->addWidget(portLineEdit, 0, 3, Qt::AlignLeft);
    mainLayout->addWidget(commandTextEdit, 1, 0, 1, 8);
    mainLayout->addWidget(consoleTextEdit, 2, 0, 1, 8);
    mainLayout->addWidget(buttonBox2, 0, 4, 1, 4);
    mainLayout->setRowStretch(2,1000);
    portLineEdit->setFocus();
    consoleBox->setLayout(mainLayout);
}

void Dialog::requestNewFortune()
{
    qDebug() << "ENTERING" << __func__;
    sendButton->setEnabled(false);
    blockSize = 0;
    if (!tcpSocket->isOpen()){
        openConnection();
    } else {
        sendRequest(QString(commandTextEdit->toPlainText()));
        commandTextEdit->setText("");
    }
}

void Dialog::sendRequest(QString request)
{
    QString alertHtml  = tr("<font color=\"Red\">%1</font><br>").arg(request);
    consoleTextEdit->append(alertHtml);
    QByteArray byteArray = request.toUtf8();
    const char* cString = byteArray.constData();
    tcpSocket->write(cString);
    qDebug() << cString << "written to socket";
}

void Dialog::openConnection()
{
    qDebug() << "ENTERING" << __func__;
    if (!tcpSocket->isOpen())
    {
        tcpSocket->abort();
        tcpSocket->connectToHost(hostCombo->text(), portLineEdit->text().toInt());
    }
}

void Dialog::messageReceived()
{
    qDebug() << "ENTERING" << __func__;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    blockSize = tcpSocket->bytesAvailable()/sizeof(char);
    char * data = new char[blockSize];
    in.readRawData(data,blockSize);
    data[blockSize] = '\0';

    consoleTextEdit->append(data);
    consoleTextEdit->verticalScrollBar()->setSliderPosition(consoleTextEdit->verticalScrollBar()->maximum());

    currentMessage.append(QString(data));    

    if (currentMessage.contains('\n')){
        QStringList query = currentMessage.split(QRegExp("\n"));
        for (int i = 0; i < query.length()-1; i++){
            qDebug() << "message" << i+1 << ":" << QString(query.at(i));
            //messageQueue.enqueue(QString(query.at(i)));
            parseJsonResponse(QString(query.at(i)));
        }
        currentMessage = QString(query.last());
        sendButton->setEnabled(true);
    }
    delete(data);
    qDebug() << "EXITING" << __func__;
}

void Dialog::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    sendButton->setEnabled(true);
}

void Dialog::enableSendButton()
{
    sendButton->setEnabled((!networkSession || networkSession->isOpen()) && !hostCombo->text().isEmpty() && !portLineEdit->text().isEmpty());
}

void Dialog::parseJsonResponse(QString result)
{
    qDebug() << "ENTERING" << __func__;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(result) + ")");
    qDebug() << "Recv. on channel " << sc.property("channel").toString();
    qDebug() << result;
    QString type = sc.property("type").toString();
    qDebug() << "Message type:" << type;

    if (type == "reply") { parseReplyMessage(sc); }
    else if ( type == "event"){  parseEventMessage(sc); }
    else { qDebug() << "RESPONSE NOT PROCESSED:" << type; }

    qDebug() << "EXITING" << __func__;
}

void Dialog::parseReplyMessage(QScriptValue sc)
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
            else { qDebug() << "ITEM NOT PROCESSED:" << id << current.toString(); }
        }
    }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseEventMessage(QScriptValue value)
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
            else if (type == "childValueChanged"){  parseChildValueChangedMessage(current);}
            else if (type == "childItemChanged"){ parseChildItemChangedMessage(current);}
            else if (type == "childInserted"){ parseChildInsertedMessage(current);}
            else if (type == "childRemoved"){  parseChildRemovedMessage(current); }
            else { qDebug() << "ITEM NOT PROCESSED:" << id << current.toString(); }
        }
    }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseValueItem(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QString id = QString(value.property("id").toString());
    qDebug() << id;

    NuvoActionItem *actionItem = findActionItem(id);
    if (actionItem != NULL)
        actionItem->setProperty("url",value.property("url").toString());

    if ( id == "volume"){
        volumeSlider->setMaximum(value.property("maxInt").toInt32());
        volumeSlider->setValue(value.property("value").property("int").toInt32());
    } else if (id == "time") {
        trackProgressBar->setMaximum(value.property("maxDouble").toInt32());
        trackProgressBar->setValue(value.property("value").property("double").toInt32());
    } else if (id == "state") {
        avState = QString(value.property("value").property("avState").toString());
    } else if (id == "mute") {
        muteButton->setEnabled(value.property("modifiable").toBool());
    } else if (id == "shuffle") {
        shuffleButton->setEnabled(value.property("modifiable").toBool());
    } else if (id == "repeat") {
        repeatButton->setEnabled(value.property("modifiable").toBool());
    } else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseChildValueChangedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QString id = QString(value.property("id").toString());
    qDebug() << id;
    if ( id == "volume"){
        volumeSlider->setValue(value.property("value").property("int").toInt32());
    } else if (id == "time") {
        trackProgressBar->setValue(value.property("value").property("double").toInt32());
    } else if (id == "mute") {
        bool state = value.property("value").property("bool").toBool();
        if (state) {
            QPixmap pixmap8(":/images/player_icons/player_seeker_muted.png");
            QIcon buttonIcon8(pixmap8);
            muteButton->setIcon(buttonIcon8);
        } else {
            QPixmap pixmap8(":/images/player_icons/player_seeker.png");
            QIcon buttonIcon8(pixmap8);
            muteButton->setIcon(buttonIcon8);
        }
    } else if (id == "shuffle") {
        bool state = value.property("value").property("bool").toBool();
        if (state) {
            QPixmap pixmap8(":/images/player_icons/player_icon_shuffle_on_normal.png");
            QIcon buttonIcon8(pixmap8);
            shuffleButton->setIcon(buttonIcon8);
        } else {
            QPixmap pixmap8(":/images/player_icons/player_icon_shuffle_off_normal.png");
            QIcon buttonIcon8(pixmap8);
            shuffleButton->setIcon(buttonIcon8);
        }
    } else if (id == "repeat") {
        QString state(value.property("value").property("avRepeatMode").toString());
        if (state == "all") {
            QPixmap pixmap8(":/images/player_icons/player_icon_repeat_on_normal.png");
            QIcon buttonIcon8(pixmap8);
            repeatButton->setIcon(buttonIcon8);
        } else {
            QPixmap pixmap8(":/images/player_icons/player_icon_repeat_off_normal.png");
            QIcon buttonIcon8(pixmap8);
            repeatButton->setIcon(buttonIcon8);
        }
    } else if (id == "state") {
        avState = QString(value.property("value").property("avState").toString());
        qDebug() << avState;
        emit avStateChanged();

    } else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseChildItemChangedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QString id = QString(value.property("id").toString());
    qDebug() << id;
    if ( id == "volume"){
        volumeSlider->setValue(value.property("value").property("int").toInt32());
    } else if (id == "time") {
        trackProgressBar->setMaximum(value.property("item").property("maxDouble").toInt32());
        trackProgressBar->setValue(value.property("item").property("value").property("int").toInt32());
    } else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseChildInsertedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    parseActionItem(value.property("item"));
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseChildRemovedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QString id(value.property("id").toString());
    qDebug() << id;
    NuvoActionItem *actionItem = findActionItem(id);

    if (actionItem)
        actionItem->setProperty("url","");

    if ( id == "next"){ nextButton->setEnabled(false); }
    else if ( id == "play"){  playButton->setEnabled(false); }
    else if ( id == "pause"){ pauseButton->setEnabled(false); }
    else if ( id == "previous"){ prevButton->setEnabled(false); }
    else if ( id == "stop"){ stopButton->setEnabled(false); }
    else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseActionItem(QScriptValue value)
{
    qDebug() << "ENTERING" << __func__;
    QString url(value.property("url").toString());
    QString id(value.property("id").toString());
    qDebug() << id << ":" << url;
    NuvoActionItem *actionItem = findActionItem(id);
    if (actionItem)
        actionItem->setProperty("url",url);
    if ( id == "next"){ nextButton->setEnabled(true);  }
    else if ( id == "play"){  playButton->setEnabled(true); }
    else if ( id == "pause"){ pauseButton->setEnabled(true); }
    else if ( id == "previous"){ prevButton->setEnabled(true); }
    else if ( id == "stop"){ stopButton->setEnabled(true); }
    else if ( id == "like"){ likeButton->setEnabled(true); }
    else if ( id == "dislike"){ dislikeButton->setEnabled(true); }
    else { qDebug() << "ITEM NOT PROCESSED:" << id; }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseTrackMetadata(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    labels[0]->setText(value.property("title").toString());
    labels[1]->setText(tr("<b>%1</b>").arg(value.property("description").toString()));
    labels[2]->setText(value.property("longDescription").toString());
    QUrl url(value.property("icon").toString());
    QNetworkRequest request(url);
    m_netwManager->get(request);
    qDebug() << "EXITING" << __func__;
}

NuvoActionItem* Dialog::findActionItem(QString id)
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

void Dialog::slot_netwManagerFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
        return;
    }

    QByteArray jpegData = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(jpegData);
    imageLabel->setPixmap(pixmap.scaledToHeight(100)); // or whatever your labels name is
}

void Dialog::onAvStateChange(){
    qDebug() << "AvState Change:" << avState;
}

void Dialog::updateProgressBar(){
    if (avState == "playing" && trackProgressBar->maximum() > 0) {
        trackProgressBar->setValue(trackProgressBar->value()+1);
    }
}



