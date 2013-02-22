#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QScriptEngine>
#include <QHash>

#include "nuvogroup.h"
#include "nuvotransportcontrol.h"

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
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog();
    void parseJsonResponse(QString json);

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


    enum { NumGridRows = 3, NumButtons = 5 };

    NuvoGroup* group;

    QMenuBar *menuBar;
    QGroupBox *transportControlsBox;
    QGroupBox *metadataBox;
    QGroupBox *nowPlayingBox;
    QGroupBox *consoleBox;
    QTextEdit *consoleWindow;
    QLabel *labels[NumGridRows];

    QPushButton *buttons[NumButtons];
    QPushButton *nextButton, *playButton, *pauseButton;

    QDialogButtonBox *buttonBox;
    bool paused = false;
    QLabel *hostLabel;
    QLabel *portLabel;
    QLabel *imageLabel;
    QLineEdit *hostCombo;
    QLineEdit *portLineEdit;
    QTextEdit *commandTextEdit;
    QTextEdit *consoleTextEdit;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox2;

    QMenu *fileMenu;
    QAction *exitAction;

    QTcpSocket *tcpSocket;
    QString currentFortune;
    quint16 blockSize;

    QString currentMessage;
    QString lastMessage;

    QNetworkSession *networkSession;
    QNetworkAccessManager *m_netwManager;

    NuvoTransportControl *nextActionItem, *playActionItem, *pauseActionItem;

public slots:
    void testFunction();

private slots:
    void requestNewFortune();
    void messageReceived();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableGetFortuneButton();
    void openConnection();
    void slot_netwManagerFinished(QNetworkReply *reply);
    void nextButtonPressed();
    void playButtonPressed();
    void sendRequest(QString request);
};

#endif // DIALOG_H
