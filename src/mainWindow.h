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
    #include "tileSources/OSMTileSource.h"
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

        void on_actionOpen_DEM_triggered();

        void on_actionMeteopointNewArkimet_triggered();
        void on_actionMeteopointOpen_triggered();
        void on_actionMeteopointClose_triggered();
        void on_actionMeteopointDownload_triggered();

        void on_actionMeteogridOpen_triggered();
        void on_actionMeteogridClose_triggered();

        void on_actionOpen_project_triggered();
        void on_actionClose_project_triggered();
        void on_actionSave_project_as_triggered();
        void on_actionSave_project_triggered();

        void on_actionShowPointsHide_triggered();
        void on_actionShowPointsLocation_triggered();
        void on_actionShowPointsCurrent_triggered();
        void on_actionShowGridHide_triggered();
        void on_actionShowGridLocation_triggered();
        void on_actionShowGridCurrent_triggered();
        void on_actionShowPointsElab_triggered();
        void on_actionShowGridElab_triggered();
        void on_meteoPoints_clicked();
        void on_grid_clicked();
        void on_actionShowPointsAnomalyAbs_triggered();
        void on_actionShowGridAnomalyAbs_triggered();
        void on_actionShowPointsAnomalyPerc_triggered();
        void on_actionShowGridAnomalyPerc_triggered();
        void on_actionShowPointsClimate_triggered();
        void on_actionShowGridClimate_triggered();

        void on_rasterOpacitySlider_sliderMoved(int position);
        void on_meteoGridOpacitySlider_sliderMoved(int position);

        void on_actionMapTerrain_triggered();
        void on_actionMapOpenStreetMap_triggered();
        void on_actionMapESRISatellite_triggered();

        void on_actionRectangle_Selection_triggered();
        void on_dateChanged();
        void on_rasterScaleButton_clicked();
        void on_variableButton_clicked();
        void on_frequencyButton_clicked();

        void enableAllDataset(bool toggled);
        void disableAllButton(bool toggled);

        void on_actionVariableQualitySpatial_triggered();
        void on_rasterRestoreButton_clicked();
        void on_timeEdit_valueChanged(int myHour);
        void on_dateEdit_dateChanged(const QDate &date);

        void on_actionInterpolation_to_DEM_triggered();
        void on_actionInterpolationCurrentTime_triggered();
        void on_actionInterpolationSettings_triggered();
        void on_actionWriteTAD_triggered();
        void on_actionLoadTAD_triggered();
        void on_actionSaveGridCurrentData_triggered();
        void on_actionInterpolateSaveGridPeriod_triggered();

        void on_actionCompute_elaboration_triggered();
        void on_actionCompute_anomaly_triggered();
        void on_actionCompute_climate_triggered();

        void on_actionParameters_triggered();
        void on_actionRadiationSettings_triggered();

        void on_actionClimate_fields_triggered();

        void on_actionOpen_aggregation_DB_triggered();
        void on_actionNew_aggregation_DB_triggered();
        bool on_actionAggregate_from_grid_triggered();

        void updateMaps();

        #ifdef NETCDF
            void on_actionNetCDF_Open_triggered();
            void on_actionNetCDF_Close_triggered();
            void on_actionNetCDF_ShowMetadata_triggered();
            void on_actionMeteogridExportNetcdf_triggered();
        #endif
        void showMeteoWidget();


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

        void mouseMoveEvent(QMouseEvent * event);

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

        void setMapSource(OSMTileSource::OSMTileType mySource);
        QString selectArkimetDataset(QDialog* datasetDialog);

        QPoint getMapPos(const QPoint& pos);
        bool isInsideMap(const QPoint& pos);

        void renderDEM();
        void clearDEM();
        void updateVariable();
        void updateDateTime();
        void resetMeteoPoints();
        void addMeteoPoints();
        void drawMeteoPoints();
        void redrawMeteoPoints(visualizationType showType, bool updateColorScale);
        void drawMeteoGrid();
        void redrawMeteoGrid(visualizationType showType, bool showInterpolationResult);
        void drawProject();
        void redrawTitle();

        void checkSaveProject();

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
