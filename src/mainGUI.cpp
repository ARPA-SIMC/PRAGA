#include "mainWindow.h"
#include <QApplication>
#include <QMessageBox>

#include "pragaProject.h"


bool checkEnvironmentGUI(QString pragaHome)
{
    if (pragaHome == "")
    {
        QString warning = "Set PRAGA_HOME in the environment variables:"
                          "\n$PRAGA_HOME = path of praga directory";

        QMessageBox::critical(nullptr, "Missing environment", warning);
        return false;
    }

    if (!QDir(pragaHome).exists())
    {
        QString warning = "Wrong environment!\n"
                          "Set correct $PRAGA_HOME variable:\n"
                          "$PRAGA_HOME = path of praga directory";

        QMessageBox::critical(nullptr, pragaHome, warning);
        return false;
    }

    return true;
}


int mainGUI(int argc, char *argv[], QString pragaHome, PragaProject& myProject)
{
    QApplication myApp(argc, argv);
    QApplication::setOverrideCursor(Qt::ArrowCursor);

    if (!checkEnvironmentGUI(pragaHome))
        return PRAGA_ENV_ERROR;

    if (! myProject.start(pragaHome))
        return PRAGA_ERROR;

    if (! myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini"))
        return PRAGA_ERROR;

    MainWindow w;
    w.show();

    return myApp.exec();
}


