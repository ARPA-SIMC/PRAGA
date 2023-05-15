#ifndef MAINGUI_H
#define MAINGUI_H

    class QString;
    class PragaProject;

    bool checkEnvironmentGUI(QString pragaHome);
    QString searchDefaultPragaPath(QString startPath, PragaProject& myProject);
    int mainGUI(int argc, char *argv[], QString pragaHome, PragaProject& myProject);

#endif // MAINGUI_H
