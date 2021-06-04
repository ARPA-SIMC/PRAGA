#ifndef PRAGASHELL_H
#define PRAGASHELL_H

    #ifndef PRAGAPROJECT_H
        #include "pragaProject.h"
    #endif

    QStringList getPragaCommandList();
    bool cmdList(PragaProject* myProject);

    bool executeCommand(QStringList argumentList, PragaProject* myProject);
    bool pragaShell(PragaProject* myProject);
    bool pragaBatch(PragaProject* myProject, QString batchFileName);

    bool cmdOpenPragaProject(PragaProject* myProject, QStringList argumentList);
    bool cmdDownload(PragaProject* myProject, QStringList argumentList);
    bool cmdInterpolationGridPeriod(PragaProject* myProject, QStringList argumentList);
    bool cmdAggregationGridPeriod(PragaProject* myProject, QStringList argumentList);
    bool cmdHourlyDerivedVariablesGrid(PragaProject* myProject, QStringList argumentList);
    //bool cmdLoadForecast(PragaProject* myProject, QStringList argumentList);

    #ifdef NETCDF
        bool cmdNetcdfExport(PragaProject* myProject, QStringList argumentList);
        bool cmdExportXMLElabToNetcdf(PragaProject* myProject, QStringList argumentList);
    #endif

#endif // PRAGASHELL_H
