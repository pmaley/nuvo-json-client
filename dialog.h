#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

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
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog();

private:
    void createMenu();
    void createTransportControlsBox();
    void createConsoleWindow();
    void createMetadataBox(QStringList trackMetadata);
    void createNowPlayingBox();

    enum { NumGridRows = 3, NumButtons = 5 };

    QMenuBar *menuBar;
    QGroupBox *transportControlsBox;
    QGroupBox *metadataBox;
    QGroupBox *nowPlayingBox;
    QTextEdit *consoleWindow;
    QLabel *labels[NumGridRows];
    QPushButton *buttons[NumButtons];
    QDialogButtonBox *buttonBox;

    QMenu *fileMenu;
    QAction *exitAction;

public slots:
    void testFunction();

};

#endif // DIALOG_H
