#include "mainWindow.h"
#include <QApplication>
#include <QMessageBox>


bool checkGUI(QString pragaHome)
{
    if (pragaHome == "")
    {
        QString warning = "Set PRAGA_HOME in the environment variables:"
                          "\n$PRAGA_HOME = path of praga directory";

        QMessageBox::information(nullptr, "Missing environment", warning);
        return false;
    }

    if (!QDir(pragaHome).exists())
    {
        QString warning = "Wrong environment!\n"
                          "Set correct $PRAGA_HOME variable:\n"
                          "$PRAGA_HOME = path of praga directory";

        QMessageBox::information(nullptr, pragaHome, warning);
        return false;
    }

    return true;
}

int mainGUI(int argc, char *argv[], QString pragaHome)
{
    if (!checkGUI(pragaHome))
        return PRAGA_ENV_ERROR;

    QApplication myApp(argc, argv);

    QApplication::setOverrideCursor(Qt::ArrowCursor);
    MainWindow w;
    w.show();

    return myApp.exec();
}


