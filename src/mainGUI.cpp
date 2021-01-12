#include "mainWindow.h"
#include <QApplication>
#include <QMessageBox>


int mainGUI(int argc, char *argv[], QString pragaHome)
{
    if (pragaHome == "")
    {
        QString warning = "Set PRAGA_HOME in the environment variables:"
                          "\n$PRAGA_HOME = path of praga directory";

        QMessageBox::information(nullptr, "Missing environment", warning);
        return -1;
    }

    if (!QDir(pragaHome).exists())
    {
        QString warning = "Wrong environment!\n"
                          "Set correct $PRAGA_HOME variable:\n"
                          "$PRAGA_HOME = path of praga directory";

        QMessageBox::information(nullptr, pragaHome, warning);
        return -1;
    }

    QApplication myApp(argc, argv);

    QApplication::setOverrideCursor(Qt::ArrowCursor);
    MainWindow w;
    w.show();

    return myApp.exec();
}


