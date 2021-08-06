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

    #ifndef IMPORTDATAXML_H
        #include "importDataXML.h"
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

        ImportDataXML* importData;
        QString projectPragaFolder;

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

        bool downloadDailyDataArkimet(QList<QString> variables, bool prec0024, QDate startDate, QDate endDate, bool showInfo);
        bool downloadHourlyDataArkimet(QList<QString> variables, QDate startDate, QDate endDate, bool showInfo);

        bool interpolationMeteoGrid(meteoVariable myVar, frequencyType myFrequency, const Crit3DTime& myTime, bool showInfo);
        bool interpolationMeteoGridPeriod(QDate dateIni, QDate dateFin, QList <meteoVariable> variables, QList<meteoVariable> aggrVariables, bool saveRasters, int saveIntervalDays);
        bool saveGrid(meteoVariable myVar, frequencyType myFrequency, const Crit3DTime& myTime, bool showInfo);
        bool timeAggregateGridVarHourlyInDaily(meteoVariable dailyVar, Crit3DDate dateIni, Crit3DDate dateFin);
        bool timeAggregateGrid(QDate dateIni, QDate dateFin, QList <meteoVariable> variables, bool loadData, bool saveData);
        bool hourlyDerivedVariablesGrid(QDate first, QDate last, bool loadData, bool saveData);

        bool elaborationPointsCycle(bool isAnomaly, bool showInfo);
        bool elaborationPointsCycleGrid(bool isAnomaly, bool showInfo);
        bool elaborationCheck(bool isMeteoGrid, bool isAnomaly);
        bool elaboration(bool isMeteoGrid, bool isAnomaly, bool saveClima);
        bool showClimateFields(bool isMeteoGrid, QList<QString> *climateDbElab, QList<QString> *climateDbVarList);
        void saveClimateResult(bool isMeteoGrid, QString climaSelected, int climateIndex, bool showInfo);
        bool deleteClima(bool isMeteoGrid, QString climaSelected);
        bool climatePointsCycle(bool showInfo);
        bool climatePointsCycleGrid(bool showInfo);
        bool averageSeriesOnZonesMeteoGrid(meteoVariable variable, meteoComputation elab1MeteoComp,
                                           QString aggregationString, float threshold, gis::Crit3DRasterGrid* zoneGrid,
                                           QDate startDate, QDate endDate, QString periodType,
                                           std::vector<float> &outputValues, bool showInfo);
        bool getIsElabMeteoPointsValue() const;
        void setIsElabMeteoPointsValue(bool value);
        bool dbMeteoPointDataCount(QDate myFirstDate, QDate myLastDate, meteoVariable myVar, QString dataset, std::vector<int> &myCounter);
        bool dbMeteoGridMissingData(QDate myFirstDate, QDate myLastDate, meteoVariable myVar, QList<QDate> &dateList, QList<QString> &idList);

        int executePragaCommand(QList<QString> argumentList, bool* isCommandFound);
        bool parserXMLImportData(QString xmlName, bool isGrid);
        bool loadXMLImportData(QString fileName);

        #ifdef NETCDF
                bool exportMeteoGridToNetCDF(QString fileName);
                bool exportXMLElabGridToNetcdf(QString xmlName);
        #endif
    };


#endif // PRAGAPROJECT_H
