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
    transportControlsBox = new QGroupBox("Transport Controls");
    QHBoxLayout *layout = new QHBoxLayout;

    QStringList transportControls = QStringList() << "thumbs up" << "thumbs down"
                                                  << "Prev" << "Play" << "Pause" << "Next";

    for (int i = 0; i < transportControls.length(); ++i) {
        buttons[i] = new QPushButton(transportControls[i]);
        layout->addWidget(buttons[i]);
    }
    transportControlsBox->setLayout(layout);
}

void Dialog::createMetadataBox()
{
    metadataBox = new QGroupBox(tr("Track Metadata"));
    QVBoxLayout *layout = new QVBoxLayout;

    QStringList trackMetadata = QStringList() << "Fear Before the March of Flames Radio" << "Dog Sized Bird"
                                                  << "Fear Before the March of Flames - The Always Open Mouth";

    for (int i = 0; i < trackMetadata.length(); ++i) {
        labels[i] = new QLabel(trackMetadata[i]);
        layout->addWidget(labels[i]);
    }
    metadataBox->setLayout(layout);
}

void Dialog::createNowPlayingBox()
{
    nowPlayingBox = new QGroupBox(tr("Now Playing"));
    QGridLayout *layout = new QGridLayout;
    QPixmap *image = new QPixmap("/Users/bradyt/Documents/nuvo-json-client/img/aom.jpg");
    image = new QPixmap(image->scaledToWidth(200));
    QLabel *imageLabel = new QLabel();
    imageLabel->setPixmap(*image);
    layout->addWidget(imageLabel,0,0);
    createMetadataBox();
    layout->addWidget(metadataBox,0,1);
    layout->setColumnStretch(1, 10);
    layout->setColumnStretch(2, 30);
    nowPlayingBox->setLayout(layout);
}

void Dialog::createConsoleWindow()
{
    consoleWindow = new QTextEdit();
    consoleWindow->setPlainText("Test console output");
    consoleWindow->setReadOnly(true);
}
