#include "mainWindow.h"
#include <QApplication>
#include <QMessageBox>

#include "pragaProject.h"


bool checkEnvironmentGUI(QString pragaHome)
{
    if (pragaHome.isEmpty())
    {
        QString warning = "Set PRAGA_HOME in the environment variables:"
                          "\n$PRAGA_HOME = path of praga directory";

        QMessageBox::critical(nullptr, "Missing environment", warning);
        return false;
    }

    if (! QDir(pragaHome).exists())
    {
        QString warning = pragaHome + "  doesn't exist!"
                          "\nSet correct $PRAGA_HOME in the environment variables:"
                          "\n$PRAGA_HOME = path of praga directory";

        QMessageBox::critical(nullptr, "Wrong environment!", warning);
        return false;
    }

    return true;
}


QString searchDefaultPragaPath(QString startPath, PragaProject& myProject)
{
    QString myRoot = QDir::rootPath();
    QString pragaPath = startPath;

    // Installation on other volume (for example D:)
    QString myVolume = pragaPath.left(3);

    bool isFound = false;
    while (! isFound)
    {
        if (QDir(pragaPath + "/DATA").exists())
        {
            isFound = true;
            break;
        }
        if (QDir::cleanPath(pragaPath) == myRoot || QDir::cleanPath(pragaPath) == myVolume)
            break;

        pragaPath = QFileInfo(pragaPath).dir().absolutePath();
    }

    if (! isFound)
    {
        myProject.logError("DATA directory is missing");
        return "";
    }

    return QDir::cleanPath(pragaPath);
}


int mainGUI(int argc, char *argv[], QString pragaHome, PragaProject& myProject)
{
    QApplication myApp(argc, argv);
    QApplication::setOverrideCursor(Qt::ArrowCursor);

    // only for Windows without right to set environment
    if (QSysInfo::productType() == "windows" && pragaHome == "")
    {
        QString appPath = myApp.applicationDirPath();
        pragaHome = searchDefaultPragaPath(appPath, myProject);
    }

    if (! checkEnvironmentGUI(pragaHome))
    {
        QString appPath = myApp.applicationDirPath();
        pragaHome = searchDefaultPragaPath(appPath, myProject);
        if (!checkEnvironmentGUI(pragaHome))
            return PRAGA_ENV_ERROR;
    }

    if (! myProject.start(pragaHome))
        return PRAGA_ERROR;

    if (! myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini"))
        return PRAGA_ERROR;

    MainWindow w;
    w.show();

    return myApp.exec();
}

