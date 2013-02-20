#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QScriptEngine>
#include <QHash>

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
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog();
    void parseJson(QString json);

private:
    void createMenu();
    void createTransportControlsBox();
    void createConsoleBox();
    void createMetadataBox(QStringList trackMetadata);
    void createNowPlayingBox();

    enum { NumGridRows = 3, NumButtons = 5 };

    QMenuBar *menuBar;
    QGroupBox *transportControlsBox;
    QGroupBox *metadataBox;
    QGroupBox *nowPlayingBox;
    QGroupBox *consoleBox;
    QTextEdit *consoleWindow;
    QLabel *labels[NumGridRows];
    QPushButton *buttons[NumButtons];
    QDialogButtonBox *buttonBox;
    bool paused = false;
    QLabel *hostLabel;
    QLabel *portLabel;
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

    QNetworkSession *networkSession;

public slots:
    void testFunction();

private slots:
    void requestNewFortune();
    void readFortune();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableGetFortuneButton();

};

#endif // DIALOG_H
