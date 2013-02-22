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

    m_netwManager = new QNetworkAccessManager(this);
    connect(m_netwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished(QNetworkReply*)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(nowPlayingBox);
    mainLayout->addWidget(transportControlsBox);
    mainLayout->addWidget(consoleBox);
    setLayout(mainLayout);

    setWindowTitle(tr("NWAS API Controller"));
    resize(1000,1000);

    prevActionItem = new NuvoActionItem("prev","");
    stopActionItem = new NuvoActionItem("stop","");
    pauseActionItem = new NuvoActionItem("pause","");
    playActionItem = new NuvoActionItem("play","");
    nextActionItem = new NuvoActionItem("next","");
    likeActionItem = new NuvoActionItem("like","");
    dislikeActionItem = new NuvoActionItem("dislike","");
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
    connect(nextButton,SIGNAL(clicked()),this,SLOT(nextButtonPressed()));

    // Create play button
    playButton = new QPushButton();
    QPixmap pixmap2(":/images/player_icons/play_normal.png");
    QIcon buttonIcon2(pixmap2);
    playButton->setIcon(buttonIcon2);
    playButton->setIconSize(pixmap2.rect().size());
    connect(playButton,SIGNAL(clicked()),this,SLOT(playButtonPressed()));

    // Create pause button
    pauseButton = new QPushButton();
    QPixmap pixmap3(":/images/player_icons/pause_normal.png");
    QIcon buttonIcon3(pixmap3);
    pauseButton->setIcon(buttonIcon3);
    pauseButton->setIconSize(pixmap3.rect().size());
    connect(pauseButton,SIGNAL(clicked()),this,SLOT(pauseButtonPressed()));

    // Create prev button
    prevButton = new QPushButton();
    QPixmap pixmap4(":/images/player_icons/prev_normal.png");
    QIcon buttonIcon4(pixmap4);
    prevButton->setIcon(buttonIcon4);
    prevButton->setIconSize(pixmap4.rect().size());
    connect(prevButton,SIGNAL(clicked()),this,SLOT(prevButtonPressed()));

    // Create stop button
    stopButton = new QPushButton();
    QPixmap pixmap5(":/images/player_icons/stop_normal.png");
    QIcon buttonIcon5(pixmap5);
    stopButton->setIcon(buttonIcon5);
    stopButton->setIconSize(pixmap5.rect().size());
    connect(stopButton,SIGNAL(clicked()),this,SLOT(stopButtonPressed()));

    // Create thumbs up button
    likeButton = new QPushButton();
    QPixmap pixmap6(":/images/player_icons/like_normal.png");
    QIcon buttonIcon6(pixmap6);
    likeButton->setIcon(buttonIcon6);
    likeButton->setIconSize(pixmap6.rect().size());
    connect(likeButton,SIGNAL(clicked()),this,SLOT(likeButtonPressed()));

    // Create thumbsdown button
    dislikeButton = new QPushButton();
    QPixmap pixmap7(":/images/player_icons/dislike_normal.png");
    QIcon buttonIcon7(pixmap7);
    dislikeButton->setIcon(buttonIcon7);
    dislikeButton->setIconSize(pixmap7.rect().size());
    connect(dislikeButton,SIGNAL(clicked()),this,SLOT(dislikeButtonPressed()));

    layout->addWidget(likeButton);
    layout->addWidget(dislikeButton);
    layout->addWidget(prevButton);
    layout->addWidget(stopButton);
    layout->addWidget(pauseButton);
    layout->addWidget(playButton);
    layout->addWidget(nextButton);
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

void Dialog::createNowPlayingBox()
{
    nowPlayingBox = new QGroupBox();
    QGridLayout *layout = new QGridLayout;
    QPixmap *image = new QPixmap(":/images/aom.jpg");
    image = new QPixmap(image->scaledToHeight(100));
    imageLabel = new QLabel();
    imageLabel->setPixmap(*image);
    layout->addWidget(imageLabel,0,0);
    QStringList trackMetadata = QStringList() << "Fear Before the March of Flames Radio" << "<b>Dog Sized Bird</b>"
                                                  << "Fear Before the March of Flames - The Always Open Mouth";
    createMetadataBox(trackMetadata);
    layout->addWidget(metadataBox,0,1);
    layout->setColumnStretch(1, 10);
    layout->setColumnStretch(2, 10);
    nowPlayingBox->setLayout(layout);
}

void Dialog::testFunction(){
    qDebug() << "ENTERING" << __func__;
    QPixmap* pixmap;
    if (paused == false){
        pixmap = new QPixmap(":/images/player_icons/pause_normal@2x.png");
        paused = true;
    } else {
        pixmap = new QPixmap(":/images/player_icons/play_normal@2x.png");
        paused = false;
    }
    QIcon ButtonIcon(*pixmap);
    buttons[3]->setIcon(ButtonIcon);
    buttons[3]->setIconSize(pixmap->rect().size());
}

void Dialog::prevButtonPressed(){ invokeAction(prevActionItem); }
void Dialog::nextButtonPressed(){ invokeAction(nextActionItem); }
void Dialog::playButtonPressed(){  invokeAction(playActionItem); }
void Dialog::pauseButtonPressed(){ invokeAction(pauseActionItem); }
void Dialog::stopButtonPressed(){ invokeAction(stopActionItem); }
void Dialog::likeButtonPressed(){ invokeAction(likeActionItem); }
void Dialog::dislikeButtonPressed(){ invokeAction(dislikeActionItem); }

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

    hostCombo = new QLineEdit("192.168.1.21");
    portLineEdit = new QLineEdit("4747");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    commandTextEdit = new QTextEdit("{ \"id\" : \"req-1\", \"url\" : \"/stable/av/\", \"method\" : \"browse\", \"params\" : { \"count\" : -1 } }");
    commandTextEdit->setFixedHeight(50);
    consoleTextEdit = new QTextEdit;
    consoleTextEdit->setReadOnly(true);

    hostLabel->setBuddy(hostCombo);
    portLabel->setBuddy(portLineEdit);

    sendButton = new QPushButton(tr("send"));
    sendButton->setDefault(true);
    quitButton = new QPushButton(tr("quit"));

    buttonBox2 = new QDialogButtonBox;
    buttonBox2->addButton(sendButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(quitButton, QDialogButtonBox::RejectRole);

    tcpSocket = new QTcpSocket(this);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(messageReceived()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostCombo, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(commandTextEdit, 2, 0, 1, 2);
    mainLayout->addWidget(consoleTextEdit, 3, 0, 1, 2);
    mainLayout->addWidget(buttonBox2, 4, 0, 1, 2);

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

    QString type = sc.property("type").toString();
    qDebug() << "Message type:" << type;
    if (type == "reply") {
        parseReplyMessage(sc);
    } else if ( type == "event"){
        parseEventMessage(sc);
    }
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
            if (current.property("id").toString() == "info"){
                parseTrackMetadata(current);
            } else if ( current.property("type").toString() == "action"){
                parseActionItem(current);
            }
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
            if (current.property("id").toString() == "info"){
                parseTrackMetadata(current.property("item"));
            } else if (current.property("type").toString() == "childValueChanged"){
                parseChildValueChangedMessage(current);
            } else if (current.property("type").toString() == "childInserted"){
                parseChildInsertedMessage(current);
            } else if (current.property("type").toString() == "childRemoved"){
                parseChildRemovedMessage(current);
            }
        }
    }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseChildValueChangedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.property("id").toString();
    qDebug() << "EXITING" << __func__;

}

void Dialog::parseChildInsertedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.property("id").toString();
    parseActionItem(value.property("item"));
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseChildRemovedMessage(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.property("id").toString();
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
    if ( id == "next"){
        return nextActionItem;
    } else if ( id == "play"){
        return playActionItem;
    } else if ( id == "pause"){
        return pauseActionItem;
    } else if ( id == "previous"){
        return prevActionItem;
    } else if ( id == "stop"){
        return stopActionItem;
    } else if ( id == "like"){
        return likeActionItem;
    } else if ( id == "dislike"){
        return dislikeActionItem;
    } else {
        return NULL;
    }
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
