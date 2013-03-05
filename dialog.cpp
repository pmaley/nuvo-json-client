
#include "dialog.h"
#include "NuvoActionItem.h"

Dialog::Dialog()
{
    nuvo = new NuvoApiClient();
    //connect(nuvo, SIGNAL(avChanged()), this, SLOT(redisplay()));
    connect(nuvo, SIGNAL(albumArtChanged()), this, SLOT(updateAlbumArt()));
    connect(nuvo, SIGNAL(progressBarChanged()), this, SLOT(updateProgressBar()));
    connect(nuvo->tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onConnectionStateChange()));
    connect(nuvo, SIGNAL(raiseError(const QString &)), this, SLOT(displayErrorMessage(const QString &)));
    connect(nuvo, SIGNAL(displayText(const QString &)), this, SLOT(displayLog(const QString &)));
    connect(nuvo, SIGNAL(transportChanged()), this, SLOT(updateTransportControls()));
    connect(nuvo, SIGNAL(volumeChanged()), this, SLOT(updateVolume()));
    connect(nuvo, SIGNAL(metadataChanged()), this, SLOT(updateMetadata()));
    connect(nuvo, SIGNAL(avStateChanged()), this, SLOT(onAvStateChange()));
    connect(nuvo, SIGNAL(browseDataChanged()), this, SLOT(updateBrowseWindow()));

    createMenu();
    createTransportControlsBox();
    createNowPlayingBox();
    createConsoleBox();

    browseView = new QTreeView;
    browseView->setRootIsDecorated(false);
    browseView->setAlternatingRowColors(true);
    //browseModel = new QStandardItemModel(0, 1);
    browseModel = new QStandardItemModel(0,2);
    browseView->setModel(browseModel);
    connect(browseView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(browseItemClicked(QModelIndex)));

    progressBarTimer = new QTimer(this);
    progressBarTimer->start(1000);
    connect(progressBarTimer, SIGNAL(timeout()), this, SLOT(incrementProgressBar()));
    connect(volumeSlider, SIGNAL(sliderReleased()), this, SLOT(volumeSliderAdjusted()));

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(nowPlayingBox,0,0);
    mainLayout->addWidget(consoleBox,1,0,1,2);
    mainLayout->addWidget(browseView,0,1,1,1);
    mainLayout->setRowStretch(1,1000);
    setLayout(mainLayout);

    setWindowTitle(tr("NWAS API Controller"));
    resize(1000,1000);

    connect(browseButton, SIGNAL(clicked()), nuvo, SLOT(browseContainer()));

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

void Dialog::createMetadataBox()
{
    metadataBox = new QGroupBox();
    QVBoxLayout *layout = new QVBoxLayout;
    for (int i = 0; i < NumGridRows; ++i) {
        labels[i] = new QLabel();
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

    createMetadataBox();

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

void Dialog::prevButtonPressed(){ nuvo->invokeAction(nuvo->prevActionItem); }
void Dialog::nextButtonPressed(){ nuvo->invokeAction(nuvo->nextActionItem); }
void Dialog::playButtonPressed(){  nuvo->invokeAction(nuvo->playActionItem); }
void Dialog::pauseButtonPressed(){ nuvo->invokeAction(nuvo->pauseActionItem); }
void Dialog::stopButtonPressed(){ nuvo->invokeAction(nuvo->stopActionItem); }
void Dialog::likeButtonPressed(){ nuvo->invokeAction(nuvo->likeActionItem); }
void Dialog::dislikeButtonPressed(){ nuvo->invokeAction(nuvo->dislikeActionItem); }
void Dialog::volumeSliderAdjusted(){ nuvo->updateValue(nuvo->volumeActionItem, volumeSlider->value()); }
void Dialog::muteButtonPressed(){ nuvo->toggleValue(nuvo->muteActionItem); }
void Dialog::shuffleButtonPressed(){ nuvo->toggleValue(nuvo->shuffleActionItem); }
void Dialog::repeatButtonPressed(){ nuvo->toggleValue(nuvo->repeatActionItem); }

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
    sendButton->setEnabled(false);
    quitButton = new QPushButton(tr("Quit"));
    connectButton = new QPushButton(tr("Connect"));
    disconnectButton = new QPushButton(tr("Disconnect"));
    disconnectButton->setEnabled(false);
    browseButton = new QPushButton(tr("Browse"));


    buttonBox2 = new QDialogButtonBox;
    buttonBox2->addButton(sendButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(connectButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(disconnectButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(quitButton, QDialogButtonBox::RejectRole);
    buttonBox2->addButton(browseButton, QDialogButtonBox::ActionRole);

    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectToHost2()));
    connect(disconnectButton, SIGNAL(clicked()), nuvo, SLOT(disconnectFromHost()));
    connect(sendButton, SIGNAL(clicked()), this, SLOT(generateNewRequest()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

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

void Dialog::generateNewRequest()
{
    qDebug() << "ENTERING" << __func__;
    //sendButton->setEnabled(false);
    nuvo->sendRequest(QString(commandTextEdit->toPlainText()));
    commandTextEdit->setText("");
}

void Dialog::displayLog(const QString &err){
    consoleTextEdit->append(QString(tr("%1").arg(err)));
    consoleTextEdit->verticalScrollBar()->setSliderPosition(consoleTextEdit->verticalScrollBar()->maximum());
}

void Dialog::connectToHost2(){
    nuvo->connectToHost(hostCombo->text(), portLineEdit->text().toInt());
}

void Dialog::onAvStateChange(){
    qDebug() << "AvState Change:" << nuvo->avState;
}

void Dialog::onConnectionStateChange(){
    qDebug() << "Connection State Change";
    if (!nuvo->tcpSocket->isOpen()){
        connectButton->setEnabled(true);
        sendButton->setEnabled(false);
        disconnectButton->setEnabled(false);
        portLineEdit->setEnabled(true);
        hostCombo->setEnabled(true);
    } else {
        connectButton->setEnabled(false);
        sendButton->setEnabled(true);
        disconnectButton->setEnabled(true);
        portLineEdit->setEnabled(false);
        hostCombo->setEnabled(false);
    }
}

void Dialog::incrementProgressBar(){
    if (nuvo->avState == "playing" && trackProgressBar->maximum() > 0) {
        trackProgressBar->setValue(trackProgressBar->value()+1);
    }
}
void Dialog::updateProgressBar(){
    trackProgressBar->setMaximum(nuvo->progressMax);
    trackProgressBar->setValue(nuvo->progressPos);
}

void Dialog::redisplay(){
    //TODO update screen w/ current client's info
    qDebug() << "REDISPLAY";
    updateVolume();
    updateMetadata();
    updateAlbumArt();
    updateTransportControls();
}

void Dialog::updateVolume()
{
    volumeSlider->setMaximum(nuvo->volumeMax);
    volumeSlider->setValue(nuvo->volume);
}

void Dialog::updateMetadata()
{
    labels[0]->setText(nuvo->metadata1);
    labels[1]->setText(nuvo->metadata2);
    labels[2]->setText(nuvo->metadata3);
}

void Dialog::displayErrorMessage(const QString &err){
    QMessageBox::information(this, tr("NuVo API"),tr("The following error occurred: %1.").arg(err));
}

void Dialog::updateAlbumArt(){
    imageLabel->setPixmap(nuvo->albumArt.scaledToHeight(100));
}

void Dialog::updateTransportControls()
{
    likeButton->setEnabled(nuvo->likeActionItem->property("active").toBool());
    dislikeButton->setEnabled(nuvo->dislikeActionItem->property("active").toBool());
    prevButton->setEnabled(nuvo->prevActionItem->property("active").toBool());
    nextButton->setEnabled(nuvo->nextActionItem->property("active").toBool());
    stopButton->setEnabled(nuvo->stopActionItem->property("active").toBool());
    pauseButton->setEnabled(nuvo->pauseActionItem->property("active").toBool());
    playButton->setEnabled(nuvo->playActionItem->property("active").toBool());
    muteButton->setEnabled(nuvo->muteActionItem->property("active").toBool());
    shuffleButton->setEnabled(nuvo->shuffleActionItem->property("active").toBool());
    repeatButton->setEnabled(nuvo->repeatActionItem->property("active").toBool());
}

void Dialog::updateBrowseWindow()
{
    qDebug() << "ENTERING" << __func__;
    browseModel->setRowCount(0);
//    int row = browseModel->rowCount();
    qDebug() << nuvo->browseList.size();
    qDebug() << browseModel->rowCount();
    for (int i = 0; i < nuvo->browseList.size(); i++) {
        qDebug() << i << nuvo->browseList.at(i)->title;
        browseModel->insertRow(i);
        //browseModel->item(i,0)->setEditable(false);
        browseModel->setData(browseModel->index(i, 0), nuvo->browseList.at(i)->title);
        browseModel->setData(browseModel->index(i, 1), i);
    }
    qDebug() << browseModel->rowCount();
    qDebug() << "EXITING" << __func__;
}

void Dialog::browseItemClicked(QModelIndex index)
{
    qDebug() << "ENTERING" << __func__;
    //QStandardItem *item = browseModel->itemFromIndex(index);
    qDebug() << browseModel->itemFromIndex(index)->text();
    qDebug() << browseModel->itemFromIndex(index)->data();
    int i = browseModel->itemFromIndex(index)->text().toInt();
    qDebug() << i;
    NuvoContainerItem *item = nuvo->browseList.at(i);
    qDebug() << item->title;
    qDebug() << item->url;
    if (item->av == true)
        //nuvo->invokeAction();
        qDebug() << "TODO";
    else
        nuvo->browseContainer(item->url);
    qDebug() << "EXITING" << __func__;
}



