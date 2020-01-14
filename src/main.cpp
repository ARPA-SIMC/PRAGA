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

    QNetworkProxyFactory::setUseSystemConfiguration(true);

    if (! myProject.start(myApp.applicationDirPath()))
        return -1;

    if (! myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini"))
        return -1;

    if (myProject.modality == MODE_GUI)
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        MainWindow w;
        w.show();
        return myApp.exec();
    }
    else if (myProject.modality == MODE_CONSOLE)
    {
        return pragaShell(&myProject);
    }
    else if (myProject.modality == MODE_BATCH)
    {
        return pragaBatch(&myProject, argv[1]);
    }
}
