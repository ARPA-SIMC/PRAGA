#ifndef PRAGAPROJECT_H
#define PRAGAPROJECT_H

    #ifndef CRIT3DCLIMATE_H
        #include "crit3dClimate.h"
    #endif

    #ifndef PROJECT_H
        #include "project.h"
    #endif

    #ifndef METEOMAPS_H
        #include "meteoMaps.h"
    #endif

    #ifndef PRAGAMETEOMAPS_H
        #include "pragaMeteoMaps.h"
    #endif

    #ifdef NETCDF
        #include "netcdfHandler.h"
    #endif

    class PragaProject : public Project
    {
    private:

    public:
        gis::Crit3DRasterGrid dataRaster;
        Crit3DDailyMeteoMaps* pragaDailyMaps;
        PragaHourlyMeteoMaps* pragaHourlyMaps;

        aggregationMethod grdAggrMethod;

        Crit3DClimate* clima;
        Crit3DClimate* climaFromDb;
        Crit3DClimate* referenceClima;

        bool isElabMeteoPointsValue;
        QString climateIndex;

        QSettings* pragaDefaultSettings;
        std::map<QString, QList<int> > idArkimetHourlyMap;
        std::map<QString, QList<int> > idArkimetDailyMap;

        #ifdef NETCDF
            NetCDFHandler netCDF;
        #endif

        PragaProject();

        void initializePragaProject();
        void clearPragaProject();

        void createPragaProject(QString path_, QString name_, QString description_);
        void savePragaProject();
        void savePragaParameters();

        bool loadPragaProject(QString myFileName);
        bool loadPragaSettings();

        gis::Crit3DRasterGrid* getPragaMapFromVar(meteoVariable myVar);

        bool downloadDailyDataArkimet(QStringList variables, bool prec0024, QDate startDate, QDate endDate, bool showInfo);
        bool downloadHourlyDataArkimet(QStringList variables, QDate startDate, QDate endDate, bool showInfo);

        bool interpolationMeteoGrid(meteoVariable myVar, frequencyType myFrequency, const Crit3DTime& myTime, bool showInfo);
        bool interpolationMeteoGridPeriod(QDate dateIni, QDate dateFin,
                                          QList <meteoVariable> hourlyVariables, QList <meteoVariable> dailyVariables,
                                          bool saveRasters);
        bool interpolationMeteoGridPeriod(QDate dateIni, QDate dateFin,
                                          QList <meteoVariable> hourlyVariables, bool saveRasters);
        bool saveGrid(meteoVariable myVar, frequencyType myFrequency, const Crit3DTime& myTime, bool showInfo);
        bool gridAggregateVarHourlyInDaily(meteoVariable dailyVar, Crit3DDate dateIni, Crit3DDate dateFin);
        bool aggregationMeteoGrid(QDate dateIni, QDate dateFin, QList <meteoVariable> variables);

        bool elaborationPointsCycle(bool isAnomaly, bool showInfo);
        bool elaborationPointsCycleGrid(bool isAnomaly, bool showInfo);
        bool elaborationCheck(bool isMeteoGrid, bool isAnomaly);
        bool elaboration(bool isMeteoGrid, bool isAnomaly, bool saveClima);
        bool showClimateFields(bool isMeteoGrid, QStringList *climateDbElab, QStringList *climateDbVarList);
        void saveClimateResult(bool isMeteoGrid, QString climaSelected, int climateIndex, bool showInfo);
        bool deleteClima(bool isMeteoGrid, QString climaSelected);
        bool climatePointsCycle(bool showInfo);
        bool climatePointsCycleGrid(bool showInfo);
        bool averageSeriesOnZonesMeteoGrid(meteoVariable variable, meteoComputation elab1MeteoComp,
                                           aggregationMethod spatialElab, float threshold, gis::Crit3DRasterGrid* zoneGrid,
                                           QDate startDate, QDate endDate, QString periodType,
                                           std::vector<float> &outputValues, bool showInfo);
        bool getIsElabMeteoPointsValue() const;
        void setIsElabMeteoPointsValue(bool value);

        bool executePragaCommand(QStringList argumentList, bool* isCommandFound);

        #ifdef NETCDF
                bool exportMeteoGridToNetCDF(QString fileName);
                bool exportXMLElabGridToNetcdf(QString xmlName);
        #endif
    };


#endif // PRAGAPROJECT_H
