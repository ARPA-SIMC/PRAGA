#include "mainWindow.h"
#include "pragaProject.h"
#include "pragaShell.h"
#include "shell.h"

#include <cstdio>
#include <QApplication>
#include <QtNetwork/QNetworkProxy>
#include <QMessageBox>


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
        QMessageBox::information(nullptr, "Error in proxy configuration!", "");
        return false;
    }

    return true;
}


QCoreApplication* createApplication(int &argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
        if (!qstrcmp(argv[i], "-no-gui"))
            return new QCoreApplication(argc, argv);
    return new QApplication(argc, argv);
}


int main(int argc, char *argv[])
{
    // set modality (default: GUI)
    if (argc > 1)
    {
        QString arg1 = QString::fromStdString(argv[1]);
        if (arg1.toUpper() == "CONSOLE")
        {
            myProject.modality = MODE_CONSOLE;
        }
        else
        {
            myProject.modality = MODE_BATCH;
        }
    }

    QApplication myApp(argc, argv);

    // proxy
    //setProxy("proxy-sc.arpa.emr.net", 8080);
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // read environment
    QProcessEnvironment myEnvironment = QProcessEnvironment::systemEnvironment();
    QString pragaHome = myEnvironment.value("PRAGA_HOME");

    // check praga home
    if (pragaHome == "")
    {
        QString warning = "Set PRAGA_HOME in the environment variables:"
                          "\nPRAGA_HOME = path of praga directory";
        QMessageBox::information(nullptr, "Missing environment", warning);
        return -1;
    }
    if (!QDir(pragaHome).exists())
    {
        QString warning = "Set correct PRAGA_HOME in the environment variables:"
                          "\nPRAGA_HOME = path of praga directory";
        QMessageBox::information(nullptr, "Wrong environment: " + pragaHome, warning);
        return -1;
    }

    if (! myProject.start(pragaHome))
        return -1;

    if (! myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini"))
        return -1;

    // start modality
    if (myProject.modality == MODE_GUI)
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        MainWindow w;
        w.show();
        return myApp.exec();
    }
    else if (myProject.modality == MODE_CONSOLE)
    {
        if (! pragaShell(&myProject))
            return -1;
    }
    else if (myProject.modality == MODE_BATCH)
    {
        if (! pragaBatch(&myProject, argv[1]))
            return -1;
    }
}
