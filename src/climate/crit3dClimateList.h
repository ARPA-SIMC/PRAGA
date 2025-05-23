#ifndef CRIT3DCLIMATELIST_H
#define CRIT3DCLIMATELIST_H

    #ifndef METEO_H
        #include "meteo.h"
    #endif

    #include "QList"
    #include "QString"
    #include "QDate"


    enum climatePeriod{ dailyPeriod, decadalPeriod, monthlyPeriod, seasonalPeriod, annualPeriod, genericPeriod, noPeriodType};

    const std::map<std::string, meteoComputation> MapMeteoComputation = {
      { "average", average },
      { "stdDev", stdDev },
      { "sum", sum },
      { "maxInList", maxInList },
      { "minInList", minInList },
      { "differenceWithThreshold", differenceWithThreshold },
      { "lastDayBelowThreshold", lastDayBelowThreshold },
      { "sumAbove", sumAbove },
      { "avgAbove", avgAbove },
      { "stdDevAbove", stdDevAbove },
      { "percentile", percentile },
      { "median", median },
      { "freqPositive", freqPositive },
      { "daysAbove", daysAbove },
      { "daysBelow", daysBelow },
      { "consecutiveDaysAbove", consecutiveDaysAbove },
      { "consecutiveDaysBelow", consecutiveDaysBelow },
      { "prevailingWindDir", prevailingWindDir },
      { "trend", trend },
      { "mannKendall", mannKendall },
      { "phenology", phenology },
      { "winkler", winkler },
      { "huglin", huglin },
      { "fregoni", fregoni },
      { "correctedDegreeDaysSum", correctedDegreeDaysSum },
      { "erosivityFactorElab", erosivityFactorElab },
      { "rainIntensityElab", rainIntensityElab },
      { "yearmax", yearMax },
      { "yearmin", yearMin },
      { "yearmin", yearMin }
    };

    class Crit3DClimateList
    {

    public:
        Crit3DClimateList();
        ~Crit3DClimateList();

        QList<QString> listClimateElab() const;
        void setListClimateElab(const QList<QString> &listClimateElab);

        void resetListClimateElab();

        std::vector<int> listYearStart() const;
        void setListYearStart(const std::vector<int> &listYearStart);

        std::vector<int> listYearEnd() const;
        void setListYearEnd(const std::vector<int> &listYearEnd);

        std::vector<meteoVariable> listVariable() const;
        void setListVariable(const std::vector<meteoVariable> &listVariable);

        std::vector<QString> listPeriodStr() const;
        void setListPeriodStr(const std::vector<QString> &listPeriodStr);

        std::vector<climatePeriod> listPeriodType() const;
        void setListPeriodType(const std::vector<climatePeriod> &listPeriodType);

        std::vector<QDate> listGenericPeriodDateStart() const;
        void setListGenericPeriodDateStart(const std::vector<QDate> &listGenericPeriodDateStart);

        std::vector<QDate> listGenericPeriodDateEnd() const;
        void setListGenericPeriodDateEnd(const std::vector<QDate> &listGenericPeriodDateEnd);

        std::vector<int> listNYears() const;
        void setListNYears(const std::vector<int> &listNYears);

        std::vector<QString> listElab1() const;
        void setListElab1(const std::vector<QString> &listElab1);

        std::vector<float> listParam1() const;
        void setListParam1(const std::vector<float> &listParam1);

        std::vector<bool> listParam1IsClimate() const;
        void setListParam1IsClimate(const std::vector<bool> &listParam1IsClimate);

        std::vector<QString> listParam1ClimateField() const;
        void setListParam1ClimateField(const std::vector<QString> &listParam1ClimateField);

        std::vector<QString> listElab2() const;
        void setListElab2(const std::vector<QString> &listElab2);

        std::vector<float> listParam2() const;
        void setListParam2(const std::vector<float> &listParam2);

        void parserElaboration(QString &errorStr);
        bool parserGenericPeriodString(int index);

        void insertDailyCumulated(bool dailyCumulated);
        std::vector<bool> listDailyCumulated() const;

        std::vector<int> listOffset() const;
        void setListOffset(int offset);


    private:

        QList<QString> _listClimateElab;
        std::vector<int> _listYearStart;
        std::vector<int> _listYearEnd;
        std::vector<meteoVariable> _listVariable;
        std::vector<QString> _listPeriodStr;
        std::vector<climatePeriod> _listPeriodType;
        std::vector<QDate> _listGenericPeriodDateStart;
        std::vector<QDate> _listGenericPeriodDateEnd;
        std::vector<int> _listNYears;
        std::vector<QString> _listElab1;
        std::vector<float> _listParam1;
        std::vector<bool> _listParam1IsClimate;
        std::vector<QString> _listParam1ClimateField;
        std::vector<QString> _listElab2;
        std::vector<float> _listParam2;
        std::vector<bool> _listDailyCumulated;
        std::vector<int> _listOffset;

    };


#endif // CRIT3DCLIMATELIST_H
