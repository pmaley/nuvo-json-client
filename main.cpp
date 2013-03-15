#include <QApplication>
#include "dialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QIcon icon(":images/Icon@2x.png");

    Dialog dialog;
    dialog.setWindowIcon(icon);
    dialog.show();

    return app.exec();
}
