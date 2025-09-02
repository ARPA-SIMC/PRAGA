#include "drought.h"
#include "commonConstants.h"
#include "basicMath.h"
#include "gammaFunction.h"
#include "meteo.h"

#include <algorithm>


Drought::Drought(droughtIndex _index, int _firstYear, int _lastYear, Crit3DDate _date, Crit3DMeteoPoint* _meteoPoint, Crit3DMeteoSettings* _meteoSettings)
{
    this->_index = _index;
    this->_firstYear = _firstYear;
    this->_lastYear = _lastYear;
    this->_date = _date;
    this->_meteoPoint = _meteoPoint;
    this->_meteoSettings = _meteoSettings;
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

    std::vector<float> sumSeries(_meteoPoint->nrObsDataDaysM);
    int start, end;

    if (_computeAll)
    {
        start = _timeScale;
        end = _meteoPoint->nrObsDataDaysM;
        for (int i = 0; i <= _timeScale; i++)
        {
            sumSeries[i] = NODATA;
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
        for (int i = 0; i < (start - _timeScale); i++)
        {
            sumSeries[i] = NODATA;
        }
    }

    for (int j = start; j <= end; j++)
    {
        sumSeries[j] = 0;
        for(int i = 0; i <= _timeScale; i++)
        {
            if ((j-i) >= 0 && j < _meteoPoint->nrObsDataDaysM)
            {
                if (_index == INDEX_SPI)
                {
                    if (_meteoPoint->obsDataM[j-i].prec != NODATA)
                    {
                        sumSeries[j] += _meteoPoint->obsDataM[j-i].prec;
                    }
                    else
                    {
                        sumSeries[j] = NODATA;
                        break;
                    }
                }
                else if(_index == INDEX_SPEI)
                {
                    if (_meteoPoint->obsDataM[j-i].prec != NODATA && _meteoPoint->obsDataM[j-i].et0_hs != NODATA)
                    {
                        sumSeries[j] += _meteoPoint->obsDataM[j-i].prec - _meteoPoint->obsDataM[j-i].et0_hs;
                    }
                    else if (_meteoPoint->obsDataM[j-i].bic != NODATA)
                    {
                        sumSeries[j] += _meteoPoint->obsDataM[j-i].bic;
                    }
                    else
                    {
                        sumSeries[j] = NODATA;
                        break;
                    }
                }
            }
            else
            {
                sumSeries[j] = NODATA;
            }
        }
    }

    // initialize
    for (int i = 0; i < _meteoPoint->nrObsDataDaysM; i++)
    {
        droughtResults.push_back(NODATA);
    }

    for (int j = start; j <= end; j++)
    {
        int myMonthIndex = (j % 12)+1;  // start from 1

        if (sumSeries[j] != NODATA)
        {
            if (_index == INDEX_SPI)
            {
                float gammaCDFRes = generalizedGammaCDF(sumSeries[j], currentGamma[myMonthIndex-1].beta, currentGamma[myMonthIndex-1].gamma, currentGamma[myMonthIndex-1].pzero);
                if (gammaCDFRes > 0 && gammaCDFRes < 1)
                {
                    droughtResults[j] = standardGaussianInvCDF(gammaCDFRes);
                }
            }
            else if(_index == INDEX_SPEI)
            {
                double logLogisticRes = logLogisticCDFRobust(sumSeries[j], currentLogLogistic[myMonthIndex-1].alpha,
                                                      currentLogLogistic[myMonthIndex-1].beta, currentLogLogistic[myMonthIndex-1].gamma);
                if (logLogisticRes > 0 && logLogisticRes < 1)
                {
                    droughtResults[j] = standardGaussianInvCDF(logLogisticRes);
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
    std::vector<float> sumSeries;
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
        sumSeries.push_back(0);
        for(int i = 0; i <= _timeScale; i++)
        {
            nTot = nTot + 1;
            if (_meteoPoint->obsDataM[j-i].prec != NODATA && _meteoPoint->obsDataM[j-i].et0_hs != NODATA)
            {
                sumSeries[n] += _meteoPoint->obsDataM[j-i].prec - _meteoPoint->obsDataM[j-i].et0_hs;
                count++;
            }
            else if (_meteoPoint->obsDataM[j-i].bic != NODATA)
            {
                sumSeries[n] += _meteoPoint->obsDataM[j-i].bic;
                count++;
            }
            else
            {
                sumSeries[n] = NODATA;
                count = 0;
                break;
            }
        }
        if ((float)count / nTot < (minPerc / 100))
        {
            sumSeries[n] = NODATA;
        }
        n = n + 1;
    }

    for (int i = 0; i < 12; i++)
    {
        int myMonth = ((_meteoPoint->obsDataM[indexStart]._month + i -1) % 12)+1;  //start from 1
        n = 0;
        monthSeries.clear();
        for (int j=i; j < sumSeries.size(); j=j+12)
        {
            if (sumSeries[j] != NODATA)
            {
                monthSeries.push_back(sumSeries[j]);
                n++;
            }
        }

        if (float(n) / (sumSeries.size()/12.) >= minPerc / 100.)
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

