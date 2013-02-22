#include <QtWidgets>
#include <QtNetwork>
#include <QScriptValueIterator>

#include "dialog.h"
#include "nuvogroup.h"
#include "nuvotransportcontrol.h"

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

    group = new NuvoGroup();
    playActionItem = new NuvoTransportControl("play","");
    nextActionItem = new NuvoTransportControl("next","");
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

    QStringList transportControls = QStringList() << "dislike_normal@2x.png" << "like_normal@2x.png" << "prev_normal@2x.png";

    // Create next button
    nextButton = new QPushButton();
    QPixmap pixmap1(":/images/player_icons/next_normal@2x.png");
    QIcon buttonIcon1(pixmap1);
    nextButton->setIcon(buttonIcon1);
    nextButton->setIconSize(pixmap1.rect().size());
    connect(nextButton,SIGNAL(clicked()),this,SLOT(nextButtonPressed()));

    // Create play button
    playButton = new QPushButton();
    QPixmap pixmap2(":/images/player_icons/play_normal@2x.png");
    QIcon buttonIcon2(pixmap2);
    playButton->setIcon(buttonIcon2);
    playButton->setIconSize(pixmap2.rect().size());
    connect(playButton,SIGNAL(clicked()),this,SLOT(playButtonPressed()));

    // Create pause button
//    pauseButton = new QPushButton();
//    QPixmap pixmap3(":/images/player_icons/pause_normal@2x.png");
//    QIcon buttonIcon3(pixmap3);
//    pauseButton->setIcon(buttonIcon3);
//    pauseButton->setIconSize(pixmap3.rect().size());
//    connect(pauseButton,SIGNAL(clicked()),this,SLOT(pauseButtonPressed()));


    for (int i = 0; i < transportControls.length(); ++i) {
        buttons[i] = new QPushButton();
        QPixmap pixmap(tr(":/images/player_icons/%1").arg(transportControls[i]));
        QIcon buttonIcon(pixmap);
        buttons[i]->setIcon(buttonIcon);
        buttons[i]->setIconSize(pixmap.rect().size());
        connect(buttons[i],SIGNAL(clicked()),this,SLOT(testFunction()));
        layout->addWidget(buttons[i]);
    }
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

void Dialog::nextButtonPressed(){
    qDebug() << "ENTERING" << __func__;
    QString url(nextActionItem->property("url").toString());
    QString request(tr(" { \"id\":\"next\", \"url\":\"%1\", \"method\":\"invoke\" }").arg(url));
    sendRequest(request);
    qDebug() << "EXITING" << __func__;
}

void Dialog::playButtonPressed(){
    qDebug() << "ENTERING" << __func__;
    QString url(playActionItem->property("url").toString());
    QString request(tr(" { \"id\":\"play\", \"url\":\"%1\", \"method\":\"invoke\" }").arg(url));
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

    getFortuneButton = new QPushButton(tr("send"));
    getFortuneButton->setDefault(true);
    quitButton = new QPushButton(tr("quit"));

    buttonBox2 = new QDialogButtonBox;
    buttonBox2->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(quitButton, QDialogButtonBox::RejectRole);

    tcpSocket = new QTcpSocket(this);

    connect(getFortuneButton, SIGNAL(clicked()), this, SLOT(requestNewFortune()));
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
    getFortuneButton->setEnabled(false);
    blockSize = 0;
    if (!tcpSocket->isOpen()){
        openConnection();
    } else {
        QString command = commandTextEdit->toPlainText();
        QString alertHtml  = tr("<font color=\"Red\">%1</font><br>").arg(command);
        consoleTextEdit->append(alertHtml);
        sendRequest(command);
        commandTextEdit->setText("");
    }
}

void Dialog::sendRequest(QString request)
{
    QByteArray byteArray = request.toUtf8();
    const char* cString = byteArray.constData();
    tcpSocket->write(cString);
    qDebug() << cString << "written to socket";
}

void Dialog::openConnection()
{
    qDebug() << "ENTERING" << __func__;
    if (!tcpSocket->isOpen()){
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
    currentMessage.append(QString(data));

    if (currentMessage.contains('\n')){
        lastMessage = QString(currentMessage);
        currentMessage = QString("");
        parseJsonResponse(QString(lastMessage));
        consoleTextEdit->verticalScrollBar()->setSliderPosition(consoleTextEdit->verticalScrollBar()->maximum());
        getFortuneButton->setEnabled(true);
    }
    delete(data);
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

    getFortuneButton->setEnabled(true);
}

void Dialog::enableGetFortuneButton()
{
    getFortuneButton->setEnabled((!networkSession || networkSession->isOpen()) && !hostCombo->text().isEmpty() && !portLineEdit->text().isEmpty());
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

void Dialog::parseEventMessage(QScriptValue sc)
{
    qDebug() << "ENTERING" << __func__;
    if (sc.property("event").isArray() ){
        qDebug() << "IS ARRAY";
        QScriptValueIterator it(sc.property("event"));
        while (it.hasNext()) {
            it.next();
            QScriptValue current = it.value();
            if (current.property("id").toString() == "info"){
                parseTrackMetadata(current.property("item"));
            }
        }
    }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseActionItem(QScriptValue value)
{
    qDebug() << "ENTERING" << __func__;
    qDebug() << value.property("id").toString();
    qDebug() << value.property("url").toString();
    if ( value.property("id").toString() == "next"){
        nextActionItem->setProperty("url",value.property("url").toString());
    } else if ( value.property("id").toString() == "play"){
        playActionItem->setProperty("url",value.property("url").toString());
    } else if ( value.property("id").toString() == "pause"){
       // pauseActionItem->setProperty("url",value.property("url").toString());
    }
    qDebug() << "EXITING" << __func__;
}

void Dialog::parseTrackMetadata(QScriptValue value){
    qDebug() << "ENTERING" << __func__;
    QScriptValue item = value;
    qDebug() << item.property("title").toString();
    qDebug() << item.property("longDescription").toString();
    qDebug() << item.property("description").toString();
    labels[0]->setText(item.property("title").toString());
    labels[1]->setText(tr("<b>%1</b>").arg(item.property("description").toString()));
    labels[2]->setText(item.property("longDescription").toString());
    QUrl url(item.property("icon").toString());
    QNetworkRequest request(url);
    m_netwManager->get(request);
    qDebug() << "EXITING" << __func__;
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
