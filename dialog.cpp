
#include "dialog.h"

Dialog::Dialog()
{
    nuvo = new NuvoApiClient();
    connect(nuvo, SIGNAL(albumArtChanged()), this, SLOT(updateAlbumArt()));
    connect(nuvo, SIGNAL(progressBarChanged()), this, SLOT(updateProgressBar()));
    connect(nuvo->tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onConnectionStateChange()));
    connect(nuvo, SIGNAL(raiseError(const QString &)), this, SLOT(displayErrorMessage(const QString &)));
    connect(nuvo, SIGNAL(displayText(const QString &)), this, SLOT(displayLog(const QString &)));
    connect(nuvo, SIGNAL(transportChanged()), this, SLOT(updateTransportControls()));
    connect(nuvo, SIGNAL(metadataChanged()), this, SLOT(updateMetadata()));
    connect(nuvo, SIGNAL(browseDataChanged()), this, SLOT(updateBrowseWindow()));
    connect(nuvo, SIGNAL(refreshDisplay()), this, SLOT(redisplay()));
    connect(nuvo, SIGNAL(zoneListChanged()), this, SLOT(updateZonesList()));
    connect(nuvo, SIGNAL(browseListCleared()), this, SLOT(clearBrowseWindow()));

    image = NULL;

    createMenu();
    createTransportControlsBox();
    createNowPlayingBox();
    createConsoleBox();
    createBrowseBox();

    progressBarTimer = new QTimer(this);
    progressBarTimer->start(1000);
    connect(progressBarTimer, SIGNAL(timeout()), this, SLOT(incrementProgressBar()));
    connect(volumeSlider, SIGNAL(sliderReleased()), this, SLOT(volumeSliderAdjusted()));

    mainLayout = new QGridLayout();
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(nowPlayingBox,0,0);
    mainLayout->addWidget(consoleBox,1,0,1,2);
    mainLayout->addWidget(browseBox,0,1,1,1);

    mainLayout->setRowStretch(1,1000);
    setLayout(mainLayout);
    mainLayout->setColumnStretch(1,200);

    setWindowTitle(tr("NWAS API Controller"));
    adjustSize();

    connect(backBrowseButton, SIGNAL(clicked()), nuvo, SLOT(browseUpOne()));
    connect(zonesCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(zoneSelected(QString)));

    BonjourBrowser *browser = new BonjourBrowser(this);
    browser->browseForServiceType("_nuvoplayer._tcp");
    connect(browser,
                SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
                this, SLOT(updateRecords(const QList<BonjourRecord> &)));

}

void Dialog::updateRecords(
      const QList<BonjourRecord> &list)
{
    foreach (BonjourRecord record, list) {
        qDebug() << record.serviceName;
        QVariant variant;
        variant.setValue(record);

        if (!resolver) {
            resolver = new BonjourResolver(this);
            connect(resolver, SIGNAL(bonjourRecordResolved(const QHostInfo &, int)),
                this, SLOT(dnsRecordResolved(const QHostInfo &, int)));
        }
        resolver->resolveBonjourRecord(variant.value<BonjourRecord>());
    }
}

Dialog::~Dialog()
{
    delete nuvo;
    delete progressBarTimer;
    delete mainLayout;
}

void Dialog::dnsRecordResolved(const QHostInfo & info, int num){
    QString address(info.addresses().takeFirst().toString());
    qDebug() << address;
    //stCombo->setText(address);
    nuvo->connectToHost(address, 4747);
}

void Dialog::createBrowseBox()
{
    browseBox = new QGroupBox();
    QGridLayout *layout = new QGridLayout();
    zonesCombo = new QComboBox();
    backBrowseButton = new QPushButton(tr("^"));

    browseView = new QTreeView;
    browseView->setRootIsDecorated(false);
    browseView->setAlternatingRowColors(true);
    browseModel = new QStandardItemModel(0, 1);
    browseView->setModel(browseModel);
    connect(browseView, SIGNAL(clicked(QModelIndex)), this, SLOT(browseItemClicked(QModelIndex)));

    layout->addWidget(backBrowseButton, 0, 0, 1, 1);
    layout->addWidget(zonesCombo, 0, 1, 1, 4);
    layout->addWidget(browseView, 1, 0, 1, 5);
    browseBox->setLayout(layout);
}


void Dialog::createMenu()
{
    menuBar = new QMenuBar;
    fileMenu = new QMenu(tr("&File"), this);
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);
    //connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}

void Dialog::createTransportControlsBox()
{
    transportControlsBox = new QGroupBox();
    transportControlsBox2 = new QGroupBox();
    QHBoxLayout *layout = new QHBoxLayout;
    QHBoxLayout *layout2 = new QHBoxLayout();

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
    muteButton->setDefault(false);
    muteButton->setCheckable(true);
    connect(muteButton,SIGNAL(clicked()),this,SLOT(muteButtonPressed()));

    // Create shuffle button
    shuffleButton = new QPushButton();
    QPixmap pixmap9(":/images/player_icons/player_icon_shuffle_off_normal.png");
    QIcon buttonIcon9(pixmap9);
    shuffleButton->setIcon(buttonIcon9);
    shuffleButton->setEnabled(false);
    shuffleButton->setCheckable(true);
    connect(shuffleButton,SIGNAL(clicked()),this,SLOT(shuffleButtonPressed()));

    // Create repeat button
    repeatButton = new QPushButton();
    QPixmap pixmap10(":/images/player_icons/player_icon_repeat_off_normal.png");
    QIcon buttonIcon10(pixmap10);
    repeatButton->setIcon(buttonIcon10);
    repeatButton->setEnabled(false);
    connect(repeatButton,SIGNAL(clicked()),this,SLOT(repeatButtonPressed()));

    // Create hide button
    hideButton = new QPushButton();
    connect(hideButton,SIGNAL(clicked()),this,SLOT(hideButtonPressed()));

    // Create volume slider
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setMaximum(100);

    layout->addWidget(likeButton);
    layout->addWidget(dislikeButton);
    layout->addWidget(prevButton);
    layout->addWidget(stopButton);
    layout->addWidget(pauseButton);
    layout->addWidget(playButton);
    layout->addWidget(nextButton);
    transportControlsBox->setLayout(layout);

    layout2->addWidget(muteButton);
    layout2->addWidget(volumeSlider);
    layout2->addWidget(shuffleButton);
    layout2->addWidget(repeatButton);
    layout2->addWidget(hideButton);
    transportControlsBox2->setLayout(layout2);
}

void Dialog::createMetadataBox()
{
    metadataBox = new QGroupBox();
    metadataBox->setFixedWidth(400);
    QVBoxLayout *layout = new QVBoxLayout;

    QPixmap *temp = new QPixmap(":/images/aom.jpg");
    //image = new QPixmap(temp->scaledToHeight(100));
    QPixmap image(temp->scaledToHeight(100));
    delete temp;

    imageLabel = new QLabel();
    //imageLabel->setPixmap(*image);
    imageLabel->setPixmap(image);
    layout->addWidget(imageLabel);
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

    createMetadataBox();
    trackProgressBar = new QProgressBar();

    layout->addWidget(imageLabel,0,0,1,1);
    layout->addWidget(metadataBox,0,1,1,1);
    layout->addWidget(trackProgressBar,1,0,1,2);
    layout->addWidget(transportControlsBox2,3,0,1,2);
    layout->addWidget(transportControlsBox,4,0,1,2);

    layout->setColumnStretch(1, 10);
    layout->setColumnStretch(2, 10);
    nowPlayingBox->setLayout(layout);
}

void Dialog::prevButtonPressed(){ nuvo->invokeAction("prev"); }
void Dialog::nextButtonPressed(){ nuvo->invokeAction("next"); }
void Dialog::playButtonPressed(){ nuvo->invokeAction("play"); }
void Dialog::pauseButtonPressed(){ nuvo->invokeAction("pause"); }
void Dialog::stopButtonPressed(){ nuvo->invokeAction("stop"); }
void Dialog::likeButtonPressed(){ nuvo->invokeAction("like"); }
void Dialog::dislikeButtonPressed(){ nuvo->invokeAction("dislike"); }
void Dialog::volumeSliderAdjusted(){ nuvo->updateVolume("volume", volumeSlider->value()); }
void Dialog::muteButtonPressed(){ nuvo->toggleValue("volume"); }
void Dialog::shuffleButtonPressed(){ nuvo->toggleValue("shuffle"); }
void Dialog::repeatButtonPressed(){ nuvo->toggleValue("repeat"); }
void Dialog::hideButtonPressed()
{
    consoleBox->setVisible(!consoleBox->isVisible());
    adjustSize();
}

void Dialog::createConsoleBox()
{
    consoleBox = new QGroupBox();
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));

    hostCombo = new QLineEdit("");
    hostCombo->setFixedWidth(100);
    portLineEdit = new QLineEdit("4747");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    portLineEdit->setFixedWidth(50);
    commandTextEdit = new QTextEdit();
    commandTextEdit->setFixedHeight(50);
    consoleTextEdit = new QTextEdit;
    consoleTextEdit->setReadOnly(true);

    hostLabel->setBuddy(hostCombo);
    portLabel->setBuddy(portLineEdit);

    sendButton = new QPushButton(tr("Send"));
    sendButton->setEnabled(false);
    quitButton = new QPushButton(tr("Quit"));
    connectButton = new QPushButton(tr("Connect"));
    disconnectButton = new QPushButton(tr("Disconnect"));
    disconnectButton->setEnabled(false);

    buttonBox2 = new QDialogButtonBox;
    buttonBox2->addButton(sendButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(connectButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(disconnectButton, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(quitButton, QDialogButtonBox::RejectRole);

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
    consoleBox->setVisible(false);
}

void Dialog::generateNewRequest()
{
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


void Dialog::redisplay(){
    updateVolume();
    updateMetadata();
    updateTransportControls();
}

void Dialog::updateZonesList()
{
    QList<QString> items = nuvo->getZonesList();
    zonesCombo->clear();
    for (int i = 0; i < items.size(); i++){
        zonesCombo->addItem(QString(items.at(i)));
    }
    redisplay();
}

void Dialog::updateVolume()
{
    qDebug() << "ENTERING" << __func__;
    volumeSlider->setValue(nuvo->getAvValue("volume"));
    qDebug() << "EXITING" << __func__;
}

void Dialog::updateProgressBar(){
    trackProgressBar->setMaximum(nuvo->progressMax);
    trackProgressBar->setValue(nuvo->progressPos);
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
    likeButton->setEnabled(nuvo->getItemActive("like"));
    dislikeButton->setEnabled(nuvo->getItemActive("dislike"));
    prevButton->setEnabled(nuvo->getItemActive("prev"));
    nextButton->setEnabled(nuvo->getItemActive("next"));
    stopButton->setEnabled(nuvo->getItemActive("stop"));
    pauseButton->setEnabled(nuvo->getItemActive("pause"));
    playButton->setEnabled(nuvo->getItemActive("play"));
    muteButton->setEnabled(nuvo->getItemActive("volume"));
    muteButton->setChecked(nuvo->getMuteState());
    muteButton->setDefault(false);
    shuffleButton->setEnabled(nuvo->getItemActive("shuffle"));
    shuffleButton->setChecked(nuvo->getShuffleState());
    repeatButton->setEnabled(nuvo->getItemActive("repeat"));
    repeatButton->setChecked(nuvo->getRepeatState());
}
void Dialog::clearBrowseWindow()
{
    browseModel->setRowCount(0);
}

void Dialog::updateBrowseWindow()
{
    QList<QString> items = nuvo->getBrowseItems();
    browseModel->setRowCount(0);
    for (int i = 0; i < items.size(); i++) {
        browseModel->insertRow(i);
        browseModel->setData(browseModel->index(i, 0), QString(items.at(i)) );
    }
    QStringList a(nuvo->getBrowseHeader());
    browseModel->setHorizontalHeaderLabels(a);
}

void Dialog::browseItemClicked(QModelIndex index)
{
    nuvo->browseClick(index.row());
}

void Dialog::zoneSelected(QString zone)
{
    nuvo->changeCurrentZone(zone);
}



