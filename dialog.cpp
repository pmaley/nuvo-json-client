#include <QtWidgets>
#include <QtNetwork>
#include <QScriptValueIterator>
#include "dialog.h"


Dialog::Dialog()
{
    createMenu();
    createTransportControlsBox();
    createNowPlayingBox();
    createConsoleBox();


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(nowPlayingBox);
    mainLayout->addWidget(transportControlsBox);
    mainLayout->addWidget(consoleBox);
    setLayout(mainLayout);

    setWindowTitle(tr("NWAS API Controller"));
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

    QStringList transportControls = QStringList() << "dislike_normal@2x.png" << "like_normal@2x.png"
                                                  << "prev_normal@2x.png" << "play_normal@2x.png" << "next_normal@2x.png";
    QStringList transportNames = QStringList() << "dislike" << "like" << "prev" << "play" << "next";

    for (int i = 0; i < transportControls.length(); ++i) {
        buttons[i] = new QPushButton();
        QPixmap pixmap(tr(":/images/player_icons/%1").arg(transportControls[i]));
        QIcon ButtonIcon(pixmap);
        buttons[i]->setIcon(ButtonIcon);
        buttons[i]->setIconSize(pixmap.rect().size());
        connect(buttons[i],SIGNAL(clicked()),this,SLOT(testFunction()));
        layout->addWidget(buttons[i]);
    }
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
    QLabel *imageLabel = new QLabel();
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
    consoleWindow->append(tr("PRESSED BUTTON. paused = %2").arg(paused));
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
    consoleWindow->repaint();
}

void Dialog::createConsoleBox(){
    consoleBox = new QGroupBox();
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));

    //hostCombo = new QLineEdit("mps4e.nuvotechnologies.com");
    hostCombo = new QLineEdit("127.0.0.1");
    //portLineEdit = new QLineEdit("23");
    portLineEdit = new QLineEdit("2000");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    commandTextEdit = new QTextEdit;
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
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
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

void Dialog::requestNewFortune() {
    qDebug() << "In requestNewFortune()";
    getFortuneButton->setEnabled(false);
    blockSize = 0;
    if (!tcpSocket->isOpen()){
        openConnection();
    } else {
        QString command = commandTextEdit->toPlainText();
        QByteArray byteArray = command.toUtf8();
        const char* cString = byteArray.constData();
        tcpSocket->write(cString);
        qDebug() << cString << "written to socket";
        commandTextEdit->setText("");
    }
}

void Dialog::openConnection() {
    qDebug() << "In openConnection()";
    if (!tcpSocket->isOpen()){
        tcpSocket->abort();
        tcpSocket->connectToHost(hostCombo->text(), portLineEdit->text().toInt());
    }
}

void Dialog::readFortune()
{
    qDebug() << "In readFortune()";
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    blockSize = tcpSocket->bytesAvailable()/sizeof(char);
    char * data = new char[blockSize];
    in.readRawData(data,blockSize);
    data[blockSize] = '\0';
    if ( strlen(data) != blockSize )
        qDebug() << "DATA SIZE DOES NOT MATCH BLOCK SIZE";
    consoleTextEdit->append(data);
    qDebug() << data;
    parseJson(QString(data));
    consoleTextEdit->verticalScrollBar()->setSliderPosition(consoleTextEdit->verticalScrollBar()->maximum());
    getFortuneButton->setEnabled(true);
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

void Dialog::parseJson(QString result){
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(result) + ")");
    QScriptValueIterator it(sc);
    while (it.hasNext()) {
        it.next();
        qDebug() << it.name() << ": " << it.value().toString();
    }
}
