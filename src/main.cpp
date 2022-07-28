#include "pragaProject.h"
#include "pragaShell.h"
#include "shell.h"
#include "mainGUI.h"
#include "commonConstants.h"
#include "statistics.h"
#include <QCoreApplication>
#include <cstdio>
#include <iostream>
#include <QtNetwork/QNetworkProxy>
#include <QProcessEnvironment>
#include <QDir>
#include <QDebug>


PragaProject myProject;

// check $PRAGA_HOME
bool checkEnvironmentConsole(QString pragaHome)
{
    #ifdef _WIN32
        attachOutputToConsole();
    #endif

    if (pragaHome == "")
    {
        QString error = "\nSet PRAGA_HOME in the environment variables:\n"
                        "$PRAGA_HOME = path of praga directory\n";

        myProject.logError(error);
        return false;
    }

    if (!QDir(pragaHome).exists())
    {
        QString error = "\nWrong environment!\n"
                        "Set correct $PRAGA_HOME variable:\n"
                        "$PRAGA_HOME = path of praga directory\n";

        myProject.logError(error);
        return false;
    }

    return true;
}


bool setProxy(QString hostName, unsigned short port)
{
    QNetworkProxy myProxy;

    myProxy.setType(QNetworkProxy::HttpProxy);
    myProxy.setHostName(hostName);
    myProxy.setPort(port);

    try {
       QNetworkProxy::setApplicationProxy(myProxy);
    }
    catch (...) {
        std::cout << "Error in proxy configuration:" << hostName.toStdString();
        return false;
    }

    return true;
}


int main(int argc, char *argv[])
{
    // set modality (default: GUI)
    myProject.modality = MODE_GUI;

    if (argc > 1)
    {
        QString arg1 = QString::fromStdString(argv[1]);
        if (arg1.toUpper() == "CONSOLE" || arg1.toUpper() == "SHELL")
        {
            myProject.modality = MODE_CONSOLE;
        }
        else
        {
            myProject.modality = MODE_BATCH;
        }
    }

    //setProxy("proxy-sc.arpa.emr.net", 8080);
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // read environment
    QProcessEnvironment myEnvironment = QProcessEnvironment::systemEnvironment();
    QString pragaHome = myEnvironment.value("PRAGA_HOME");
    QString display = myEnvironment.value("DISPLAY");

    // only for Linux headless
    if (QSysInfo::productType() != "windows" && QSysInfo::productType() != "osx")
    {
        if (myProject.modality == MODE_GUI && display.isEmpty())
        {
            // server headless, switch modality
            myProject.modality = MODE_CONSOLE;
        }
    }

    // start GUI
    if (myProject.modality == MODE_GUI)
    {
        return mainGUI(argc, argv, pragaHome, myProject);
    }

    if (!checkEnvironmentConsole(pragaHome))
        return PRAGA_ENV_ERROR;

    if (! myProject.start(pragaHome))
        return PRAGA_ERROR;

    if (! myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini"))
        return PRAGA_ERROR;

    QCoreApplication myApp(argc, argv);

    // start modality
    if (myProject.modality == MODE_CONSOLE)
    {
        return pragaShell(&myProject);
    }
    else if (myProject.modality == MODE_BATCH)
    {
        return pragaBatch(&myProject, argv[1]);
    }

    return 0;
}
