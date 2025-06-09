#ifndef DROUGHT_H
#define DROUGHT_H

#ifndef METEOPOINT_H
    #include "meteoPoint.h"
#endif

//SPI Gamma Distribution
struct gammaParam {
    double beta;
    double gamma;
    double pzero;
};

// SPEI Log-Logistic Distribution
struct logLogisticParam {
    double alpha;
    double beta;
    double gamma;
};

class Drought
{
public:
    Drought(droughtIndex index, int firstYear, int lastYear, Crit3DDate date, Crit3DMeteoPoint* meteoPoint, Crit3DMeteoSettings* meteoSettings);

    droughtIndex getIndex() const { return _index; }
    void setIndex(const droughtIndex &value) { _index = value; }

    int getTimeScale() const { return _timeScale; }
    void setTimeScale(int value) { _timeScale = value; }

    int getFirstYear() const { return _firstYear; }
    void setFirstYear(int value) { _firstYear = value;}

    int getLastYear() const { return _lastYear; }
    void setLastYear(int value) { _lastYear = value; }

    bool getComputeAll() const { return _computeAll; }
    void setComputeAll(bool value) { _computeAll = value; }

    void setMeteoPoint(Crit3DMeteoPoint *value) { _meteoPoint = value; }

    Crit3DMeteoSettings* getMeteoSettings() const { return _meteoSettings; }

    Crit3DDate getDate() const { return _date; }
    void setDate(const Crit3DDate &myDate) { _date = myDate; }

    float getCurrentPercentileValue() const { return currentPercentileValue; }

    void setVar(const meteoVariable &var) { _var = var; }

    float computeDroughtIndex();
    bool computeSpiParameters();
    bool computeSpeiParameters();
    bool computePercentileValuesCurrentDay();


private:
    Crit3DMeteoPoint* _meteoPoint;
    Crit3DMeteoSettings* _meteoSettings;
    Crit3DDate _date;
    droughtIndex _index;
    meteoVariable _var;
    int _timeScale;
    int _firstYear;
    int _lastYear;
    bool _computeAll;

    gammaParam gammaStruct;
    logLogisticParam logLogisticStruct;
    std::vector<gammaParam> currentGamma;
    std::vector<logLogisticParam> currentLogLogistic;
    std::vector<float> droughtResults;
    float currentPercentileValue;
};


#endif // DROUGHT_H



