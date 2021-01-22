#include "pragaShell.h"
#include "shell.h"
#include "utilities.h"
#include <QFile>
#include <QTextStream>


QStringList getPragaCommandList()
{
    QStringList cmdList = getSharedCommandList();

    // praga commands
    cmdList.append("List         | ListCommands");
    cmdList.append("Proj         | OpenProject");
    cmdList.append("Netcdf       | ExportNetcdf");
    cmdList.append("XMLToNetcdf  | ExportXMLElaborationsToNetcdf");

    return cmdList;
}


bool cmdList(PragaProject* myProject)
{
    QStringList list = getPragaCommandList();

    myProject->logInfo("Available PRAGA Console commands:");
    myProject->logInfo("(short  | long version)");
    for (int i = 0; i < list.size(); i++)
    {
        myProject->logInfo(list[i]);
    }

    return true;
}


bool PragaProject::executePragaCommand(QStringList argumentList, bool* isCommandFound)
{
    *isCommandFound = false;
    if (argumentList.size() == 0) return false;

    QString command = argumentList[0].toUpper();

    if (command == "LIST" || command == "LISTCOMMANDS")
    {
        *isCommandFound = true;
        return cmdList(this);
    }
    else if (command == "PROJ" || command == "OPENPROJECT")
    {
        *isCommandFound = true;
        return cmdOpenPragaProject(this, argumentList);
    }
    else if (command == "DOWNLOAD")
    {
        *isCommandFound = true;
        return cmdDownload(this, argumentList);
    }
    else if (command == "GRIDDING" || command == "INTERPOLATIONGRIDPERIOD")
    {
        *isCommandFound = true;
        return cmdInterpolationGridPeriod(this, argumentList);
    }
    else if (command == "GRIDAGGREGATION" || command == "GRIDAGGR")
    {
        *isCommandFound = true;
        return cmdAggregationGridPeriod(this, argumentList);
    }
    else if (command == "NETCDF" || command == "NETCDFEXPORT")
    {
        *isCommandFound = true;
        return cmdNetcdfExport(this, argumentList);
    }
    else if (command == "XMLTONETCDF" || command == "XMLNETCDFEXPORT")
    {
        *isCommandFound = true;
        return cmdExportXMLElabToNetcdf(this, argumentList);
    }
    else
    {
        // other specific Praga commands
        // ...
    }

    return false;
}

bool cmdOpenPragaProject(PragaProject* myProject, QStringList argumentList)
{
    if (argumentList.size() < 2)
    {
        myProject->logError("Missing project name");
        return false;
    }

    QString projectFolder = "";
    if (getFilePath(argumentList.at(1)) == "")
    {
        QString filename = argumentList.at(1);
        projectFolder = filename.left(filename.length()-4) + "/";
    }

    QString projectName = myProject->getCompleteFileName(argumentList.at(1), PATH_PROJECT+projectFolder);

    if (! myProject->loadPragaProject(projectName))
    {
        myProject->logError();
        return false;
    }

    return true;
}

bool cmdDownload(PragaProject* myProject, QStringList argumentList)
{
    if (argumentList.size() < 2)
    {
        myProject->logError("Missing parameters for download");
        return false;
    }

    QDate dateIni, dateFin;
    QStringList varString, dailyVarString, hourlyVarString;
    QString var;
    meteoVariable meteoVar;
    bool prec0024 = true;
    frequencyType myFreq;

    for (int i = 1; i < argumentList.size(); i++)
    {
        if (argumentList[i].left(3) == "-v:")
        {
            varString = argumentList[i].right(argumentList[i].length()-3).split(",");
            foreach (var,varString)
            {
                meteoVar = getMeteoVar(var.toStdString());
                if (meteoVar == noMeteoVar)
                {
                    myProject->logError("Unknown variable: " + var);
                    return false;
                }
                else
                {
                    myFreq = getVarFrequency(meteoVar);
                    if (myFreq == noFrequency)
                    {
                        myProject->logError("Unknown frequency for variable : " + var);
                        return false;
                    }
                    else if (myFreq == daily)
                        dailyVarString.append(var);
                    else if (myFreq == hourly)
                        hourlyVarString.append(var);
                }
            }
        }
        else if (argumentList.at(i).left(4) == "-d1:")
            dateIni = QDate::fromString(argumentList[i].right(argumentList[i].length()-4), "dd/MM/yyyy");
        else if (argumentList.at(i).left(4) == "-d2:")
            dateFin = QDate::fromString(argumentList[i].right(argumentList[i].length()-4), "dd/MM/yyyy");
        else if (argumentList.at(i).left(10) == "-yesterday")
        {
            dateIni = QDate::currentDate().addDays(-1);
            dateFin = dateIni;
        }
        else if (argumentList.at(i).left(3) == "-p9")
            prec0024 = false;
    }

    if (! dateIni.isValid())
    {
        myProject->logError("Wrong initial date");
        return false;
    }

    if (! dateFin.isValid())
    {
        myProject->logError("Wrong final date");
        return false;
    }

    if (dailyVarString.size() > 0)
        if (! myProject->downloadDailyDataArkimet(dailyVarString, prec0024, dateIni, dateFin, false))
            return false;

    if (hourlyVarString.size() > 0)
        if (! myProject->downloadHourlyDataArkimet(hourlyVarString, dateIni, dateFin, false))
            return false;

    return true;
}


bool cmdInterpolationGridPeriod(PragaProject* myProject, QStringList argumentList)
{
    if (argumentList.size() < 2)
    {
        myProject->logError("Missing parameters for gridding");
        return false;
    }

    QDate dateIni, dateFin;
    bool saveRasters = false;
    QList <QString> varString, aggrVarString;
    QList <meteoVariable> variables, aggrVariables;
    QString var;
    meteoVariable meteoVar;
    int saveInterval = 1;
    bool parseSaveInterval = true;

    for (int i = 1; i < argumentList.size(); i++)
    {
        if (argumentList[i].left(3) == "-v:")
        {
            varString = argumentList[i].right(argumentList[i].length()-3).split(",");
            foreach (var,varString)
            {
                meteoVar = getMeteoVar(var.toStdString());
                if (meteoVar != noMeteoVar) variables << meteoVar;
            }
        }
        else if (argumentList[i].left(3) == "-a:")
        {
            varString = argumentList[i].right(argumentList[i].length()-3).split(",");
            foreach (var,varString)
            {
                meteoVar = getMeteoVar(var.toStdString());
                if (meteoVar != noMeteoVar) aggrVariables << meteoVar;
            }
        }
        else if (argumentList.at(i).left(4) == "-d1:")
            dateIni = QDate::fromString(argumentList[i].right(argumentList[i].length()-4), "dd/MM/yyyy");
        else if (argumentList.at(i).left(4) == "-d2:")
            dateFin = QDate::fromString(argumentList[i].right(argumentList[i].length()-4), "dd/MM/yyyy");
        else if (argumentList.at(i).left(10) == "-yesterday")
        {
            dateIni = QDate::currentDate().addDays(-1);
            dateFin = dateIni;
        }
        else if (argumentList.at(i).left(2) == "-r")
            saveRasters = true;
        else if (argumentList.at(i).left(3) == "-s:")
            saveInterval = argumentList[i].right(argumentList[i].length()-3).toInt(&parseSaveInterval, 1);

    }

    if (! dateIni.isValid())
    {
        myProject->logError("Wrong initial date");
        return false;
    }

    if (! dateFin.isValid())
    {
        myProject->logError("Wrong final date");
        return false;
    }

    if (saveInterval == NODATA || ! parseSaveInterval)
    {
        myProject->logError("Wrong save interval number");
        return false;
    }

    if (! myProject->interpolationMeteoGridPeriod(dateIni, dateFin, variables, aggrVariables, saveRasters, saveInterval))
        return false;

    return true;
}

bool cmdAggregationGridPeriod(PragaProject* myProject, QStringList argumentList)
{
    if (argumentList.size() < 2)
    {
        myProject->logError("Missing parameters for aggregation");
        return false;
    }

    QDate dateIni, dateFin;
    QList <meteoVariable> variables;
    meteoVariable meteoVar;

    for (int i = 1; i < argumentList.size(); i++)
    {
        if (argumentList[i].left(3) == "-v:")
        {
            meteoVar = getMeteoVar(argumentList[i].right(argumentList[i].length()-3).toStdString());
            if (meteoVar != noMeteoVar) variables << meteoVar;
        }
        else if (argumentList.at(i).left(4) == "-d1:")
        {
            QString dateIniStr = argumentList[i].right(argumentList[i].length()-4);
            dateIni = QDate::fromString(dateIniStr, "dd/MM/yyyy");
        }
        else if (argumentList.at(i).left(4) == "-d2:")
        {
            QString dateFinStr = argumentList[i].right(argumentList[i].length()-4);
            dateFin = QDate::fromString(dateFinStr, "dd/MM/yyyy");
        }

    }

    if (! myProject->timeAggregateGrid(dateIni, dateFin, variables, true, true))
        return false;

    return true;
}

bool executeCommand(QStringList argumentList, PragaProject* myProject)
{
    if (argumentList.size() == 0) return false;
    bool isCommandFound, isExecuted;

    myProject->logInfo(getTimeStamp(argumentList));

    isExecuted = executeSharedCommand(myProject, argumentList, &isCommandFound);
    if (isCommandFound) return isExecuted;

    isExecuted = myProject->executePragaCommand(argumentList, &isCommandFound);
    if (isCommandFound) return isExecuted;

    myProject->logError("This is not a valid PRAGA command.");
    return false;
}


bool pragaBatch(PragaProject* myProject, QString scriptFileName)
{
    #ifdef _WIN32
        attachOutputToConsole();
    #endif

    myProject->logInfo("\nPRAGA v1");
    myProject->logInfo("Execute script: " + scriptFileName);

    if (scriptFileName == "")
    {
        myProject->logError("No script file provided");
        return false;
    }

    QFile scriptFile(scriptFileName);
    if(! scriptFile.open (QIODevice::ReadOnly))
    {
        myProject->logError(scriptFile.errorString());
        return false;
    }

    QTextStream myStream (&scriptFile);
    QString cmdLine;

    while (! scriptFile.atEnd())
    {
        cmdLine = scriptFile.readLine();
        QStringList argumentList = getArgumentList(cmdLine);
        if (! executeCommand(argumentList, myProject))
            return false;
    }

    myProject->logInfo("Batch finished at: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    scriptFile.close();

    #ifdef _WIN32
        // Send "enter" to release application from the console
        // This is a hack, but if not used the console doesn't know the application has
        // returned. The "enter" key only sent if the console window is in focus.
        if (isConsoleForeground()) sendEnterKey();
    #endif

    return true;
}


bool pragaShell(PragaProject* myProject)
{
    #ifdef _WIN32
        openNewConsole();
    #endif
    while (! myProject->requestedExit)
    {
        QString commandLine = getCommandLine("PRAGA");
        if (commandLine != "")
        {
            QStringList argumentList = getArgumentList(commandLine);
            executeCommand(argumentList, myProject);
        }
    }

    return true;
}

#ifdef NETCDF

    bool cmdNetcdfExport(PragaProject* myProject, QStringList argumentList)
    {
        if (argumentList.size() < 2)
        {
            myProject->logError("Missing netcdf name");
            return false;
        }

        QString netcdfName = myProject->getCompleteFileName(argumentList.at(1), PATH_PROJECT);
        if (! myProject->checkMeteoGridForExport())
        {
            return false;
        }

        if (! myProject->exportMeteoGridToNetCDF(netcdfName))
        {
            return false;
        }
        return true;
    }

    bool cmdExportXMLElabToNetcdf(PragaProject* myProject, QStringList argumentList)
    {
        if (argumentList.size() < 2)
        {
            myProject->logError("Missing xml name");
            return false;
        }

        QString xmlName = myProject->getCompleteFileName(argumentList.at(1), PATH_PROJECT);
        if (!myProject->exportXMLElabGridToNetcdf(xmlName))
        {
            return false;
        }

        return true;
    }

#endif
