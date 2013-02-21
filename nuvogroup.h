#ifndef NUVOGROUP_H
#define NUVOGROUP_H

#include <QDialog>
#include <QTcpSocket>
#include <QScriptEngine>
#include <QHash>

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
QT_END_NAMESPACE

class NuvoGroup : public QObject
{
    Q_OBJECT

public:
    NuvoGroup();

private:

public slots:

private slots:


};
#endif // NUVOGROUP_H
