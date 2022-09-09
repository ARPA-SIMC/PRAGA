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
        ~MainWindow() override;

    private slots:

        void on_actionFileMeteogridOpen_triggered();
        void on_actionFileMeteogridClose_triggered();
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

        void on_variableButton_clicked();
        void on_frequencyButton_clicked();

        void enableAllDataset(bool toggled);
        void disableAllButton(bool toggled);

        void on_actionMeteopointQualitySpatial_triggered();

        void on_dateChanged();
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
        void callNewPointStatisticsWidget(std::string id, bool isGrid);
        void callNewHomogeneityTestWidget(std::string id);
        void callNewSynchronicityTestWidget(std::string id);
        void callSetSynchronicityReference(std::string id);
        void callChangeOrogCode(std::string id, int orogCode);

        void on_meteoPoints_clicked();
        void on_grid_clicked();

        void on_dayBeforeButton_clicked();

        void on_dayAfterButton_clicked();

        void on_actionMeteogridMissingData_triggered();

        void on_actionImport_data_XML_grid_triggered();

        void on_actionPointProperties_import_triggered();

        void on_actionAll_active_triggered();

        void on_actionAll_notActive_triggered();

        void on_actionSelected_active_triggered();

        void on_actionSelected_notActive_triggered();

        void on_actionFrom_point_list_active_triggered();

        void on_actionDeletePoint_selected_triggered();

        void on_actionDeletePoint_notSelected_triggered();

        void on_actionDeletePoint_notActive_triggered();

        void on_actionWith_NO_DATA_notActive_triggered();

        void on_actionDeleteData_Active_triggered();

        void on_actionDeleteData_notActive_triggered();

        void on_actionDeleteData_selected_triggered();

        void on_actionWith_Criteria_active_triggered();

        void on_actionWith_Criteria_notActive_triggered();

        void on_actionView_not_active_points_toggled(bool state);

        void on_action_Proxy_graph_triggered();

        void on_actionFileMeteopointNewCsv_triggered();

        void on_actionPointData_import_triggered();

        void on_actionNewMeteoGrid_triggered();

        void on_actionFileMeteogridExportRaster_triggered();

        void on_actionUpdate_properties_triggered();

        void on_actionUpdate_meteo_points_triggered();

        void on_actionUpdate_datasets_triggered();

        void on_actionPointStyleCircle_triggered();

        void on_actionPointStyleText_triggered();

        void on_actionPointStyleText_multicolor_triggered();

        void on_actionUnmark_all_points_triggered();

        bool on_actionSpatialAggregationFromGrid_triggered();

        void on_actionSpatialAggregationOpenDB_triggered();

        void on_actionSpatialAggregationNewDB_triggered();

        void on_actionInterpolationCrossValidation_triggered();

        void on_actionExport_current_data_triggered();

        void on_actionFileExportInterpolation_triggered();

        void on_actionFileDemOpen_triggered();

        void on_actionMark_from_pointlist_triggered();

        void on_actionSearch_point_triggered();

        void on_actionFileMeteogridDelete_triggered();

        void on_actioFileMeteogrid_Load_current_data_triggered();

        void on_actionMeteoGrid_Set_color_scale_triggered();

        void on_actionMeteoGrid_Reverse_color_scale_triggered();

        void on_flagMeteoGrid_Dynamic_color_scale_triggered(bool isChecked);

        void on_flagMeteoGrid_Fixed_color_scale_triggered(bool isChecked);

        void on_actionShiftDataAll_triggered();

        void on_actionShiftDataSelected_triggered();

        void on_actionMeteoGridActiveAll_triggered();

        void on_actionMeteoGridActiveWith_DEM_triggered();

        void on_actionFileMeteogridPlanGriddingPeriod_triggered();

    protected:
        /*!
         * \brief mouseReleaseEvent call moveCenter
         * \param event
         */
        void mouseReleaseEvent(QMouseEvent *event) override;

        /*!
         * \brief mouseDoubleClickEvent implements zoom In and zoom Out
         * \param event
         */
        void mouseDoubleClickEvent(QMouseEvent * event) override;

        void mousePressEvent(QMouseEvent *event) override;

        void resizeEvent(QResizeEvent * event) override;

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
        RubberBand *rubberBand;
        visualizationType currentPointsVisualization;
        visualizationType currentGridVisualization;
        bool viewNotActivePoints;
        QActionGroup *showPointsGroup;
        QActionGroup *showGridGroup;

        QList<QCheckBox*> datasetCheckbox;
        QCheckBox* all;

        void setTileSource(WebTileSource::WebTileType tileType);
        QString selectArkimetDataset(QDialog* datasetDialog);

        QPoint getMapPos(const QPoint& pos);
        bool isInsideMap(const QPoint& pos);
        bool updateSelection(const QPoint& pos);

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

        bool checkMeteoGridColorScale();
        void setColorScaleRange(bool isFixed);

        bool loadMeteoPoints(QString dbName);
        bool loadMeteoGrid(QString xmlName);
        bool newMeteoGrid(QString xmlName);
        bool openRaster(QString fileName, gis::Crit3DRasterGrid *myRaster);
        void setCurrentRaster(gis::Crit3DRasterGrid *myRaster);
        void interpolateDemGUI();
        void interpolateGridGUI();
        void interpolateCrossValidationGUI();
        void showElabResult(bool updateColorSCale, bool isMeteoGrid, bool isAnomaly, bool isAnomalyPerc, bool isClima, QString index);
        void closeEvent(QCloseEvent *event) override;

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
