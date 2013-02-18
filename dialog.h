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
    void createHorizontalGroupBox();
    void createVerticalGroupBox();
    void createGridGroupBox();
    void createFormGroupBox();

    enum { NumGridRows = 3, NumButtons = 5 };

    QMenuBar *menuBar;
    QGroupBox *horizontalGroupBox;
    QGroupBox *verticalGroupBox;
    QGroupBox *gridGroupBox;
    QGroupBox *formGroupBox;
    QTextEdit *smallEditor;
    QTextEdit *bigEditor;
    QLabel *labels[NumGridRows];
    QLineEdit *lineEdits[NumGridRows];
    QPushButton *buttons[NumButtons];
    QDialogButtonBox *buttonBox;

    QMenu *fileMenu;
    QAction *exitAction;
};

#endif // DIALOG_H
