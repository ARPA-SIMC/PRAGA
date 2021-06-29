#ifndef PRAGASHELL_H
#define PRAGASHELL_H

    #ifndef PRAGAPROJECT_H
        #include "pragaProject.h"
    #endif

    QList<QString> getPragaCommandList();
    bool cmdList(PragaProject* myProject);

    bool executeCommand(QList<QString> argumentList, PragaProject* myProject);
    bool pragaShell(PragaProject* myProject);
    bool pragaBatch(PragaProject* myProject, QString batchFileName);

    bool cmdOpenPragaProject(PragaProject* myProject, QList<QString> argumentList);
    bool cmdDownload(PragaProject* myProject, QList<QString> argumentList);
    bool cmdInterpolationGridPeriod(PragaProject* myProject, QList<QString> argumentList);
    bool cmdAggregationGridPeriod(PragaProject* myProject, QList<QString> argumentList);
    bool cmdHourlyDerivedVariablesGrid(PragaProject* myProject, QList<QString> argumentList);
    bool cmdGridAggregationOnZones(PragaProject* myProject, QList<QString> argumentList);
    //bool cmdLoadForecast(PragaProject* myProject, QList<QString> argumentList);

    #ifdef NETCDF
        bool cmdNetcdfExport(PragaProject* myProject, QList<QString> argumentList);
        bool cmdExportXMLElabToNetcdf(PragaProject* myProject, QList<QString> argumentList);
    #endif

#endif // PRAGASHELL_H
