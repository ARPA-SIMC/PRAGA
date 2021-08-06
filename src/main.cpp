#include "pragaProject.h"
#include "pragaShell.h"
#include "shell.h"
#include "mainGUI.h"
#include "commonConstants.h"

#include <QCoreApplication>
#include <cstdio>
#include <iostream>
#include <QtNetwork/QNetworkProxy>
#include <QProcessEnvironment>
#include <QDir>


PragaProject myProject;


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


/*
QCoreApplication* createApplication(int &argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
        if (!qstrcmp(argv[i], "-no-gui"))
            return new QCoreApplication(argc, argv);
    return new QApplication(argc, argv);
}*/


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

    if (QSysInfo::productType() != "windows" && QSysInfo::productType() != "osx")
    {
        // qDebug() << "LINUX";
        if (myProject.modality == MODE_GUI && display.isEmpty())
        {
            // server headless, switch modality
            myProject.modality = MODE_CONSOLE;
        }
    }

    // check $PRAGA_HOME
    if (pragaHome == "")
    {
        QString warning = "Set PRAGA_HOME in the environment variables:"
                          "\n$PRAGA_HOME = path of praga directory\n";

        std::cout << warning.toStdString() << std::flush;
        return PRAGA_ENV_ERROR;
    }

    if (!QDir(pragaHome).exists())
    {
        QString warning = "Wrong environment!\n"
                          "Set correct $PRAGA_HOME variable:\n"
                          "$PRAGA_HOME = path of praga directory\n";

        std::cout << warning.toStdString() << std::flush;
        return PRAGA_ENV_ERROR;
    }

    if (! myProject.start(pragaHome))
        return PRAGA_ERROR;

    if (! myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini"))
        return PRAGA_ERROR;

    // start modality
    if (myProject.modality == MODE_GUI)
    {
        return mainGUI(argc, argv, pragaHome);
    }
    else if (myProject.modality == MODE_CONSOLE)
    {
        QCoreApplication myApp(argc, argv);
        return pragaShell(&myProject);
    }
    else if (myProject.modality == MODE_BATCH)
    {
        QCoreApplication myApp(argc, argv);
        return pragaBatch(&myProject, argv[1]);
    }
}
