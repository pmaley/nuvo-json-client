#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QScriptEngine>
#include <QHash>
#include <QQueue>
#include <QProgressBar>

#include "nuvoactionitem.h"

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
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog();
    void parseJsonResponse(QString json);

signals:
     void avStateChanged();

private:
    void createMenu();
    void createTransportControlsBox();
    void createConsoleBox();
    void createMetadataBox(QStringList trackMetadata);
    void createNowPlayingBox();
    void parseReplyMessage(QScriptValue sc);
    void parseEventMessage(QScriptValue sc);
    void parseTrackMetadata(QScriptValue value);
    void parseActionItem(QScriptValue value);
    void parseValueItem(QScriptValue value);
    void parseChildValueChangedMessage(QScriptValue value);
    void parseChildItemChangedMessage(QScriptValue value);
    void parseChildInsertedMessage(QScriptValue value);
    void parseChildRemovedMessage(QScriptValue value);
    void invokeAction(NuvoActionItem *action);
    void updateValue(NuvoActionItem *actionItem, int value);
    void toggleValue(NuvoActionItem *actionItem);
    NuvoActionItem* findActionItem(QString id);

    enum { NumGridRows = 3, NumButtons = 5 };

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
    QPushButton *sendButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox2;
    QSlider *volumeSlider;
    QProgressBar *trackProgressBar;

    QMenu *fileMenu;
    QAction *exitAction;

    QTcpSocket *tcpSocket;
    QString currentFortune;
    quint16 blockSize;

    QString currentMessage;
    QString lastMessage;
    QString avState;

    QNetworkSession *networkSession;
    QNetworkAccessManager *m_netwManager;

    NuvoActionItem *nextActionItem, *playActionItem, *pauseActionItem,
                            *prevActionItem, *stopActionItem, *likeActionItem,
                            *dislikeActionItem, *volumeActionItem, *muteActionItem,
                            *shuffleActionItem, *repeatActionItem;

    QQueue<QString> messageQueue;

public slots:

private slots:
    void requestNewFortune();
    void messageReceived();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableSendButton();
    void openConnection();
    void slot_netwManagerFinished(QNetworkReply *reply);
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
    void sendRequest(QString request);
    void onAvStateChange();
};

#endif // DIALOG_H
