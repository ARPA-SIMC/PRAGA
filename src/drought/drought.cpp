#include "drought.h"
#include "commonConstants.h"
#include "basicMath.h"
#include "gammaFunction.h"
#include "meteo.h"

#include <algorithm>


Drought::Drought(droughtIndex _index, int _firstYear, int _lastYear, Crit3DDate _date, Crit3DMeteoPoint* _meteoPoint, Crit3DMeteoSettings* _meteoSettings)
{
    _index = _index;
    _firstYear = _firstYear;
    _lastYear = _lastYear;
    _date = _date;
    _meteoPoint = _meteoPoint;
    _meteoSettings = _meteoSettings;
    _timeScale = 3; //default
    _computeAll = false;  //default
    _var = monthlyPrecipitation;  //default
    gammaStruct.beta = NODATA;
    gammaStruct.gamma = NODATA;
    gammaStruct.pzero = NODATA;
    logLogisticStruct.alpha = NODATA;
    logLogisticStruct.beta = NODATA;
    logLogisticStruct.gamma = NODATA;
    for (int i = 0; i<12; i++)
    {
        currentGamma.push_back(gammaStruct);
        currentLogLogistic.push_back(logLogisticStruct);
    }
    currentPercentileValue = NODATA;
}



float Drought::computeDroughtIndex()
{
    _timeScale = _timeScale - 1; // _index start from 0
    if (_index == INDEX_SPI)
    {
        if (! computeSpiParameters())
        {
            return NODATA;
        }
    }
    else if (_index == INDEX_SPEI)
    {
        if (! computeSpeiParameters())
        {
            return NODATA;
        }
    }

    int start, end;
    std::vector<float> mySum(_meteoPoint->nrObsDataDaysM);
    for (int i = 0; i < _meteoPoint->nrObsDataDaysM; i++)
    {
        droughtResults.push_back(NODATA);
    }

    if (_computeAll)
    {
        start = _timeScale;
        end = _meteoPoint->nrObsDataDaysM;
        for (int i = 0; i <= _timeScale; i++)
        {
            mySum[i] = NODATA;
        }
    }
    else
    {
        int currentYear = _date.year;
        int currentMonth = _date.month;
        end = (currentYear - _meteoPoint->obsDataM[0]._year)*12 + currentMonth-_meteoPoint->obsDataM[0]._month; // starts from 0
        start = end; // parte da 0
        if (end >= _meteoPoint->nrObsDataDaysM)
        {
            return NODATA;
        }
        for (int i = 0; i < start-_timeScale; i++)
        {
            mySum[i] = NODATA;
        }
    }

    for (int j = start; j <= end; j++)
    {
        mySum[j] = 0;
        for(int i = 0; i <= _timeScale; i++)
        {
            if ((j-i)>=0 && j < _meteoPoint->nrObsDataDaysM)
            {
                if (_index == INDEX_SPI)
                {
                    if (_meteoPoint->obsDataM[j-i].prec != NODATA)
                    {
                        mySum[j] = mySum[j] + _meteoPoint->obsDataM[j-i].prec;
                    }
                    else
                    {
                        mySum[j] = NODATA;
                        break;
                    }
                }
                else if(_index == INDEX_SPEI)
                {
                    if (_meteoPoint->obsDataM[j-i].prec != NODATA && _meteoPoint->obsDataM[j-i].et0_hs != NODATA)
                    {
                        mySum[j] = mySum[j] + _meteoPoint->obsDataM[j-i].prec - _meteoPoint->obsDataM[j-i].et0_hs;
                    }
                    else
                    {
                        mySum[j] = NODATA;
                        break;
                    }
                }
            }
            else
            {
                mySum[j] = NODATA;
            }
        }
    }

    for (int j = start; j <= end; j++)
    {
        int myMonthIndex = (j % 12)+1;  //start from 1

        if (mySum[j] != NODATA)
        {
            if (_index == INDEX_SPI)
            {
                float gammaCDFRes = generalizedGammaCDF(mySum[j], currentGamma[myMonthIndex-1].beta, currentGamma[myMonthIndex-1].gamma, currentGamma[myMonthIndex-1].pzero);
                if (gammaCDFRes > 0 && gammaCDFRes < 1)
                {
                    droughtResults[j] = float(standardGaussianInvCDF(gammaCDFRes));
                }
            }
            else if(_index == INDEX_SPEI)
            {
                float logLogisticRes = logLogisticCDF(mySum[j], currentLogLogistic[myMonthIndex-1].alpha, currentLogLogistic[myMonthIndex-1].beta, currentLogLogistic[myMonthIndex-1].gamma);
                if (logLogisticRes > 0 && logLogisticRes < 1)
                {
                    droughtResults[j] = float(standardGaussianInvCDF(logLogisticRes));
                }
            }
        }
    }

    return droughtResults[end];
}


bool Drought::computeSpiParameters()
{
    if (_meteoPoint->nrObsDataDaysM == 0)
    {
        return false;
    }

    if (_meteoPoint->obsDataM[0]._year > _lastYear || _meteoPoint->obsDataM[_meteoPoint->nrObsDataDaysM-1]._year < _firstYear)
    {
        return false;
    }
    int indexStart  = (_firstYear - _meteoPoint->obsDataM[0]._year)*12;
    if (indexStart < _timeScale)
    {
        indexStart = _timeScale;
    }
    if (_meteoPoint->obsDataM[indexStart]._year > _lastYear)
    {
        return false;
    }

    int lastYearStation = std::min(_meteoPoint->obsDataM[_meteoPoint->nrObsDataDaysM-1]._year, _lastYear);

    int n = 0;
    float count = 0;
    int nTot = 0;
    std::vector<float> mySums;
    std::vector<float> monthSeries;
    float minPerc = _meteoSettings->getMinimumPercentage();

    for (int j = indexStart; j<_meteoPoint->nrObsDataDaysM; j++)
    {
        if (_meteoPoint->obsDataM[j]._year > lastYearStation)
        {
            break;
        }
        count = 0;
        nTot = 0;
        mySums.push_back(0);
        for(int i = 0; i<= _timeScale; i++)
        {
            nTot = nTot + 1;
            if (_meteoPoint->obsDataM[j-i].prec != NODATA)
            {
                mySums[n] = mySums[n] + _meteoPoint->obsDataM[j-i].prec;
                count = count + 1;
            }
            else
            {
                    mySums[n] = NODATA;
                    count = 0;
                    break;
            }
        }
        if ( (float)count / nTot < (minPerc / 100) )
        {
            mySums[n] = NODATA;
        }
        n = n + 1;
    }

    for (int i = 0; i<12; i++)
    {
        int myMonth = ((_meteoPoint->obsDataM[indexStart]._month + i -1) % 12)+1;  //start from 1
        n = 0;

        monthSeries.clear();
        for (int j=i; j<mySums.size(); j=j+12)
        {
            if (mySums[j] != NODATA)
            {
                monthSeries.push_back(mySums[j]);
                n = n + 1;
            }
        }

        if ((float)n / (mySums.size()/12) >= minPerc / 100)
        {
            generalizedGammaFitting(monthSeries, n, &(currentGamma[myMonth-1].beta), &(currentGamma[myMonth-1].gamma),  &(currentGamma[myMonth-1].pzero));
        }
    }
    return true;
}

bool Drought::computeSpeiParameters()
{
    if (_meteoPoint->nrObsDataDaysM == 0)
    {
        return false;
    }

    if (_meteoPoint->obsDataM[0]._year > _lastYear || _meteoPoint->obsDataM[_meteoPoint->nrObsDataDaysM-1]._year < _firstYear)
    {
        return false;
    }

    int indexStart  = (_firstYear - _meteoPoint->obsDataM[0]._year)*12;
    if (indexStart < _timeScale)
    {
        indexStart = _timeScale;
    }
    if (_meteoPoint->obsDataM[indexStart]._year > _lastYear)
    {
        return false;
    }

    int lastYearStation = std::min(_meteoPoint->obsDataM[_meteoPoint->nrObsDataDaysM-1]._year, _lastYear);

    int n = 0;
    float count = 0;
    int nTot = 0;
    std::vector<float> mySums;
    std::vector<float> monthSeries;
    std::vector<float> pwm(3);
    float minPerc = _meteoSettings->getMinimumPercentage();

    for (int j = indexStart; j<_meteoPoint->nrObsDataDaysM; j++)
    {
        if (_meteoPoint->obsDataM[j]._year > lastYearStation)
        {
            break;
        }
        count = 0;
        nTot = 0;
        mySums.push_back(0);
        for(int i = 0; i<=_timeScale; i++)
        {
            nTot = nTot + 1;
            if (_meteoPoint->obsDataM[j-i].prec != NODATA && _meteoPoint->obsDataM[j-i].et0_hs != NODATA)
            {
                mySums[n] = mySums[n] + _meteoPoint->obsDataM[j-i].prec - _meteoPoint->obsDataM[j-i].et0_hs;
                count = count + 1;
            }
            else
            {
                    mySums[n] = NODATA;
                    count = 0;
                    break;
            }
        }
        if ( (float)count / nTot < (minPerc / 100))
        {
            mySums[n] = NODATA;
        }
        n = n + 1;
    }

    for (int i = 0; i<12; i++)
    {

        int myMonth = ((_meteoPoint->obsDataM[indexStart]._month + i -1) % 12)+1;  //start from 1
        n = 0;
        monthSeries.clear();
        for (int j=i; j<mySums.size(); j=j+12)
        {
            if (mySums[j] != NODATA)
            {
                monthSeries.push_back(mySums[j]);
                n = n + 1;
            }
        }

        if ((float)n / (mySums.size()/12) >= minPerc / 100)
        {
            // Sort values
            sorting::quicksortAscendingFloat(monthSeries, 0, int(monthSeries.size())-1);
            // Compute probability weighted moments
            probabilityWeightedMoments(monthSeries, n, pwm, 0, 0, false);
            // Fit a Log Logistic probability function
            logLogisticFitting(pwm, &currentLogLogistic[myMonth-1].alpha, &currentLogLogistic[myMonth-1].beta, &currentLogLogistic[myMonth-1].gamma);
        }
    }
    return true;
}

bool Drought::computePercentileValuesCurrentDay()
{
    if (_var == noMeteoVar)
    {
        return false;
    }
    if (_meteoPoint->obsDataM[0]._year > _lastYear || _meteoPoint->obsDataM[_meteoPoint->nrObsDataDaysM-1]._year < _firstYear)
    {
        return false;
    }
    int indexStart  = (_firstYear - _meteoPoint->obsDataM[0]._year)*12;
    if (_meteoPoint->obsDataM[indexStart]._year > _lastYear)
    {
        return false;
    }
    int lastYearStation = std::min(_meteoPoint->obsDataM[_meteoPoint->nrObsDataDaysM-1]._year, _lastYear);
    int myMonth = _meteoPoint->obsDataM[indexStart + _date.month - 1]._month;
    std::vector<float> myValues;
    int nValid = 0;
    int nTot = 0;
    float minPerc = _meteoSettings->getMinimumPercentage();

    for (int j = indexStart+myMonth-1; j < _meteoPoint->nrObsDataDaysM ; j=j+12)
    {
        if (_meteoPoint->obsDataM[j]._year > lastYearStation)
        {
            break;
        }
        Crit3DDate mydate(1,_meteoPoint->obsDataM[j]._month,_meteoPoint->obsDataM[j]._year);
        float myValue = _meteoPoint->getMeteoPointValueM(mydate, _var);
        if (myValue != NODATA)
        {
            myValues.push_back(myValue);
            nValid = nValid + 1;
            nTot = nTot + 1;
        }
        else
        {
            nTot = nTot + 1;
        }
    }
    if (nTot > 0)
    {
        if ((float)nValid/nTot >= minPerc / 100)
        {
            int _index = (_date.year - _meteoPoint->obsDataM[0]._year)*12 + _date.month -_meteoPoint->obsDataM[0]._month; // starts from 0
            if (_index < _meteoPoint->nrObsDataDaysM)
            {
                Crit3DDate mydate(1,_meteoPoint->obsDataM[_index]._month,_meteoPoint->obsDataM[_index]._year);
                float myValue = _meteoPoint->getMeteoPointValueM(mydate, _var);
                if (myValue != NODATA)
                {
                    currentPercentileValue = sorting::percentileRank(myValues, myValue, true);
                }
            }
        }
    }
    return true;
}

