#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QScriptEngine>
#include <QHash>
#include <QQueue>
#include <QProgressBar>
#include <QTimer>
#include <QStandardItemModel>

#include "nuvoactionitem.h"
#include "nuvoapiclient.h"

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QGridLayout;
class QTcpSocket;
class QNetworkSession;
class QNetworkAccessManager;
class QNetworkReply;
class QSlider;
class QTreeView;
class QTreeWidgetItem;
class QStandardItemModel;
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog();


signals:

     void connectionStateChanged();

private:
    void createMenu();
    void createTransportControlsBox();
    void createConsoleBox();
    void createMetadataBox(QStringList trackMetadata);
    void createNowPlayingBox();






    enum { NumGridRows = 3, NumButtons = 5 };
    NuvoApiClient* nuvo;
    QMenuBar *menuBar;
    QGroupBox *transportControlsBox;
    QGroupBox *metadataBox;
    QGroupBox *nowPlayingBox;
    QGroupBox *consoleBox;
    QTextEdit *consoleWindow;
    QLabel *labels[NumGridRows];

    QPushButton *buttons[NumButtons];
    QPushButton *nextButton, *playButton, *pauseButton,
                *prevButton, *stopButton, *likeButton,
                *dislikeButton, *shuffleButton, *repeatButton,
                *muteButton;

    QDialogButtonBox *buttonBox;
    QLabel *hostLabel;
    QLabel *portLabel;
    QLabel *imageLabel;
    QLineEdit *hostCombo;
    QLineEdit *portLineEdit;
    QTextEdit *commandTextEdit;
    QTextEdit *consoleTextEdit;
    QPushButton *sendButton, *quitButton, *connectButton,
                *disconnectButton;
    QDialogButtonBox *buttonBox2;
    QSlider *volumeSlider;
    QProgressBar *trackProgressBar;
    QTimer *progressBarTimer;
    QTreeView *browseView;
    QStandardItemModel *browseModel;


    QMenu *fileMenu;
    QAction *exitAction;


    QString currentFortune;
    quint16 blockSize;


    QString lastMessage;





    QQueue<QString> messageQueue;

public slots:

private slots:
    void generateNewRequest();
    void prevButtonPressed();
    void nextButtonPressed();
    void playButtonPressed();
    void pauseButtonPressed();
    void stopButtonPressed();
    void likeButtonPressed();
    void dislikeButtonPressed();
    void volumeSliderAdjusted();
    void shuffleButtonPressed();
    void repeatButtonPressed();
    void muteButtonPressed();

    void onAvStateChange();
    void onConnectionStateChange();
    void incrementProgressBar();
    void connectToHost2();
    void redisplay();
    void displayErrorMessage();
    void updateAlbumArt();
};

#endif // DIALOG_H
