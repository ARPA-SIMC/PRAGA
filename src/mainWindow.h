#ifndef MAINWINDOW_H
#define MAINWINDOW_H

    #include <QMainWindow>
    #include <QList>
    #include <QCheckBox>
    #include <QGroupBox>
    #include <QActionGroup>

    #include "Position.h"
    #include "rubberBand.h"
    #include "MapGraphicsView.h"
    #include "MapGraphicsScene.h"
    #include "tileSources/WebTileSource.h"
    #include "mapGraphicsRasterObject.h"
    #include "stationMarker.h"
    #include "colorLegend.h"
    #include "dbArkimet.h"
    #include "pragaProject.h"

    enum visualizationType {notShown, showLocation, showCurrentVariable, showElaboration, showAnomalyAbsolute, showAnomalyPercentage, showClimate};

    namespace Ui
    {
        class MainWindow;
    }

    /*!
     * \brief The MainWindow class
     */
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:

        explicit MainWindow( QWidget *parent = nullptr);
        ~MainWindow();

    private slots:

        void on_actionFileMeteogridOpen_triggered();
        void on_actionFileMeteogridClose_triggered();
        void on_actionFileOpenDEM_triggered();
        void on_actionFileOpenProject_triggered();
        void on_actionFileCloseProject_triggered();
        void on_actionFileSaveProjectAs_triggered();
        void on_actionFileSaveProject_triggered();
        void on_actionFileMeteopointNewArkimet_triggered();
        void on_actionFileMeteopointOpen_triggered();
        void on_actionFileMeteopointClose_triggered();
        void on_actionFileMeteopointDownload_triggered();

        void on_actionShowPointsHide_triggered();
        void on_actionShowPointsLocation_triggered();
        void on_actionShowPointsCurrent_triggered();
        void on_actionShowGridHide_triggered();
        void on_actionShowGridLocation_triggered();
        void on_actionShowGridCurrent_triggered();
        void on_actionShowPointsElab_triggered();
        void on_actionShowGridElab_triggered();
        void on_actionShowPointsAnomalyAbs_triggered();
        void on_actionShowGridAnomalyAbs_triggered();
        void on_actionShowPointsAnomalyPerc_triggered();
        void on_actionShowGridAnomalyPerc_triggered();
        void on_actionShowPointsClimate_triggered();
        void on_actionShowGridClimate_triggered();

        void on_actionMeteopointRectangleSelection_triggered();
        void on_actionMeteopointDataCount_triggered();

        void on_rasterOpacitySlider_sliderMoved(int position);
        void on_meteoGridOpacitySlider_sliderMoved(int position);

        void on_dateChanged();
        void on_rasterScaleButton_clicked();
        void on_variableButton_clicked();
        void on_frequencyButton_clicked();

        void enableAllDataset(bool toggled);
        void disableAllButton(bool toggled);

        void on_actionMeteopointQualitySpatial_triggered();
        void on_rasterRestoreButton_clicked();
        void on_timeEdit_valueChanged(int myHour);
        void on_dateEdit_dateChanged(const QDate &date);

        void on_actionInterpolationDem_triggered();
        void on_actionInterpolationMeteogridCurrentTime_triggered();
        void on_actionInterpolationSettings_triggered();
        void on_actionInterpolationMeteogridSaveCurrentData_triggered();
        void on_actionInterpolationMeteogridPeriod_triggered();
        void on_actionTopographicDistanceMapsSave_triggered();
        void on_actionTopographicDistanceMapsLoad_triggered();

        void on_actionElaboration_triggered();
        void on_actionAnomaly_triggered();
        void on_actionClimate_triggered();
        void on_actionClimateFields_triggered();

        void on_actionRadiationSettings_triggered();

        void on_actionSettingsParameters_triggered();

        void on_actionSettingsMapOpenStreetMap_triggered();
        void on_actionSettingsMapGoogleMap_triggered();
        void on_actionSettingsMapGoogleSatellite_triggered();
        void on_actionSettingsMapGoogleTerrain_triggered();
        void on_actionSettingsMapGoogleHybrid_triggered();
        void on_actionSettingsMapEsriSatellite_triggered();
        void on_actionSettingsMapStamenTerrain_triggered();

        void on_actionAnalysisOpenAggregationDB_triggered();
        void on_actionAnalysisNewAggregationDB_triggered();
        bool on_actionAnalysisAggregateFromGrid_triggered();

        void updateMaps();
        void mouseMove(const QPoint& eventPos);

        #ifdef NETCDF
            void on_actionNetCDF_Open_triggered();
            void on_actionNetCDF_Close_triggered();
            void on_actionNetCDF_ShowMetadata_triggered();
            void on_actionFileMeteogridExportNetcdf_triggered();
        #endif
        void callNewMeteoWidget(std::string id, std::string name, bool isGrid);
        void callAppendMeteoWidget(std::string id, std::string name, bool isGrid);

        void on_meteoPoints_clicked();
        void on_grid_clicked();

        void on_dayBeforeButton_clicked();

        void on_dayAfterButton_clicked();

    protected:
        /*!
         * \brief mouseReleaseEvent call moveCenter
         * \param event
         */
        void mouseReleaseEvent(QMouseEvent *event);

        /*!
         * \brief mouseDoubleClickEvent implements zoom In and zoom Out
         * \param event
         */
        void mouseDoubleClickEvent(QMouseEvent * event);

        void mousePressEvent(QMouseEvent *event);

        void resizeEvent(QResizeEvent * event);

    private:
        Ui::MainWindow* ui;

        Position* startCenter;
        MapGraphicsScene* mapScene;
        MapGraphicsView* mapView;
        RasterObject* rasterObj;
        RasterObject* meteoGridObj;
        ColorLegend *rasterLegend;
        ColorLegend *meteoPointsLegend;
        ColorLegend *meteoGridLegend;
        QList<StationMarker*> pointList;
        RubberBand *myRubberBand;
        visualizationType currentPointsVisualization;
        visualizationType currentGridVisualization;
        QActionGroup *showPointsGroup;
        QActionGroup *showGridGroup;

        QList<QCheckBox*> datasetCheckbox;
        QCheckBox* all;

        void setTileSource(WebTileSource::WebTileType tileType);
        QString selectArkimetDataset(QDialog* datasetDialog);

        QPoint getMapPos(const QPoint& pos);
        bool isInsideMap(const QPoint& pos);

        void renderDEM();
        void clearDEM();
        void updateVariable();
        void updateDateTime();
        void resetMeteoPointsMarker();
        void addMeteoPoints();
        void drawMeteoPoints();
        void redrawMeteoPoints(visualizationType showType, bool updateColorScale);
        void drawMeteoGrid();
        void redrawMeteoGrid(visualizationType showType, bool showInterpolationResult);
        void drawProject();
        void redrawTitle();

        void checkSaveProject();
        void closeMeteoPoints();
        void closeMeteoGrid();

        bool loadMeteoPoints(QString dbName);
        bool loadMeteoGrid(QString xmlName);
        bool openRaster(QString fileName, gis::Crit3DRasterGrid *myRaster);
        bool openShape(QString fileName);
        void setCurrentRaster(gis::Crit3DRasterGrid *myRaster);
        void interpolateDemGUI();
        void interpolateGridGUI();
        void showElabResult(bool updateColorSCale, bool isMeteoGrid, bool isAnomaly, bool isAnomalyPerc, bool isClima, QString index);

        #ifdef NETCDF
            void netCDF_exportDataSeries(gis::Crit3DGeoPoint geoPoint);
            void closeNetCDF();
        #endif
    };


    class KeyboardFilter : public QObject
    {
        Q_OBJECT
    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;
    };


#endif // MAINWINDOW_H
