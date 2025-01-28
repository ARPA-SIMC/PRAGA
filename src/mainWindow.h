#ifndef MAINWINDOW_H
#define MAINWINDOW_H

    #include <QMainWindow>
    #include <QList>
    #include <QCheckBox>
    #include <QActionGroup>

    #include "rubberBand.h"
    #include "MapGraphicsView.h"
    #include "MapGraphicsScene.h"
    #include "tileSources/WebTileSource.h"
    #include "mapGraphicsRasterObject.h"
    #include "mapGraphicsRasterUtm.h"
    #include "stationMarker.h"
    #include "colorLegend.h"
    #include "ArrowObject.h"
    #include "squareMarker.h"

    enum visualizationType {notShown, showLocation, showCurrentVariable, showCVResidual, showElaboration, showAnomalyAbsolute, showAnomalyPercentage, showClimate};

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
        // File --> Meteopoint --> Arkimet
        void on_actionFileMeteopointArkimetDownload_triggered();
        void on_actionFileMeteopointArkimetUpdatePointProperties_triggered();
        void on_actionFileMeteopointArkimetUpdateMeteopoints_triggered();
        void on_actionFileMeteopointArkimetUpdateDatasets_triggered();

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

        void on_actionElaboration_Daily_data_triggered();
        void on_actionAnomaly_triggered();
        void on_actionClimate_triggered();

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
            void on_actionFileNetCDF_Open_triggered();
            void on_actionNetCDF_Close_triggered();
            void on_actionNetCDF_ShowMetadata_triggered();

            void on_netCDFButtonVariable_clicked();
            void on_actionFileMeteogridExportNetcdf_triggered();

            void on_actionShowNetcdfHide_triggered();
            void on_actionShowNetcdfLocation_triggered();
            void on_actionShowNetcdfVariable_triggered();

            void on_actionNetcdf_ColorScale_SetType_triggered();
            void on_actionNetcdf_ColorScale_Reverse_triggered();
            void on_actionNetcdf_ColorScale_Fixed_triggered();
            void on_actionNetcdf_ColorScale_RangeVariable_triggered();
        #endif

        void callNewMeteoWidget(std::string id, std::string name, std::string dataset, double altitude, std::string lapseRate, bool isGrid);
        void callAppendMeteoWidget(std::string id, std::string name, std::string dataset, double altitude, std::string lapseRate, bool isGrid);
        void callNewPointStatisticsWidget(std::string id, bool isGrid);
        void callNewHomogeneityTestWidget(std::string id);
        void callNewSynchronicityTestWidget(std::string id);
        void callSetSynchronicityReference(std::string id);
        void callChangeOrogCode(std::string id, int orogCode);
        void callMarkPoint(std::string myId);
        void callUnmarkPoint(std::string myId);
        void callLocalProxyGraph(const gis::Crit3DGeoPoint point);

        void on_dayBeforeButton_clicked();

        void on_dayAfterButton_clicked();

        void on_actionMeteogridMissingData_triggered();

        void on_actionImport_data_XML_grid_triggered();

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

        void on_actionFileMeteopointData_XMLimport_triggered();

        void on_actionFileMeteopointProperties_import_triggered();

        void on_actionNewMeteoGrid_triggered();

        void on_actionFileMeteogridExportRaster_triggered();

        void on_actionPointStyleCircle_triggered();

        void on_actionPointStyleText_triggered();

        void on_actionPointStyleText_multicolor_triggered();

        void on_actionUnmark_all_points_triggered();

        void on_actionSpatialAggregationFromGrid_triggered();

        void on_actionSpatialAggregationOpenDB_triggered();

        void on_actionSpatialAggregationNewDB_triggered();

        void on_actionInterpolationCVCurrentTime_triggered();

        void on_actionFileExportInterpolation_triggered();

        void on_actionFileDemOpen_triggered();

        void on_actionMark_from_pointlist_triggered();

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

        void on_actionInterpolationMeteogridGriddingTaskAdd_triggered();

        void on_actionInterpolationMeteogridGriddingTaskRemove_triggered();

        void on_actionDemRestore_triggered();

        void on_actionSearchPointName_triggered();

        void on_actionSearchPointId_triggered();

        void on_actionMeteoPointsClear_selection_triggered();

        void on_actionShow_InfoProject_triggered();

        void on_actionCompute_monthly_data_from_daily_triggered();

        void on_actionCompute_daily_from_Hourly_all_triggered();

        void on_actionCompute_daily_from_Hourly_selected_triggered();

        void on_netcdfOpacitySlider_sliderMoved(int position);

        void on_actionClimateMeteoPoints_triggered();

        void on_actionClimateMeteoGrid_triggered();

        void on_actionStatistical_Summary_triggered();

        void on_actionDemRangeFixed_triggered(bool isChecked);

        void on_actionDemRangeDynamic_triggered(bool isChecked);

        void on_actionExport_MeteoGrid_toCsv_triggered();

        void on_actionExport_MeteoPoints_toCsv_triggered();

        void on_actionOpenShell_triggered();

        void on_actionView_output_points_triggered();

        void on_actionFileOutputPoints_NewFromCsv_triggered();
        void on_actionFileOutputPointsClose_triggered();
        void on_actionFileOutputPointsOpen_triggered();

        void on_actionInterpolationOutputPointsCurrentTime_triggered();

        void on_actionInterpolationOutputPointsPeriod_triggered();

        void on_actionCompute_drought_triggered();

        void on_actionFileMeteogrid_ExportDailyData_triggered();

        void on_actionFileMeteopointData_XMLexport_triggered();

        void on_actionFileMeteogridData_XMLexport_triggered();

        void on_actionWaterTable_importLocation_triggered();

        void on_actionWaterTable_importDepth_triggered();

        void on_actionWaterTable_showLocation_triggered();

        void on_actionWaterTable_computeSingleWell_triggered();

        void on_actionWaterTable_showParameters_triggered();

        void on_actionWaterTable_showDepth_triggered();

        void on_actionWaterTable_Hide_triggered();

        void on_actionWaterTable_computeAllWells_triggered();

        void on_actionWaterTable_showId_triggered();

        void on_actionSpatialAggregationAssignAltitude_triggered();

        void on_actionInterpolationTopographicIndex_triggered();

        void on_actionInterpolationCVPeriod_triggered();

        void on_actionSpatialAggregationFromGrid_hourly_triggered();

        void on_actionInterpolationWriteGlocalWeightMaps_triggered();

        void on_actionHide_supplemental_stations_toggled(bool arg1);

        void on_actionFileLoadInterpolation_triggered();

        void on_actionDemZoom_to_layer_triggered();

        void on_actionElaboration_Hourly_data_triggered();

        void on_actionShowPointsCVResidual_triggered();

        void on_actionShowInfo_triggered();

        void on_actionMark_macro_area_stations_triggered();

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

        void keyPressEvent(QKeyEvent * event ) override;

        void closeEvent(QCloseEvent *event) override;

    private:
        Ui::MainWindow* ui;

        Position* startCenter;
        MapGraphicsScene* mapScene;
        MapGraphicsView* mapView;

        RasterUtmObject* rasterObj;
        RasterObject* meteoGridObj;
        RasterObject* netcdfObj;

        ColorLegend *rasterLegend;
        ColorLegend *meteoPointsLegend;
        ColorLegend *meteoGridLegend;
        ColorLegend *netcdfLegend;

        QList<StationMarker*> pointList;
        QList<SquareMarker*> outputPointList;
        QList<SquareMarker*> wellsListObj;
        QList<ArrowObject*> windVectorList;

        RubberBand *rubberBand;
        visualizationType currentPointsVisualization;
        visualizationType currentGridVisualization;
        visualizationType currentNetcdfVisualization;
        int currentNetcdfVariable;

        bool viewNotActivePoints;
        bool viewOutputPoints;
        bool hideSupplementals;

        QActionGroup *showPointsGroup;
        QActionGroup *showGridGroup;
        QActionGroup *showNetcdfGroup;

        QList<QCheckBox*> datasetCheckbox;
        QCheckBox* all;

        void setTileSource(WebTileSource::WebTileType tileType);
        QString selectArkimetDataset(QDialog* datasetDialog);

        QPoint getMapPos(const QPoint& pos);
        bool isInsideMap(const QPoint& pos);
        bool updateSelection(const QPoint& pos);

        void renderDEM();
        void zoomToDEM();
        void clearDEM();
        void updateVariable();
        void updateDateTime();
        void clearMeteoPointsMarker();
        void clearOutputPointMarkers();
        void addOutputPointsGUI();
        void clearWindVectorObjects();
        void addMeteoPoints();
        void drawMeteoPoints();
        void drawWindVector(int i);
        void redrawMeteoPoints(visualizationType showType, bool updateColorScale);
        void drawMeteoGrid();
        void redrawMeteoGrid(visualizationType showType, bool showInterpolationResult);
        void redrawAllData();
        void drawProject();
        void drawWindowTitle();

        void checkSaveProject();
        void closeMeteoPoints();
        void closeMeteoGrid();

        bool checkMeteoGridColorScale();
        void setColorScaleRangeMeteoGrid(bool isFixed);
        void setColorScaleRangeNetcdf(bool isFixed);

        bool checkDEMColorScale();
        void setColorScaleRangeDEM(bool isFixed);

        bool loadMeteoPoints(QString dbFileName);
        bool loadMeteoGrid(QString xmlName);
        bool newMeteoGrid(QString xmlName);
        void setCurrentRaster(gis::Crit3DRasterGrid *myRaster);
        void interpolateDemGUI();
        void interpolateGridGUI();
        void interpolateCrossValidationGUI();
        void showElabResult(bool updateColorSCale, bool isMeteoGrid, bool isAnomaly, bool isAnomalyPerc, bool isClima, QString index);
        void showCVResult();
        void searchMeteoPoint(bool isName);
        void computeDailyFromHourly_MeteoPoints(const QList<std::string>& pointList);

        void Export_to_png();
        void redrawOutputPoints();
        void addWellPointsGUI();
        void clearWellPointMarkers();

        #ifdef NETCDF
            void redrawNetcdf();
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
