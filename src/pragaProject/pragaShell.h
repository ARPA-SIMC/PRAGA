#ifndef PRAGASHELL_H
#define PRAGASHELL_H

    #ifndef PRAGAPROJECT_H
        #include "pragaProject.h"
    #endif

    QList<QString> getPragaCommandList();
    int cmdList(PragaProject* myProject);
    int pragaVersion(PragaProject* myProject);

    int cmdOpenPragaProject(PragaProject* myProject, const QList<QString> &argumentList);
    int cmdDownload(PragaProject* myProject, const QList<QString> &argumentList);
    int cmdInterpolationGridPeriod(PragaProject* myProject, QList<QString> argumentList);
    int cmdInterpolationCrossValidation(PragaProject* myProject, QList<QString> argumentList);
    int cmdAggregationGridPeriod(PragaProject* myProject, QList<QString> argumentList);
    int cmdHourlyDerivedVariablesGrid(PragaProject* myProject, QList<QString> argumentList);
    int cmdGridAggregationOnZones(PragaProject* myProject, QList<QString> argumentList);
    int cmdMonthlyIntegrationVariablesGrid(PragaProject* myProject, QList<QString> argumentList);
    int cmdExportDailyGridToRaster(PragaProject* myProject, QList<QString> argumentList);
    int cmdComputeClimatePointsXML(PragaProject* myProject, QList<QString> argumentList);
    int cmdCleanClimatePoint(PragaProject* myProject);
    int cmdDroughtIndexPoint(PragaProject* myProject, QList<QString> argumentList);
    int cmdSaveLogDataProceduresGrid(PragaProject* myProject, QList<QString> argumentList);
    int cmdComputeRadiationList(PragaProject* myProject, QList<QString> argumentList);

    #ifdef NETCDF
        int cmdDroughtIndexGrid(PragaProject* myProject, QList<QString> argumentList);
        int cmdNetcdfExport(PragaProject* myProject, QList<QString> argumentList);
        int cmdExportXMLElabToNetcdf(PragaProject* myProject, QList<QString> argumentList);
    #endif

#endif // PRAGASHELL_H
