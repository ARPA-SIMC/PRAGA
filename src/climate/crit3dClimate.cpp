#include <QtSql>

#include "commonConstants.h"
#include "crit3dClimate.h"
#include "meteo.h"
#include "crit3dClimateList.h"


Crit3DClimate::Crit3DClimate()
{
    _climateElab = "";
    _yearStart = NODATA;
    _yearEnd = NODATA;
    _periodType = noPeriodType;
    _variable = noMeteoVar;
    _periodStr = "";
    _genericPeriodDateStart.setDate(1800,1,1);
    _genericPeriodDateEnd.setDate(1800,1,1);
    _hourStart = NODATA;
    _hourEnd = NODATA;
    _nYears = NODATA;
    _elab1 = "";
    _param1 = NODATA;
    _param1IsClimate = false;
    _param1ClimateField = "";
    _param1ClimateIndex = NODATA;
    _isClimateAnomalyFromDb = false;
    _elab2 = "";
    _param2 = NODATA;
    _dailyCumulated = false;
    _offset = 0;

    _currentVar = noMeteoVar;
    _currentElab1 = "";
    _currentYearStart = NODATA;
    _currentYearEnd = NODATA;
    _currentPeriodType = noPeriodType;

    elabSettings = new Crit3DElaborationSettings();
    listElab = new Crit3DClimateList();
}

void Crit3DClimate::resetParam()
{
    _climateElab = "";
    _yearStart = NODATA;
    _yearEnd = NODATA;
    _periodType = noPeriodType;
    _variable = noMeteoVar;
    _periodStr = "";
    _genericPeriodDateStart.setDate(1800,1,1);
    _genericPeriodDateEnd.setDate(1800,1,1);
    _hourStart = NODATA;
    _hourEnd = NODATA;
    _nYears = NODATA;
    _elab1 = "";
    _param1 = NODATA;
    _param1IsClimate = false;
    _param1ClimateField = "";
    _param1ClimateIndex = NODATA;
    _isClimateAnomalyFromDb = false;
    _elab2 = "";
    _param2 = NODATA;
    _dailyCumulated = false;
    _offset = 0;

}

void Crit3DClimate::resetCurrentValues()
{
    _currentVar = noMeteoVar;
    _currentElab1 = "";
    _currentYearStart = NODATA;
    _currentYearEnd = NODATA;
    _currentPeriodType = noPeriodType;
}

void Crit3DClimate::copyParam(Crit3DClimate* myClimate)
{
    _db = myClimate->db();
    _climateElab = myClimate->climateElab();
    _yearStart = myClimate->yearStart();
    _yearEnd = myClimate->yearEnd();
    _periodType = myClimate->periodType();
    _variable = myClimate->variable();
    _periodStr = myClimate->periodStr();
    _genericPeriodDateStart = myClimate->genericPeriodDateStart();
    _genericPeriodDateEnd = myClimate->genericPeriodDateEnd();
    _hourStart = myClimate->hourStart();
    _hourEnd = myClimate->hourEnd();
    _nYears = myClimate->nYears();
    _elab1 = myClimate->elab1();
    _param1 = myClimate->param1();
    _param1IsClimate = myClimate->param1IsClimate();
    _param1ClimateField = myClimate->param1ClimateField();
    _param1ClimateIndex = myClimate->getParam1ClimateIndex();
    _isClimateAnomalyFromDb = myClimate->getIsClimateAnomalyFromDb();
    _elab2 = myClimate->elab2();
    _param2 = myClimate->param2();
    _dailyCumulated = myClimate->dailyCumulated();
    _offset = myClimate->offset();

}

Crit3DClimate::~Crit3DClimate()
{
}

QString Crit3DClimate::climateElab() const
{
    return _climateElab;
}

void Crit3DClimate::setClimateElab(const QString &climateElab)
{
    _climateElab = climateElab;
}


meteoVariable Crit3DClimate::variable() const
{
    return _variable;
}

void Crit3DClimate::setVariable(const meteoVariable &variable)
{
    _variable = variable;
}

QDate Crit3DClimate::genericPeriodDateStart() const
{
    return _genericPeriodDateStart;
}

void Crit3DClimate::setGenericPeriodDateStart(const QDate &genericPeriodDateStart)
{
    _genericPeriodDateStart = genericPeriodDateStart;
}

QDate Crit3DClimate::genericPeriodDateEnd() const
{
    return _genericPeriodDateEnd;
}

void Crit3DClimate::setGenericPeriodDateEnd(const QDate &genericPeriodDateEnd)
{
    _genericPeriodDateEnd = genericPeriodDateEnd;
}

int Crit3DClimate::nYears() const
{
    return _nYears;
}

void Crit3DClimate::setNYears(int nYears)
{
    _nYears = nYears;
}

QString Crit3DClimate::elab1() const
{
    return _elab1;
}

void Crit3DClimate::setElab1(const QString &elab1)
{
    _elab1 = elab1;
}

float Crit3DClimate::param1() const
{
    return _param1;
}

void Crit3DClimate::setParam1(float param1)
{
    _param1 = param1;
}

bool Crit3DClimate::param1IsClimate() const
{
    return _param1IsClimate;
}

void Crit3DClimate::setParam1IsClimate(bool param1IsClimate)
{
    _param1IsClimate = param1IsClimate;
}

QString Crit3DClimate::param1ClimateField() const
{
    return _param1ClimateField;
}

void Crit3DClimate::setParam1ClimateField(const QString &param1ClimateField)
{
    _param1ClimateField = param1ClimateField;
}

QString Crit3DClimate::elab2() const
{
    return _elab2;
}

void Crit3DClimate::setElab2(const QString &elab2)
{
    _elab2 = elab2;
}

float Crit3DClimate::param2() const
{
    return _param2;
}

void Crit3DClimate::setParam2(float param2)
{
    _param2 = param2;
}

climatePeriod Crit3DClimate::periodType() const
{
    return _periodType;
}

void Crit3DClimate::setPeriodType(const climatePeriod &periodType)
{
    _periodType = periodType;
}

QString Crit3DClimate::periodStr() const
{
    return _periodStr;
}

void Crit3DClimate::setPeriodStr(const QString &periodStr)
{
    _periodStr = periodStr;
}

Crit3DElaborationSettings *Crit3DClimate::getElabSettings() const
{
    return elabSettings;
}

void Crit3DClimate::setElabSettings(Crit3DElaborationSettings *value)
{
    elabSettings = value;
}

meteoVariable Crit3DClimate::getCurrentVar() const
{
    return _currentVar;
}

void Crit3DClimate::setCurrentVar(const meteoVariable &currentVar)
{
    _currentVar = currentVar;
}

QString Crit3DClimate::getCurrentElab1() const
{
    return _currentElab1;
}

void Crit3DClimate::setCurrentElab1(const QString &currentElab1)
{
    _currentElab1 = currentElab1;
}

int Crit3DClimate::getCurrentYearStart() const
{
    return _currentYearStart;
}

void Crit3DClimate::setCurrentYearStart(int currentYearStart)
{
    _currentYearStart = currentYearStart;
}

int Crit3DClimate::getCurrentYearEnd() const
{
    return _currentYearEnd;
}

void Crit3DClimate::setCurrentYearEnd(int currentYearEnd)
{
    _currentYearEnd = currentYearEnd;
}

climatePeriod Crit3DClimate::getCurrentPeriodType() const
{
    return _currentPeriodType;
}

void Crit3DClimate::setCurrentPeriodType(const climatePeriod &currentPeriodType)
{
    _currentPeriodType = currentPeriodType;
}

Crit3DClimateList *Crit3DClimate::getListElab() const
{
    return listElab;
}

void Crit3DClimate::resetListElab()
{
    listElab->resetListClimateElab();
}

void Crit3DClimate::setListElab(Crit3DClimateList *value)
{
    listElab = value;
}

int Crit3DClimate::getParam1ClimateIndex() const
{
    return _param1ClimateIndex;
}

void Crit3DClimate::setParam1ClimateIndex(int param1ClimateIndex)
{
    _param1ClimateIndex = param1ClimateIndex;
}

bool Crit3DClimate::getIsClimateAnomalyFromDb() const
{
    return _isClimateAnomalyFromDb;
}

void Crit3DClimate::setIsClimateAnomalyFromDb(bool isClimateAnomalyFromDb)
{
    _isClimateAnomalyFromDb = isClimateAnomalyFromDb;
}

bool Crit3DClimate::dailyCumulated() const
{
    return _dailyCumulated;
}

void Crit3DClimate::setDailyCumulated(bool newDailyCumulated)
{
    _dailyCumulated = newDailyCumulated;
}

void Crit3DClimate::setOffset(int offset)
{
    _offset = offset;
}

int Crit3DClimate::offset()
{
    return _offset;
}

const QSqlDatabase& Crit3DClimate::db() const
{
    return _db;
}

void Crit3DClimate::setDb(const QSqlDatabase &db)
{
    _db = db;
}


