#ifndef DROUGHT_H
#define DROUGHT_H

#ifndef METEOPOINT_H
    #include "meteoPoint.h"
#endif

enum droughtIndex {INDEX_SPI, INDEX_SPEI, INDEX_DECILES};

class Drought
{
public:
    Drought(droughtIndex index, int firstYear, int lastYear, Crit3DMeteoPoint* meteoPoint, Crit3DMeteoSettings* meteoSettings);

    droughtIndex getIndex() const;
    void setIndex(const droughtIndex &value);

    int getTimeScale() const;
    void setTimeScale(int value);

    int getFirstYear() const;
    void setFirstYear(int value);

    int getLastYear() const;
    void setLastYear(int value);

    bool getComputeAll() const;
    void setComputeAll(bool value);

    float computeDroughtIndex();
    bool computeSpiParameters();
    bool computeSpeiParameters();

    void setMeteoPoint(Crit3DMeteoPoint *value);

    Crit3DMeteoSettings *getMeteoSettings() const;

private:
    Crit3DMeteoPoint* meteoPoint;
    Crit3DMeteoSettings* meteoSettings;
    droughtIndex index;
    int timeScale;
    int firstYear;
    int lastYear;
    bool computeAll;

};

#endif // DROUGHT_H


