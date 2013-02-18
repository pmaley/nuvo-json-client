#include <QtWidgets>
#include "dialog.h"


Dialog::Dialog()
{
    createMenu();
    createTransportControlsBox();
    createNowPlayingBox();
    createConsoleWindow();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(nowPlayingBox);
    mainLayout->addWidget(transportControlsBox);
    mainLayout->addWidget(consoleWindow);
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
    image = new QPixmap(image->scaledToWidth(200));
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

void Dialog::createConsoleWindow()
{
    consoleWindow = new QTextEdit();
    consoleWindow->setPlainText("Test console output");
    consoleWindow->setReadOnly(true);
}

void Dialog::testFunction(){
    consoleWindow->append("Test button press");
    consoleWindow->repaint();
}
