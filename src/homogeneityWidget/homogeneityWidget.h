#ifndef HOMOGENEITYWIDGET_H
#define HOMOGENEITYWIDGET_H

    #include <QtWidgets>
    #include <QtCharts>
    #include "homogeneityChartView.h"
    #include "annualSeriesChartView.h"
    #include "meteoPoint.h"
    #include "dbMeteoPointsHandler.h"
    #include "crit3dClimate.h"
    #include "interpolationSettings.h"
    #include "interpolationPoint.h"


    class Crit3DHomogeneityWidget : public QWidget
    {
        Q_OBJECT

        public:
        Crit3DHomogeneityWidget(Crit3DMeteoPointsDbHandler* meteoPointsDbPointer, QList<Crit3DMeteoPoint>& nearMeteoPointsList,
                                const QList<std::string>& sortedIdList, const std::vector<float> &distanceList, const QList<QString> &jointStationsList,
                                const QDate &firstDaily, const QDate &lastDaily, Crit3DMeteoSettings *meteoSettings, QSettings *settings,
                                Crit3DClimateParameters *climateParameters, Crit3DQuality* quality);

            ~Crit3DHomogeneityWidget();

            void closeEvent(QCloseEvent *event);
            void changeMethod(const QString& methodName);
            void changeVar(const QString& varName);
            void changeYears();
            void plotAnnualSeries();
            void on_actionChangeLeftAxis();
            void on_actionExportHomogeneityGraph();
            void on_actionExportAnnualGraph();
            void on_actionExportAnnualData();
            void on_actionExportHomogeneityData();
            void addJointStationClicked();
            void deleteJointStationClicked();
            void saveToDbClicked();
            void updateYears();
            void findReferenceStations();
            void addFoundStationClicked();
            void deleteFoundStationClicked();
            void executeClicked();
            void checkValueAndMerge(const Crit3DMeteoPoint& meteoPointGet, Crit3DMeteoPoint* meteoPointSet, const QDate& myDate);

    private:
            Crit3DMeteoPointsDbHandler* _meteoPointsDbPointer;
            QList<Crit3DMeteoPoint> _nearMeteoPointsList;
            QList<std::string> _jointPointsIdList;
            QList<int> _jointIndexList;
            Crit3DClimate _climate;
            QDate _firstDaily;
            QDate _lastDaily;
            std::vector<float> _annualSeries;
            QList<std::string> _sortedIdList;
            std::vector<float> _distanceList;

            QMap<QString, std::string> mapNameId;
            QMap<QString, std::vector<float>> mapNameAnnualSeries;

            Crit3DMeteoSettings *meteoSettings;
            QSettings *settings;
            Crit3DClimateParameters *climateParameters;
            Crit3DQuality* quality;

            QComboBox variable;
            QComboBox method;
            QComboBox yearFrom;
            QComboBox yearTo;
            meteoVariable myVar;
            QPushButton find;
            HomogeneityChartView *homogeneityChartView;
            AnnualSeriesChartView *annualSeriesChartView;
            QComboBox jointStationsCombo;
            QPushButton addJointStation;
            QPushButton deleteJointStation;
            QPushButton saveToDb;
            QListWidget jointStationsSelected;
            QLineEdit minNumStations;
            QListWidget listFoundStations;
            QListWidget listSelectedStations;
            QList<QString> listAllFound;
            QPushButton addStationFoundButton;
            QPushButton deleteStationFoundButton;
            QTableWidget stationsTable;
            QLabel resultLabel;
            QPushButton execute;

            QString myError;
            double averageValue;
            float SNHT_T95_VALUES [10] {5.7f,6.95f,7.65f,8.1f,8.45f,8.65f,8.8f,8.95f,9.05f,9.15f};

            int getJointStationIndex(const std::string& id);
    };


#endif // HOMOGENEITYWIDGET_H
