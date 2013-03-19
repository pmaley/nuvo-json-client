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

#include "nuvoapiclient.h"
#include "bonjourbrowser.h"
#include "bonjourresolver.h"
#include "overlay.h"

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

class Dialog : public QWidget
{
    Q_OBJECT

public:
    Dialog();
    ~Dialog();

signals:
     void connectionStateChanged();

private:
    void createMenu();
    void createTransportControlsBox();
    void createConsoleBox();
    void createMetadataBox();
    void createNowPlayingBox();
    void createBrowseBox();

    Overlay *overlay, *statusOverlay;
    void resizeEvent(QResizeEvent *event);

    enum { NumGridRows = 3, NumButtons = 5 };
    NuvoApiClient* nuvo;
    QPixmap *image;

    QSplitter *splitter;

    QGridLayout *mainLayout;
    QTreeWidget *treeWidget;
    BonjourResolver *resolver;

    QMenuBar *menuBar;
    QGroupBox *transportControlsBox, *transportControlsBox2, *metadataBox, *nowPlayingBox, *consoleBox, *browseBox;
    QTextEdit *consoleWindow;
    QLabel *labels[NumGridRows];

    QComboBox * zonesCombo;

    QPushButton *buttons[NumButtons];
    QPushButton *nextButton, *playButton, *pauseButton,
                *prevButton, *stopButton, *likeButton,
                *dislikeButton, *shuffleButton, *repeatButton,
                *muteButton;
    QPushButton *backBrowseButton;
    QPushButton *clearButton;

    QDialogButtonBox *buttonBox, *buttonBox2;
    QLabel *hostLabel, *portLabel, *imageLabel;
    QLineEdit *hostCombo, *portLineEdit;
    QTextEdit *commandTextEdit, *consoleTextEdit;
    QPushButton *sendButton, *quitButton, *connectButton,
                *disconnectButton;

    QSlider *volumeSlider;
    QProgressBar *trackProgressBar;
    QTimer *progressBarTimer;
    QTreeView *browseView;
    QStandardItemModel *browseModel;


    QMenu *fileMenu;
    QAction *exitAction, *debugAction;

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
    void hideButtonPressed();
    void zoneSelected(QString zone);
    void clearBrowseWindow();

    void onConnectionStateChange();
    void incrementProgressBar();
    void connectToHost2();
    void redisplay();
    void displayUnformattedText(const QString &err);
    void displayFormattedText(const QString &err);
    void displayErrorMessage(const QString &err);
    void updateAlbumArt();
    void updateProgressBar();
    void updateTransportControls();
    void updateMetadata();
    void updateVolume();
    void updateBrowseWindow();
    void browseItemClicked(QModelIndex index);
    void updateZonesList();
    void updateRecords(const QList<BonjourRecord> &list);
    void dnsRecordResolved(const QHostInfo &info, int port);
    void clearConsoleWindow();
};

#endif // DIALOG_H
