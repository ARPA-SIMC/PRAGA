#include "basicMath.h"
#include "climate.h"
#include "crit3dElabList.h"
#include "dbClimate.h"
#include "download.h"
#include "dbAggregationsHandler.h"
#include "formInfo.h"
#include "utilities.h"
#include "project.h"
#include "aggregation.h"
#include "interpolationCmd.h"
#include "pragaProject.h"
#include "iostream" //debug
#include <qdebug.h>


PragaProject::PragaProject()
{
    initializePragaProject();
}

bool PragaProject::getIsElabMeteoPointsValue() const
{
    return isElabMeteoPointsValue;
}

void PragaProject::setIsElabMeteoPointsValue(bool value)
{
    isElabMeteoPointsValue = value;
}

void PragaProject::initializePragaProject()
{
    clima = new Crit3DClimate();
    climaFromDb = nullptr;
    referenceClima = nullptr;
    pragaDefaultSettings = nullptr;
    pragaDailyMaps = nullptr;
}

void PragaProject::clearPragaProject()
{
    if (isProjectLoaded) clearProject();

    dataRaster.clear();

    if (clima != nullptr)
    {
        delete clima;
        clima = nullptr;
    }

    if (pragaDailyMaps != nullptr)
    {
        delete pragaDailyMaps;
        pragaDailyMaps = nullptr;
    }
}

void PragaProject::createPragaProject(QString path_, QString name_, QString description_)
{
    createProject(path_, name_, description_);
    savePragaParameters();
}

void PragaProject::savePragaProject()
{
    saveProject();
    savePragaParameters();
}

bool PragaProject::loadPragaProject(QString myFileName)
{
    clearPragaProject();
    initializeProject();
    initializePragaProject();

    if (myFileName == "") return(false);

    if (! loadProjectSettings(myFileName))
        return false;

    if (! loadProject())
        return false;

    if (! loadPragaSettings())
        return false;

    if (DEM.isLoaded)
    {
        pragaDailyMaps = new Crit3DDailyMeteoMaps(DEM);
        pragaHourlyMaps = new PragaHourlyMeteoMaps(DEM);
    }

    isProjectLoaded = true;

    if (projectName != "")
    {
        logInfo("Project " + projectName + " loaded");
    }
    return true;
}


bool PragaProject::loadPragaSettings()
{
    pragaDefaultSettings = new QSettings(getDefaultPath() + PATH_SETTINGS + "pragaDefault.ini", QSettings::IniFormat);

    Q_FOREACH (QString group, parameters->childGroups())
    {
        if (group == "elaboration")
        {
            parameters->beginGroup(group);
            Crit3DElaborationSettings* elabSettings = clima->getElabSettings();

            if (parameters->contains("anomaly_pts_max_distance") && !parameters->value("anomaly_pts_max_distance").toString().isEmpty())
            {
                elabSettings->setAnomalyPtsMaxDistance(parameters->value("anomaly_pts_max_distance").toFloat());
            }
            if (parameters->contains("anomaly_pts_max_delta_z") && !parameters->value("anomaly_pts_max_delta_z").toString().isEmpty())
            {
                elabSettings->setAnomalyPtsMaxDeltaZ(parameters->value("anomaly_pts_max_delta_z").toFloat());
            }
            if (parameters->contains("grid_min_coverage") && !parameters->value("grid_min_coverage").toString().isEmpty())
            {
                elabSettings->setGridMinCoverage(parameters->value("grid_min_coverage").toFloat());
            }
            if (parameters->contains("compute_tmed") && !parameters->value("compute_tmed").toString().isEmpty())
            {
                elabSettings->setAutomaticTmed(parameters->value("compute_tmed").toBool());
            }
            if (parameters->contains("compute_et0hs") && !parameters->value("compute_et0hs").toString().isEmpty())
            {
                elabSettings->setAutomaticETP(parameters->value("compute_et0hs").toBool());
            }
            if (parameters->contains("merge_joint_stations") && !parameters->value("merge_joint_stations").toString().isEmpty())
            {
                elabSettings->setMergeJointStations(parameters->value("merge_joint_stations").toBool());
            }

            parameters->endGroup();

        }
        else if (group == "id_arkimet")
        {
            parameters->beginGroup(group);
            QStringList myList;
            QList<int> intList;
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirTemperatureAvg))) )
            {
                intList.clear();
                QString dailyTavg = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirTemperatureAvg));
                myList = parameters->value(dailyTavg).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailyTavg] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirTemperatureMax))) )
            {
                intList.clear();
                QString dailyTmax = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirTemperatureMax));
                myList = parameters->value(dailyTmax).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailyTmax] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirTemperatureMin))) )
            {
                intList.clear();
                QString dailyTmin = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirTemperatureMin));
                myList = parameters->value(dailyTmin).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailyTmin] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyPrecipitation))) )
            {
                intList.clear();
                QString dailyPrec = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyPrecipitation));
                myList = parameters->value(dailyPrec).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailyPrec] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirRelHumidityAvg))) )
            {
                intList.clear();
                QString dailRHavg = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirRelHumidityAvg));
                myList = parameters->value(dailRHavg).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailRHavg] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirRelHumidityMax))) )
            {
                intList.clear();
                QString dailRHmax = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirRelHumidityMax));
                myList = parameters->value(dailRHmax).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailRHmax] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirRelHumidityMin))) )
            {
                intList.clear();
                QString dailRHmin = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyAirRelHumidityMin));
                myList = parameters->value(dailRHmin).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailRHmin] = intList;
            }
            if (parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyGlobalRadiation))))
            {
                intList.clear();
                QString dailyRad = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyGlobalRadiation));
                myList = parameters->value(dailyRad).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailyRad] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyWindScalarIntensityAvg))) )
            {
                intList.clear();
                QString dailyWindScalIntAvg = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyWindScalarIntensityAvg));
                myList = parameters->value(dailyWindScalIntAvg).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailyWindScalIntAvg] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyWindScalarIntensityMax))) )
            {
                intList.clear();
                QString dailyWindScalIntMax = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyWindScalarIntensityMax));
                myList = parameters->value(dailyWindScalIntMax).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailyWindScalIntMax] = intList;
            }
            if ( parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyWindVectorDirectionPrevailing))) )
            {
                intList.clear();
                QString dailyWindVecDirPrev = QString::fromStdString(getKeyStringMeteoMap(MapDailyMeteoVar, dailyWindVectorDirectionPrevailing));
                myList = parameters->value(dailyWindVecDirPrev).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetDailyMap[dailyWindVecDirPrev] = intList;
            }
            if (parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, airTemperature))))
            {
                intList.clear();
                QString airTemp = QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, airTemperature));
                myList = parameters->value(airTemp).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetHourlyMap[airTemp] = intList;
            }
            if (parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, precipitation))))
            {
                intList.clear();
                QString prec = QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, precipitation));
                myList = parameters->value(prec).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetHourlyMap[prec] = intList;
            }
            if (parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, airRelHumidity))))
            {
                intList.clear();
                QString airRelH = QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, airRelHumidity));
                myList = parameters->value(airRelH).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetHourlyMap[airRelH] = intList;
            }
            if (parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, globalIrradiance))))
            {
                intList.clear();
                QString globalIrr = QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, globalIrradiance));
                myList = parameters->value(globalIrr).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetHourlyMap[globalIrr] = intList;
            }
            if (parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, windScalarIntensity))))
            {
                intList.clear();
                QString windScaInt = QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, windScalarIntensity));
                myList = parameters->value(windScaInt).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetHourlyMap[windScaInt] = intList;
            }
            if (parameters->contains(QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, windVectorDirection))))
            {
                intList.clear();
                QString windVecDir = QString::fromStdString(getKeyStringMeteoMap(MapHourlyMeteoVar, windVectorDirection));
                myList = parameters->value(windVecDir).toStringList();
                for (int i = 0; i < myList.size(); i++)
                {
                    if (myList[i].toInt() > 0 && !intList.contains(myList[i].toInt()))
                    {
                        intList << myList[i].toInt();
                    }
                }
                idArkimetHourlyMap[windVecDir] = intList;
            }

/*
            for(std::map<QString, QList<int> >::const_iterator it = idArkimetDailyMap.begin();
                it != idArkimetDailyMap.end(); ++it)
            {
                qDebug() << "idArkimetDailyMap " << it->first << ":" << it->second << "\n";
            }
            for(std::map<QString, QList<int> >::const_iterator it = idArkimetHourlyMap.begin();
                it != idArkimetHourlyMap.end(); ++it)
            {
                qDebug() << "idArkimetHourlyMap " << it->first << ":" << it->second << "\n";
            }

*/
            parameters->endGroup();

        }

    }

    return true;
}


bool PragaProject::saveGrid(meteoVariable myVar, frequencyType myFrequency, const Crit3DTime& myTime, bool showInfo)
{
    std::string id;
    FormInfo myInfo;
    int infoStep = 1;

    if (myFrequency == daily)
    {
        if (showInfo)
        {
            QString infoStr = "Save meteo grid daily data";
            infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);
        }

        for (int row = 0; row < this->meteoGridDbHandler->gridStructure().header().nrRows; row++)
        {
            if (showInfo)
            {
                if ((row % infoStep) == 0)
                    myInfo.setValue(row);
            }
            for (int col = 0; col < this->meteoGridDbHandler->gridStructure().header().nrCols; col++)
            {
                if (this->meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
                {
                    if (!this->meteoGridDbHandler->gridStructure().isFixedFields())
                    {
                        this->meteoGridDbHandler->saveCellCurrentGridDaily(&errorString, QString::fromStdString(id), QDate(myTime.date.year, myTime.date.month, myTime.date.day), this->meteoGridDbHandler->getDailyVarCode(myVar), this->meteoGridDbHandler->meteoGrid()->meteoPoint(row,col).currentValue);
                    }
                    else
                    {
                        this->meteoGridDbHandler->saveCellCurrentGridDailyFF(&errorString, QString::fromStdString(id), QDate(myTime.date.year, myTime.date.month, myTime.date.day), QString::fromStdString(this->meteoGridDbHandler->getDailyPragaName(myVar)), this->meteoGridDbHandler->meteoGrid()->meteoPoint(row,col).currentValue);
                    }
                }
            }
        }
    }
    else if (myFrequency == hourly)
    {
        if (showInfo)
        {
            QString infoStr = "Save meteo grid hourly data";
            infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);
        }

        for (int row = 0; row < this->meteoGridDbHandler->gridStructure().header().nrRows; row++)
        {
            if (showInfo && (row % infoStep) == 0)
                myInfo.setValue(row);
            for (int col = 0; col < this->meteoGridDbHandler->gridStructure().header().nrCols; col++)
            {
                if (this->meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
                {
                    if (!this->meteoGridDbHandler->gridStructure().isFixedFields())
                    {
                        this->meteoGridDbHandler->saveCellCurrentGridHourly(&errorString, QString::fromStdString(id), QDateTime(QDate(myTime.date.year, myTime.date.month, myTime.date.day), QTime(myTime.getHour(), myTime.getMinutes(), myTime.getSeconds())), this->meteoGridDbHandler->getHourlyVarCode(myVar), this->meteoGridDbHandler->meteoGrid()->meteoPoint(row,col).currentValue);
                    }
                    else
                    {
                        this->meteoGridDbHandler->saveCellCurrentGridHourlyFF(&errorString, QString::fromStdString(id), QDateTime(QDate(myTime.date.year, myTime.date.month, myTime.date.day), QTime(myTime.getHour(), myTime.getMinutes(), myTime.getSeconds())), QString::fromStdString(this->meteoGridDbHandler->getHourlyPragaName(myVar)), this->meteoGridDbHandler->meteoGrid()->meteoPoint(row,col).currentValue);
                    }
                }
            }
        }
    }

    if (showInfo) myInfo.close();

    return true;
}

bool PragaProject::elaborationCheck(bool isMeteoGrid, bool isAnomaly)
{

    if (isMeteoGrid)
    {
        if (this->meteoGridDbHandler == nullptr)
        {
            errorString = "Load grid";
            return false;
        }
        else
        {
            if (this->clima == nullptr)
            {
                this->clima = new Crit3DClimate(); 
            }
            clima->setDb(this->meteoGridDbHandler->db());
        }
    }
    else
    {
        if (this->meteoPointsDbHandler == nullptr)
        {
            errorString = "Load meteo Points";
            return false;
        }
        else
        {
            if (this->clima == nullptr)
            {
                this->clima = new Crit3DClimate();
            }
            clima->setDb(this->meteoPointsDbHandler->getDb());
        }
    }
    if (isAnomaly)
    {
        if (this->referenceClima == nullptr)
        {
            this->referenceClima = new Crit3DClimate();
        }
        if (isMeteoGrid)
        {
            referenceClima->setDb(this->meteoGridDbHandler->db());
        }
        else
        {
            referenceClima->setDb(this->meteoPointsDbHandler->getDb());
        }
    }

    return true;
}

bool PragaProject::showClimateFields(bool isMeteoGrid, QStringList* climateDbElab, QStringList* climateDbVarList)
{
    QSqlDatabase db;
    if (isMeteoGrid)
    {
        if (this->meteoGridDbHandler == nullptr)
        {
            errorString = "Load grid";
            return false;
        }
        db = this->meteoGridDbHandler->db();
    }
    else
    {
        if (this->meteoPointsDbHandler == nullptr)
        {
            errorString = "Load meteo Points";
            return false;
        }
        db = this->meteoPointsDbHandler->getDb();
    }
    QStringList climateTables;

    if ( !showClimateTables(db, &errorString, &climateTables) )
    {
        errorString = "No climate tables";
        return false;
    }
    else
    {
        for (int i=0; i < climateTables.size(); i++)
        {
            selectAllElab(db, &errorString, climateTables[i], climateDbElab);
        }
        if (climateDbElab->isEmpty())
        {
            errorString = "Empty climate tables";
            return false;
        }
    }
    for (int i=0; i < climateDbElab->size(); i++)
    {
        QString elab = climateDbElab->at(i);
        QStringList words = elab.split('_');
        QString var = words[1];
        if (!climateDbVarList->contains(var))
        {
            climateDbVarList->append(var);
        }
    }
    return true;

}

void PragaProject::saveClimateResult(bool isMeteoGrid, QString climaSelected, int climateIndex, bool showInfo)
{

    FormInfo myInfo;
    int infoStep = 0;
    QString infoStr;

    QSqlDatabase db;
    QList<float> results;

    Crit3DClimateList climateList;
    QStringList climate;
    climate.push_back(climaSelected);

    climateList.setListClimateElab(climate);
    climateList.parserElaboration();

    // copy elaboration to clima
    clima->setYearStart(climateList.listYearStart().at(0));
    clima->setYearEnd(climateList.listYearEnd().at(0));
    clima->setPeriodType(climateList.listPeriodType().at(0));
    clima->setPeriodStr(climateList.listPeriodStr().at(0));
    clima->setGenericPeriodDateStart(climateList.listGenericPeriodDateStart().at(0));
    clima->setGenericPeriodDateEnd(climateList.listGenericPeriodDateEnd().at(0));
    clima->setNYears(climateList.listNYears().at(0));
    clima->setVariable(climateList.listVariable().at(0));
    clima->setElab1(climateList.listElab1().at(0));
    clima->setElab2(climateList.listElab2().at(0));
    clima->setParam1(climateList.listParam1().at(0));
    clima->setParam2(climateList.listParam2().at(0));
    clima->setParam1IsClimate(climateList.listParam1IsClimate().at(0));
    clima->setParam1ClimateField(climateList.listParam1ClimateField().at(0));

    QString table = "climate_" + climateList.listPeriodStr().at(0);

    if (isMeteoGrid)
    {
        if (showInfo)
        {
            infoStr = "Read Climate - Meteo Grid";
            infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);
        }
        std::string id;
        db = this->meteoGridDbHandler->db();
        for (int row = 0; row < meteoGridDbHandler->gridStructure().header().nrRows; row++)
        {
            if (showInfo && (row % infoStep) == 0 )
            {
                 myInfo.setValue(row);
            }
            for (int col = 0; col < meteoGridDbHandler->gridStructure().header().nrCols; col++)
            {
                if (meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
                {
                    Crit3DMeteoPoint* meteoPoint = meteoGridDbHandler->meteoGrid()->meteoPointPointer(row,col);
                    results = readElab(db, table.toLower(), &errorString, QString::fromStdString(meteoPoint->id), climaSelected);
                    if (results.size() < climateIndex)
                    {
                        errorString = "climate index error";
                        meteoPoint->climate = NODATA;
                    }
                    else
                    {
                        float value = results[climateIndex-1];
                        meteoPoint->climate = value;
                    }
                }
             }
        }
        meteoGridDbHandler->meteoGrid()->setIsElabValue(true);
    }
    else
    {
        if (showInfo)
        {
            infoStr = "Read Climate - Meteo Points";
            infoStep = myInfo.start(infoStr, nrMeteoPoints);
        }
        db = this->meteoPointsDbHandler->getDb();
        for (int i = 0; i < nrMeteoPoints; i++)
        {
            if (meteoPoints[i].active)
            {
                if (showInfo && (i % infoStep) == 0)
                {
                    myInfo.setValue(i);
                }
                QString id = QString::fromStdString(meteoPoints[i].id);
                results = readElab(db, table.toLower(), &errorString, id, climaSelected);
                if (results.size() < climateIndex)
                {
                    errorString = "climate index error";
                    meteoPoints[i].climate = NODATA;
                }
                else
                {
                    float value = results[climateIndex-1];
                    meteoPoints[i].climate = value;
                }
            }
        }
        setIsElabMeteoPointsValue(true);

    }
    if (showInfo) myInfo.close();
}

bool PragaProject::deleteClima(bool isMeteoGrid, QString climaSelected)
{
    QSqlDatabase db;

    QStringList words = climaSelected.split('_');
    QString period = words[2];
    QString table = "climate_" + period;

    if (isMeteoGrid)
    {
        db = this->meteoGridDbHandler->db();
    }
    else
    {
        db = this->meteoPointsDbHandler->getDb();
    }

    return deleteElab(db, &errorString, table.toLower(), climaSelected);
}


bool PragaProject::elaboration(bool isMeteoGrid, bool isAnomaly, bool saveClima)
{
    if (isMeteoGrid)
    {
        if (saveClima)
        {
            if (!climatePointsCycleGrid(true))
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        if (!isAnomaly)
        {
            if (!elaborationPointsCycleGrid(isAnomaly, true))
            {
                return false;
            }
        }
        else
        {
            if (!elaborationPointsCycleGrid(isAnomaly, true))
            {
                return false;
            }
        }
        meteoGridDbHandler->meteoGrid()->setIsElabValue(true);
    }
    else
    {
        if (saveClima)
        {
            if (!climatePointsCycle(true))
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        if (!isAnomaly)
        {
            if (!elaborationPointsCycle(isAnomaly, true))
            {
                return false;
            }
        }
        else
        {
            if (!elaborationPointsCycle(isAnomaly, true))
            {
                return false;
            }
        }
        setIsElabMeteoPointsValue(true);
    }

    return true;
}

bool PragaProject::elaborationPointsCycle(bool isAnomaly, bool showInfo)
{

    bool isMeteoGrid = 0; // meteoPoint
    int validCell = 0;

    FormInfo myInfo;
    int infoStep;
    QString infoStr;

    errorString.clear();

    Crit3DClimate* climaUsed = new Crit3DClimate();

    if (isAnomaly)
    {
        climaUsed->copyParam(referenceClima);
        if (showInfo)
        {
            infoStr = "Anomaly - Meteo Points";
            infoStep = myInfo.start(infoStr, nrMeteoPoints);
        }
    }
    else
    {
        climaUsed->copyParam(clima);
        if (showInfo)
        {
            infoStr = "Elaboration - Meteo Points";
            infoStep = myInfo.start(infoStr, nrMeteoPoints);
        }
    }


    QDate startDate(climaUsed->yearStart(), climaUsed->genericPeriodDateStart().month(), climaUsed->genericPeriodDateStart().day());
    QDate endDate(climaUsed->yearEnd(), climaUsed->genericPeriodDateEnd().month(), climaUsed->genericPeriodDateEnd().day());

    if (climaUsed->nYears() > 0)
    {
        endDate.setDate(climaUsed->yearEnd() + climaUsed->nYears(), climaUsed->genericPeriodDateEnd().month(), climaUsed->genericPeriodDateEnd().day());
    }
    else if (climaUsed->nYears() < 0)
    {
        startDate.setDate(climaUsed->yearStart() + climaUsed->nYears(), climaUsed->genericPeriodDateStart().month(), climaUsed->genericPeriodDateStart().day());
    }


//    if (clima->elab1() == "phenology")
//    {
//        Then currentPheno.setPhenoPoint i;  // TODO
//    }

    Crit3DMeteoPoint* meteoPointTemp = new Crit3DMeteoPoint;

    for (int i = 0; i < nrMeteoPoints; i++)
    {

        if (meteoPoints[i].active)
        {

            // copy data to MPTemp
            meteoPointTemp->id = meteoPoints[i].id;
            meteoPointTemp->point.utm.x = meteoPoints[i].point.utm.x;  // LC to compute distance in passingClimateToAnomaly
            meteoPointTemp->point.utm.y = meteoPoints[i].point.utm.y;  // LC to compute distance in passingClimateToAnomaly
            meteoPointTemp->point.z = meteoPoints[i].point.z;
            meteoPointTemp->latitude = meteoPoints[i].latitude;
            meteoPointTemp->elaboration = meteoPoints[i].elaboration;

            // meteoPointTemp should be init
            meteoPointTemp->nrObsDataDaysH = 0;
            meteoPointTemp->nrObsDataDaysD = 0;

            if (showInfo && (i % infoStep) == 0)
                        myInfo.setValue(i);


            if (isAnomaly && climaUsed->getIsClimateAnomalyFromDb())
            {
                if ( passingClimateToAnomaly(&errorString, meteoPointTemp, climaUsed, meteoPoints, nrMeteoPoints, clima->getElabSettings()) )
                {
                    validCell = validCell + 1;
                }
            }
            else
            {
                if ( elaborationOnPoint(&errorString, meteoPointsDbHandler, nullptr, meteoPointTemp, climaUsed, isMeteoGrid, startDate, endDate, isAnomaly, meteoSettings))
                {
                    validCell = validCell + 1;
                }
            }


            // save result to MP
            meteoPoints[i].elaboration = meteoPointTemp->elaboration;
            meteoPoints[i].anomaly = meteoPointTemp->anomaly;
            meteoPoints[i].anomalyPercentage = meteoPointTemp->anomalyPercentage;

        }

    } // end for
    if (showInfo) myInfo.close();

    if (validCell == 0)
    {
        if (errorString.isEmpty())
        {
            errorString = "no valid cells available";
        }
        delete meteoPointTemp;
        delete climaUsed;
        return false;
    }
    else
    {
        delete meteoPointTemp;
        delete climaUsed;
        return true;
    }

}


bool PragaProject::elaborationPointsCycleGrid(bool isAnomaly, bool showInfo)
{

    bool isMeteoGrid = true; // grid
    int validCell = 0;

    std::string id;

    FormInfo myInfo;
    int infoStep;
    QString infoStr;

    errorString.clear();

    Crit3DClimate* climaUsed = new Crit3DClimate();

    if (isAnomaly)
    {
        climaUsed->copyParam(referenceClima);
        if (showInfo)
        {
            infoStr = "Anomaly - Meteo Grid";
            infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);
        }
    }
    else
    {
        climaUsed->copyParam(clima);
        if (showInfo)
        {
            infoStr = "Elaboration - Meteo Grid";
            infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);
        }
    }

    QDate startDate(climaUsed->yearStart(), climaUsed->genericPeriodDateStart().month(), climaUsed->genericPeriodDateStart().day());
    QDate endDate(climaUsed->yearEnd(), climaUsed->genericPeriodDateEnd().month(), climaUsed->genericPeriodDateEnd().day());

    if (climaUsed->nYears() > 0)
    {
        endDate.setDate(climaUsed->yearEnd() + climaUsed->nYears(), climaUsed->genericPeriodDateEnd().month(), climaUsed->genericPeriodDateEnd().day());
    }
    else if (climaUsed->nYears() < 0)
    {
        startDate.setDate(climaUsed->yearStart() + climaUsed->nYears(), climaUsed->genericPeriodDateStart().month(), climaUsed->genericPeriodDateStart().day());
    }


     Crit3DMeteoPoint* meteoPointTemp = new Crit3DMeteoPoint;

     for (int row = 0; row < meteoGridDbHandler->gridStructure().header().nrRows; row++)
     {
         if (showInfo && (row % infoStep) == 0)
             myInfo.setValue(row);

         for (int col = 0; col < meteoGridDbHandler->gridStructure().header().nrCols; col++)
         {

            if (meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
            {

                Crit3DMeteoPoint* meteoPoint = meteoGridDbHandler->meteoGrid()->meteoPointPointer(row,col);

                // copy data to MPTemp
                meteoPointTemp->id = meteoPoint->id;
                meteoPointTemp->point.z = meteoPoint->point.z;
                meteoPointTemp->latitude = meteoPoint->latitude;
                meteoPointTemp->elaboration = meteoPoint->elaboration;

                // meteoPointTemp should be init
                meteoPointTemp->nrObsDataDaysH = 0;
                meteoPointTemp->nrObsDataDaysD = 0;

                if (isAnomaly && climaUsed->getIsClimateAnomalyFromDb())
                {
                    if ( passingClimateToAnomalyGrid(&errorString, meteoPointTemp, climaUsed))
                    {
                        validCell = validCell + 1;
                    }
                }
                else
                {
                    if  ( elaborationOnPoint(&errorString, nullptr, meteoGridDbHandler, meteoPointTemp, climaUsed, isMeteoGrid, startDate, endDate, isAnomaly, meteoSettings))
                    {
                        validCell = validCell + 1;
                    }
                }

                // save result to MP
                meteoPoint->elaboration = meteoPointTemp->elaboration;
                meteoPoint->anomaly = meteoPointTemp->anomaly;
                meteoPoint->anomalyPercentage = meteoPointTemp->anomalyPercentage;

            }

        }
    }

    if (showInfo) myInfo.close();

    if (validCell == 0)
    {
        if (errorString.isEmpty())
        {
            errorString = "no valid cells available";
        }
        delete meteoPointTemp;
        delete climaUsed;
        return false;
    }
    else
    {
        delete meteoPointTemp;
        delete climaUsed;
        return true;
    }

}

bool PragaProject::climatePointsCycle(bool showInfo)
{
    bool isMeteoGrid = false;
    FormInfo myInfo;
    int infoStep;
    QString infoStr;

    int validCell = 0;
    QDate startDate;
    QDate endDate;
    bool changeDataSet = true;

    errorString.clear();
    clima->resetCurrentValues();

    if (showInfo)
    {
        infoStr = "Climate  - Meteo Points";
        infoStep = myInfo.start(infoStr, nrMeteoPoints);
    }

    // parser all the list
    Crit3DClimateList* climateList = clima->getListElab();
    climateList->parserElaboration();

    Crit3DMeteoPoint* meteoPointTemp = new Crit3DMeteoPoint;
    for (int i = 0; i < nrMeteoPoints; i++)
    {
        if (meteoPoints[i].active)
        {

            if (showInfo && (i % infoStep) == 0)
            {
                myInfo.setValue(i);
            }

            meteoPointTemp->id = meteoPoints[i].id;
            meteoPointTemp->point.z = meteoPoints[i].point.z;
            meteoPointTemp->latitude = meteoPoints[i].latitude;
            changeDataSet = true;

            std::vector<float> outputValues;

            for (int j = 0; j < climateList->listClimateElab().size(); j++)
            {

                clima->resetParam();
                clima->setClimateElab(climateList->listClimateElab().at(j));


                if (climateList->listClimateElab().at(j)!= nullptr)
                {

                    // copy current elaboration to clima
                    clima->setYearStart(climateList->listYearStart().at(j));
                    clima->setYearEnd(climateList->listYearEnd().at(j));
                    clima->setPeriodType(climateList->listPeriodType().at(j));
                    clima->setPeriodStr(climateList->listPeriodStr().at(j));
                    clima->setGenericPeriodDateStart(climateList->listGenericPeriodDateStart().at(j));
                    clima->setGenericPeriodDateEnd(climateList->listGenericPeriodDateEnd().at(j));
                    clima->setNYears(climateList->listNYears().at(j));
                    clima->setVariable(climateList->listVariable().at(j));
                    clima->setElab1(climateList->listElab1().at(j));
                    clima->setElab2(climateList->listElab2().at(j));
                    clima->setParam1(climateList->listParam1().at(j));
                    clima->setParam2(climateList->listParam2().at(j));
                    clima->setParam1IsClimate(climateList->listParam1IsClimate().at(j));
                    clima->setParam1ClimateField(climateList->listParam1ClimateField().at(j));

                    if (clima->periodType() == genericPeriod)
                    {
                        startDate.setDate(clima->yearStart(), clima->genericPeriodDateStart().month(), clima->genericPeriodDateStart().day());
                        endDate.setDate(clima->yearEnd() + clima->nYears(), clima->genericPeriodDateEnd().month(), clima->genericPeriodDateEnd().day());
                    }
                    else if (clima->periodType() == seasonalPeriod)
                    {
                        startDate.setDate(clima->yearStart() -1, 12, 1);
                        endDate.setDate(clima->yearEnd(), 12, 31);
                    }
                    else
                    {
                        startDate.setDate(clima->yearStart(), 1, 1);
                        endDate.setDate(clima->yearEnd(), 12, 31);
                    }
                }
                else
                {
                    errorString = "parser elaboration error";
                    delete meteoPointTemp;
                    return false;
                }

                if (climateOnPoint(&errorString, meteoPointsDbHandler, nullptr, clima, meteoPointTemp, outputValues, isMeteoGrid, startDate, endDate, changeDataSet, meteoSettings))
                {
                    validCell = validCell + 1;
                }
                changeDataSet = false;

            }

        }
    }
    if (showInfo) myInfo.close();

    if (validCell == 0)
    {
        if (errorString.isEmpty())
        {
            errorString = "no valid cells available";
        }
        delete meteoPointTemp;
        return false;
    }
    else
    {
        delete meteoPointTemp;
        return true;
    }
}


bool PragaProject::climatePointsCycleGrid(bool showInfo)
{

    bool isMeteoGrid = true;
    FormInfo myInfo;
    int infoStep;
    QString infoStr;

    int validCell = 0;
    QDate startDate;
    QDate endDate;
    std::string id;
    bool changeDataSet = true;

    errorString.clear();
    clima->resetCurrentValues();

    if (showInfo)
    {
        infoStr = "Climate  - Meteo Grid";
        infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);
    }

    // parser all the list
    Crit3DClimateList* climateList = clima->getListElab();
    climateList->parserElaboration();

    Crit3DMeteoPoint* meteoPointTemp = new Crit3DMeteoPoint;
    for (int row = 0; row < meteoGridDbHandler->gridStructure().header().nrRows; row++)
    {
        if (showInfo && (row % infoStep) == 0)
            myInfo.setValue(row);

        for (int col = 0; col < meteoGridDbHandler->gridStructure().header().nrCols; col++)
        {

           if (meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
           {

               Crit3DMeteoPoint* meteoPoint = meteoGridDbHandler->meteoGrid()->meteoPointPointer(row,col);

               meteoPointTemp->id = meteoPoint->id;
               meteoPointTemp->point.z = meteoPoint->point.z;
               meteoPointTemp->latitude = meteoPoint->latitude;

               changeDataSet = true;
               std::vector<float> outputValues;

               for (int j = 0; j < climateList->listClimateElab().size(); j++)
               {

                   clima->resetParam();
                   clima->setClimateElab(climateList->listClimateElab().at(j));


                   if (climateList->listClimateElab().at(j)!= nullptr)
                   {

                       // copy current elaboration to clima
                       clima->setYearStart(climateList->listYearStart().at(j));
                       clima->setYearEnd(climateList->listYearEnd().at(j));
                       clima->setPeriodType(climateList->listPeriodType().at(j));
                       clima->setPeriodStr(climateList->listPeriodStr().at(j));
                       clima->setGenericPeriodDateStart(climateList->listGenericPeriodDateStart().at(j));
                       clima->setGenericPeriodDateEnd(climateList->listGenericPeriodDateEnd().at(j));
                       clima->setNYears(climateList->listNYears().at(j));
                       clima->setVariable(climateList->listVariable().at(j));
                       clima->setElab1(climateList->listElab1().at(j));
                       clima->setElab2(climateList->listElab2().at(j));
                       clima->setParam1(climateList->listParam1().at(j));
                       clima->setParam2(climateList->listParam2().at(j));
                       clima->setParam1IsClimate(climateList->listParam1IsClimate().at(j));
                       clima->setParam1ClimateField(climateList->listParam1ClimateField().at(j));

                       if (clima->periodType() == genericPeriod)
                       {
                           startDate.setDate(clima->yearStart(), clima->genericPeriodDateStart().month(), clima->genericPeriodDateStart().day());
                           endDate.setDate(clima->yearEnd() + clima->nYears(), clima->genericPeriodDateEnd().month(), clima->genericPeriodDateEnd().day());
                       }
                       else if (clima->periodType() == seasonalPeriod)
                       {
                           startDate.setDate(clima->yearStart() -1, 12, 1);
                           endDate.setDate(clima->yearEnd(), 12, 31);
                       }
                       else
                       {
                           startDate.setDate(clima->yearStart(), 1, 1);
                           endDate.setDate(clima->yearEnd(), 12, 31);
                       }

                   }
                   else
                   {
                       errorString = "parser elaboration error";
                       delete meteoPointTemp;
                       return false;
                   }

                   if (climateOnPoint(&errorString, nullptr, meteoGridDbHandler, clima, meteoPointTemp, outputValues, isMeteoGrid, startDate, endDate, changeDataSet, meteoSettings))
                   {
                       validCell = validCell + 1;
                   }
                   changeDataSet = false;

               }

           }
       }
   }

   if (showInfo) myInfo.close();

   if (validCell == 0)
   {
       if (errorString.isEmpty())
       {
           errorString = "no valid cells available";
       }
       delete meteoPointTemp;
       return false;
    }
    else
    {
        delete meteoPointTemp;
        return true;
    }

}

bool PragaProject::downloadDailyDataArkimet(QStringList variables, bool prec0024, QDate startDate, QDate endDate, bool showInfo)
{
    const int MAXDAYS = 30;

    QString id, dataset;
    QStringList datasetList;
    QList<QStringList> idList;

    QList<int> arkIdVar;
    Download* myDownload = new Download(meteoPointsDbHandler->getDbName());

    for( int i=0; i < variables.size(); i++ )
    {
        if ( !idArkimetDailyMap[variables[i]].isEmpty())
        {
            arkIdVar.append(idArkimetDailyMap[variables[i]]);
        }
        else
        {
            arkIdVar.append(myDownload->getDbArkimet()->getId(variables[i]));
        }
    }

    int index, nrPoints = 0;
    for( int i=0; i < nrMeteoPoints; i++ )
    {
        if (getMeteoPointSelected(i))
        {
            nrPoints ++;

            id = QString::fromStdString(meteoPoints[i].id);
            dataset = QString::fromStdString(meteoPoints[i].dataset);

            if (!datasetList.contains(dataset))
            {
                datasetList << dataset;
                QStringList myList;
                myList << id;
                idList.append(myList);
            }
            else
            {
                index = datasetList.indexOf(dataset);
                idList[index].append(id);
            }
        }
    }

    FormInfo myInfo;
    QString infoStr;

    int nrDays = startDate.daysTo(endDate) + 1;
    if (showInfo) myInfo.start(infoStr, nrPoints*nrDays);

    int currentPoints = 0.;
    for( int i=0; i < datasetList.size(); i++ )
    {
        QDate date1 = startDate;
        QDate date2 = std::min(date1.addDays(MAXDAYS-1), endDate);

        while (date1 <= endDate)
        {
            if (showInfo)
            {
                myInfo.setText("Download data from: " + date1.toString("yyyy-MM-dd") + " to: " + date2.toString("yyyy-MM-dd") + " dataset:" + datasetList[i]);
                currentPoints += idList[i].size() * (date1.daysTo(date2) + 1);
                myInfo.setValue(currentPoints);
            }

            myDownload->downloadDailyData(date1, date2, datasetList[i], idList[i], arkIdVar, prec0024);

            date1 = date2.addDays(1);
            date2 = std::min(date1.addDays(MAXDAYS-1), endDate);
        }
    }

    if (showInfo) myInfo.close();
    delete myDownload;
    return true;
}


bool PragaProject::downloadHourlyDataArkimet(QStringList variables, QDate startDate, QDate endDate, bool showInfo)
{
    const int MAXDAYS = 7;


    QList<int> arkIdVar;
    Download* myDownload = new Download(meteoPointsDbHandler->getDbName());

    for( int i=0; i < variables.size(); i++ )
    {
        if ( !idArkimetHourlyMap[variables[i]].isEmpty())
        {
            arkIdVar.append(idArkimetHourlyMap[variables[i]]);
        }
        else
        {
            arkIdVar.append(myDownload->getDbArkimet()->getId(variables[i]));
        }
    }

    int index, nrPoints = 0;
    QString id, dataset;
    QStringList datasetList;
    QList<QStringList> idList;

    for( int i=0; i < nrMeteoPoints; i++ )
    {
        if (getMeteoPointSelected(i))
        {
            nrPoints ++;

            id = QString::fromStdString(meteoPoints[i].id);
            dataset = QString::fromStdString(meteoPoints[i].dataset);

            if (!datasetList.contains(dataset))
            {
                datasetList << dataset;
                QStringList myList;
                myList << id;
                idList.append(myList);
            }
            else
            {
                index = datasetList.indexOf(dataset);
                idList[index].append(id);
            }
        }
    }

    FormInfo myInfo;
    QString infoStr;

    int nrDays = startDate.daysTo(endDate) + 1;
    if (showInfo) myInfo.start(infoStr, nrPoints*nrDays);

    int currentPoints = 0.;
    for( int i=0; i < datasetList.size(); i++ )
    {
        QDate date1 = startDate;
        QDate date2 = std::min(date1.addDays(MAXDAYS-1), endDate);

        while (date1 <= endDate)
        {
            if (showInfo)
            {
                myInfo.setText("Download data from: " + date1.toString("yyyy-MM-dd") + " to:" + date2.toString("yyyy-MM-dd") + " dataset:" + datasetList[i]);
                currentPoints += idList[i].size() * (date1.daysTo(date2) + 1);
                myInfo.setValue(currentPoints);
            }

            myDownload->downloadHourlyData(date1, date2, datasetList[i], idList[i], arkIdVar);

            date1 = date2.addDays(1);
            date2 = std::min(date1.addDays(MAXDAYS-1), endDate);
        }
    }

    if (showInfo) myInfo.close();
    delete myDownload;
    return true;
}


bool PragaProject::averageSeriesOnZonesMeteoGrid(meteoVariable variable, meteoComputation elab1MeteoComp, aggregationMethod spatialElab, float threshold, gis::Crit3DRasterGrid* zoneGrid, QDate startDate, QDate endDate, QString periodType, std::vector<float> &outputValues, bool showInfo)
{

    QString aggregationString = QString::fromStdString(getKeyStringAggregationMethod(spatialElab));
    std::vector <std::vector<int> > meteoGridRow(zoneGrid->header->nrRows, std::vector<int>(zoneGrid->header->nrCols, NODATA));
    std::vector <std::vector<int> > meteoGridCol(zoneGrid->header->nrRows, std::vector<int>(zoneGrid->header->nrCols, NODATA));
    meteoGridDbHandler->meteoGrid()->saveRowColfromZone(zoneGrid, meteoGridRow, meteoGridCol);


    float percValue;
    bool isMeteoGrid = true;
    std::string id;
    unsigned int zoneIndex = 0;
    int indexSeries = 0;
    float value;
    std::vector<float> outputSeries;
    std::vector <std::vector<int>> indexRowCol(meteoGridDbHandler->gridStructure().header().nrRows, std::vector<int>(meteoGridDbHandler->gridStructure().header().nrCols, NODATA));

    gis::updateMinMaxRasterGrid(zoneGrid);
    std::vector <std::vector<float> > zoneVector((unsigned int)(zoneGrid->maximum+1), std::vector<float>());

    FormInfo myInfo;
    QString infoStr;
    int infoStep = myInfo.start(infoStr, this->meteoGridDbHandler->gridStructure().header().nrRows);

    Crit3DMeteoPoint* meteoPointTemp = new Crit3DMeteoPoint;

     for (int row = 0; row < meteoGridDbHandler->gridStructure().header().nrRows; row++)
     {
         if (showInfo && (row % infoStep) == 0)
             myInfo.setValue(row);

         for (int col = 0; col < meteoGridDbHandler->gridStructure().header().nrCols; col++)
         {

            if (meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
            {

                Crit3DMeteoPoint* meteoPoint = meteoGridDbHandler->meteoGrid()->meteoPointPointer(row,col);

                // copy data to MPTemp
                meteoPointTemp->id = meteoPoint->id;
                meteoPointTemp->point.z = meteoPoint->point.z;
                meteoPointTemp->latitude = meteoPoint->latitude;
                meteoPointTemp->elaboration = meteoPoint->elaboration;

                // meteoPointTemp should be init
                meteoPointTemp->nrObsDataDaysH = 0;
                meteoPointTemp->nrObsDataDaysD = 0;

                outputValues.clear();
                bool dataLoaded = preElaboration(&errorString, nullptr, meteoGridDbHandler, meteoPointTemp, isMeteoGrid, variable, elab1MeteoComp, startDate, endDate, outputValues, &percValue, meteoSettings, clima->getElabSettings());
                if (dataLoaded)
                {
                    outputSeries.insert(outputSeries.end(), outputValues.begin(), outputValues.end());
                    indexRowCol[row][col] = indexSeries;
                    indexSeries = indexSeries + 1;
                }
            }
        }
    }

     int nrDays = int(startDate.daysTo(endDate) + 1);
     std::vector< std::vector<float> > dailyElabAggregation(nrDays, std::vector<float>(int(zoneGrid->maximum+1), NODATA));

     for (int day = 0; day < nrDays; day++)
     {

         for (int zoneRow = 0; zoneRow < zoneGrid->header->nrRows; zoneRow++)
         {
             for (int zoneCol = 0; zoneCol < zoneGrid->header->nrRows; zoneCol++)
             {

                float zoneValue = zoneGrid->value[zoneRow][zoneCol];
                if ( zoneValue != zoneGrid->header->flag)
                {
                    zoneIndex = (unsigned int)(zoneValue);

                    if (meteoGridRow[zoneRow][zoneCol] != NODATA && meteoGridCol[zoneRow][zoneCol] != NODATA)
                    {
                        if (indexRowCol[meteoGridRow[zoneRow][zoneCol]][meteoGridCol[zoneRow][zoneCol]] != NODATA)
                        {
                            value = outputSeries.at(indexRowCol[meteoGridRow[zoneRow][zoneCol]][meteoGridCol[zoneRow][zoneCol]]*outputValues.size()+day);
                            if (value != meteoGridDbHandler->gridStructure().header().flag)
                            {
                                zoneVector[zoneIndex].push_back(value);
                            }
                        }
                    }
                }
             }
         }

         for (unsigned int zonePos = 0; zonePos < zoneVector.size(); zonePos++)
         {
            std::vector<float> validValues;
            validValues = zoneVector[zonePos];
            if (threshold != NODATA)
            {
                extractValidValuesWithThreshold(validValues, threshold);
            }

            float res = NODATA;
            int size = int(validValues.size());

            switch (spatialElab)
            {
                case aggrAverage:
                    {
                        res = statistics::mean(validValues, size);
                        break;
                    }
                case aggrMedian:
                    {

                        res = sorting::percentile(validValues, &size, 50.0, true);
                        break;
                    }
                case aggrStdDeviation:
                    {
                        res = statistics::standardDeviation(validValues, size);
                        break;
                    }
            }
            dailyElabAggregation[unsigned(day)][zonePos] = res;
         }
         // clear zoneVector
         for (unsigned int zonePos = 0; zonePos < zoneVector.size(); zonePos++)
         {
            zoneVector[zonePos].clear();
         }

     }
     // save dailyElabAggregation result into DB
     if (!aggregationDbHandler->saveAggrData(int(zoneGrid->maximum+1), aggregationString, periodType, QDateTime(startDate), QDateTime(endDate), variable, dailyElabAggregation))
     {
         errorString = aggregationDbHandler->error();
         return false;
     }
     return true;

}


void PragaProject::savePragaParameters()
{
    parameters->beginGroup("elaboration");
        parameters->setValue("anomaly_pts_max_distance", QString::number(double(clima->getElabSettings()->getAnomalyPtsMaxDistance())));
        parameters->setValue("anomaly_pts_max_delta_z", QString::number(double(clima->getElabSettings()->getAnomalyPtsMaxDeltaZ())));
        parameters->setValue("grid_min_coverage", QString::number(double(clima->getElabSettings()->getGridMinCoverage())));
        parameters->setValue("compute_tmed", clima->getElabSettings()->getAutomaticTmed());
        parameters->setValue("compute_et0hs", clima->getElabSettings()->getAutomaticETP());
        parameters->setValue("merge_joint_stations", clima->getElabSettings()->getMergeJointStations());
    parameters->endGroup();
}

QString getMapFileOutName(meteoVariable myVar, QDate myDate, int myHour)
{
    std::string myName = getMeteoVarName(myVar);
    if (myName == "") return "";

    QString name = QString::fromStdString(myName);
    name += "_" + myDate.toString(Qt::ISODate);
    if (getVarFrequency(myVar) == hourly) name += "_" + QString::number(myHour);

    return name;
}

gis::Crit3DRasterGrid* PragaProject::getPragaMapFromVar(meteoVariable myVar)
{
    gis::Crit3DRasterGrid* myGrid = nullptr;

    myGrid = getHourlyMeteoRaster(myVar);
    if (myGrid == nullptr) myGrid = pragaHourlyMaps->getMapFromVar(myVar);
    if (myGrid == nullptr) myGrid = pragaDailyMaps->getMapFromVar(myVar);

    return myGrid;
}


bool PragaProject::timeAggregateGridVarHourlyInDaily(meteoVariable dailyVar, Crit3DDate dateIni, Crit3DDate dateFin)
{
    Crit3DDate myDate;
    Crit3DMeteoPoint* meteoPoint;

    for (unsigned col = 0; col < unsigned(meteoGridDbHandler->gridStructure().header().nrCols); col++)
        for (unsigned row = 0; row < unsigned(meteoGridDbHandler->gridStructure().header().nrRows); row++)
        {
            meteoPoint = meteoGridDbHandler->meteoGrid()->meteoPointPointer(row, col);
            if (meteoPoint->active)
                if (! aggregatedHourlyToDaily(dailyVar, meteoPoint, dateIni, dateFin, meteoSettings))
                    return false;
        }

    return true;
}

bool PragaProject::timeAggregateGrid(QDate dateIni, QDate dateFin, QList <meteoVariable> variables, bool loadData, bool saveData)
{
    // check variables
    if (variables.size() == 0)
    {
        logError("No variable");
        return false;
    }

    // check meteo grid
    if (! meteoGridLoaded)
    {
        logError("No meteo grid");
        return false;
    }

    // check dates
    if (dateIni.isNull() || dateFin.isNull() || dateIni > dateFin)
    {
        logError("Wrong period");
        return false;
    }

    // now only hourly-->daily
    if (loadData)
    {
        logInfo("Loading grid data... ");
        loadMeteoGridHourlyData(QDateTime(dateIni, QTime(1,0)), QDateTime(dateFin.addDays(1), QTime(0,0)), false);
    }

    foreach (meteoVariable myVar, variables)
        if (getVarFrequency(myVar) == daily)
            if (! timeAggregateGridVarHourlyInDaily(myVar, getCrit3DDate(dateIni), getCrit3DDate(dateFin))) return false;

    // saving hourly and daily meteo grid data to DB
    if (saveData)
    {
        QString myError;
        logInfo("Saving meteo grid data");
        if (! meteoGridDbHandler->saveGridData(&myError, QDateTime(dateIni, QTime(1,0,0)), QDateTime(dateFin.addDays(1), QTime(0,0,0)), variables)) return false;
    }

    return true;
}

bool PragaProject::interpolationMeteoGridPeriod(QDate dateIni, QDate dateFin, QList <meteoVariable> variables, QList <meteoVariable> aggrVariables, bool saveRasters)
{
    // check variables
    if (variables.size() == 0)
    {
        logError("No variable");
        return false;
    }

    // check meteo point
    if (! meteoPointsLoaded || nrMeteoPoints == 0)
    {
        logError("No meteo points");
        return false;
    }

    // check meteo grid
    if (! meteoGridLoaded)
    {
        logError("No meteo grid");
        return false;
    }

    // check dates
    if (dateIni.isNull() || dateFin.isNull() || dateIni > dateFin)
    {
        logError("Wrong period");
        return false;
    }

    if (interpolationSettings.getUseTAD())
    {
        logInfo("Loading topographic distance maps...");
        if (! loadTopographicDistanceMaps(false))
            return false;
    }

    //order variables for derived computation

    std::string id;
    std::string errString;
    QString myError, rasterName, varName;
    int myHour;
    QDate myDate = dateIni;
    gis::Crit3DRasterGrid* myGrid = new gis::Crit3DRasterGrid();
    meteoVariable myVar;
    frequencyType freq;
    bool isDaily = false, isHourly = false;

    // find out needed frequency
    foreach (myVar, variables)
    {
        freq = getVarFrequency(myVar);
        if (freq == hourly)
            isHourly = true;
        else if (freq == daily)
            isDaily = true;
    }

    int currentYear = NODATA;

    logInfo("Loading meteo points data... ");
    //load also one day in advance (for transmissivity)
    if (! loadMeteoPointsData(dateIni.addDays(-1), dateFin, isHourly, isDaily, false))
        return false;

    while (myDate <= dateFin)
    {
        // check proxy grid series
        if (currentYear != myDate.year())
        {
            if (checkProxyGridSeries(&interpolationSettings, DEM, proxyGridSeries, myDate))
            {
                logInfo("Interpolating proxy grid series...");
                if (! readProxyValues()) return false;
            }
        }

        if (isHourly)
        {
            for (myHour = 1; myHour <= 24; myHour++)
            {
                logInfo("Interpolating hourly variables for " + myDate.toString("dd/MM/yyyy") + " " + QString("%1").arg(myHour, 2, 10, QChar('0')) + ":00");

                foreach (myVar, variables)
                {
                    if (getVarFrequency(myVar) == hourly)
                    {
                        varName = QString::fromStdString(getMeteoVarName(myVar));
                        logInfo(varName);

                        if (myVar == airRelHumidity && interpolationSettings.getUseDewPoint()) {
                            if (interpolationSettings.getUseInterpolatedTForRH())
                                passInterpolatedTemperatureToHumidityPoints(getCrit3DTime(myDate, myHour));
                            if (! interpolationDemMain(airDewTemperature, getCrit3DTime(myDate, myHour), hourlyMeteoMaps->mapHourlyTdew, false)) return false;
                            hourlyMeteoMaps->computeRelativeHumidityMap(hourlyMeteoMaps->mapHourlyRelHum);

                            myGrid = getPragaMapFromVar(myVar);
                            rasterName = getMapFileOutName(airDewTemperature, myDate, myHour);
                            if (rasterName != "") gis::writeEsriGrid(getProjectPath().toStdString() + rasterName.toStdString(), myGrid, &errString);
                        }
                        else if (myVar == windVectorDirection || myVar == windVectorIntensity) {
                            if (! interpolationDemMain(windVectorX, getCrit3DTime(myDate, myHour), getPragaMapFromVar(windVectorX), false)) return false;
                            if (! interpolationDemMain(windVectorY, getCrit3DTime(myDate, myHour), getPragaMapFromVar(windVectorY), false)) return false;
                            if (! pragaHourlyMaps->computeWindVector()) return false;
                        }
                        else if (myVar == leafWetness) {
                            hourlyMeteoMaps->computeLeafWetnessMap() ;
                        }
                        else if (myVar == referenceEvapotranspiration) {
                            hourlyMeteoMaps->computeET0PMMap(DEM, radiationMaps);
                        }
                        else {
                            if (! interpolationDemMain(myVar, getCrit3DTime(myDate, myHour), getPragaMapFromVar(myVar), false)) return false;
                        }

                        myGrid = getPragaMapFromVar(myVar);
                        if (myGrid == nullptr) return false;

                        //save raster
                        if (saveRasters)
                        {
                            rasterName = getMapFileOutName(myVar, myDate, myHour);
                            if (rasterName != "") gis::writeEsriGrid(getProjectPath().toStdString() + rasterName.toStdString(), myGrid, &errString);
                        }

                        meteoGridDbHandler->meteoGrid()->spatialAggregateMeteoGrid(myVar, hourly, getCrit3DDate(myDate), myHour, 0, &DEM, myGrid, interpolationSettings.getMeteoGridAggrMethod());

                    }
                }
            }
        }

        if (isDaily)
        {
            logInfo("Interpolating daily variables for " + myDate.toString("dd/MM/yyyy"));

            foreach (myVar, variables)
            {
                if (getVarFrequency(myVar) == daily)
                {
                    varName = QString::fromStdString(getMeteoVarName(myVar));
                    logInfo(varName);

                    if (myVar == dailyReferenceEvapotranspirationHS) {
                        pragaDailyMaps->computeHSET0Map(&gisSettings, getCrit3DDate(myDate));
                    }
                    else {
                        if (! interpolationDemMain(myVar, getCrit3DTime(myDate, 0), getPragaMapFromVar(myVar), false)) return false;
                    }

                    // fix daily temperatures consistency
                    if (myVar == dailyAirTemperatureMax || myVar == dailyAirTemperatureMin) {
                        if (! pragaDailyMaps->fixDailyThermalConsistency()) return false;
                    }

                    //save raster
                    if (saveRasters)
                    {
                        rasterName = getMapFileOutName(myVar, myDate, 0);
                        gis::writeEsriGrid(getProjectPath().toStdString() + rasterName.toStdString(), getPragaMapFromVar(myVar), &errString);
                    }

                    meteoGridDbHandler->meteoGrid()->spatialAggregateMeteoGrid(myVar, daily, getCrit3DDate(myDate), myHour, 0, &DEM, myGrid, interpolationSettings.getMeteoGridAggrMethod());

                }
            }
        }

        myDate = myDate.addDays(1);
    }

    if (aggrVariables.count() > 0)
    {
        logInfo("Time integration");
        if (! timeAggregateGrid( dateIni, dateFin, aggrVariables, false, false)) return false;
    }

    // saving hourly and daily meteo grid data to DB
    logInfo("Saving meteo grid data");
    meteoGridDbHandler->saveGridData(&myError, QDateTime(dateIni, QTime(1,0,0)), QDateTime(dateFin.addDays(1), QTime(0,0,0)), variables);

    // restore original proxy grids
    logInfo("Restoring proxy grids");
    if (! loadProxyGrids())
        return false;

    return true;

}

bool PragaProject::interpolationMeteoGrid(meteoVariable myVar, frequencyType myFrequency, const Crit3DTime& myTime, bool showInfo)
{
    if (meteoGridDbHandler != nullptr)
    {
        gis::Crit3DRasterGrid *myRaster = new gis::Crit3DRasterGrid;
        if (!interpolationDemMain(myVar, myTime, myRaster, showInfo))
        {
            return false;
        }

        meteoGridDbHandler->meteoGrid()->spatialAggregateMeteoGrid(myVar, myFrequency, myTime.date, myTime.getHour(),
                            myTime.getMinutes(), &DEM, myRaster, interpolationSettings.getMeteoGridAggrMethod());
        meteoGridDbHandler->meteoGrid()->fillMeteoRaster();
    }
    else
    {
        errorString = "Open a Meteo Grid before.";
        return false;
    }

    return true;
}


#ifdef NETCDF
    bool PragaProject::exportMeteoGridToNetCDF(QString fileName)
    {
        if (! checkMeteoGridForExport()) return false;

        NetCDFHandler* netcdf = new NetCDFHandler();

        if (! netcdf->createNewFile(fileName.toStdString()))
        {
            logError("Wrong filename: " + fileName);
            return false;
        }

        if (! netcdf->writeGeoDimensions(meteoGridDbHandler->gridStructure().header()))
        {
            logError("Error in writing geo dimensions.");
            return false;
        }

        if (! netcdf->writeData_NoTime(meteoGridDbHandler->meteoGrid()->dataMeteoGrid))
        {
            logError("Error in writing data.");
            return false;
        }

        netcdf->close();
        delete netcdf;

        return true;
    }

    bool PragaProject::exportXMLElabGridToNetcdf(QString xmlName)
    {
        if (meteoGridDbHandler == nullptr)
        {
            return false;
        }
        Crit3DElabList *listXMLElab = new Crit3DElabList();
        Crit3DAnomalyList *listXMLAnomaly = new Crit3DAnomalyList();

        if (xmlName == "")
        {
            errorString = "Empty XML name";
            delete listXMLElab;
            delete listXMLAnomaly;
            return false;
        }
        if (!parseXMLElaboration(listXMLElab, listXMLAnomaly, xmlName, &errorString))
        {
            delete listXMLElab;
            delete listXMLAnomaly;
            return false;
        }
        if (listXMLElab->isMeteoGrid() == false)
        {
            errorString = "Datatype is not Grid";
            delete listXMLElab;
            delete listXMLAnomaly;
            return false;
        }
        if (listXMLElab->listAll().isEmpty() && listXMLAnomaly->listAll().isEmpty())
        {
            errorString = "There are not valid Elaborations or Anomalies";
            delete listXMLElab;
            delete listXMLAnomaly;
            return false;
        }
        if (clima == nullptr)
        {
            clima = new Crit3DClimate();
        }
        if (referenceClima == nullptr && !listXMLAnomaly->listAll().isEmpty())
        {
            referenceClima = new Crit3DClimate();
        }

        for (int i = 0; i<listXMLElab->listAll().size(); i++)
        {
            clima->setVariable(listXMLElab->listVariable()[i]);
            clima->setYearStart(listXMLElab->listYearStart()[i]);
            clima->setYearEnd(listXMLElab->listYearEnd()[i]);
            clima->setPeriodStr(listXMLElab->listPeriodStr()[i]);
            clima->setPeriodType(listXMLElab->listPeriodType()[i]);

            clima->setGenericPeriodDateStart(listXMLElab->listDateStart()[i]);
            clima->setGenericPeriodDateEnd(listXMLElab->listDateEnd()[i]);
            clima->setNYears(listXMLElab->listNYears()[i]);
            clima->setElab1(listXMLElab->listElab1()[i]);

            if (!listXMLElab->listParam1IsClimate()[i])
            {
                clima->setParam1IsClimate(false);
                clima->setParam1(listXMLElab->listParam1()[i]);
            }
            else
            {
                clima->setParam1IsClimate(true);
                clima->setParam1ClimateField(listXMLElab->listParam1ClimateField()[i]);
                int climateIndex = getClimateIndexFromElab(listXMLElab->listDateStart()[i], listXMLElab->listParam1ClimateField()[i]);
                clima->setParam1ClimateIndex(climateIndex);

            }
            clima->setElab2(listXMLElab->listElab2()[i]);
            clima->setParam2(listXMLElab->listParam2()[i]);

            elaborationPointsCycleGrid(false, false);
            meteoGridDbHandler->meteoGrid()->fillMeteoRasterElabValue();
            QString netcdfName = getCompleteFileName("ELAB_"+listXMLElab->listAll()[i]+".nc", PATH_PROJECT);
            exportMeteoGridToNetCDF(netcdfName);
            // reset param
            clima->resetParam();
            // reset current values
            clima->resetCurrentValues();
        }

        for (int i = 0; i<listXMLAnomaly->listAll().size(); i++)
        {
            clima->setVariable(listXMLAnomaly->listVariable()[i]);
            clima->setYearStart(listXMLAnomaly->listYearStart()[i]);
            clima->setYearEnd(listXMLAnomaly->listYearEnd()[i]);
            clima->setPeriodStr(listXMLAnomaly->listPeriodStr()[i]);
            clima->setPeriodType(listXMLAnomaly->listPeriodType()[i]);

            clima->setGenericPeriodDateStart(listXMLAnomaly->listDateStart()[i]);
            clima->setGenericPeriodDateEnd(listXMLAnomaly->listDateEnd()[i]);
            clima->setNYears(listXMLAnomaly->listNYears()[i]);
            clima->setElab1(listXMLAnomaly->listElab1()[i]);

            if (!listXMLAnomaly->listParam1IsClimate()[i])
            {
                clima->setParam1IsClimate(false);
                clima->setParam1(listXMLAnomaly->listParam1()[i]);
            }
            else
            {
                clima->setParam1IsClimate(true);
                clima->setParam1ClimateField(listXMLAnomaly->listParam1ClimateField()[i]);
                int climateIndex = getClimateIndexFromElab(listXMLAnomaly->listDateStart()[i], listXMLElab->listParam1ClimateField()[i]);
                clima->setParam1ClimateIndex(climateIndex);

            }
            clima->setElab2(listXMLAnomaly->listElab2()[i]);
            clima->setParam2(listXMLAnomaly->listParam2()[i]);

            referenceClima->setVariable(listXMLAnomaly->listVariable()[i]);
            referenceClima->setYearStart(listXMLAnomaly->listRefYearStart()[i]);
            referenceClima->setYearEnd(listXMLAnomaly->listRefYearEnd()[i]);
            referenceClima->setPeriodStr(listXMLAnomaly->listRefPeriodStr()[i]);
            referenceClima->setPeriodType(listXMLAnomaly->listRefPeriodType()[i]);

            referenceClima->setGenericPeriodDateStart(listXMLAnomaly->listRefDateStart()[i]);
            referenceClima->setGenericPeriodDateEnd(listXMLAnomaly->listRefDateEnd()[i]);
            referenceClima->setNYears(listXMLAnomaly->listRefNYears()[i]);
            referenceClima->setElab1(listXMLAnomaly->listRefElab1()[i]);

            if (!listXMLAnomaly->listRefParam1IsClimate()[i])
            {
                referenceClima->setParam1IsClimate(false);
                referenceClima->setParam1(listXMLAnomaly->listRefParam1()[i]);
            }
            else
            {
                referenceClima->setParam1IsClimate(true);
                referenceClima->setParam1ClimateField(listXMLAnomaly->listRefParam1ClimateField()[i]);
                int climateIndex = getClimateIndexFromElab(listXMLAnomaly->listRefDateStart()[i], listXMLAnomaly->listRefParam1ClimateField()[i]);
                referenceClima->setParam1ClimateIndex(climateIndex);
            }
            referenceClima->setElab2(listXMLAnomaly->listRefElab2()[i]);
            referenceClima->setParam2(listXMLAnomaly->listRefParam2()[i]);

            elaborationPointsCycleGrid(false, false);
            elaborationPointsCycleGrid(true, false);
            QString netcdfName;
            if (!listXMLAnomaly->isPercentage()[i])
            {
                meteoGridDbHandler->meteoGrid()->fillMeteoRasterAnomalyValue();
                netcdfName = getCompleteFileName("ANOMALY_"+listXMLAnomaly->listAll()[i]+".nc", PATH_PROJECT);
            }
            else
            {
                meteoGridDbHandler->meteoGrid()->fillMeteoRasterAnomalyPercValue();
                netcdfName = getCompleteFileName("ANOMALY_"+listXMLAnomaly->listAll()[i]+".nc", PATH_PROJECT);
            }

            exportMeteoGridToNetCDF(netcdfName);
            // reset param
            clima->resetParam();
            referenceClima->resetParam();
            // reset current values
            clima->resetCurrentValues();
            referenceClima->resetCurrentValues();
        }

        delete listXMLElab;
        delete listXMLAnomaly;
        return true;
    }

#endif

