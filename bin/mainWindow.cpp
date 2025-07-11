#include <sstream>
#include "math.h"

#include "mainWindow.h"
#include "ui_mainWindow.h"

#include "commonConstants.h"
#include "basicMath.h"
#include "spatialControl.h"
#include "utilities.h"
#include "interpolation.h"
#include "formTimePeriod.h"
#include "formSelection.h"
#include "formSelectionSource.h"
#include "formText.h"
#include "dbMeteoPointsHandler.h"
#include "download.h"
#include "dialogSelection.h"
#include "dialogDownloadMeteoData.h"
#include "dialogMeteoComputation.h"
#include "dialogMeteoHourlyComputation.h"
#include "dialogComputeDroughtIndex.h"
#include "dialogClimateFields.h"
#include "dialogSeriesOnZones.h"
#include "dialogInterpolation.h"
#include "dialogRadiation.h"
#include "dialogPragaSettings.h"
#include "dialogPragaProject.h"
#include "dialogPointProperties.h"
#include "dialogCellSize.h"
#include "dialogSelectDataset.h"
#include "dialogAddMissingStation.h"
#include "dialogAddRemoveDataset.h"
#include "dialogShiftData.h"
#include "dialogComputeData.h"
#include "meteoWidget.h"
#include "pragaShell.h"
#include "dialogExportDataGrid.h"
#include "dialogSelectWell.h"
#include "squareMarker.h"
#include "gis.h"

extern PragaProject myProject;

#define MAPBORDER 10


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);

    this->rubberBand = nullptr;

    // Set the MapGraphics Scene and View
    this->mapScene = new MapGraphicsScene(this);
    this->mapView = new MapGraphicsView(mapScene, this->ui->widgetMap);

    // set color legends
    this->meteoPointsLegend = new ColorLegend(this->ui->widgetColorLegendPoints);
    this->meteoPointsLegend->resize(this->ui->widgetColorLegendPoints->size());
    this->meteoPointsLegend->colorScale = myProject.meteoPointsColorScale;

    this->rasterLegend = new ColorLegend(this->ui->widgetColorLegendRaster);
    this->rasterLegend->resize(this->ui->widgetColorLegendRaster->size());

    this->meteoGridLegend = new ColorLegend(this->ui->widgetColorLegendGrid);
    this->meteoGridLegend->resize(this->ui->widgetColorLegendGrid->size());

    this->netcdfLegend = new ColorLegend(this->ui->widgetColorLegendNetcdf);
    this->netcdfLegend->resize(this->ui->widgetColorLegendNetcdf->size());

    // Set tiles source
    this->setTileSource(WebTileSource::OPEN_STREET_MAP);

    // Set start size and position
    this->startCenter = new Position (myProject.gisSettings.startLocation.longitude,
                                     myProject.gisSettings.startLocation.latitude, 0.0);
    this->mapView->setZoomLevel(8);
    this->mapView->centerOn(startCenter->lonLat());

    // Set raster objects
    this->rasterObj = new RasterUtmObject(this->mapView);
    this->meteoGridObj = new RasterObject(this->mapView);
    this->netcdfObj = new RasterObject(this->mapView);

    // set opacity
    this->rasterObj->setOpacity(this->ui->rasterOpacitySlider->value() / 100.0);
    this->meteoGridObj->setOpacity(this->ui->meteoGridOpacitySlider->value() / 100.0);
    this->netcdfObj->setOpacity(this->ui->netcdfOpacitySlider->value() / 100.0);

    // set color legend
    this->rasterObj->setColorLegend(this->rasterLegend);
    this->meteoGridObj->setColorLegend(this->meteoGridLegend);
    this->netcdfObj->setColorLegend(this->netcdfLegend);

    // add raster objects
    this->mapView->scene()->addObject(this->rasterObj);
    this->mapView->scene()->addObject(this->meteoGridObj);
    this->mapView->scene()->addObject(this->netcdfObj);

    connect(this->mapView, SIGNAL(zoomLevelChanged(quint8)), this, SLOT(updateMaps()));
    connect(this->mapView, SIGNAL(mouseMoveSignal(const QPoint&)), this, SLOT(mouseMove(const QPoint&)));

    KeyboardFilter *keyboardFilter = new KeyboardFilter();
    this->ui->dateEdit->installEventFilter(keyboardFilter);
    //connect(this->ui->dateEdit, SIGNAL(editingFinished()), this, SLOT(on_dateChanged()));

    // show menu
    showPointsGroup = new QActionGroup(this);
    showPointsGroup->setExclusive(true);
    showPointsGroup->addAction(this->ui->actionShowPointsHide);
    showPointsGroup->addAction(this->ui->actionShowPointsLocation);
    showPointsGroup->addAction(this->ui->actionShowPointsCurrent);
    showPointsGroup->addAction(this->ui->actionShowPointsCVResidual);
    showPointsGroup->addAction(this->ui->actionShowPointsElab);
    showPointsGroup->addAction(this->ui->actionShowPointsAnomalyAbs);
    showPointsGroup->addAction(this->ui->actionShowPointsAnomalyPerc);
    showPointsGroup->addAction(this->ui->actionShowPointsClimate);

    showPointsGroup->setEnabled(false);
    this->ui->menuShowPointsAnomaly->setEnabled(false);

    showGridGroup = new QActionGroup(this);
    showGridGroup->setExclusive(true);
    showGridGroup->addAction(this->ui->actionShowGridHide);
    showGridGroup->addAction(this->ui->actionShowGridLocation);
    showGridGroup->addAction(this->ui->actionShowGridCurrent);
    showGridGroup->addAction(this->ui->actionShowGridElab);
    showGridGroup->addAction(this->ui->actionShowGridAnomalyAbs);
    showGridGroup->addAction(this->ui->actionShowGridAnomalyPerc);
    showGridGroup->addAction(this->ui->actionShowGridClimate);

    showGridGroup->setEnabled(false);
    this->ui->menuShowGridAnomaly->setEnabled(false);

    showNetcdfGroup = new QActionGroup(this);
    showNetcdfGroup->setExclusive(true);
    showNetcdfGroup->addAction(this->ui->actionShowNetcdfHide);
    showNetcdfGroup->addAction(this->ui->actionShowNetcdfLocation);
    showNetcdfGroup->addAction(this->ui->actionShowNetcdfVariable);

    showNetcdfGroup->setEnabled(false);

    currentPointsVisualization = notShown;
    currentGridVisualization = notShown;
    currentNetcdfVisualization = notShown;
    viewNotActivePoints = false;
    viewOutputPoints = true;
    hideSupplementals = false;
    currentNetcdfVariable = NODATA;

    ui->groupBoxElaboration->hide();
    ui->groupBoxNetcdf->hide();
    ui->groupBoxCV->hide();

    updateVariable();
    updateDateTime();

    setWindowTitle(myProject.getVersion());

    #ifndef NETCDF
        ui->menuFileNetCDF->setEnabled(false);
    #endif

    showMaximized();
}


MainWindow::~MainWindow()
{
    delete rasterObj;
    delete meteoGridObj;
    delete netcdfObj;

    delete meteoPointsLegend;
    delete rasterLegend;
    delete meteoGridLegend;
    delete netcdfLegend;

    delete mapView;
    delete mapScene;
    delete ui;
}

bool KeyboardFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
}


// SLOT
void MainWindow::keyPressEvent(QKeyEvent * event)
{
    try
    {
        // zoom in/out
        if( event->key() == Qt::Key_Plus)
        {
            this->mapView->zoomIn();
        }
        else if( event->key() == Qt::Key_Minus)
        {
            this->mapView->zoomOut();
        }
    }
    catch (...)
    {
        myProject.logWarning("Exception catch in keyPressEvent.");
    }
}


void MainWindow::mouseMove(const QPoint& mapPos)
{
    if (! isInsideMap(mapPos)) return;

    // rubber band
    if (rubberBand != nullptr && rubberBand->isActive)
    {
        QPoint widgetPos = mapPos + QPoint(MAPBORDER, MAPBORDER);
        rubberBand->setGeometry(QRect(rubberBand->getOrigin(), widgetPos).normalized());
        return;
    }

    Position geoPos = this->mapView->mapToScene(mapPos);
    QString status = "Lat:" + QString::number(geoPos.latitude())
                   + " - Lon:" + QString::number(geoPos.longitude());

    // raster
    float value = NODATA;
    if (rasterObj->isLoaded && rasterObj->visible())
    {
        value = rasterObj->getValue(geoPos);

        if (!isEqual(value, NODATA))
            status += " - Raster: " + QString::number(double(value),'f',1);

        gis::Crit3DGeoPoint geoPoint = gis::Crit3DGeoPoint(geoPos.latitude(), geoPos.longitude());
        int row, col;
        gis::getRowColFromLatLon(rasterObj->getLatLonHeader(), geoPoint, &row, &col);
        status += " - Row: " + QString::number(row) + ", col: " + QString::number(col);

    }

    // meteoGrid
    value = NODATA;
    gis::Crit3DGeoPoint geoPoint = gis::Crit3DGeoPoint(geoPos.latitude(), geoPos.longitude());
    if (meteoGridObj->isLoaded && currentGridVisualization != notShown)
    {
        int row, col;
        if (meteoGridObj->getRowCol(geoPoint, &row, &col) &&
            myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->active)
        {
            std::string id = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->id;
            std::string name = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->name;
            QString elev = QString::number(myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->point.z);
            switch(currentGridVisualization)
            {
                case showCurrentVariable:
                {
                    value = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->currentValue;
                    break;
                }
                case showCVResidual:
                {
                    value = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->residual;
                    break;
                }
                case showElaboration:
                {
                    value = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->elaboration;
                    break;
                }
                case showAnomalyAbsolute:
                {
                    value = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->anomaly;
                    break;
                }
                case showAnomalyPercentage:
                {
                    value = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->anomalyPercentage;
                    break;
                }
                case showClimate:
                {
                    value = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->climate;
                    break;
                }
                default:
                {
                    break;
                }
            }

            status += " - Grid cell: " + QString::fromStdString(id + " " + name) + " - Row: " + QString::number(row) + " - Col: " + QString::number(col) + " - Height:" + elev;
            if (!isEqual(value, NODATA))
            {
                status += " - Value: " + QString::number(double(value), 'f', 2);
            }
        }
    }

#ifdef NETCDF
    if (myProject.netCDF.isLoaded() && currentNetcdfVisualization == showCurrentVariable)
    {
        value = netcdfObj->getValue(geoPoint);
        if (!isEqual(value, NODATA))
        {
            status += " - NetCDF value: " + QString::number(double(value), 'f', 1);
        }
    }
#endif

    this->ui->statusBar->showMessage(status);
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    ui->widgetMap->setGeometry(ui->widgetMap->x(), 0, this->width() - ui->widgetMap->x(), this->height() - 45);
    mapView->resize(ui->widgetMap->size());
}


QPoint MainWindow::getMapPos(const QPoint& pos)
{
    QPoint mapPos;
    int dx = ui->widgetMap->x();
    int dy = ui->widgetMap->y() + ui->menuBar->height();
    mapPos.setX(pos.x() - dx - MAPBORDER);
    mapPos.setY(pos.y() - dy - MAPBORDER);
    return mapPos;
}


bool MainWindow::isInsideMap(const QPoint& pos)
{
    if (pos.x() > 0 && pos.y() > 0 &&
        pos.x() < (mapView->width() - MAPBORDER*2) &&
        pos.y() < (mapView->height() - MAPBORDER*2) )
    {
        return true;
    }
    else return false;
}


bool MainWindow::updateSelection(const QPoint& pos)
{
    if (rubberBand == nullptr || !rubberBand->isActive || !rubberBand->isVisible() )
        return false;

    QPoint lastCornerOffset = getMapPos(pos);
    QPoint firstCornerOffset = rubberBand->getOrigin() - QPoint(MAPBORDER, MAPBORDER);
    QPoint pixelTopLeft;
    QPoint pixelBottomRight;
    bool isAdd = false;

    if (firstCornerOffset.y() > lastCornerOffset.y())
    {
        if (firstCornerOffset.x() > lastCornerOffset.x())
        {
            // bottom to left
            pixelTopLeft = lastCornerOffset;
            pixelBottomRight = firstCornerOffset;
            isAdd = false;
        }
        else
        {
            // bottom to right
            pixelTopLeft = QPoint(firstCornerOffset.x(), lastCornerOffset.y());
            pixelBottomRight = QPoint(lastCornerOffset.x(), firstCornerOffset.y());
            isAdd = true;
        }
    }
    else
    {
        if (firstCornerOffset.x() > lastCornerOffset.x())
        {
            // top to left
            pixelTopLeft = QPoint(lastCornerOffset.x(), firstCornerOffset.y());
            pixelBottomRight = QPoint(firstCornerOffset.x(), lastCornerOffset.y());
            isAdd = false;
        }
        else
        {
            // top to right
            pixelTopLeft = firstCornerOffset;
            pixelBottomRight = lastCornerOffset;
            isAdd = true;
        }
    }

    QPointF topLeft = this->mapView->mapToScene(pixelTopLeft);
    QPointF bottomRight = this->mapView->mapToScene(pixelBottomRight);
    QRectF rectF(topLeft, bottomRight);

    for (int i = 0; i < pointList.size(); i++)
    {
        if (rectF.contains(pointList[i]->longitude(), pointList[i]->latitude()))
        {
            if (isAdd)
            {
                myProject.meteoPoints[i].selected = true;
            }
            else
            {
                myProject.meteoPoints[i].selected = false;
            }
        }
    }

    rubberBand->isActive = false;
    rubberBand->hide();

    return true;
}


void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    updateMaps();

    if (this->updateSelection(event->pos()))
    {
        this->redrawMeteoPoints(currentPointsVisualization, false);
    }
}


// zoom in/out
void MainWindow::mouseDoubleClickEvent(QMouseEvent * event)
{
    QPoint mapPos = getMapPos(event->pos());
    if (! isInsideMap(mapPos))
        return;

    Position newCenter = this->mapView->mapToScene(mapPos);

    try
    {
        if (event->button() == Qt::LeftButton)
            this->mapView->zoomIn();
        else
            this->mapView->zoomOut();

        this->mapView->centerOn(newCenter.lonLat());
    }
    catch (...)
    {
        myProject.logWarning("Exception catch in mouseDoubleClickEvent");
    }
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QPoint mapPos = getMapPos(event->pos());
    if (! isInsideMap(mapPos)) return;

    if (event->button() == Qt::RightButton)
    {
        bool menuLoaded = false;

        if (rubberBand != nullptr)
        {
            QPoint widgetPos = mapPos + QPoint(MAPBORDER, MAPBORDER);
            rubberBand->setOrigin(widgetPos);
            rubberBand->setGeometry(QRect(widgetPos, QSize()));
            rubberBand->isActive = true;
            rubberBand->show();
            return;
        }

        Position geoPos = mapView->mapToScene(mapPos);
        gis::Crit3DGeoPoint geoPoint = gis::Crit3DGeoPoint(geoPos.latitude(), geoPos.longitude());

        int row, col;
        // GRID - context menu (solo se caricata, visibile e cella attiva)
        if (meteoGridObj->isLoaded && currentGridVisualization != notShown && meteoGridObj->getRowCol(geoPoint, &row, &col))
        {
            std::string id = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[unsigned(row)][unsigned(col)]->id;
            std::string dataset = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[unsigned(row)][unsigned(col)]->dataset;
            std::string name = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[unsigned(row)][unsigned(col)]->name;

            if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[unsigned(row)][unsigned(col)]->active)
            {
                QMenu menu;
                QAction *openMeteoWidget = menu.addAction("Open new meteo widget");
                QAction *appendMeteoWidget = menu.addAction("Append to last meteo widget");
                menu.addSeparator();
                QAction *openPointStatisticsWidget = menu.addAction("Open point statistics widget");

                QAction *openProxyGraph;
                QAction *markMacroAreaStations;
                QAction *addMacroAreaLR;
                QAction *unmarkStations;
                if (myProject.meteoPointsLoaded && (myProject.interpolationSettings.getUseLocalDetrending() || myProject.interpolationSettings.getUseGlocalDetrending()))
                {
                    menu.addSeparator();
                    openProxyGraph = menu.addAction("Open local proxy graph");
                    markMacroAreaStations = menu.addAction("Mark macroarea stations");
                    addMacroAreaLR = menu.addAction("Plot macroarea lapse rate on global proxy widget");
                    unmarkStations = menu.addAction("Unmark stations");

                }

                QAction *selection =  menu.exec(QCursor::pos());
                menuLoaded = true;

                if (myProject.meteoGridDbHandler->meteoGrid()->gridStructure().isEnsemble())
                {
                    appendMeteoWidget->setEnabled(false);
                }
                else
                {
                    appendMeteoWidget->setEnabled(true);
                }

                if (selection != nullptr)
                {
                    if (selection == openMeteoWidget)
                    {
                        myProject.showMeteoWidgetGrid(id, dataset, false);
                    }
                    if (selection == appendMeteoWidget)
                    {
                        myProject.showMeteoWidgetGrid(id, dataset, true);
                    }
                    else if (selection == openPointStatisticsWidget)
                    {
                        myProject.showPointStatisticsWidgetGrid(id);
                    }
                    if (selection == openProxyGraph)
                    {
                        callLocalProxyGraph(geoPoint);
                    }
                    if (selection == markMacroAreaStations || selection == addMacroAreaLR)
                    {
                        //check must be done for both actions
                        if (! myProject.interpolationSettings.getUseGlocalDetrending())
                            return;

                        if ( ! myProject.interpolationSettings.isGlocalReady(false))
                        {
                            if (! myProject.loadGlocalAreasMap() || ! myProject.loadGlocalStationsAndCells(false, myProject.getCompleteFileName(myProject.glocalPointsName, PATH_GEO)))
                            {
                                myProject.logError("Error in loading glocal files.");
                                return;
                            }
                        }

                        if (! myProject.meteoPointsLoaded)
                        {
                            myProject.logError(ERROR_STR_MISSING_DB);
                            return;
                        }

                        gis::Crit3DUtmPoint myUtm;
                        gis::getUtmFromLatLon(myProject.gisSettings.utmZone, geoPoint, &myUtm);

                        if (selection == markMacroAreaStations)
                        {
                            myProject.setMarkedPointsOfMacroArea(int(gis::getValueFromXY(*(myProject.interpolationSettings.getMacroAreasMap()), myUtm.x, myUtm.y)), viewNotActivePoints);
                            redrawMeteoPoints(currentPointsVisualization, true);
                        }
                        else if (selection == addMacroAreaLR)
                        {
                            myProject.showProxyGraph(int(gis::getValueFromXY(*(myProject.interpolationSettings.getMacroAreasMap()), myUtm.x, myUtm.y)));
                        }
                    }
                    if (selection == unmarkStations)
                    {
                        for (int i = 0; i < myProject.nrMeteoPoints; i++)
                        {
                            myProject.meteoPoints[i].marked = false;
                        }

                        redrawMeteoPoints(currentPointsVisualization, true);
                    }
                    // TODO: other actions

                }
            }
        }

#ifdef NETCDF
        else if (myProject.netCDF.isLoaded())
        {
            Position geoPos = mapView->mapToScene(mapPos);
            gis::Crit3DGeoPoint geoPoint = gis::Crit3DGeoPoint(geoPos.latitude(), geoPos.longitude());

            netCDF_exportDataSeries(geoPoint);
            return;
        }
#endif
        // solo se non si è già acceso menu per grid o netcdf apro questo
        if (myProject.meteoPointsLoaded && ! menuLoaded)
        {
            QMenu menu;

            QAction *openProxyGraph;
            QAction *markMacroAreaStations;
            QAction *addMacroAreaLR;
            QAction *unmarkStations;
            if (myProject.interpolationSettings.getUseLocalDetrending() || myProject.interpolationSettings.getUseGlocalDetrending())
               openProxyGraph = menu.addAction("Open local proxy graph");
            if (myProject.interpolationSettings.getUseGlocalDetrending())
            {
                markMacroAreaStations = menu.addAction("Mark all stations of this macroarea");
                addMacroAreaLR = menu.addAction("Plot macroarea lapse rate on global proxy widget");
                unmarkStations = menu.addAction("Unmark stations");
            }

            QAction *selection =  menu.exec(QCursor::pos());

            if (selection != nullptr)
            {
                if (selection == openProxyGraph)
                {
                    callLocalProxyGraph(geoPoint);
                }
                else if (selection == markMacroAreaStations || selection == addMacroAreaLR)
                {
                    if (! myProject.interpolationSettings.getUseGlocalDetrending())
                        return;

                    if ( ! myProject.interpolationSettings.isGlocalReady(false))
                    {
                        if (! myProject.loadGlocalAreasMap() || ! myProject.loadGlocalStationsAndCells(false, myProject.getCompleteFileName(myProject.glocalPointsName, PATH_GEO)))
                        {
                            return;
                        }
                    }

                    gis::Crit3DUtmPoint myUtm;
                    gis::getUtmFromLatLon(myProject.gisSettings.utmZone, geoPoint, &myUtm);

                    if (selection == markMacroAreaStations)
                    {
                        myProject.setMarkedPointsOfMacroArea(int(gis::getValueFromXY(*(myProject.interpolationSettings.getMacroAreasMap()), myUtm.x, myUtm.y)), viewNotActivePoints);
                        redrawMeteoPoints(currentPointsVisualization, true);
                    }
                    else if (selection == addMacroAreaLR)
                    {
                        if (! myProject.meteoPointsLoaded)
                        {
                            myProject.logError(ERROR_STR_MISSING_DB);
                            return;
                        }
                        myProject.showProxyGraph(int(gis::getValueFromXY(*(myProject.interpolationSettings.getMacroAreasMap()), myUtm.x, myUtm.y)));
                    }
                }
                else if (selection == unmarkStations)
                {
                    for (int i = 0; i < myProject.nrMeteoPoints; i++)
                    {
                        myProject.meteoPoints[i].marked = false;
                    }

                    redrawMeteoPoints(currentPointsVisualization, true);
                }
            }
        }
    }
}


void MainWindow::on_rasterOpacitySlider_sliderMoved(int position)
{
    this->rasterObj->setOpacity(position / 100.0);
}


void MainWindow::on_meteoGridOpacitySlider_sliderMoved(int position)
{
    this->meteoGridObj->setOpacity(position / 100.0);
}


void MainWindow::on_netcdfOpacitySlider_sliderMoved(int position)
{
    this->netcdfObj->setOpacity(position / 100.0);
}


void MainWindow::on_actionMeteoPointsClear_selection_triggered()
{
    myProject.clearSelectedPoints();
    redrawMeteoPoints(currentPointsVisualization, false);
}


void MainWindow::on_actionMeteopointRectangleSelection_triggered()
{
    if (rubberBand != nullptr)
    {
        delete rubberBand;
        rubberBand = nullptr;
    }

    if (ui->actionMeteopointRectangleSelection->isChecked())
    {
        rubberBand = new RubberBand(QRubberBand::Rectangle, this->mapView);
     }
}


void MainWindow::updateMaps()
{
    try
    {
        rasterObj->updateCenter();
        meteoGridObj->updateCenter();
        netcdfObj->updateCenter();

        rasterLegend->update();
        meteoGridLegend->update();
        netcdfLegend->update();
    }
    catch (...)
    {
        myProject.logWarning("Exception catch in updateMaps function.");
    }
}


void MainWindow::clearDEM()
{
    this->rasterObj->clear();
    emit this->rasterObj->redrawRequested();
    this->rasterLegend->setVisible(false);
    ui->labelRasterScale->setText("");
    this->ui->rasterOpacitySlider->setEnabled(false);
}


void MainWindow::zoomToDEM()
{
    if (! myProject.DEM.isLoaded)
        return;

    double xRatio = rasterObj->getSizeX() / double(ui->widgetMap->size().width());
    double yRatio = rasterObj->getSizeY() / double(ui->widgetMap->size().height());
    double ratio = std::max(xRatio, yRatio);
    double zoomLevel = round(log2(1. / ratio));
    mapView->setZoomLevel(quint8(zoomLevel));

    // center map
    Position center = rasterObj->getRasterCenter();
    mapView->centerOn(center.longitude(), center.latitude());

    updateMaps();
}


void MainWindow::renderDEM()
{
    setCurrentRaster(&(myProject.DEM));
    ui->labelRasterScale->setText(QString::fromStdString(getVariableString(noMeteoTerrain)));
    ui->rasterOpacitySlider->setEnabled(true);
    rasterLegend->setVisible(true);

    zoomToDEM();
}


QString MainWindow::selectArkimetDataset(QDialog* datasetDialog) {

        datasetDialog->exec();

        if (datasetDialog->result() == QDialog::Accepted)
        {

            QString datasetSelected = "";
            foreach (QCheckBox *checkBox, datasetCheckbox)
            {
                if (checkBox->isChecked())
                {
                    datasetSelected = datasetSelected % "'" % checkBox->text() % "',";
                }
            }

            if (!datasetSelected.isEmpty())
            {
                datasetSelected = datasetSelected.left(datasetSelected.size() - 1);
                return datasetSelected;
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setText("Select a dataset");
                msgBox.exec();
                return selectArkimetDataset(datasetDialog);
            }
        }
        else
            return "";
}


void MainWindow::enableAllDataset(bool toggled)
{
    bool AllChecked = 1;

    foreach (QCheckBox *checkBox, datasetCheckbox)
    {
        if (toggled)
        {
            checkBox->setChecked(toggled);
        }
        else
        {
            if (!checkBox->isChecked())
            {
                AllChecked = 0;
            }
        }
    }

    foreach (QCheckBox *checkBox, datasetCheckbox)
    {
        if(AllChecked)
        {
            checkBox->setChecked(toggled);
        }
    }
}


void MainWindow::disableAllDataset(bool toggled)
{
    if (! toggled)
    {
        if (allDatasets->isChecked())
        {
            allDatasets->setChecked(false);
        }
    }
}


void MainWindow::on_actionFileMeteopointArkimetLoadVM_triggered()
{
    if(myProject.nrMeteoPoints == 0)
    {
        myProject.logWarning(ERROR_STR_MISSING_DB);
        return;
    }

    QList<QString> fileList = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more VM files to open",
                            "",
                            "Files (*.VM2 *.csv *.txt)");
    if (fileList.isEmpty())
        return;

    // check prec
    bool isPrec0024 = true;
    QDialog precDialog;
    precDialog.setFixedWidth(350);
    precDialog.setWindowTitle("Choose daily precipitation time");
    QVBoxLayout precLayout;
    QRadioButton prec00("00-24");
    QRadioButton prec08("08-08");

    QDialogButtonBox confirm(QDialogButtonBox::Ok);

    precDialog.connect(&confirm, SIGNAL(accepted()), &precDialog, SLOT(accept()));

    precLayout.addWidget(&prec00);
    precLayout.addWidget(&prec08);
    precLayout.addWidget(&confirm);
    precDialog.setLayout(&precLayout);
    precDialog.exec();

    if (prec08.isChecked())
        isPrec0024 = false;

    if (! myProject.readVmArkimetData(fileList, daily, isPrec0024))
    {
        myProject.logError();
        return;
    }

    loadMeteoPoints(myProject.meteoPointsDbHandler->getDbName());
}


void MainWindow::on_actionFileMeteopointArkimetDownload_triggered()
{
    if(myProject.nrMeteoPoints == 0)
    {
        myProject.logWarning(ERROR_STR_MISSING_DB);
        return;
    }

    DialogDownloadMeteoData downloadDialog;
    if (downloadDialog.result() != QDialog::Accepted)
    {
        return;
    }
    else
    {
        QDate firstDate = downloadDialog.getFirstDate();
        QDate lastDate = downloadDialog.getLastDate();
        if (! downloadDialog.getVarD().isEmpty())
        {
            if (! myProject.downloadDailyDataArkimet(downloadDialog.getVarD(), downloadDialog.getPrec0024(), firstDate, lastDate, true))
             {
                myProject.logError();
             }
        }
        if (! downloadDialog.getVarH().isEmpty())
        {
            if (! myProject.downloadHourlyDataArkimet(downloadDialog.getVarH(), firstDate, lastDate, true))
             {
                myProject.logError();
             }
        }
    }

    this->loadMeteoPoints(myProject.meteoPointsDbHandler->getDbName());
}


void MainWindow::clearMeteoPointsMarker()
{
    for (int i = pointList.size()-1; i >= 0; i--)
    {
        mapView->scene()->removeObject(pointList[i]);
    }
    pointList.clear();

    datasetCheckbox.clear();
}


void MainWindow::clearOutputPointMarkers()
{
    for (int i = 0; i < outputPointList.size(); i++)
    {
        mapView->scene()->removeObject(outputPointList[i]);
        delete outputPointList[i];
    }

    outputPointList.clear();
}


void MainWindow::clearWindVectorObjects()
{
    for (int i = 0; i < windVectorList.size(); i++)
    {
        mapView->scene()->removeObject(windVectorList[i]);
        delete windVectorList[i];
    }

    windVectorList.clear();
}


void MainWindow::on_actionMeteopointQualitySpatial_triggered()
{
    myProject.checkSpatialQuality = ui->actionMeteopointQualitySpatial->isChecked();
    updateVariable();
    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::interpolateDemGUI()
{
    meteoVariable myVar;
    switch(currentPointsVisualization)
    {
        case showCurrentVariable:
        {
            myVar = myProject.getCurrentVariable();
            break;
        }
        case showElaboration:
        {
            myVar = elaboration;
            break;
        }
        case showAnomalyAbsolute:
        {
            myVar = anomaly;
            break;
        }
        default:
        {
            myProject.logError("No variable to interpolate.");
            return;
        }
    }

    myProject.logInfoGUI("Interpolating on DEM...");
    bool isComputed = false;

    if (myVar == airRelHumidity && myProject.interpolationSettings.getUseDewPoint())
    {
        if (! myProject.interpolationDemMain(airTemperature, myProject.getCrit3DCurrentTime(), myProject.hourlyMeteoMaps->mapHourlyTair)) return;

        if (myProject.interpolationSettings.getUseInterpolatedTForRH())
            myProject.passInterpolatedTemperatureToHumidityPoints(myProject.getCrit3DCurrentTime(), myProject.meteoSettings);

        if (myProject.interpolationDemMain(airDewTemperature, myProject.getCrit3DCurrentTime(), myProject.hourlyMeteoMaps->mapHourlyTdew))
        {
            if (! myProject.dataRaster.initializeGrid(myProject.DEM)) return;

            myProject.hourlyMeteoMaps->computeRelativeHumidityMap(&myProject.dataRaster);
            isComputed = true;
        }
    }
    else
    {
        isComputed = myProject.interpolationDemMain(myVar, myProject.getCrit3DCurrentTime(), &(myProject.dataRaster));
    }

    if (isComputed)
    {
        if (myVar == elaboration)
            setColorScale(myProject.clima->variable(), myProject.dataRaster.colorScale);
        else
            setColorScale(myVar, myProject.dataRaster.colorScale);

        setCurrentRaster(&(myProject.dataRaster));
        ui->labelRasterScale->setText(QString::fromStdString(getVariableString(myVar)));
    }
    else
    {
        myProject.logError();
    }

    myProject.closeLogInfo();
}


void MainWindow::interpolateGridGUI()
{
    if (myProject.interpolationMeteoGrid(myProject.getCurrentVariable(), myProject.getCurrentFrequency(),
                                             myProject.getCrit3DCurrentTime()))
    {
        redrawMeteoGrid(showCurrentVariable, true);
    }
    else
         myProject.logError();
}


void MainWindow::updateVariable()
{
    meteoVariable myVar = myProject.getCurrentVariable();

    // FREQUENCY
    if (myProject.getCurrentFrequency() == noFrequency)
    {
        this->ui->labelFrequency->setText("None");
    }
    else
    {
        if (myProject.getCurrentFrequency() == daily)
        {
            this->ui->labelFrequency->setText("Daily");

            meteoVariable newVar = updateMeteoVariable(myVar, daily);
            if (newVar != noMeteoVar)
            {
                myProject.setCurrentVariable(newVar);
            }
        }

        else if (myProject.getCurrentFrequency() == hourly)
        {
            this->ui->labelFrequency->setText("Hourly");

            meteoVariable newVar = updateMeteoVariable(myVar, hourly);
            if (newVar != noMeteoVar)
            {
                myProject.setCurrentVariable(newVar);
            }
        }

        else if (myProject.getCurrentFrequency() == monthly)
        {
            this->ui->labelFrequency->setText("Monthly");

            meteoVariable newVar = updateMeteoVariable(myVar, monthly);
            if (newVar != noMeteoVar)
            {
                myProject.setCurrentVariable(newVar);
            }
        }
    }

    std::string myString = getVariableString(myProject.getCurrentVariable());
    ui->labelVariable->setText(QString::fromStdString(myString));
}


void MainWindow::updateDateTime()
{
    int myHour = myProject.getCurrentHour();
    this->ui->dateEdit->setDate(myProject.getCurrentDate());
    this->ui->timeEdit->setValue(myHour);
}


void MainWindow::redrawAllData()
{
    redrawMeteoPoints(currentPointsVisualization, true);
    redrawMeteoGrid(currentGridVisualization, false);
    #ifdef NETCDF
        redrawNetcdf();
    #endif
}


void MainWindow::on_dateChanged()
{
    QDate date = this->ui->dateEdit->date();

    if (date != myProject.getCurrentDate())
    {
        myProject.loadMeteoPointsData(date, date, true, true, true);
        myProject.loadMeteoGridData(date, date, true);
        myProject.setCurrentDate(date);

        redrawAllData();
    }
}


void MainWindow::on_timeEdit_valueChanged(int myHour)
{
    if (myHour != myProject.getCurrentHour())
    {
        myProject.setCurrentHour(myHour);

        if (myProject.getCurrentFrequency() == hourly)
        {
            redrawAllData();
        }
    }
}


#ifdef NETCDF

    void MainWindow::redrawNetcdf()
    {
        if (! myProject.netCDF.isLoaded())
            return;

        gis::Crit3DRasterGrid* netcdfRaster = myProject.netCDF.getRasterPointer();

        switch(currentNetcdfVisualization)
        {
        case notShown:
        {
            netcdfRaster->setConstantValue(NODATA);
            netcdfObj->setDrawBorders(false);
            netcdfLegend->setVisible(false);
            break;
        }

        case showLocation:
        {
            netcdfRaster->setConstantValue(NODATA);
            netcdfObj->setDrawBorders(true);
            netcdfLegend->setVisible(false);
            break;
        }

        case showCurrentVariable:
        {
            netcdfObj->setDrawBorders(false);

            if (currentNetcdfVariable == NODATA)
            {
                netcdfLegend->setVisible(false);
                ui->labelNetcdfVariable->setText("");
                return;
            }

            std::string errorStr;
            if (myProject.netCDF.extractVariableMap_old(currentNetcdfVariable, myProject.getCrit3DCurrentTime(), errorStr))
            {
                gis::updateMinMaxRasterGrid(netcdfRaster);

                netcdfLegend->setVisible(true);
                netcdfLegend->update();
            }

            break;
        }
        default: { }
        }

        netcdfObj->updateCenter();
        emit netcdfObj->redrawRequested();
    }


    void MainWindow::on_actionFileNetCDF_Open_triggered()
    {
        QString netcdfPath = myProject.getDefaultPath() + PATH_NETCDF;
        QString fileName = QFileDialog::getOpenFileName(this, "Open NetCDF data", netcdfPath, "NetCDF files (*.nc)");
        if (fileName == "") return;

        closeNetCDF();

        myProject.netCDF.initialize(myProject.gisSettings.utmZone);

        if (! myProject.netCDF.readProperties(fileName.toStdString()) )
        {
            myProject.logError("Error in reading file: " + fileName);
            return;
        }

        gis::Crit3DRasterGrid* netcdfRaster = myProject.netCDF.getRasterPointer();

        if (myProject.netCDF.isLatLon || myProject.netCDF.isRotatedLatLon)
        {
            netcdfObj->initializeLatLon(netcdfRaster, myProject.gisSettings, myProject.netCDF.latLonHeader, true);
        }
        else
        {
            netcdfObj->initializeUTM(netcdfRaster, myProject.gisSettings, true);
        }

        // resize map
        double size = log2(2000. / netcdfObj->getRasterMaxSize());
        this->mapView->setZoomLevel(quint8(size));

        // center map
        gis::Crit3DGeoPoint* center = netcdfObj->getRasterCenter();
        this->mapView->centerOn(qreal(center->longitude), qreal(center->latitude));

        netcdfRaster->header->flag = myProject.netCDF.missingValue;
        netcdfRaster->setConstantValue(myProject.netCDF.missingValue);
        currentNetcdfVisualization = showLocation;

        // default colorScale: precipitation (radar)
        setColorScale(precipitation, netcdfRaster->colorScale);
        netcdfLegend->colorScale = netcdfRaster->colorScale;

        showNetcdfGroup->setEnabled(true);
        ui->groupBoxNetcdf->setVisible(true);
        netcdfObj->setVisible(true);

        // set current date and hour (last data)
        if (! myProject.meteoPointsLoaded && ! myProject.meteoGridLoaded)
        {
            QDateTime lastDateTime = getQDateTime(myProject.netCDF.getFirstTime());
            myProject.setCurrentDate(lastDateTime.date());
            myProject.setCurrentHour(lastDateTime.time().hour());
            updateDateTime();
        }

        redrawNetcdf();
    }


    void MainWindow::closeNetCDF()
    {
        if (! myProject.netCDF.isLoaded()) return;

        myProject.netCDF.close();

        currentNetcdfVariable = NODATA;
        netcdfLegend->setVisible(false);

        ui->labelNetcdfVariable->setText("");
        ui->groupBoxNetcdf->setVisible(false);

        showNetcdfGroup->setEnabled(false);

        netcdfObj->clear();
    }


    void MainWindow::on_actionNetCDF_Close_triggered()
    {
        closeNetCDF();
    }

    void MainWindow::on_actionNetCDF_ShowMetadata_triggered()
    {
        if (! myProject.netCDF.isLoaded())
        {
            myProject.logWarning(ERROR_STR_MISSING_NETCDF);
            return;
        }

        QDialog myDialog;
        myDialog.setWindowTitle("NetCDF metadata");

        QTextBrowser textBrowser;
        textBrowser.setText(QString::fromStdString(myProject.netCDF.getMetadata()));

        QVBoxLayout mainLayout;
        mainLayout.addWidget(&textBrowser);

        myDialog.setLayout(&mainLayout);
        myDialog.setFixedSize(800,600);
        myDialog.exec();
    }


    void MainWindow::on_netCDFButtonVariable_clicked()
    {
        int idVar;

        if (netCDF_ChooseVariable(&(myProject.netCDF), idVar, myProject.getCurrentFrequency()))
        {
            currentNetcdfVariable = idVar;
            currentNetcdfVisualization = showCurrentVariable;
            ui->labelNetcdfVariable->setText(QString::fromStdString(myProject.netCDF.getVariableFromId(idVar).getVarName()));
            redrawNetcdf();
        }
    }


    // extract data series
    void MainWindow::netCDF_exportDataSeries(gis::Crit3DGeoPoint geoPoint)
    {
        if (myProject.netCDF.isPointInside(geoPoint))
        {
            int idVar;
            QDateTime firstTime, lastTime;

            if (netCDF_ExportDataSeries(&(myProject.netCDF), idVar, firstTime, lastTime))
            {
                std::stringstream buffer;
                if (! myProject.netCDF.exportDataSeries(idVar, geoPoint, getCrit3DTime(firstTime), getCrit3DTime(lastTime), &buffer))
                {
                    myProject.logError(QString::fromStdString(buffer.str()));
                    return;
                }
                else
                {
                    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save data series", "", "csv files (*.csv)");
                    std::ofstream myFile;
                    myFile.open(fileName.toStdString());
                    myFile << buffer.str();
                    myFile.close();
                }
            }
        }
    }


    void MainWindow::on_actionFileMeteogridExportNetcdf_triggered()
    {
        if (! myProject.checkMeteoGridForExport()) return;

        QString fileName = QFileDialog::getSaveFileName(this, tr("Save current data of meteo grid"), "", tr("NetCDF files (*.nc)"));

        if (fileName != "")
        {
            myProject.exportMeteoGridToNetCDF(fileName, "Meteogrid", "variable", "unit", NO_DATE, 0, 0, 0);
        }
    }

    void MainWindow::on_actionShowNetcdfHide_triggered()
    {
        ui->actionShowNetcdfHide->setChecked(true);
        currentNetcdfVisualization = notShown;
        redrawNetcdf();
    }

    void MainWindow::on_actionShowNetcdfLocation_triggered()
    {
        ui->actionShowNetcdfLocation->setChecked(true);
        currentNetcdfVisualization = showLocation;
        redrawNetcdf();
    }

    void MainWindow::on_actionShowNetcdfVariable_triggered()
    {
        ui->actionShowNetcdfVariable->setChecked(true);
        currentNetcdfVisualization = showCurrentVariable;
        redrawNetcdf();
    }

    void MainWindow::on_actionNetcdf_ColorScale_SetType_triggered()
    {
        if (! myProject.netCDF.isLoaded())
        {
            myProject.logWarning(ERROR_STR_MISSING_NETCDF);
            return;
        }

        // choose color scale
        meteoVariable myVar = chooseColorScale();
        if (myVar != noMeteoVar)
        {
            setColorScale(myVar, this->netcdfObj->getRasterPointer()->colorScale);
        }

        redrawNetcdf();
    }


    void MainWindow::on_actionNetcdf_ColorScale_Reverse_triggered()
    {
        if (! myProject.netCDF.isLoaded())
        {
            myProject.logWarning(ERROR_STR_MISSING_NETCDF);
            return;
        }

        reverseColorScale(this->netcdfObj->getRasterPointer()->colorScale);
        redrawNetcdf();
    }


    void MainWindow::setColorScaleRangeNetcdf(bool isFixed)
    {
        if (! myProject.netCDF.isLoaded())
        {
            myProject.logWarning(ERROR_STR_MISSING_NETCDF);
            return;
        }

        if (isFixed)
        {
            // choose minimum
            float minimum = this->netcdfObj->getRasterPointer()->colorScale->minimum();
            QString valueStr = editValue("Choose minimum value", QString::number(minimum));
            if (valueStr == "") return;
            minimum = valueStr.toFloat();

            // choose maximum
            float maximum = this->netcdfObj->getRasterPointer()->colorScale->maximum();
            valueStr = editValue("Choose maximum value", QString::number(maximum));
            if (valueStr == "") return;
            maximum = valueStr.toFloat();

            // set range
            this->netcdfObj->getRasterPointer()->colorScale->setRange(minimum, maximum);
            this->netcdfObj->getRasterPointer()->colorScale->setFixedRange(true);
        }
        else
        {
            this->netcdfObj->getRasterPointer()->colorScale->setFixedRange(false);
        }

        redrawNetcdf();
    }


    void MainWindow::on_actionNetcdf_ColorScale_Fixed_triggered()
    {
        ui->actionNetcdf_ColorScale_Fixed->setChecked(true);
        ui->actionNetcdf_ColorScale_RangeVariable->setChecked(false);
        setColorScaleRangeNetcdf(true);
    }


    void MainWindow::on_actionNetcdf_ColorScale_RangeVariable_triggered()
    {
        ui->actionNetcdf_ColorScale_RangeVariable->setChecked(true);
        ui->actionNetcdf_ColorScale_Fixed->setChecked(false);
        setColorScaleRangeNetcdf(false);
    }
#endif


void MainWindow::drawMeteoPoints()
{
    clearMeteoPointsMarker();
    clearWindVectorObjects();

    if (! myProject.meteoPointsLoaded || myProject.nrMeteoPoints == 0) return;
    addMeteoPoints();

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData (currentDate, currentDate, true, true, true);

    showPointsGroup->setEnabled(true);
    ui->actionShowPointsCurrent->setEnabled(false);
    ui->actionShowPointsCVResidual->setEnabled(false);
    ui->actionShowPointsElab->setEnabled(false);
    ui->actionShowPointsClimate->setEnabled(false);

    ui->actionMeteopointRectangleSelection->setEnabled(true);
    ui->actionMeteoPointsClear_selection->setEnabled(true);
    ui->menuSearch_points->setEnabled(true);
    ui->menuMark_points->setEnabled(true);
    ui->menuActive_points->setEnabled(true);
    ui->menuSelected_points->setEnabled(true);
    ui->menuDeactive_points->setEnabled(true);
    ui->menuDelete_points->setEnabled(true);
    ui->menuDelete_data->setEnabled(true);
    ui->menuShift_data->setEnabled(true);
    ui->actionMeteopointDataCount->setEnabled(true);
    ui->menuCompute_daily_data_from_hourly->setEnabled(true);

    if (currentPointsVisualization == notShown) currentPointsVisualization = showLocation;
    redrawMeteoPoints(currentPointsVisualization, true);

    updateDateTime();
}


void MainWindow::drawWindVector(int i)
{
    float dx = myProject.meteoPoints[i].getMeteoPointValue(myProject.getCrit3DCurrentTime(),
                                                           windVectorX,  myProject.meteoSettings);
    float dy = myProject.meteoPoints[i].getMeteoPointValue(myProject.getCrit3DCurrentTime(),
                                                           windVectorY,  myProject.meteoSettings);
    if (isEqual(dx, NODATA) || isEqual(dy, NODATA))
        return;

    ArrowObject* arrow = new ArrowObject(qreal(dx * 10), qreal(dy * 10), QColor(Qt::black));
    arrow->setLatitude(myProject.meteoPoints[i].latitude);
    arrow->setLongitude(myProject.meteoPoints[i].longitude);
    windVectorList.append(arrow);

    mapView->scene()->addObject(windVectorList.last());
}


void MainWindow::redrawMeteoPoints(visualizationType showType, bool updateColorScale)
{
    currentPointsVisualization = showType;
    ui->groupBoxElaboration->hide();
    ui->groupBoxCV->hide();

    if (pointList.size() == 0) return;

    // initialize (hide all meteo points)
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        pointList[i]->setVisible(false);
        pointList[i]->setMarked(myProject.meteoPoints[i].marked);
    }
    clearWindVectorObjects();

    meteoPointsLegend->setVisible(true);

    switch(currentPointsVisualization)
    {
        case notShown:
        {
            meteoPointsLegend->setVisible(false);
            this->ui->actionShowPointsHide->setChecked(true);
            ui->actionMeteopointRectangleSelection->setEnabled(false);
            break;
        }

        case showLocation:
        {
            this->ui->actionShowPointsLocation->setChecked(true);
            ui->actionMeteopointRectangleSelection->setEnabled(true);
            for (int i = 0; i < myProject.nrMeteoPoints; i++)
            {
                myProject.meteoPoints[i].currentValue = NODATA;

                if (myProject.meteoPoints[i].selected)
                {
                    pointList[i]->setFillColor(QColor(Qt::yellow));
                    pointList[i]->setRadius(5);
                }
                else
                {
                    if (myProject.meteoPoints[i].active)
                    {
                        if (myProject.meteoPoints[i].lapseRateCode == primary)
                        {
                            pointList[i]->setFillColor(QColor(Qt::white));
                            pointList[i]->setRadius(5);
                        }
                        else if (myProject.meteoPoints[i].lapseRateCode == secondary)
                        {
                            pointList[i]->setFillColor(QColor(Qt::black));
                            pointList[i]->setRadius(5);
                        }
                        else if (myProject.meteoPoints[i].lapseRateCode == supplemental)
                        {
                            pointList[i]->setFillColor(QColor(Qt::gray));
                            pointList[i]->setRadius(4);
                        }
                    }
                    else
                    {
                        pointList[i]->setFillColor(QColor(Qt::red));
                        pointList[i]->setRadius(5);
                    }
                }

                pointList[i]->setCurrentValue(NODATA);
                pointList[i]->setToolTip();

                // hide not active points
                bool isVisible = ((myProject.meteoPoints[i].active || viewNotActivePoints) && !(hideSupplementals && myProject.meteoPoints[i].lapseRateCode == supplemental));
                pointList[i]->setVisible(isVisible);
            }

            myProject.meteoPointsColorScale->setRange(NODATA, NODATA);
            meteoPointsLegend->update();
            break;
        }

        case showCurrentVariable:
        {
            this->ui->actionShowPointsCurrent->setChecked(true);
            ui->actionMeteopointRectangleSelection->setEnabled(true);

            // quality control
            std::string errorStdStr;
            checkData(myProject.quality, myProject.getCurrentVariable(), myProject.meteoPoints,
                      myProject.nrMeteoPoints, myProject.getCrit3DCurrentTime(), &myProject.qualityInterpolationSettings, myProject.meteoSettings,
                      &(myProject.climateParameters), myProject.checkSpatialQuality, errorStdStr);

            if (updateColorScale)
            {
                float minimum, maximum;
                myProject.getMeteoPointsRange(minimum, maximum, viewNotActivePoints);

                myProject.meteoPointsColorScale->setRange(minimum, maximum);
            }

            roundColorScale(myProject.meteoPointsColorScale, 4, true);
            setColorScale(myProject.getCurrentVariable(), myProject.meteoPointsColorScale);
            bool isWindVector = (myProject.getCurrentVariable() == windVectorIntensity
                                 || myProject.getCurrentVariable() == windVectorDirection);

            Crit3DColor *myColor;
            for (int i = 0; i < myProject.nrMeteoPoints; i++)
            {
                if (int(myProject.meteoPoints[i].currentValue) != NODATA || myProject.meteoPoints[i].marked)
                {
                    if (myProject.meteoPoints[i].quality == quality::accepted)
                    {
                        pointList[i]->setRadius(5);
                        myColor = myProject.meteoPointsColorScale->getColor(myProject.meteoPoints[i].currentValue);
                        pointList[i]->setFillColor(QColor(myColor->red, myColor->green, myColor->blue));
                        pointList[i]->setOpacity(1.0);
                        if (isWindVector)
                            drawWindVector(i);
                    }
                    else if (! myProject.meteoPoints[i].marked)
                    {
                        // Wrong data
                        pointList[i]->setRadius(10);
                        pointList[i]->setFillColor(QColor(Qt::black));
                        pointList[i]->setOpacity(0.5);
                    }

                    pointList[i]->setCurrentValue(myProject.meteoPoints[i].currentValue);
                    pointList[i]->setQuality(myProject.meteoPoints[i].quality);
                    pointList[i]->setToolTip();

                    // hide not active points
                    bool isVisible = ((myProject.meteoPoints[i].active || viewNotActivePoints || myProject.meteoPoints[i].marked) && !(hideSupplementals && myProject.meteoPoints[i].lapseRateCode == supplemental));
                    pointList[i]->setVisible(isVisible);
                }
            }

            meteoPointsLegend->update();
            break;
        }

        case showCVResidual:
        {
            this->ui->actionShowPointsCVResidual->setChecked(true);
            ui->actionMeteopointRectangleSelection->setEnabled(true);
            showCVResult();
            break;
        }

        case showElaboration:
        {
            this->ui->actionShowPointsElab->setChecked(true);
            ui->actionMeteopointRectangleSelection->setEnabled(true);
            showElabResult(true, false, false, false, false, nullptr);
            break;
        }

        case showAnomalyAbsolute:
        {
            this->ui->actionShowPointsAnomalyAbs->setChecked(true);
            ui->actionMeteopointRectangleSelection->setEnabled(true);
            showElabResult(true, false, true, false, false, nullptr);
            break;
        }

        case showAnomalyPercentage:
        {
            this->ui->actionShowPointsAnomalyPerc->setChecked(true);
            ui->actionMeteopointRectangleSelection->setEnabled(true);
            showElabResult(true, false, true, true, false, nullptr);
            break;
        }

        case showClimate:
        {
            this->ui->actionShowPointsClimate->setChecked(true);
            ui->actionMeteopointRectangleSelection->setEnabled(true);
            showElabResult(true, false, false, false, true, myProject.climateIndex);
            break;
        }
    }
}


bool MainWindow::loadMeteoPoints(QString dbFileName)
{
    myProject.logInfoGUI("Load meteo points DB: " + dbFileName);
    bool isOk = myProject.loadMeteoPointsDB(dbFileName);
    myProject.closeLogInfo();

    drawMeteoPoints();

    if (! isOk)
        myProject.logError();

    return isOk;
}


void MainWindow::drawMeteoGrid()
{
    if (! myProject.meteoGridLoaded || myProject.meteoGridDbHandler == nullptr) return;

    myProject.meteoGridDbHandler->meteoGrid()->createRasterGrid();

    if (myProject.meteoGridDbHandler->gridStructure().isUTM() == false)
    {
        meteoGridObj->initializeLatLon(&(myProject.meteoGridDbHandler->meteoGrid()->dataMeteoGrid), myProject.gisSettings, myProject.meteoGridDbHandler->gridStructure().header(), true);
    }
    else
    {
        meteoGridObj->initializeUTM(&(myProject.meteoGridDbHandler->meteoGrid()->dataMeteoGrid), myProject.gisSettings, true);
    }

    meteoGridLegend->colorScale = myProject.meteoGridDbHandler->meteoGrid()->dataMeteoGrid.colorScale;
    ui->meteoGridOpacitySlider->setEnabled(true);

    if (myProject.loadGridDataAtStart)
    {
        myProject.loadMeteoGridData(myProject.getCurrentDate(), myProject.getCurrentDate(), true);
    }

    showGridGroup->setEnabled(true);
    this->ui->menuActive_cells->setEnabled(true);
    this->ui->actionCompute_monthly_data_from_daily->setEnabled(true);
    if (myProject.getCurrentVariable() != noMeteoVar && myProject.getCurrentFrequency() != noFrequency)
    {
        this->ui->actionShowGridCurrent->setEnabled(true);
    }
    else
    {
        this->ui->actionShowGridCurrent->setEnabled(false);
    }
    this->ui->actionShowGridElab->setEnabled(false);
    this->ui->actionShowGridClimate->setEnabled(false);

    // resize map
    double size = this->meteoGridObj->getRasterMaxSize();
    size = log2(1000 / size);
    this->mapView->setZoomLevel(quint8(size));

    // center map
    gis::Crit3DGeoPoint* center = this->meteoGridObj->getRasterCenter();
    this->mapView->centerOn(qreal(center->longitude), qreal(center->latitude));

    if (currentGridVisualization == notShown) currentGridVisualization = showLocation;
    redrawMeteoGrid(currentGridVisualization, false);

    updateDateTime();
    updateMaps();
}


void MainWindow::redrawMeteoGrid(visualizationType showType, bool showInterpolationResult)
{
    currentGridVisualization = showType;
    ui->groupBoxElaboration->hide();
    ui->groupBoxCV->hide();

    if (! myProject.meteoGridLoaded || myProject.meteoGridDbHandler == nullptr) return;

    switch(currentGridVisualization)
    {
        case notShown:
        {
            this->ui->actionShowGridHide->setChecked(true);
            myProject.meteoGridDbHandler->meteoGrid()->fillMeteoRasterNoData();
            meteoGridObj->setDrawBorders(false);
            meteoGridLegend->setVisible(false);
            break;
        }

        case showLocation:
        {
            this->ui->actionShowGridLocation->setChecked(true);
            myProject.meteoGridDbHandler->meteoGrid()->fillMeteoRasterNoData();
            meteoGridObj->setDrawBorders(true);
            break;
        }

        case showCurrentVariable:
        {
            this->ui->actionShowGridCurrent->setChecked(true);

            meteoGridObj->setDrawBorders(false);
            meteoVariable variable = myProject.getCurrentVariable();

            if (! showInterpolationResult)
            {
                frequencyType frequency = myProject.getCurrentFrequency();

                if (myProject.getCurrentVariable() == noMeteoVar)
                {
                    meteoGridLegend->setVisible(false);
                    ui->labelMeteoGridScale->setText("");
                    return;
                }

                Crit3DDate myDate = getCrit3DDate(myProject.getCurrentDate());

                if (frequency == hourly)
                {
                    myProject.meteoGridDbHandler->meteoGrid()->fillCurrentHourlyValue(myDate, myProject.getCurrentHour(), 0, variable);
                }
                else if (frequency == daily)
                {
                    myProject.meteoGridDbHandler->meteoGrid()->fillCurrentDailyValue(myDate, variable, myProject.meteoSettings);
                }
                else if (frequency == monthly)
                {
                    myProject.meteoGridDbHandler->meteoGrid()->fillCurrentMonthlyValue(myDate, variable);
                }
                else
                    return;

                myProject.meteoGridDbHandler->meteoGrid()->fillMeteoRaster();
            }

            meteoGridLegend->setVisible(true);

            setColorScale(variable, myProject.meteoGridDbHandler->meteoGrid()->dataMeteoGrid.colorScale);
            ui->labelMeteoGridScale->setText(QString::fromStdString(getVariableString(myProject.getCurrentVariable())));

            meteoGridLegend->update();
            break;
        }

        case showElaboration:
        {
            this->ui->actionShowGridElab->setChecked(true);
            bool updateColorSCale = true;
            bool isMeteoGrid = true;
            bool isAnomaly = false;
            bool isClimate = false;
            bool isAnomalyPerc = false;
            showElabResult(updateColorSCale, isMeteoGrid, isAnomaly, isAnomalyPerc, isClimate, nullptr);
            break;
        }
        case showAnomalyAbsolute:
        {
            this->ui->actionShowGridAnomalyAbs->setChecked(true);
            showElabResult(true, true, true, false, false, nullptr);
            break;
        }
        case showAnomalyPercentage:
        {
            this->ui->actionShowGridAnomalyPerc->setChecked(true);
            showElabResult(true, true, true, true, false, nullptr);
            break;
        }
        case showClimate:
        {
            this->ui->actionShowGridClimate->setChecked(true);
            showElabResult(true, true, false, false, true, myProject.climateIndex);
            break;
        }
    }

    emit meteoGridObj->redrawRequested();
}


bool MainWindow::loadMeteoGrid(QString xmlName)
{
    if (myProject.loadMeteoGridDB(xmlName))
    {
        drawMeteoGrid();
        update();
        return true;
    }
    else
    {
        myProject.logError();
        return false;
    }
}


bool MainWindow::newMeteoGrid(QString xmlName)
{
    if (myProject.newMeteoGridDB(xmlName))
    {
        drawMeteoGrid();
        this->update();
        return true;
    }
    else
    {
        myProject.logError();
        return false;
    }
}


void MainWindow::addMeteoPoints()
{
    myProject.clearSelectedPoints();

    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        StationMarker* point = new StationMarker(5.0, true, QColor(Qt::white));

        point->setFlag(MapGraphicsObject::ObjectIsMovable, false);
        point->setLatitude(myProject.meteoPoints[i].latitude);
        point->setLongitude(myProject.meteoPoints[i].longitude);
        point->setId(myProject.meteoPoints[i].id);
        point->setName(myProject.meteoPoints[i].name);
        point->setDataset(myProject.meteoPoints[i].dataset);
        point->setAltitude(myProject.meteoPoints[i].point.z);
        point->setLapseRateCode(myProject.meteoPoints[i].lapseRateCode);
        point->setRegion(myProject.meteoPoints[i].region);
        point->setProvince(myProject.meteoPoints[i].province);
        point->setMunicipality(myProject.meteoPoints[i].municipality);
        point->setCurrentValue(qreal(myProject.meteoPoints[i].currentValue));
        point->setQuality(myProject.meteoPoints[i].quality);

        if (!myProject.meteoPoints[i].active)
        {
            point->setActive(false);
            point->setFillColor(QColor(Qt::red));
            point->setRadius(5);
            if (viewNotActivePoints)
                point->setVisible(true);
            else
                point->setVisible(false);
        }
        else
        {
            // primary is already white
            if (myProject.meteoPoints[i].lapseRateCode == secondary)
            {
                point->setFillColor(QColor(Qt::black));
                point->setRadius(5);
            }
            else if (myProject.meteoPoints[i].lapseRateCode == supplemental)
            {
                point->setFillColor(QColor(Qt::gray));
                point->setRadius(4);
            }
        }

        this->pointList.append(point);
        this->mapView->scene()->addObject(this->pointList[i]);

        point->setToolTip();
        connect(point, SIGNAL(newStationClicked(std::string, std::string, std::string, double, std::string, bool)), this, SLOT(callNewMeteoWidget(std::string, std::string, std::string, double, std::string, bool)));
        connect(point, SIGNAL(appendStationClicked(std::string, std::string, std::string, double, std::string, bool)), this, SLOT(callAppendMeteoWidget(std::string, std::string, std::string, double, std::string, bool)));
        connect(point, SIGNAL(newPointStatisticsClicked(std::string, bool)), this, SLOT(callNewPointStatisticsWidget(std::string, bool)));
        connect(point, SIGNAL(newHomogeneityTestClicked(std::string)), this, SLOT(callNewHomogeneityTestWidget(std::string)));
        connect(point, SIGNAL(newSynchronicityTestClicked(std::string)), this, SLOT(callNewSynchronicityTestWidget(std::string)));
        connect(point, SIGNAL(setSynchronicityReferenceClicked(std::string)), this, SLOT(callSetSynchronicityReference(std::string)));
        connect(point, SIGNAL(changeOrogCodeClicked(std::string, int)), this, SLOT(callChangeOrogCode(std::string, int)));
        connect(point, SIGNAL(markPoint(std::string)), this, SLOT(callMarkPoint(std::string)));
        connect(point, SIGNAL(unmarkPoint(std::string)), this, SLOT(callUnmarkPoint(std::string)));
    }
}

void MainWindow::callNewMeteoWidget(std::string id, std::string name, std::string dataset, double altitude, std::string lapseRate, bool isGrid)
{
    bool isAppend = false;
    if (isGrid)
    {
        myProject.showMeteoWidgetGrid(id, dataset, isAppend);
    }
    else
    {
        myProject.showMeteoWidgetPoint(id, name, dataset, altitude, lapseRate, isAppend);
    }
    return;
}

void MainWindow::callAppendMeteoWidget(std::string id, std::string name, std::string dataset, double altitude, std::string lapseRate, bool isGrid)
{
    bool isAppend = true;
    if (isGrid)
    {
        myProject.showMeteoWidgetGrid(id, dataset, isAppend);
    }
    else
    {
        myProject.showMeteoWidgetPoint(id, name, dataset, altitude, lapseRate, isAppend);
    }
    return;
}

void MainWindow::callLocalProxyGraph(const gis::Crit3DGeoPoint point)
{
    if(myProject.getCurrentFrequency() == noFrequency)
    {
        myProject.logError("Select data frequency (daily or hourly)");
        return;
    }

    myProject.showLocalProxyGraph(point);
    return;
}

void MainWindow::callNewPointStatisticsWidget(std::string id, bool isGrid)
{
    if (isGrid)
    {
        myProject.showPointStatisticsWidgetGrid(id);
    }
    else
    {
        myProject.showPointStatisticsWidgetPoint(id);
    }
    return;
}

void MainWindow::callNewHomogeneityTestWidget(std::string id)
{
    myProject.showHomogeneityTestWidgetPoint(id);
    return;
}

void MainWindow::callNewSynchronicityTestWidget(std::string id)
{
    myProject.showSynchronicityTestWidgetPoint(id);
    return;
}

void MainWindow::callSetSynchronicityReference(std::string id)
{
    myProject.setSynchronicityReferencePoint(id);
    return;
}

void MainWindow::callChangeOrogCode(std::string id, int orogCode)
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }
    if (! myProject.meteoPointsDbHandler->setOrogCode(QString::fromStdString(id), orogCode))
    {
        myProject.logError(myProject.meteoPointsDbHandler->getErrorString());
        return;
    }

    QString dbName = myProject.meteoPointsDbHandler->getDbName();
    myProject.logInfoGUI("Update...");

    myProject.loadMeteoPointsDB(dbName);
    myProject.loadMeteoPointsData(myProject.getCurrentDate(), myProject.getCurrentDate(), true, true, true);
    redrawMeteoPoints(currentPointsVisualization, true);
    myProject.closeLogInfo();
}


void MainWindow::callMarkPoint(std::string myId)
{
    bool isFound = false;
    int i;
    for (i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (myProject.meteoPoints[i].id == myId)
        {
            isFound = true;
            break;
        }
    }

    if (isFound)
    {
        myProject.meteoPoints[i].marked = true;
        redrawMeteoPoints(currentPointsVisualization, true);
    }
}


void MainWindow::callUnmarkPoint(std::string myId)
{
    bool isFound = false;
    int i;
    for (i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (myProject.meteoPoints[i].id == myId)
        {
            isFound = true;
            break;
        }
    }

    if (isFound)
    {
        myProject.meteoPoints[i].marked = false;
        redrawMeteoPoints(currentPointsVisualization, true);
    }
}


void MainWindow::on_variableButton_clicked()
{
    meteoVariable myVar = chooseMeteoVariable(myProject);
    if (myVar == noMeteoVar) return;

    myProject.setCurrentVariable(myVar);
    this->updateVariable();

    if (myProject.getCurrentFrequency() != noFrequency)
    {
        this->ui->actionShowPointsCurrent->setEnabled(true);
        this->ui->actionShowGridCurrent->setEnabled(true);
        redrawMeteoPoints(showCurrentVariable, true);
        redrawMeteoGrid(showCurrentVariable, false);
    }
}


void MainWindow::on_frequencyButton_clicked()
{
   frequencyType myFrequency = chooseFrequency(myProject);

   if (myFrequency != noFrequency)
   {
       myProject.setCurrentFrequency(myFrequency);
       updateVariable();

       if (myProject.getCurrentVariable() != noMeteoVar)
       {
           ui->actionShowPointsCurrent->setEnabled(true);
           ui->actionShowGridCurrent->setEnabled(true);
           redrawAllData();
       }
   }
}


void MainWindow::setCurrentRaster(gis::Crit3DRasterGrid *myRaster)
{
    this->rasterObj->initialize(myRaster, myProject.gisSettings);
    this->rasterLegend->colorScale = myRaster->colorScale;
    emit this->rasterObj->redrawRequested();
}


void MainWindow::on_dateEdit_dateChanged(const QDate &date)
{
    Q_UNUSED(date)
    this->on_dateChanged();
}


void MainWindow::on_actionElaboration_Daily_data_triggered()
{
    if (! myProject.meteoPointsLoaded && ! myProject.meteoGridLoaded)
    {
       myProject.logWarning(ERROR_STR_MISSING_POINT_GRID);
       return;
    }

    if (myProject.clima == nullptr)
    {
        myProject.clima = new Crit3DClimate();
    }

    bool isAnomaly = false;
    bool isClimate = false;
    DialogMeteoComputation compDialog(myProject.pragaDefaultSettings, myProject.meteoGridLoaded,
                                      myProject.meteoPointsLoaded, isAnomaly, isClimate);
    if (compDialog.result() != QDialog::Accepted)
    {
        return;
    }

    bool isMeteoGrid = compDialog.getIsMeteoGrid();
    myProject.lastElabTargetIsGrid = isMeteoGrid;
    bool showInfo = true;
    if (! myProject.computeElaboration(isMeteoGrid, isAnomaly, isClimate, showInfo))
    {
        myProject.logError();
        return;
    }

    if (isMeteoGrid)
    {
        this->ui->actionShowGridElab->setEnabled(true);
        redrawMeteoGrid(showElaboration, false);
    }
    else
    {
        this->ui->actionShowPointsElab->setEnabled(true);
        redrawMeteoPoints(showElaboration, true);
    }
}


void MainWindow::on_actionElaboration_Hourly_data_triggered()
{
    if (! myProject.meteoPointsLoaded && ! myProject.meteoGridLoaded)
    {
        myProject.logWarning(ERROR_STR_MISSING_POINT_GRID);
        return;
    }

    if (myProject.clima == nullptr)
    {
        myProject.clima = new Crit3DClimate();
    }

    DialogMeteoHourlyComputation compDialog(myProject.pragaDefaultSettings, myProject.meteoGridLoaded, myProject.meteoPointsLoaded);
    if (compDialog.result() != QDialog::Accepted)
    {
        return;
    }

    bool isMeteoGrid = compDialog.getIsMeteoGrid();
    myProject.lastElabTargetIsGrid = isMeteoGrid;
    bool showInfo = true;
    if (! myProject.computeElaborationHourly(isMeteoGrid, showInfo))
    {
        myProject.logError();
        return;
    }

    if (isMeteoGrid)
    {
        this->ui->actionShowGridElab->setEnabled(true);
        redrawMeteoGrid(showElaboration, false);
    }
    else
    {
        this->ui->actionShowPointsElab->setEnabled(true);
        redrawMeteoPoints(showElaboration, true);
    }
}


void MainWindow::on_actionAnomaly_triggered()
{
    if (!myProject.meteoPointsLoaded && !myProject.meteoGridLoaded)
    {
        myProject.logWarning(ERROR_STR_MISSING_POINT_GRID);
        return;
    }

    if (myProject.clima == nullptr)
    {
        myProject.clima = new Crit3DClimate();
    }
    if (myProject.referenceClima == nullptr)
    {
        myProject.referenceClima = new Crit3DClimate();
    }

    bool isAnomaly = true;
    bool isClimate = false;
    DialogMeteoComputation compDialog(myProject.pragaDefaultSettings, myProject.meteoGridLoaded,
                                      myProject.meteoPointsLoaded, isAnomaly, isClimate);
    if (compDialog.result() != QDialog::Accepted)
    {
        return;
    }

    bool isMeteoGrid = compDialog.getIsMeteoGrid();
    myProject.lastElabTargetIsGrid = isMeteoGrid;
    isAnomaly = false;
    bool isOk = myProject.computeElaboration(isMeteoGrid, isAnomaly, isClimate, true);
    if (! isOk)
    {
        myProject.logError();
    }
    else
    {
        isAnomaly = true;
        isOk = myProject.computeElaboration(isMeteoGrid, isAnomaly, isClimate, true);
        if (! isOk)
        {
            myProject.logError();
        }

        if (isMeteoGrid)
        {
            this->ui->menuShowGridAnomaly->setEnabled(true);
            redrawMeteoGrid(showAnomalyAbsolute, false);
        }
        else
        {
            this->ui->menuShowPointsAnomaly->setEnabled(true);
            redrawMeteoPoints(showAnomalyAbsolute, true);
        }
    }

    if (compDialog.result() == QDialog::Accepted)
        on_actionAnomaly_triggered();

    return;
}


void MainWindow::on_actionClimate_triggered()
{
    if (!myProject.meteoPointsLoaded && !myProject.meteoGridLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_POINT_GRID);
        return;
    }

    bool isMeteoPointLoaded = false;
    bool isMeteoGridLoaded = false;

    if (myProject.meteoPointsLoaded)
    {
        isMeteoPointLoaded = true;
    }
    if (myProject.meteoGridLoaded)
    {
        isMeteoGridLoaded = true;
    }

    if (myProject.clima == nullptr)
    {
        myProject.clima = new Crit3DClimate();
    }
    myProject.clima->resetListElab();

    bool isAnomaly = false;
    bool isClimate = true;
    DialogMeteoComputation compDialog(myProject.pragaDefaultSettings, isMeteoGridLoaded, isMeteoPointLoaded, isAnomaly, isClimate);
    if (compDialog.result() != QDialog::Accepted)
    {
        return;
    }

    bool isMeteoGrid = compDialog.getIsMeteoGrid();
    myProject.lastElabTargetIsGrid = isMeteoGrid;
    myProject.clima->getListElab()->setListClimateElab(compDialog.getElabSaveList());
    if (! myProject.computeElaboration(isMeteoGrid, isAnomaly, isClimate, true))
    {
        myProject.logError();
    }

    if (compDialog.result() == QDialog::Accepted)
        on_actionClimate_triggered();

    return;
}


void MainWindow::showElabResult(bool updateColorSCale, bool isMeteoGrid, bool isAnomaly, bool isAnomalyPerc, bool isClima, QString index)
{
    float value;

    if (isMeteoGrid)
    {
        meteoGridObj->setDrawBorders(false);
        if (! isAnomaly)
        {
            if (! isClima)
            {
                myProject.meteoGridDbHandler->meteoGrid()->fillMeteoRasterElabValue();
            }
            else
            {
                myProject.meteoGridDbHandler->meteoGrid()->fillMeteoRasterClimateValue();
            }
        }
        else
        {
            if (isAnomalyPerc)
            {
                myProject.meteoGridDbHandler->meteoGrid()->fillMeteoRasterAnomalyPercValue();
            }
            else
            {
                myProject.meteoGridDbHandler->meteoGrid()->fillMeteoRasterAnomalyValue();
            }

        }
        setColorScale(myProject.clima->variable(), myProject.meteoGridDbHandler->meteoGrid()->dataMeteoGrid.colorScale);
        ui->labelMeteoGridScale->setText(QString::fromStdString(getVariableString(myProject.clima->variable())));
        meteoGridLegend->setVisible(true);
        meteoGridLegend->update();
    }
    else
    {
        meteoPointsLegend->setVisible(true);

        if (updateColorSCale)
        {
            float minimum = NODATA;
            float maximum = NODATA;
            for (int i = 0; i < myProject.nrMeteoPoints; i++)
            {
                if (! isAnomaly)
                {
                    if (! isClima)
                    {
                        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].elaboration;
                    }
                    else
                    {
                        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].climate;
                    }
                }
                else
                {
                    if (isAnomalyPerc)
                    {
                        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].anomalyPercentage;
                    }
                    else
                    {
                        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].anomaly;
                    }
                }

                // hide all meteo points
                pointList[i]->setVisible(false);

                value = myProject.meteoPoints[i].currentValue;
                if (! isEqual(value, NODATA))
                {
                    if (isEqual(minimum, NODATA))
                    {
                        minimum = value;
                        maximum = value;
                    }
                    else
                    {
                        minimum = std::min(value, minimum);
                        maximum = std::max(value, maximum);
                    }
                }
            }

            myProject.meteoPointsColorScale->setRange(minimum, maximum);
            roundColorScale(myProject.meteoPointsColorScale, 4, true);
            setColorScale(myProject.clima->variable(), myProject.meteoPointsColorScale);
        }

        Crit3DColor *myColor;
        for (int i = 0; i < myProject.nrMeteoPoints; i++)
        {
            if (! updateColorSCale)
            {
                if (! isAnomaly)
                {
                    if (! isClima)
                    {
                        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].elaboration;
                    }
                    else
                    {
                        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].climate;
                    }
                }
                else
                {
                    if (isAnomalyPerc)
                    {
                        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].anomalyPercentage;
                    }
                    else
                    {
                        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].anomaly;
                    }
                }
                // hide all meteo points
                pointList[i]->setVisible(false);
            }

            if (int(myProject.meteoPoints[i].currentValue) != NODATA)
            {
                pointList[i]->setRadius(5);
                myColor = myProject.meteoPointsColorScale->getColor(myProject.meteoPoints[i].currentValue);
                pointList[i]->setFillColor(QColor(myColor->red, myColor->green, myColor->blue));
                pointList[i]->setCurrentValue(myProject.meteoPoints[i].currentValue);
                pointList[i]->setQuality(myProject.meteoPoints[i].quality);
                pointList[i]->setToolTip();

                if (!(hideSupplementals && myProject.meteoPoints[i].lapseRateCode == supplemental))
                    pointList[i]->setVisible(true);
            }
        }

        meteoPointsLegend->update();
    }

    if (int(myProject.clima->param1()) != NODATA)
    {
        if (! isAnomaly)
        {

            ui->lineEditElab1->setText(myProject.clima->elab1() + " " + QString::number(myProject.clima->param1()));
        }
        else
        {
            if (isAnomalyPerc)
            {
                ui->lineEditElab1->setText(myProject.clima->elab1() + "%Anomaly of: " + QString::number(myProject.clima->param1()));
            }
            else
            {
                ui->lineEditElab1->setText(myProject.clima->elab1() + "Anomaly of: " + QString::number(myProject.clima->param1()));
            }
        }
    }
    else
    {
        if (! isAnomaly)
        {
            ui->lineEditElab1->setText(myProject.clima->elab1());
        }
        else
        {
            if (isAnomalyPerc)
            {
                ui->lineEditElab1->setText("%Anomaly respect to " + myProject.clima->elab1());
            }
            else
            {
                ui->lineEditElab1->setText("Anomaly respect to " + myProject.clima->elab1());
            }
        }
    }
    if (myProject.clima->elab2().isEmpty())
    {
        ui->lineEditElab2->setVisible(false);
    }
    else
    {
        ui->lineEditElab2->setVisible(true);
        if (int(myProject.clima->param2()) != NODATA)
        {
            ui->lineEditElab2->setText(myProject.clima->elab2() + " " + QString::number(double(myProject.clima->param2())));
        }
        else
        {
            ui->lineEditElab2->setText(myProject.clima->elab2());
        }
    }

    if (myProject.clima->offset() == NODATA)
    {
        ui->label_offset->setVisible(false);
        ui->lineEditOffset->setVisible(false);
    }
    else
    {
        ui->label_offset->setVisible(true);
        ui->lineEditOffset->setVisible(true);
        ui->lineEditOffset->setText(QString::number(myProject.clima->offset()));
    }

    // check variable (daily or hourly)
    meteoVariable var = myProject.clima->variable();
    std::string varStr = "";
    if (MapDailyMeteoVarToString.find(var) != MapDailyMeteoVarToString.end())
    {
        varStr = MapDailyMeteoVarToString.at(var);
    }
    else if (MapHourlyMeteoVarToString.find(var) != MapHourlyMeteoVarToString.end())
    {
        varStr = MapHourlyMeteoVarToString.at(var);
    }
    ui->lineEditVariable->setText(QString::fromStdString(varStr));

    QString startDay = QString::number(myProject.clima->genericPeriodDateStart().day());
    QString startMonth = QString::number(myProject.clima->genericPeriodDateStart().month());
    QString endDay = QString::number(myProject.clima->genericPeriodDateEnd().day());
    QString endMonth = QString::number(myProject.clima->genericPeriodDateEnd().month());

    QString startYear = QString::number(myProject.clima->yearStart());
    QString endYear = QString::number(myProject.clima->yearEnd());

    if (! isClima)
    {
        ui->lineEditPeriod->setText(startDay + "/" + startMonth + "-" + endDay + "/" + endMonth + " " + startYear + "÷" + endYear);
    }
    else
    {
        if (myProject.clima->periodType() != genericPeriod && myProject.clima->periodType() != annualPeriod)
        {
            ui->lineEditPeriod->setText(startYear + "÷" + endYear + "-" + myProject.clima->periodStr() + " index: " + index);
        }
        else
        {
            ui->lineEditPeriod->setText(startYear + "÷" + endYear + "-" + myProject.clima->periodStr());
        }
    }

    ui->lineEditElab1->setReadOnly(true);
    ui->lineEditElab2->setReadOnly(true);
    ui->lineEditVariable->setReadOnly(true);
    ui->lineEditPeriod->setReadOnly(true);
    ui->lineEditOffset->setReadOnly(true);
    ui->groupBoxElaboration->show();
}

void MainWindow::showCVResult()
{
    float value;

    meteoPointsLegend->setVisible(true);

    float minimum = NODATA;
    float maximum = NODATA;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        myProject.meteoPoints[i].currentValue = myProject.meteoPoints[i].residual;

        // hide all meteo points
        pointList[i]->setVisible(false);

        value = myProject.meteoPoints[i].currentValue;
        if (! isEqual(value, NODATA))
        {
            if (isEqual(minimum, NODATA))
            {
                minimum = value;
                maximum = value;
            }
            else
            {
                minimum = std::min(value, minimum);
                maximum = std::max(value, maximum);
            }
        }
    }

    myProject.meteoPointsColorScale->setRange(minimum, maximum);
    roundColorScale(myProject.meteoPointsColorScale, 4, true);
    setColorScale(anomaly, myProject.meteoPointsColorScale);

    Crit3DColor *myColor;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (! isEqual(myProject.meteoPoints[i].currentValue, NODATA))
        {
            pointList[i]->setRadius(5);
            myColor = myProject.meteoPointsColorScale->getColor(myProject.meteoPoints[i].currentValue);
            pointList[i]->setFillColor(QColor(myColor->red, myColor->green, myColor->blue));
            pointList[i]->setCurrentValue(myProject.meteoPoints[i].currentValue);
            pointList[i]->setQuality(myProject.meteoPoints[i].quality);
            pointList[i]->setToolTip();
            if (!(hideSupplementals && myProject.meteoPoints[i].lapseRateCode == supplemental))
                pointList[i]->setVisible(true);
        }
    }

    meteoPointsLegend->update();

    // cv text output
    meteoVariable currentVariable;
    switch(currentPointsVisualization)
    {
        case showCurrentVariable:
        {
            currentVariable = myProject.getCurrentVariable();
            break;
        }
        case showElaboration:
        {
            currentVariable = elaboration;
            break;
        }
        case showAnomalyAbsolute:
        {
            currentVariable = anomaly;
            break;
        }
        default:
        {
            currentVariable = myProject.getCurrentVariable();
        }
    }

    QString cvOutput;

    Crit3DCrossValidationStatistics myStats = myProject.getCrossValidationStatistics();

    if (myProject.interpolationSettings.getUseGlocalDetrending())
    {
        double avgMAE = 0, avgMBE = 0, avgRMSE = 0, avgNSE = 0, avgR2 = 0;
        for (unsigned int i = 0; i < myProject.glocalCrossValidationStatistics.size(); i++)
        {
            avgMAE += myProject.glocalCrossValidationStatistics[i].getMeanAbsoluteError();
            avgMBE += myProject.glocalCrossValidationStatistics[i].getMeanBiasError();
            avgRMSE  += myProject.glocalCrossValidationStatistics[i].getRootMeanSquareError();
            avgNSE  += myProject.glocalCrossValidationStatistics[i].getNashSutcliffeEfficiency();
            avgR2 += myProject.glocalCrossValidationStatistics[i].getR2();
        }
        avgMAE /= myProject.glocalCrossValidationStatistics.size();
        avgMBE /= myProject.glocalCrossValidationStatistics.size();
        avgRMSE /= myProject.glocalCrossValidationStatistics.size();
        avgNSE /= myProject.glocalCrossValidationStatistics.size();
        avgR2 /= myProject.glocalCrossValidationStatistics.size();

        myStats.setMeanAbsoluteError(avgMAE);
        myStats.setMeanBiasError(avgMBE);
        myStats.setRootMeanSquareError(avgRMSE);
        myStats.setNashSutcliffeEfficiency(avgNSE);
        myStats.setR2(avgR2);
    }

    cvOutput = "Time: " + getQDateTime(myProject.getCrit3DCurrentTime()).toString() + "\n";
    cvOutput += "Variable: " + QString::fromStdString(getVariableString(currentVariable)) + "\n";

    if (myProject.interpolationSettings.getUseGlocalDetrending()) cvOutput += "Values are averaged over " + QString::number(int(myProject.glocalCrossValidationStatistics.size())) + " macroareas\n";

    cvOutput += "MAE: " + QString::number(myStats.getMeanAbsoluteError(), 'f', 3) + "\n";
    cvOutput += "MBE: " + QString::number(myStats.getMeanBiasError(), 'f', 3) + "\n";
    cvOutput += "RMSE: " + QString::number(myStats.getRootMeanSquareError(), 'f', 3) + "\n";
    cvOutput += "NSE: " + QString::number(myStats.getNashSutcliffeEfficiency(), 'f', 3) + "\n";
    cvOutput += "R2: " + QString::number(myStats.getR2(), 'f', 3) + "\n";


    if (getUseDetrendingVar(currentVariable))
    {
        int proxyNr = int(myProject.interpolationSettings.getProxyNr());

        if (proxyNr > 0)
        {
            cvOutput + "\n" + "Interpolation proxies" + "\n";
            Crit3DProxyCombination proxyCombination = myProject.interpolationSettings.getCurrentCombination();
            Crit3DProxy* myProxy;

            if (! myProject.interpolationSettings.getUseMultipleDetrending())
            {

                for (int i=0; i < proxyNr; i++)
                {
                    if (proxyCombination.isProxyActive(i))
                    {
                        myProxy = myProject.interpolationSettings.getProxy(i);

                        cvOutput += QString::fromStdString(myProxy->getName()) + ": " + (proxyCombination.isProxySignificant(i) ? "" : "not " ) + "significant" + "\n";

                        if  (proxyCombination.isProxySignificant(i))
                        {
                            cvOutput += "R2=" + QString::number(myProxy->getRegressionR2(), 'f', 3) + "\n";
                            cvOutput += "slope=" + QString::number(myProxy->getRegressionSlope(), 'f', 3) + "\n";
                        }

                        if (getProxyPragaName(myProxy->getName()) == proxyHeight)
                        {
                            cvOutput += "inversion: ";
                            cvOutput += (myProxy->getInversionIsSignificative() ? "significant\n" : "not significant\n");
                        }
                    }
                }
            }
            else if (myProject.interpolationSettings.getUseMultipleDetrending() && ! myProject.interpolationSettings.getUseLocalDetrending() && ! myProject.interpolationSettings.getUseGlocalDetrending())
            {
                std::vector<std::vector<double>> par = myProject.interpolationSettings.getFittingParameters();
                for (int i=0; i < proxyNr; i++)
                {
                    if (proxyCombination.isProxyActive(i))
                    {
                        myProxy = myProject.interpolationSettings.getProxy(i);

                        if  (proxyCombination.isProxySignificant(i))
                        {
                            cvOutput += QString::fromStdString(myProxy->getName()) + "\n";

                            if (! myProject.interpolationSettings.getUseLocalDetrending())
                            {
                                if (getProxyPragaName(myProxy->getName()) == proxyHeight)
                                {
                                    cvOutput += "function: " + QString::fromStdString(getKeyStringElevationFunction(myProxy->getFittingFunctionName())) + "\n";
                                    for (int j=0; j < par[i].size(); j++)
                                        cvOutput += "par" + QString::number(j) + ": " + QString::number(par[i][j]) + "\n";

                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (myProject.interpolationSettings.getUseTD() && getUseTdVar(currentVariable))
    {
        cvOutput + "\n";
        cvOutput += "Topographic distance coefficient\n";
        cvOutput += "Best value: " + QString::number(myProject.interpolationSettings.getTopoDist_Kh()) + "\n";
        cvOutput += "Optimization:\n";

        std::vector <float> khSeries = myProject.interpolationSettings.getKh_series();
        std::vector <float> khErrors = myProject.interpolationSettings.getKh_error_series();

        for (unsigned int i=0; i < khSeries.size(); i++)
            cvOutput += "Kh=" + QString::number(khSeries[i]) + " error=" + QString::number(khErrors[i]) + "\n";
    }

    ui->textEditCV->setPlainText(cvOutput);
    ui->groupBoxCV->show();
}


void MainWindow::on_actionInterpolationSettings_triggered()
{
    DialogInterpolation* myDialogInterpolation = new DialogInterpolation(&myProject);
    if (myProject.proxyWidget != nullptr)
        myProject.proxyWidget->plot();

    myDialogInterpolation->close();
}

void MainWindow::on_actionRadiationSettings_triggered()
{
    DialogRadiation* myDialogRadiation = new DialogRadiation(&myProject);
    myDialogRadiation->close();
}

void MainWindow::on_actionSettingsParameters_triggered()
{
    DialogPragaSettings* mySettingsDialog = new DialogPragaSettings(&myProject);
    mySettingsDialog->exec();
    if (startCenter->latitude() != myProject.gisSettings.startLocation.latitude
        || startCenter->longitude() != myProject.gisSettings.startLocation.longitude)
    {
        startCenter->setLatitude(myProject.gisSettings.startLocation.latitude);
        startCenter->setLongitude(myProject.gisSettings.startLocation.longitude);
        this->mapView->centerOn(startCenter->lonLat());
    }

    mySettingsDialog->close();
}


void MainWindow::on_actionTopographicDistanceMapsSave_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Create topographic distance maps", "Only for stations with data?",
            QMessageBox::Yes|QMessageBox::No);

    bool onlyWithData = (reply == QMessageBox::Yes);

    myProject.writeTopographicDistanceMaps(onlyWithData, true);
}

void MainWindow::on_actionTopographicDistanceMapsLoad_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Create topographic distance maps", "Only for stations with data?",
            QMessageBox::Yes|QMessageBox::No);

    bool onlyWithData = (reply == QMessageBox::Yes);

    myProject.loadTopographicDistanceMaps(onlyWithData, true);
}

void MainWindow::on_actionShowPointsHide_triggered()
{
    redrawMeteoPoints(notShown, true);
}

void MainWindow::on_actionShowPointsLocation_triggered()
{
    redrawMeteoPoints(showLocation, true);
}


void MainWindow::on_actionShowPointsCurrent_triggered()
{
    redrawMeteoPoints(showCurrentVariable, true);
}

void MainWindow::on_actionShowPointsElab_triggered()
{
    redrawMeteoPoints(showElaboration, true);
}

void MainWindow::on_actionShowPointsAnomalyAbs_triggered()
{
    redrawMeteoPoints(showAnomalyAbsolute, true);
}

void MainWindow::on_actionShowPointsAnomalyPerc_triggered()
{
    redrawMeteoPoints(showAnomalyPercentage, true);
}

void MainWindow::on_actionShowPointsClimate_triggered()
{
    redrawMeteoPoints(showClimate, true);
}

void MainWindow::on_actionShowGridHide_triggered()
{
    redrawMeteoGrid(notShown, false);
}

void MainWindow::on_actionShowGridLocation_triggered()
{
    redrawMeteoGrid(showLocation, false);
}

void MainWindow::on_actionShowGridCurrent_triggered()
{
    redrawMeteoGrid(showCurrentVariable, false);
}

void MainWindow::on_actionShowGridElab_triggered()
{
    redrawMeteoGrid(showElaboration, false);
}

void MainWindow::on_actionShowGridAnomalyAbs_triggered()
{
    redrawMeteoGrid(showAnomalyAbsolute, false);
}

void MainWindow::on_actionShowGridAnomalyPerc_triggered()
{
    redrawMeteoGrid(showAnomalyPercentage, false);
}

void MainWindow::on_actionShowGridClimate_triggered()
{
    redrawMeteoGrid(showClimate, false);
}


void MainWindow::on_actionSettingsMapOpenStreetMap_triggered()
{
    this->setTileSource(WebTileSource::OPEN_STREET_MAP);
}

void MainWindow::on_actionSettingsMapGoogleMap_triggered()
{
    this->setTileSource(WebTileSource::GOOGLE_MAP);
}

void MainWindow::on_actionSettingsMapGoogleSatellite_triggered()
{
    this->setTileSource(WebTileSource::GOOGLE_Satellite);
}

void MainWindow::on_actionSettingsMapGoogleTerrain_triggered()
{
    this->setTileSource(WebTileSource::GOOGLE_Terrain);
}

void MainWindow::on_actionSettingsMapGoogleHybrid_triggered()
{
    this->setTileSource(WebTileSource::GOOGLE_Hybrid_Satellite);
}

void MainWindow::on_actionSettingsMapEsriSatellite_triggered()
{
    this->setTileSource(WebTileSource::ESRI_WorldImagery);
}

void MainWindow::on_actionSettingsMapStamenTerrain_triggered()
{
    this->setTileSource(WebTileSource::STAMEN_Terrain);
}


void MainWindow::setTileSource(WebTileSource::WebTileType tileType)
{
    // deselect all
    ui->actionSettingsMapOpenStreetMap->setChecked(false);
    ui->actionSettingsMapStamenTerrain->setChecked(false);
    ui->actionSettingsMapEsriSatellite->setChecked(false);
    ui->actionSettingsMapGoogleMap->setChecked(false);
    ui->actionSettingsMapGoogleSatellite->setChecked(false);
    ui->actionSettingsMapGoogleHybrid->setChecked(false);
    ui->actionSettingsMapGoogleTerrain->setChecked(false);

    // check selected
    switch(tileType)
    {
    case WebTileSource::OPEN_STREET_MAP:
        ui->actionSettingsMapOpenStreetMap->setChecked(true);
        break;

    case WebTileSource::STAMEN_Terrain:
        ui->actionSettingsMapStamenTerrain->setChecked(true);
        break;

    case WebTileSource::ESRI_WorldImagery:
        ui->actionSettingsMapEsriSatellite->setChecked(true);
        break;

    case WebTileSource::GOOGLE_MAP:
        ui->actionSettingsMapGoogleMap->setChecked(true);
        break;

    case WebTileSource::GOOGLE_Satellite:
        ui->actionSettingsMapGoogleSatellite->setChecked(true);
        break;

    case WebTileSource::GOOGLE_Hybrid_Satellite:
        ui->actionSettingsMapGoogleHybrid->setChecked(true);
        break;

    case WebTileSource::GOOGLE_Terrain:
        ui->actionSettingsMapGoogleTerrain->setChecked(false);
        break;
    }

    // set tiles source
    QSharedPointer<WebTileSource> myTiles(new WebTileSource(tileType), &QObject::deleteLater);
    this->mapView->setTileSource(myTiles);
}


void MainWindow::on_actionSpatialAggregationOpenDB_triggered()
{
    QString dbName = QFileDialog::getOpenFileName(this, tr("Open DB meteo points"), "", tr("DB files (*.db)"));
    if (dbName != "")
    {
        myProject.loadAggregationdDB(dbName);
    }

}

void MainWindow::on_actionSpatialAggregationNewDB_triggered()
{
    QString templateFileName = myProject.getDefaultPath() + PATH_TEMPLATE + "template_meteo_aggregation.db";

    QString dbName = QFileDialog::getSaveFileName(this, tr("Save as"), "", tr("DB files (*.db)"));
    if (dbName == "")
        return;

    QFile dbFile(dbName);
    QFileInfo dbFileInfo(dbFile.fileName());
    if (dbFile.exists())
    {
        if (! dbFile.remove())
        {
            myProject.logError("Remove file failed: " + dbName + "\n" + dbFile.errorString());
            return;
        }
    }

    if (!QFile::copy(templateFileName, dbName))
    {
        myProject.logError("Copy file failed: " + templateFileName);
        return;
    }
    myProject.loadAggregationdDB(dbName);

    QString rasterName = QFileDialog::getOpenFileName(this, tr("Open raster"), "", tr("files (*.flt)"));
    if (rasterName.isEmpty() || ! rasterName.contains(".flt"))
    {
        myProject.logError(ERROR_STR_MISSING_DEM);
        return;
    }
    QFileInfo rasterFileInfo(rasterName);

    if (dbFileInfo.absolutePath() != rasterFileInfo.absolutePath())
    {
        myProject.logWarning("Raster and aggregation db should be in the same folder");
        QString rasterCopiedFlt = dbFileInfo.absolutePath() + "/" + rasterFileInfo.baseName() + ".flt";
        QString rasterCopiedHdr = dbFileInfo.absolutePath() + "/" + rasterFileInfo.baseName() + ".hdr";
        QString rasterHdr = rasterFileInfo.absolutePath() + "/" + rasterFileInfo.baseName() + ".hdr";
        if (!QFile::copy(rasterName, rasterCopiedFlt) || !QFile::copy(rasterHdr, rasterCopiedHdr))
        {
            myProject.logError("Copy raster failed: " + rasterName);
            return;
        }
    }

    if (! myProject.aggregationDbHandler->writeRasterName(rasterFileInfo.baseName()))
    {
        myProject.logError("Error in writing raster name into db.");
        return;
    }

    myProject.logInfoGUI("New db successfully created.");
}


bool checkAggregationData(QString &rasterFileName)
{
    if (! myProject.meteoPointsLoaded && ! myProject.meteoGridLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return false;
    }

    if (myProject.aggregationDbHandler == nullptr)
    {
        myProject.logError("Missing Aggregation DB: open or create an aggregation DB before.");
        return false;
    }

    QString rasterName;
    if (! myProject.aggregationDbHandler->getRasterName(&rasterName))
    {
        myProject.logError("Missing Raster Name inside the aggregation db.");
        return false;
    }

    QFileInfo rasterFileFltInfo(myProject.aggregationPath + "/" + rasterName + ".flt");
    QFileInfo rasterFileHdrInfo(myProject.aggregationPath + "/" + rasterName + ".hdr");
    if (! rasterFileFltInfo.exists() || ! rasterFileHdrInfo.exists())
    {
        myProject.errorString = "Raster file does not exist: " + myProject.aggregationPath + "/" + rasterName;
        myProject.logError();
        return false;
    }
    rasterFileName = myProject.aggregationPath + "/" + rasterName + ".flt";

    return true;
}


void MainWindow::on_actionSpatialAggregationFromGrid_triggered()
{
    QString rasterFileName;
    if (! checkAggregationData(rasterFileName))
        return;

    gis::Crit3DRasterGrid myRaster;
    std::string errorStr = "";
    if (! gis::openRaster(rasterFileName.toStdString(), &myRaster, myProject.gisSettings.utmZone, errorStr))
    {
        myProject.logError("Open raster failed: " + QString::fromStdString(errorStr));
        return;
    }

    QList<QString> aggregation = myProject.aggregationDbHandler->getAggregations();
    if (aggregation.isEmpty())
    {
        myProject.logError("Empty aggregation: " + myProject.aggregationDbHandler->error());
        return;
    }

    bool isHourly = false;
    DialogSeriesOnZones zoneDialog(myProject.pragaDefaultSettings, aggregation, myProject.getCurrentDate(), isHourly);
    if (zoneDialog.result() != QDialog::Accepted)
        return;

    float threshold = NODATA;
    meteoComputation elab1MeteoComp = noMeteoComp;
    if (! myProject.averageSeriesOnZonesMeteoGrid(zoneDialog.getVariable(), elab1MeteoComp, zoneDialog.getSpatialElaboration(), threshold, &myRaster, zoneDialog.getStartDate(), zoneDialog.getEndDate(), true))
    {
        myProject.logError();
    }
}


void MainWindow::on_actionSpatialAggregationFromGrid_hourly_triggered()
{
    QString rasterFileName;
    if (! checkAggregationData(rasterFileName))
        return;

    QList<QString> aggregation = myProject.aggregationDbHandler->getAggregations();
    if (aggregation.isEmpty())
    {
        myProject.logError("Empty aggregation: " + myProject.aggregationDbHandler->error());
        return;
    }

    gis::Crit3DRasterGrid myRaster;
    std::string errorStr = "";
    if (! gis::openRaster(rasterFileName.toStdString(), &myRaster, myProject.gisSettings.utmZone, errorStr))
    {
        myProject.logError("Open raster failed: " + QString::fromStdString(errorStr));
        return;
    }

    bool isHourly = true;
    DialogSeriesOnZones zoneDialog(myProject.pragaDefaultSettings, aggregation, myProject.getCurrentDate(), isHourly);
    if (zoneDialog.result() != QDialog::Accepted)
        return;

    float myThreshold = NODATA;
    meteoComputation elab1MeteoComp = average;
    meteoVariable variable = zoneDialog.getVariable();
    bool isShowInfo = true;

    if (! myProject.averageSeriesOnZonesMeteoGrid(variable, elab1MeteoComp, zoneDialog.getSpatialElaboration(), myThreshold, &myRaster, zoneDialog.getStartDate(), zoneDialog.getEndDate(), isShowInfo))
    {
        myProject.logError();
    }
}


void MainWindow::on_actionSpatialAggregationAssignAltitude_triggered()
{
    if (! myProject.assignAltitudeToAggregationPoints() )
    {
        myProject.logError();
        return;
    }

    myProject.logInfoGUI("Altitude assigned to the aggregation points.");
}


void MainWindow::drawWindowTitle()
{
    QString title = myProject.getVersion();
    if (myProject.projectName != "")
        title += " - " + myProject.projectName;

    setWindowTitle(title);
}


void MainWindow::drawProject()
{
    mapView->centerOn(startCenter->lonLat());

    if (myProject.DEM.isLoaded)
        renderDEM();

    drawMeteoPoints();
    drawMeteoGrid();

    drawWindowTitle();
}


void MainWindow::on_actionFileOpenProject_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open project file"), myProject.getDefaultPath() + PATH_PROJECT, tr("ini files (*.ini)"));
    if (fileName == "") return;

    if (myProject.isProjectLoaded)
    {
        on_actionFileMeteogridClose_triggered();
        on_actionFileMeteopointClose_triggered();
        clearDEM();

        this->ui->labelFrequency->setText("None");
        this->ui->labelVariable->setText(("None"));
    }

    if (myProject.loadPragaProject(fileName))
    {
        drawProject();
    }
    else
    {
        myProject.logError("Error in opening project: " + myProject.errorString);

        this->mapView->centerOn(startCenter->lonLat());
        if (myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini")) drawProject();
    }

    checkSaveProject();
}

void MainWindow::on_actionFileCloseProject_triggered()
{
    if (! myProject.isProjectLoaded) return;

    closeMeteoGrid();
    closeMeteoPoints();

    ui->labelFrequency->setText("None");
    ui->labelVariable->setText(("None"));
    currentGridVisualization = showLocation;
    currentPointsVisualization = showLocation;

    clearDEM();

    mapView->centerOn(startCenter->lonLat());

    if (! myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini")) return;

    drawProject();
    checkSaveProject();
}

void MainWindow::on_actionFileSaveProjectAs_triggered()
{
    DialogPragaProject* myProjectDialog = new DialogPragaProject(&myProject);
    myProjectDialog->exec();
    myProjectDialog->close();

    drawWindowTitle();
    checkSaveProject();
}

void MainWindow::on_actionFileSaveProject_triggered()
{
    myProject.saveProject();
}

void MainWindow::checkSaveProject()
{
    QString myName = myProject.projectSettings->fileName();
    if (getFileName(myName) == "default.ini" && getFilePath(myName) == myProject.getApplicationPath())
        ui->actionFileSaveProject->setEnabled(false);
    else
        ui->actionFileSaveProject->setEnabled(true);
}


void MainWindow::on_actionInterpolationDem_triggered()
{
    interpolateDemGUI();
}


void MainWindow::on_actionInterpolationOutputPointsCurrentTime_triggered()
{
    myProject.setComputeOnlyPoints(true);
    interpolateDemGUI();
    myProject.setComputeOnlyPoints(false);
}


void MainWindow::on_actionInterpolationMeteogridCurrentTime_triggered()
{
    myProject.logInfoGUI("Interpolating on meteo grid...");
    interpolateGridGUI();
    myProject.closeLogInfo();
}

void MainWindow::on_actionInterpolationMeteogridSaveCurrentData_triggered()
{
    myProject.logInfoGUI("Saving meteo grid data...");
    if (myProject.meteoGridDbHandler != nullptr)
    {
        myProject.saveGrid(myProject.getCurrentVariable(), myProject.getCurrentFrequency(),
                           myProject.getCrit3DCurrentTime(), true);
    }
    myProject.closeLogInfo();
}


void MainWindow::on_actionInterpolationMeteogridPeriod_triggered()
{
    // check meteo points
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    // check meteo grid
    if (! myProject.meteoGridLoaded || myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }

    // update first db time
    if (myProject.meteoPointsDbFirstTime.isNull() || myProject.meteoPointsDbFirstTime.toSecsSinceEpoch() == 0)
    {
        myProject.meteoPointsDbFirstTime = myProject.findDbPointFirstTime();
    }
    QDateTime myFirstTime = myProject.meteoPointsDbFirstTime;

    if (myFirstTime.isNull())
    {
        myFirstTime.setDate(myProject.getCurrentDate());
        myFirstTime.setTime(QTime(myProject.getCurrentHour(),0));
    }

    QDateTime myLastTime = myProject.meteoPointsDbLastTime;
    if (myLastTime.isNull())
    {
        myLastTime.setDate(myProject.getCurrentDate());
        myLastTime.setTime(QTime(myProject.getCurrentHour(),0));
    }

    if (myProject.getCurrentFrequency() == noFrequency)
    {
        myProject.logError("Choose frequency before.");
        return;
    }

    FormTimePeriod myForm(&myFirstTime, &myLastTime);
    myForm.show();
    if (myForm.exec() == QDialog::Rejected)
        return;

    meteoVariable myVar = chooseMeteoVariable(myProject);
    if (myVar == noMeteoVar)
        return;

    QList <meteoVariable> myVariables, aggrVariables, derVariables;
    myVariables.push_back(myVar);
    if (! myProject.interpolationMeteoGridPeriod(myFirstTime.date(), myLastTime.date(), myVariables,
                                                derVariables, aggrVariables, 1, NODATA))
    {
        myProject.logError();
        return;
    }

    myProject.meteoGridDbHandler->updateMeteoGridDate(myProject.errorString);
}


void MainWindow::on_actionInterpolationOutputPointsPeriod_triggered()
{
    // check meteo points
    if (! myProject.meteoPointsLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    // check output points
    if (! myProject.outputMeteoPointsLoaded)
    {
        myProject.logError("Open output points DB before.");
        return;
    }

    // check frequency
    if (myProject.getCurrentFrequency() == noFrequency)
    {
        myProject.logError("Choose frequency before.");
        return;
    }

    myProject.logInfoGUI("Checking available dates, wait..");
    if (myProject.meteoPointsDbFirstTime.isNull() || myProject.meteoPointsDbFirstTime.toSecsSinceEpoch() == 0)
    {
        myProject.meteoPointsDbFirstTime = myProject.findDbPointFirstTime();
    }

    QDateTime firstTime;
    firstTime.setTimeZone(QTimeZone::utc());
    firstTime = myProject.meteoPointsDbFirstTime;
    if (firstTime.isNull())
    {
        firstTime.setDate(myProject.getCurrentDate());
        firstTime.setTime(QTime(myProject.getCurrentHour(), 0));
    }

    QDateTime lastTime;
    lastTime.setTimeZone(QTimeZone::utc());
    lastTime = myProject.meteoPointsDbLastTime;
    if (lastTime.time().hour() == 00)
        lastTime = lastTime.addSecs(-3600);

    if (lastTime.isNull())
    {
        lastTime.setDate(myProject.getCurrentDate());
        lastTime.setTime(QTime(myProject.getCurrentHour(), 0));
    }
    myProject.closeLogInfo();

    // choose period
    FormTimePeriod myForm(&firstTime, &lastTime);
    myForm.setMinimumDate(firstTime.date());
    myForm.setMaximumDate(lastTime.date());
    myForm.show();
    if (myForm.exec() == QDialog::Rejected) return;

    meteoVariable myVar = chooseMeteoVariable(myProject);
    if (myVar == noMeteoVar) return;

    QList<meteoVariable> variablesList;
    variablesList.push_back(myVar);

    if (! myProject.interpolationOutputPointsPeriod(firstTime.date(), lastTime.date(), variablesList))
    {
        myProject.logError();
    }
}


void MainWindow::on_actionInterpolationCVCurrentTime_triggered()
{
    QString glocalCVPointsName;
    if (myProject.interpolationSettings.getUseGlocalDetrending())
    {
        QMessageBox msgBox;
        msgBox.setText("Cross validation with glocal detrending");
        msgBox.setInformativeText("Do you want to load a different .csv stations file?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes)
        {
            QString defaultPath = myProject.getDefaultPath() + PATH_GEO;
            glocalCVPointsName = QFileDialog::getOpenFileName(this, tr("Open file with stations"), defaultPath,
                                                            tr("Comma separated values (*.csv)"));
            if (glocalCVPointsName.isEmpty())
                return;
        }
    }

    // check glocal
    if (myProject.interpolationSettings.getUseGlocalDetrending() && (! myProject.interpolationSettings.isGlocalReady(false) || ! glocalCVPointsName.isEmpty()))
    {
        myProject.loadGlocalAreasMap();
        if (glocalCVPointsName.isEmpty())
        {
            myProject.loadGlocalStationsAndCells(false, myProject.getCompleteFileName(myProject.glocalPointsName, PATH_GEO));
        }
        else {
            myProject.loadGlocalStationsAndCells(false, myProject.getCompleteFileName(glocalCVPointsName, PATH_GEO));
        }
    }

    myProject.logInfoGUI("Cross validation...");

    meteoVariable currentVariable;
    switch(currentPointsVisualization)
    {
        case showCurrentVariable:
        {
            currentVariable = myProject.getCurrentVariable();
            break;
        }
        case showElaboration:
        {
            currentVariable = elaboration;
            break;
        }
        case showAnomalyAbsolute:
        {
            currentVariable = anomaly;
            break;
        }
        default:
        {
            currentVariable = myProject.getCurrentVariable();
        }
    }

    bool isComputed = false;


    isComputed = myProject.interpolationCv(currentVariable, myProject.getCrit3DCurrentTime());

    myProject.closeLogInfo();

    if (isComputed)
    {
        this->ui->actionShowPointsCVResidual->setEnabled(true);
        redrawMeteoPoints(showCVResidual, false);
    }
}


void MainWindow::on_actionInterpolationCVPeriod_triggered()
{
    // check meteo points
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    meteoVariable myVar = chooseMeteoVariable(myProject);
    if (myVar == noMeteoVar)
        return;

    // update first db time
    if (myProject.meteoPointsDbFirstTime.isNull() || myProject.meteoPointsDbFirstTime.toSecsSinceEpoch() == 0)
    {
        myProject.meteoPointsDbFirstTime = myProject.findDbPointFirstTime();
    }
    QDateTime myFirstTime = myProject.meteoPointsDbFirstTime;
    if (myFirstTime.isNull())
    {
        myFirstTime.setDate(myProject.getCurrentDate());
        myFirstTime.setTime(QTime(myProject.getCurrentHour(),0));
    }

    QDateTime myLastTime = myProject.meteoPointsDbLastTime;
    if (myLastTime.isNull())
    {
        myLastTime.setDate(myProject.getCurrentDate());
        myLastTime.setTime(QTime(myProject.getCurrentHour(),0));
    }

    FormTimePeriod myForm(&myFirstTime, &myLastTime);
    myForm.show();
    if (myForm.exec() == QDialog::Rejected) return;

    FormText formLoadInterval("Insert number of days for load interval", "");
    if (formLoadInterval.result() == QDialog::Rejected)
        return;

    QString loadIntervalString = formLoadInterval.getText();



    bool isValid;
    int loadInterval = loadIntervalString.toInt(&isValid);
    if (! isValid)
        myProject.logError("Invalid number of days: " + loadIntervalString);

    QString glocalCVPointsName;
    if (myProject.interpolationSettings.getUseGlocalDetrending())
    {
        QMessageBox msgBox;
        msgBox.setText("Cross validation with glocal detrending");
        msgBox.setInformativeText("Do you want to load a different .csv stations file?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes)
        {
            QString defaultPath = myProject.getDefaultPath() + PATH_GEO;
            glocalCVPointsName = QFileDialog::getOpenFileName(this, tr("Open file with stations"), defaultPath,
                                                            tr("Comma separated values (*.csv)"));
            if (glocalCVPointsName.isEmpty())
                return;
        }
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save current CV output"), "", tr("text file (*.txt)"));
    if (fileName == "") return;

    if (myProject.interpolationCrossValidationPeriod(myFirstTime.date(), myLastTime.date(), myVar, fileName, loadInterval, glocalCVPointsName))
        myProject.closeLogInfo();
    else
        myProject.logError();
}


void MainWindow::on_actionFileMeteopointNewArkimet_triggered()
{
    clearMeteoPointsMarker();

    QString templateFileName = myProject.getDefaultPath() + PATH_TEMPLATE + "template_meteo_arkimet.db";
    QString path = myProject.getProjectPath() + PATH_METEOPOINT;

    QString dbName = QFileDialog::getSaveFileName(this, tr("Save as"), path, tr("DB files (*.db)"));
    if (dbName == "")
        return;

    QFile dbFile(dbName);
    if (dbFile.exists())
    {
        myProject.closeMeteoPointsDB();
        myProject.setIsElabMeteoPointsValue(false);
        dbFile.close();
        dbFile.setPermissions(QFile::ReadOther | QFile::WriteOther);
        if (! dbFile.remove())
        {
            myProject.logError("Remove file failed: " + dbName + "\n" + dbFile.errorString());
            return;
        }
    }

    if (! QFile::copy(templateFileName, dbName))
    {
        myProject.logError("Copy file failed: " + templateFileName);
        return;
    }

    Download myDownload(dbName);

    QList<QString> dataset = myDownload.getDbArkimet()->getAllDatasetsList();

    QDialog datasetDialog;

    datasetDialog.setWindowTitle("Datasets");
    datasetDialog.setFixedWidth(500);
    QVBoxLayout layout;

    for (int i = 0; i < dataset.size(); i++)
    {
        QCheckBox* newDataset = new QCheckBox(dataset[i]);
        layout.addWidget(newDataset);

        datasetCheckbox.append(newDataset);
    }

    allDatasets = new QCheckBox("ALL");
    layout.addSpacing(30);
    layout.addWidget(allDatasets);

    connect(allDatasets, SIGNAL(toggled(bool)), this, SLOT(enableAllDataset(bool)));

    for (int i = 0; i < dataset.size(); i++)
    {
        connect(datasetCheckbox[i], SIGNAL(toggled(bool)), this, SLOT(disableAllButton(bool)));
    }

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), &datasetDialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &datasetDialog, SLOT(reject()));

    layout.addWidget(buttonBox);
    datasetDialog.setLayout(&layout);

    QString datasetSelected = selectArkimetDataset(&datasetDialog);

    if (!datasetSelected.isEmpty())
    {
        myDownload.getDbArkimet()->setDatasetsActive(datasetSelected);
        QList<QString> datasets = datasetSelected.remove("'").split(",");

        myProject.logInfoGUI("download points properties...");
        if (myDownload.getPointProperties(datasets, myProject.gisSettings.utmZone, myProject.errorString))
        {
            loadMeteoPoints(dbName);
        }
        else
        {
            myProject.logError("Error in function getPointProperties: " + myProject.errorString);
        }

        myProject.closeLogInfo();
    }
    else
    {
        QFile::remove(dbName);
    }

    delete buttonBox;
    delete allDatasets;
}


void MainWindow::on_actionFileMeteopointOpen_triggered()
{
    QString path = myProject.getProjectPath() + PATH_METEOPOINT;

    QString dbName = QFileDialog::getOpenFileName(this, tr("Open DB meteo points"), path, tr("DB files (*.db)"));
    if (! dbName.isEmpty())
    {
        currentPointsVisualization = showLocation;
        if (! myProject.meteoGridLoaded)
        {
            ui->labelFrequency->setText("None");
            ui->labelVariable->setText(("None"));
        }

        loadMeteoPoints(dbName);
    }
}


void MainWindow::on_actionFileMeteopointClose_triggered()
{
    this->closeMeteoPoints();
}

void MainWindow::closeMeteoPoints()
{
    if (myProject.meteoPointsDbHandler != nullptr)
    {
        clearMeteoPointsMarker();
        clearWindVectorObjects();
        meteoPointsLegend->setVisible(false);

        myProject.closeMeteoPointsDB();

        myProject.setIsElabMeteoPointsValue(false);
        ui->groupBoxElaboration->hide();

        ui->actionMeteopointRectangleSelection->setEnabled(false);
        ui->menuActive_points->setEnabled(false);
        ui->menuSelected_points->setEnabled(false);
        ui->menuSearch_points->setEnabled(false);
        ui->menuMark_points->setEnabled(false);
        ui->menuDeactive_points->setEnabled(false);
        ui->menuDelete_points->setEnabled(false);
        ui->menuDelete_data->setEnabled(false);
        ui->menuShift_data->setEnabled(false);
        ui->actionMeteopointDataCount->setEnabled(false);
        ui->actionMeteoPointsClear_selection->setEnabled(false);

        showPointsGroup->setEnabled(false);
        this->ui->menuShowPointsAnomaly->setEnabled(false);
    }
}


void MainWindow::on_actionFileMeteogridOpen_triggered()
{
    QString path = myProject.getProjectPath() + PATH_METEOGRID;

    QString xmlName = QFileDialog::getOpenFileName(this, tr("Open XML DB meteo grid"), path, tr("xml files (*.xml)"));
    if (xmlName != "")
    {
        closeMeteoGrid();
        loadMeteoGrid(xmlName);
    }
}


void MainWindow::closeMeteoGrid()
{
    if (myProject.meteoGridLoaded)
    {
        // set location
        redrawMeteoGrid(showLocation, false);
        this->meteoGridObj->clear();
        emit this->meteoGridObj->redrawRequested();
        this->meteoGridLegend->setVisible(false);

        myProject.closeMeteoGridDB();

        this->ui->groupBoxElaboration->hide();
        this->ui->meteoGridOpacitySlider->setEnabled(false);
        this->showGridGroup->setEnabled(false);
        this->ui->menuActive_cells->setEnabled(false);
        this->ui->actionCompute_monthly_data_from_daily->setEnabled(false);
        this->ui->menuShowGridAnomaly->setEnabled(false);
    }
}


void MainWindow::on_actionFileMeteogridClose_triggered()
{
    this->closeMeteoGrid();
}


void MainWindow::on_actionMeteopointDataCount_triggered()
{
    // check meteo point
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    meteoVariable myVar = chooseMeteoVariable(myProject);
    if (myVar == noMeteoVar) return;
    frequencyType myFreq = getVarFrequency(myVar);

    // update first db time
    if (myProject.meteoPointsDbFirstTime.isNull() || myProject.meteoPointsDbFirstTime.toSecsSinceEpoch() == 0)
    {
        myProject.meteoPointsDbFirstTime = myProject.findDbPointFirstTime();
    }
    QDateTime myFirstTime = myProject.meteoPointsDbFirstTime;
    if (myFirstTime.isNull())
    {
        myFirstTime.setDate(myProject.getCurrentDate());
        myFirstTime.setTime(QTime(myProject.getCurrentHour(),0));
    }

    QDateTime myLastTime = myProject.meteoPointsDbLastTime;
    if (myLastTime.isNull())
    {
        myLastTime.setDate(myProject.getCurrentDate());
        myLastTime.setTime(QTime(myProject.getCurrentHour(),0));
    }

    FormTimePeriod myForm(&myFirstTime, &myLastTime);
    myForm.show();
    if (myForm.exec() == QDialog::Rejected) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Data count", "Choose dataset?",
            QMessageBox::Yes|QMessageBox::No);

    QString dataset = "";
    if (reply == QMessageBox::Yes)
    {
        QList<QString> datasets = myProject.meteoPointsDbHandler->getDatasetsActive();
        bool ok;
        dataset = QInputDialog::getItem(this, tr("Data count"), tr("Choose dataset"), datasets, 0, false, &ok);
        if (! ok && dataset.isEmpty()) return;
    }

    QString myFilename  = QFileDialog::getSaveFileName(this, tr("Save as"), "", tr("text files (*.txt)"));
    if (myFilename == "") return;

    std::vector<int> myCounter;

    if (myProject.interpolationSettings.getUseGlocalDetrending() && ! myProject.interpolationSettings.isGlocalReady(false))
    {
        if (! myProject.loadGlocalAreasMap() || ! myProject.loadGlocalStationsAndCells(false, myProject.getCompleteFileName(myProject.glocalPointsName, PATH_GEO)))
        {
            myProject.logError("Unable to load glocal files.");
            return;
        }
    }

    if (myProject.dbMeteoPointDataCount(myFirstTime.date(), myLastTime.date(), myVar, dataset, myCounter))
    {
        QFile myFile(myFilename);
        if (myFile.open(QIODevice::ReadWrite))
        {
            QTextStream outStream(&myFile);

            QDate myDate = myFirstTime.date();
            short myHour;
            long i=0;

            if (! myProject.interpolationSettings.getUseGlocalDetrending())
            {
                while (myDate <= myLastTime.date())
                {
                    if (myFreq == daily)
                    {
                        outStream << myDate.toString("yyyy-MM-dd") << "," << QString::number(myCounter[i++]) + "\n";
                    }
                    else if (myFreq == hourly)
                    {
                        for (myHour = 1; myHour <= 24; myHour++)
                            outStream << myDate.toString("yyyy-MM-dd") + " " + QStringLiteral("%1").arg(myHour, 2, 10, QLatin1Char('0')) + ":00" << "," << QString::number(myCounter[i++]) + "\n";
                    }

                    myDate = myDate.addDays(1);
                }
            }
            else
            {
                std::vector <int> macroAreaCodes;
                for (int k = 0; k < myProject.interpolationSettings.getMacroAreas().size(); k++)
                {
                    if (! myProject.interpolationSettings.getMacroAreas()[k].getMeteoPoints().empty())  macroAreaCodes.push_back(k);
                }
                while (myDate <= myLastTime.date())
                {
                    if (myFreq == daily)
                    {
                        //myCounter contains counters for every macroarea so it must be split
                        for (int k = 0; k < macroAreaCodes.size(); k++)
                        {
                            outStream << myDate.toString("yyyy-MM-dd") << "," << QString::number(macroAreaCodes[k]) << "," << QString::number(myCounter[i+k]) + "\n";
                        }
                        i += (int)macroAreaCodes.size();
                    }
                    else if (myFreq == hourly)
                    {
                        //
                        for (int k = 0; k < macroAreaCodes.size(); k++)
                        {
                            for (myHour = 1; myHour <= 24; myHour++)
                            {
                                outStream << myDate.toString("yyyy-MM-dd") + " " + QStringLiteral("%1").arg(myHour, 2, 10, QLatin1Char('0')) + ":00" << "," << QString::number(macroAreaCodes[k]) << "," << QString::number(myCounter[i]) + "\n";
                                i++;
                            }
                        }
                    }

                    myDate = myDate.addDays(1);
                }
            }
            myFile.close();
            myCounter.clear();
        }
    }
}

void MainWindow::on_actionMeteogridMissingData_triggered()
{
    // check meteo grid
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError("No meteo grid DB open");
        return;
    }

    meteoVariable myVar = chooseMeteoVariable(myProject);
    if (myVar == noMeteoVar) return;

    QDateTime myFirstTime(myProject.meteoGridDbHandler->firstDate(), QTime(1,0,0), Qt::UTC);
    QDateTime myLastTime(myProject.meteoGridDbHandler->lastDate().addDays(1), QTime(0,0,0), Qt::UTC);
    if (myFirstTime.isNull())
    {
        myFirstTime.setDate(myProject.getCurrentDate());
        myFirstTime.setTime(QTime(myProject.getCurrentHour(),0));
    }
    if (myLastTime.isNull())
    {
        myLastTime.setDate(myProject.getCurrentDate());
        myLastTime.setTime(QTime(myProject.getCurrentHour(),0));
    }

    FormTimePeriod myForm(&myFirstTime, &myLastTime);
    myForm.show();
    if (myForm.exec() == QDialog::Rejected) return;

    QString myFilename  = QFileDialog::getSaveFileName(this, tr("Save as"), "", tr("text files (*.txt)"));
    if (myFilename == "") return;

    QList <QDate> myDateList;
    QList <QString> idList;

    if (myProject.dbMeteoGridMissingData(myFirstTime.date(), myLastTime.date(), myVar, myDateList, idList))
    {
        QFile myFile(myFilename);
        if (myFile.open(QIODevice::ReadWrite))
        {
            QTextStream outStream(&myFile);

            int i=0;
            foreach (QDate myDate, myDateList)
            {
                outStream << idList[i] << "," << myDate.toString("yyyy-MM-dd") << "\n";
                i++;
            }

            myFile.close();
            myDateList.clear();
        }
    }
}


void MainWindow::on_dayBeforeButton_clicked()
{
    this->ui->dateEdit->setDate(this->ui->dateEdit->date().addDays(-1));
}

void MainWindow::on_dayAfterButton_clicked()
{
    this->ui->dateEdit->setDate(this->ui->dateEdit->date().addDays(1));
}


void MainWindow::on_actionImport_data_XML_grid_triggered()
{
    // check meteo grid
    if (! myProject.meteoGridLoaded || myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }

    bool isGrid = true;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("xml files (*.xml)"));
    if (fileName.isEmpty())
        return;


    if (! myProject.parserXMLImportExportData(fileName, isGrid))
    {
        return;
    }

    QList<QString> dateFiles = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more files to open",
                            "",
                            "Files (*.prn *.csv *.txt)");
    if (dateFiles.isEmpty())
        return;

    myProject.setProgressBar("Loading data...", dateFiles.size());
    QString warningStr;

    for (int i=0; i < dateFiles.size(); i++)
    {
        myProject.updateProgressBar(i);
        if (myProject.loadXMLImportData(dateFiles[i]))
        {
            if (! myProject.errorString.isEmpty())
            {
                warningStr += dateFiles[i] + "\n" + myProject.errorString + "\n";
            }
        }
        else
        {
            if (i != dateFiles.size()-1)
            {
                // it is not the last
                QMessageBox msgBox;
                msgBox.setText("An error occurred: " + dateFiles[i]);
                msgBox.setInformativeText("Do you want to go on with other files?");
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                int ret = msgBox.exec();
                if (ret == QMessageBox::Ok)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
    }

    myProject.closeProgressBar();
    if (! warningStr.isEmpty())
    {
        myProject.logWarning("Not valid or missing values: " + warningStr);
    }

    delete myProject.inOutData;
    QString xmlName = myProject.dbGridXMLFileName;
    closeMeteoGrid();
    loadMeteoGrid(xmlName);
}


void MainWindow::on_actionFileMeteopointProperties_import_triggered()
{
    // check meteo point
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("csv files (*.csv)"));
    if (fileName.isEmpty())
        return;

    QList<QString> pointPropertiesList;
    if (! myProject.meteoPointsDbHandler->getFieldList("point_properties", pointPropertiesList))
    {
        myProject.logError("point_properties table error");
        return;
    }

    QList<QString> csvFieldsList;
    QList<QList<QString>> csvData;
    if (! parseCSV(fileName, csvFieldsList, csvData, myProject.errorString))
    {
        myProject.logError();
        return;
    }


    DialogPointProperties dialogPointProp(pointPropertiesList, csvFieldsList);
    if (dialogPointProp.result() != QDialog::Accepted)
        return;

    QList<QString> joinedList = dialogPointProp.getJoinedList();

    if (! myProject.writeMeteoPointsProperties(joinedList, csvFieldsList, csvData, false))
    {
        myProject.logError();
        return;
    }

    loadMeteoPoints(myProject.dbPointsFileName);
}


void MainWindow::on_actionFileMeteopointData_XMLimport_triggered()
{
    // check meteo point
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open XML file"), "", tr("xml files (*.xml)"));
    if (fileName.isEmpty())
        return;

    bool isGrid = false;
    if (! myProject.parserXMLImportExportData(fileName, isGrid))
        return;

    QList<QString> dateFiles = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more files to open",
                            "",
                            "Files (*.prn *.csv *.txt)");

    if (dateFiles.isEmpty())
        return;

    myProject.setProgressBar("Loading data...", dateFiles.size());
    QString warningStr;

    for (int i=0; i < dateFiles.size(); i++)
    {
        myProject.updateProgressBar(i);

        if (myProject.loadXMLImportData(dateFiles[i]))
        {
            if (! myProject.errorString.isEmpty())
            {
                warningStr += dateFiles[i] + "\n" + myProject.errorString + "\n";
            }
        }
        else
        {
            if (i != dateFiles.size()-1)
            {
                // it is not the last
                QMessageBox msgBox;
                msgBox.setText("An error occurred: " + dateFiles[i]);
                msgBox.setInformativeText("Do you want to go on with other files?");
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                int ret = msgBox.exec();

                if (ret == QMessageBox::Ok)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
    }

    myProject.closeProgressBar();

    if (! warningStr.isEmpty())
    {
        myProject.logWarning(warningStr);
    }

    delete myProject.inOutData;
    QString dbName = myProject.meteoPointsDbHandler->getDbName();
    loadMeteoPoints(dbName);
}


void MainWindow::on_actionAll_active_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    if (!myProject.meteoPointsDbHandler->setAllPointsActive())
    {
        myProject.logError("Failed to activate all points.");
        return;
    }

    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        myProject.meteoPoints[i].active = true;
    }

    myProject.clearSelectedPoints();
    redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_actionAll_notActive_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    if (!myProject.meteoPointsDbHandler->setAllPointsNotActive())
    {
        myProject.logError("Failed to deactivate all points.");
        return;
    }

    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        myProject.meteoPoints[i].active = false;
    }

    myProject.clearSelectedPoints();
    redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_actionSelected_active_triggered()
{
    if (myProject.setActiveStateSelectedPoints(true))
        redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_actionSelected_notActive_triggered()
{
    if (myProject.setActiveStateSelectedPoints(false))
        redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_actionFrom_point_list_active_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open point list file"), "", tr("text files (*.txt)"));
    if (fileName == "") return;

    if (myProject.setActiveStatePointList(fileName, true))
        redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_actionDeletePoint_selected_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<QString> pointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (myProject.meteoPoints[i].selected)
        {
            pointList << QString::fromStdString(myProject.meteoPoints[i].id);
        }
    }
    if (pointList.isEmpty())
    {
        myProject.logError("No meteo point selected.");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Are you sure?" ,
                                  QString::number(pointList.size()) + " selected points will be deleted",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (myProject.deleteMeteoPoints(pointList))
            drawMeteoPoints();
    }
}

void MainWindow::on_actionDeletePoint_notSelected_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<QString> pointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (! myProject.meteoPoints[i].selected)
        {
            pointList << QString::fromStdString(myProject.meteoPoints[i].id);
        }
    }
    if (pointList.isEmpty())
    {
        myProject.logError("All meteo points are selected.");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Are you sure?" ,
                                  QString::number(pointList.size()) + " points will be deleted",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (myProject.deleteMeteoPoints(pointList))
            drawMeteoPoints();
    }
}

void MainWindow::on_actionDeletePoint_notActive_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<QString> pointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (!myProject.meteoPoints[i].active)
        {
            pointList << QString::fromStdString(myProject.meteoPoints[i].id);
        }
    }
    if (pointList.isEmpty())
    {
        myProject.logError("All meteo points are active");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Are you sure?",
                                  QString::number(pointList.size()) + " not active points will be deleted",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (myProject.deleteMeteoPoints(pointList))
            drawMeteoPoints();
    }
}

void MainWindow::on_actionWith_NO_DATA_notActive_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    myProject.setProgressBar("Checking points...", myProject.nrMeteoPoints);
    QList<QString> pointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        myProject.updateProgressBar(i);
        if (myProject.meteoPoints[i].active)
        {
            bool existData = myProject.meteoPointsDbHandler->existData(myProject.meteoPoints[i], daily)
                            || myProject.meteoPointsDbHandler->existData(myProject.meteoPoints[i], hourly);
            if (! existData)
            {
                pointList.append(QString::fromStdString(myProject.meteoPoints[i].id));
            }
        }
    }
    myProject.closeProgressBar();

    if (pointList.isEmpty())
    {
        myProject.logError("All active points have valid data.");
        return;
    }

    myProject.logInfoGUI("Deactive points...");
    bool isOk = myProject.meteoPointsDbHandler->setActiveStatePointList(pointList, false);
    myProject.closeLogInfo();

    if (! isOk)
    {
        myProject.logError("Failed to set to not active NODATA points");
        return;
    }

    for (int j = 0; j < pointList.size(); j++)
    {
        for (int i = 0; i < myProject.nrMeteoPoints; i++)
        {
            if (myProject.meteoPoints[i].id == pointList[j].toStdString())
            {
                myProject.meteoPoints[i].active = false;
            }
        }
    }

    redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_actionDeleteData_Active_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<QString> pointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (myProject.meteoPoints[i].active)
        {
            pointList << QString::fromStdString(myProject.meteoPoints[i].id);
        }
    }

    if (pointList.isEmpty())
    {
        myProject.logError("No meteo point is active.");
        return;
    }

    if (! myProject.deleteMeteoPointsData(pointList))
    {
        myProject.logError("Failed to delete data.");
    }

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::on_actionDeleteData_notActive_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<QString> pointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (!myProject.meteoPoints[i].active)
        {
            pointList << QString::fromStdString(myProject.meteoPoints[i].id);
        }
    }

    if (pointList.isEmpty())
    {
        myProject.logError("All meteo points are active.");
        return;
    }

    if (! myProject.deleteMeteoPointsData(pointList))
    {
        myProject.logError("Failed to delete data.");
    }

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::on_actionDeleteData_selected_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<QString> pointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (myProject.meteoPoints[i].selected)
        {
            pointList << QString::fromStdString(myProject.meteoPoints[i].id);
        }
    }

    if (pointList.isEmpty())
    {
        myProject.logError("No meteo points selected.");
        return;
    }

    if (! myProject.deleteMeteoPointsData(pointList))
    {
        myProject.logError("Failed to delete data.");
    }

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::on_actionWith_Criteria_active_triggered()
{
    if (myProject.setActiveStateWithCriteria(true))
    {
        loadMeteoPoints(myProject.dbPointsFileName);
    }
}

void MainWindow::on_actionWith_Criteria_notActive_triggered()
{
    if (myProject.setActiveStateWithCriteria(false))
    {
        loadMeteoPoints(myProject.dbPointsFileName);
    }
}

void MainWindow::on_actionView_not_active_points_toggled(bool state)
{
    viewNotActivePoints = state;
    redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_action_Proxy_graph_triggered()
{
    if (myProject.proxyWidget != nullptr)
    {
        myProject.logError("Proxy graph already open");
        return;
    }

    if (! myProject.meteoPointsLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    std::vector<Crit3DProxy> proxy = myProject.interpolationSettings.getCurrentProxy();
    if (proxy.size() == 0)
    {
        myProject.logError("No proxy loaded");
        return;
    }

    if(myProject.getCurrentFrequency() == noFrequency)
    {
        myProject.logError("Select data frequency (daily or hourly)");
        return;
    }

    return myProject.showProxyGraph(NODATA);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (myProject.proxyWidget != nullptr)
    {
        myProject.proxyWidget->close();
    }
    while (!myProject.meteoWidgetGridList.isEmpty())
    {
        myProject.meteoWidgetGridList.last()->close();
    }
    while (!myProject.meteoWidgetPointList.isEmpty())
    {
        myProject.meteoWidgetPointList.last()->close();
    }

    event->accept();
}


void MainWindow::on_actionFileMeteopointNewCsv_triggered()
{
    clearMeteoPointsMarker();

    QString templateFileName = myProject.getDefaultPath() + PATH_TEMPLATE + "template_meteo.db";
    QString path = myProject.getProjectPath() + PATH_METEOPOINT;

    QString dbName = QFileDialog::getSaveFileName(this, tr("Save as"), path, tr("DB files (*.db)"));
    if (dbName.isEmpty())
        return;

    QFile dbFile(dbName);
    if (dbFile.exists())
    {
        myProject.closeMeteoPointsDB();
        myProject.setIsElabMeteoPointsValue(false);
        dbFile.close();
        dbFile.setPermissions(QFile::ReadOther | QFile::WriteOther);
        if (! dbFile.remove())
        {
            myProject.logError("Remove file failed: " + dbName + "\n" + dbFile.errorString());
            return;
        }
    }

    if (! QFile::copy(templateFileName, dbName))
    {
        myProject.logError("Copy file failed: " + templateFileName);
        return;
    }
    myProject.meteoPointsDbHandler = new Crit3DMeteoPointsDbHandler(dbName);

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("csv files (*.csv)"));
    if (fileName.isEmpty())
        return;

    QList<QString> pointPropertiesList;
    if (! myProject.meteoPointsDbHandler->getFieldList("point_properties", pointPropertiesList))
    {
        myProject.logError("Error in reading table 'point_properties'");
        return;
    }

    QList<QString> csvFieldsList;
    QList<QList<QString>> csvData;
    if (! parseCSV(fileName, csvFieldsList, csvData, myProject.errorString))
    {
        myProject.logError();
        return;
    }

    DialogPointProperties dialogPointProp(pointPropertiesList, csvFieldsList);
    if (dialogPointProp.result() != QDialog::Accepted)
    {
        return;
    }

    QList<QString> joinedList = dialogPointProp.getJoinedList();
    if (! myProject.writeMeteoPointsProperties(joinedList, csvFieldsList, csvData, false))
    {
        myProject.logError();
        return;
    }

    loadMeteoPoints(dbName);
}


void MainWindow::on_actionNewMeteoGrid_triggered()
{
    QString xmlName = QFileDialog::getOpenFileName(this, tr("New XML DB meteo grid"), "", tr("xml files (*.xml)"));
    if (xmlName != "")
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Are you sure?" ,
                                      "A new meteo grid will be created",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            closeMeteoGrid();
            newMeteoGrid(xmlName);
        }
    }
    return;
}


void MainWindow::on_actionFileMeteogridExportRaster_triggered()
{
    if (! myProject.meteoGridLoaded || myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logWarning(ERROR_STR_MISSING_GRID);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save current data of meteo grid"), "", tr("ESRI grid files (*.flt)"));

    if (fileName != "")
    {
        int defaultCellSize = myProject.computeDefaultCellSizeFromMeteoGrid(float(0.1));
        double cellSizeValue;
        DialogCellSize cellSizeDialog(defaultCellSize);
        if (cellSizeDialog.result() == QDialog::Accepted)
        {
            cellSizeValue = cellSizeDialog.getCellSize();
        }
        else
        {
            return;
        }
        if (! myProject.exportMeteoGridToRasterFlt(fileName, cellSizeValue))
        {
            myProject.logError(myProject.errorString);
            return;
        }
    }

    return;
}


void MainWindow::on_actionFileMeteopointArkimetUpdatePointProperties_triggered()
{
    if (! myProject.meteoPointsDbHandler)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }
    QList<Crit3DMeteoPoint> listMeteoPoints;
    myProject.errorString = "";
    if (! myProject.meteoPointsDbHandler->getPropertiesFromDb(listMeteoPoints, myProject.gisSettings, myProject.errorString))
    {
        myProject.logError("Error in reading table 'point_properties'\n" + myProject.errorString);
        return;
    }

    QString dbName = myProject.meteoPointsDbHandler->getDbName();
    Download myDownload(dbName);

    Crit3DMeteoPoint pointPropFromArkimet;
    QString log = "";
    bool changes;
    bool everythingUpdated = true;
    QList<QString> column;
    QList<QString> values;
    bool update = false, updateAll = false;

    myProject.setProgressBar("Checking properties...", listMeteoPoints.size());

    for (int i=0; i<listMeteoPoints.size(); i++)
    {
        myProject.updateProgressBar(i);
        column.clear();
        values.clear();
        changes = false;
        update = false;
        pointPropFromArkimet.clear();
        QString id = QString::fromStdString(listMeteoPoints[i].id);
        if (! myDownload.getPointPropertiesFromId(id,  myProject.gisSettings.utmZone, pointPropFromArkimet)
                || pointPropFromArkimet.id == "")
        {
            log = log + "Get point properties from id error, check id: "+ id + "\n";
            continue;
        }
        if (QString::fromStdString(pointPropFromArkimet.name).toUpper() != QString::fromStdString(listMeteoPoints[i].name).toUpper())
        {
            changes = true;
            log = log + "id:"+id+","+"name,"+
                   QString::fromStdString(listMeteoPoints[i].name) + ","+ QString::fromStdString(pointPropFromArkimet.name) + "\n";
            column << "name";
            values << QString::fromStdString(pointPropFromArkimet.name);
        }
        if (pointPropFromArkimet.dataset != listMeteoPoints[i].dataset)
        {
            changes = true;
            log = log + "id:"+id+","+"dataset,"+
                   QString::fromStdString(listMeteoPoints[i].dataset) + ","+ QString::fromStdString(pointPropFromArkimet.dataset) + "\n";
            column << "dataset";

            values << QString::fromStdString(pointPropFromArkimet.dataset);
        }
        if (QString::number(pointPropFromArkimet.latitude, 'f', 6) != QString::number(listMeteoPoints[i].latitude, 'f', 6))
        {
            changes = true;
            log = log + "id:"+id+","+"latitude,"+
                   QString::number(listMeteoPoints[i].latitude, 'f', 6) + ","+ QString::number(pointPropFromArkimet.latitude, 'f', 6) + "\n";
            column << "latitude";
            values << QString::number(pointPropFromArkimet.latitude, 'f', 6);
        }
        if (QString::number(pointPropFromArkimet.longitude, 'f', 6) != QString::number(listMeteoPoints[i].longitude, 'f', 6))
        {
            changes = true;
            log = log + "id:"+id+","+"longitude,"+
                   QString::number(listMeteoPoints[i].longitude, 'f', 6) + ","+ QString::number(pointPropFromArkimet.longitude, 'f', 6) + "\n";
            column << "longitude";
            values << QString::number(pointPropFromArkimet.longitude, 'f', 6);
        }
        if (QString::number(pointPropFromArkimet.point.utm.x) != QString::number(listMeteoPoints[i].point.utm.x))
        {
            changes = true;
            log = log + "id:"+id+","+"utm_x,"+
                   QString::number(listMeteoPoints[i].point.utm.x) + ","+ QString::number(pointPropFromArkimet.point.utm.x) + "\n";
            column << "utm_x";
            values << QString::number(pointPropFromArkimet.point.utm.x);
        }
        if (QString::number(pointPropFromArkimet.point.utm.y) != QString::number(listMeteoPoints[i].point.utm.y))
        {
            changes = true;
            log = log + "id:"+id+","+"utm_y,"+
                   QString::number(listMeteoPoints[i].point.utm.y) + ","+ QString::number(pointPropFromArkimet.point.utm.y) + "\n";
            column << "utm_y";
            values << QString::number(pointPropFromArkimet.point.utm.y);
        }
        if (pointPropFromArkimet.point.z != listMeteoPoints[i].point.z)
        {
            changes = true;
            log = log + "id:"+id+","+"altitude,"+
                   QString::number(listMeteoPoints[i].point.z) + ","+ QString::number(pointPropFromArkimet.point.z) + "\n";
            column << "altitude";
            values << QString::number(pointPropFromArkimet.point.z);
        }
        if (pointPropFromArkimet.state != listMeteoPoints[i].state)
        {
            changes = true;
            log = log + "id:"+id+","+"state,"+
                   QString::fromStdString(listMeteoPoints[i].state) + ","+ QString::fromStdString(pointPropFromArkimet.state) + "\n";
            column << "state";
            values << QString::fromStdString(pointPropFromArkimet.state);
        }
        if (pointPropFromArkimet.region != listMeteoPoints[i].region)
        {
            changes = true;
            log = log + "id:"+id+","+"region,"+
                   QString::fromStdString(listMeteoPoints[i].region) + ","+ QString::fromStdString(pointPropFromArkimet.region) + "\n";
            column << "region";
            values << QString::fromStdString(pointPropFromArkimet.region);
        }
        if (pointPropFromArkimet.province != listMeteoPoints[i].province)
        {
            changes = true;
            log = log + "id:"+id+","+"province,"+
                   QString::fromStdString(listMeteoPoints[i].province) + ","+ QString::fromStdString(pointPropFromArkimet.province) + "\n";
            column << "province";
            values << QString::fromStdString(pointPropFromArkimet.province);
        }
        if (pointPropFromArkimet.municipality != listMeteoPoints[i].municipality)
        {
            changes = true;
            log = log + "id:"+id+","+"municipality,"+
                   QString::fromStdString(listMeteoPoints[i].municipality) + ","+ QString::fromStdString(pointPropFromArkimet.municipality) + "\n";
            column << "municipality";
            values << QString::fromStdString(pointPropFromArkimet.municipality);
        }

        if (changes)
        {
            everythingUpdated = false;

            if (! updateAll)
            {
                QMessageBox::StandardButton reply;

                reply = QMessageBox::question(this, "Update point properties?",
                                          "Id:" + id + " point properties from arkimet are different. "
"                                           \n Columns: " + column.join(","), QMessageBox::Yes|QMessageBox::No|QMessageBox::YesToAll|QMessageBox::Cancel);

                if (reply == QMessageBox::YesToAll)
                {
                    updateAll = true;
                    update = true;
                }
                else if (reply == QMessageBox::Yes)
                {
                    update = true;
                }
                else if (reply == QMessageBox::Cancel)
                    break;
            }
            else
                update = true;

        }

        if (update)
        {
            if (!myProject.meteoPointsDbHandler->updatePointPropertiesGivenId(id, column, values))
            {
                myProject.logError("Update point properties given id error "+id);
                return;
            }
        }

    }

    myProject.closeProgressBar();

    if (everythingUpdated)
    {
        if (log == "")
        {
            myProject.logWarning("Everything already updated");
            return;
        }
        else
        {
            log += "\nAll other stations are already updated";
            myProject.logInfo(log);
        }
    }
    else
    {
        myProject.logInfo(log);
        myProject.logInfoGUI("Update...");
        myProject.closeMeteoPointsDB();
        myProject.loadMeteoPointsDB(dbName);
        drawMeteoPoints();
        myProject.closeLogInfo();
        return;
    }
}


void MainWindow::on_actionFileMeteopointArkimetUpdateMeteopoints_triggered()
{
    if (! myProject.meteoPointsDbHandler)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QString dbName = myProject.meteoPointsDbHandler->getDbName();
    QList<QString> datasetsList = myProject.meteoPointsDbHandler->getDatasetsActive();
    DialogSelectDataset selectDialog(datasetsList);
    if (selectDialog.result() == QDialog::Accepted)
    {
        QList<QString> datasetSelected = selectDialog.getSelectedDatasets();
        QList<QString> idListFromDB = myProject.meteoPointsDbHandler->getIdListGivenDataset(datasetSelected);

        Download myDownload(dbName);
        QMap<QString,QString> mapFromArkimet = myDownload.getArmiketIdList(datasetSelected);
        QList<QString> idListFromArkimet = mapFromArkimet.keys();
        QList<QString> idMissingInArkimet = removeList(idListFromDB,idListFromArkimet);

        if (!idMissingInArkimet.isEmpty())
        {
            QString log = "Id stations founded into db and missing in arkimet: ";
            foreach (const QString &idName, idMissingInArkimet)
            {
                log = log + idName + " ";
            }
            myProject.logInfo(log);
        }
        QList<QString> idMissingDb = removeList(idListFromArkimet,idListFromDB);
        if (!idMissingDb.isEmpty())
        {
            QList<QString> idMissing;
            QList<QString> nameMissing;
            foreach (const QString &idName, idMissingDb)
            {
                idMissing.append(idName);
                nameMissing.append(mapFromArkimet.value(idName));
            }

            DialogAddMissingStation addStationDialog(idMissing, nameMissing);
            if (addStationDialog.result() == QDialog::Accepted)
            {
                QList<QString> stationsSelected = addStationDialog.getSelectedStations();
                Crit3DMeteoPoint pointPropFromArkimet;
                QList<QString> column;
                QList<QString> values;
                for (int i=0; i<stationsSelected.size(); i++)
                {
                    column.clear();
                    values.clear();
                    pointPropFromArkimet.clear();
                    if (! myDownload.getPointPropertiesFromId(stationsSelected[i], myProject.gisSettings.utmZone, pointPropFromArkimet))
                    {
                        myProject.logError("Get point properties from id error");
                        return;
                    }
                    if (! myProject.meteoPointsDbHandler->writePointProperties(pointPropFromArkimet))
                    {
                        myProject.logError("Write point properties error");
                        return;
                    }
                }
                myProject.logInfoGUI("Update...");
                myProject.closeMeteoPointsDB();
                myProject.loadMeteoPointsDB(dbName);
                drawMeteoPoints();
                myProject.closeLogInfo();
            }
        }
        else
        {
            myProject.logWarning("No stations to add");
        }
    }
    return;
}

void MainWindow::on_actionFileMeteopointArkimetUpdateDatasets_triggered()
{
    if (! myProject.meteoPointsDbHandler)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }
    QString dbName = myProject.meteoPointsDbHandler->getDbName();
    QList<QString> allDatasetsList = myProject.meteoPointsDbHandler->getAllDatasetsList();
    QList<QString> dbDatasetsList = myProject.meteoPointsDbHandler->getDatasetList();
    QList<QString> datasetAvailable = removeList(allDatasetsList,dbDatasetsList);
    DialogAddRemoveDataset addDatasetDialog(datasetAvailable, dbDatasetsList);
    if (addDatasetDialog.result() == QDialog::Accepted)
    {
        QList<QString> datasetSelected = addDatasetDialog.getDatasetDb();
        QList<QString> datasetToDelete = removeList(dbDatasetsList, datasetSelected);
        QList<QString> datasetToAdd = removeList(datasetSelected, dbDatasetsList);
        if (! datasetToDelete.isEmpty())
        {
            if (! myProject.meteoPointsDbHandler->deleteAllPointsFromDataset(datasetToDelete))
            {
                myProject.logError("Delete all points error");
                return;
            }
        }
        if (! datasetToAdd.isEmpty())
        {
            Download myDownload(dbName);
            if (! myDownload.getPointProperties(datasetToAdd, myProject.gisSettings.utmZone, myProject.errorString))
            {
                myProject.logError("Error in function getPointProperties: " + myProject.errorString);
                return;
            }
        }

        myProject.closeMeteoPointsDB();
        myProject.loadMeteoPointsDB(dbName);
        drawMeteoPoints();
    }

    return;
}


void MainWindow::on_actionPointStyleCircle_triggered()
{
    ui->actionPointStyleCircle->setChecked(true);
    ui->actionPointStyleText->setChecked(false);
    ui->actionPointStyleText_multicolor->setChecked(false);

    for (int i = 0; i < pointList.size(); i++)
    {
        pointList[i]->showText(false);
        pointList[i]->setMultiColorText(false);
    }
}


void MainWindow::on_actionPointStyleText_triggered()
{
    ui->actionPointStyleCircle->setChecked(false);
    ui->actionPointStyleText->setChecked(true);
    ui->actionPointStyleText_multicolor->setChecked(false);

    for (int i = 0; i < pointList.size(); i++)
    {
        pointList[i]->showText(true);
        pointList[i]->setMultiColorText(false);
    }
}


void MainWindow::on_actionPointStyleText_multicolor_triggered()
{
    ui->actionPointStyleCircle->setChecked(false);
    ui->actionPointStyleText->setChecked(false);
    ui->actionPointStyleText_multicolor->setChecked(true);

    for (int i = 0; i < pointList.size(); i++)
    {
        pointList[i]->setMultiColorText(true);
    }
}


void MainWindow::on_actionUnmark_all_points_triggered()
{
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        myProject.meteoPoints[i].marked = false;
    }

    redrawMeteoPoints(currentPointsVisualization, true);
}



void MainWindow::on_actionExport_MeteoPoints_toCsv_triggered()
{
    QString csvFileName = QFileDialog::getSaveFileName(this, tr("Save current data"), "", tr("csv files (*.csv)"));

    if (csvFileName != "")
    {
        QFile myFile(csvFileName);
        if (! myFile.open(QIODevice::WriteOnly | QFile::Truncate))
        {
            myProject.logError("Open CSV failed: " + csvFileName + "\n ");
            return;
        }

        QTextStream myStream (&myFile);
        myStream.setRealNumberNotation(QTextStream::FixedNotation);
        myStream.setRealNumberPrecision(1);
        QString header = "id,name,dataset,state,region,province,municipality,lapse_rate_code,lat,lon,utmx,utmy,altitude,value";
        myStream << header << "\n";
        for (int i = 0; i < myProject.nrMeteoPoints; i++)
        {
            if (! isEqual(myProject.meteoPoints[i].currentValue, NODATA))
            {
                myStream << QString::fromStdString(myProject.meteoPoints[i].id)
                         << "," << QString::fromStdString(myProject.meteoPoints[i].name)
                         << "," << QString::fromStdString(myProject.meteoPoints[i].dataset)
                         << "," << QString::fromStdString(myProject.meteoPoints[i].state)
                         << "," << QString::fromStdString(myProject.meteoPoints[i].region)
                         << "," << QString::fromStdString(myProject.meteoPoints[i].province)
                         << "," << QString::fromStdString(myProject.meteoPoints[i].municipality)
                         << "," << QString::number(myProject.meteoPoints[i].lapseRateCode)
                         << "," << QString::number(myProject.meteoPoints[i].latitude)
                         << "," << QString::number(myProject.meteoPoints[i].longitude)
                         << "," << QString::number(myProject.meteoPoints[i].point.utm.x)
                         << "," << QString::number(myProject.meteoPoints[i].point.utm.y)
                         << "," << QString::number(myProject.meteoPoints[i].point.z)
                         << "," << QString::number(myProject.meteoPoints[i].currentValue) << "\n";
            }
        }
        myFile.close();

        return;
    }
}


void MainWindow::on_actionFileLoadInterpolation_triggered()
{
    meteoVariable myVar = myProject.getCurrentVariable();
    if (myVar == noMeteoVar)
    {
        myProject.logWarning("Choose variable before.");
        return;
    }

    QString defaultPath = myProject.getProjectPath() + PATH_OUTPUT;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load interpolated raster"), defaultPath, tr("ESRI grid files (*.flt)"));

    // load raster
    if (! fileName.isEmpty())
    {
        QString fileWithoutExtension = QFileInfo(fileName).absolutePath() + QDir::separator() + QFileInfo(fileName).baseName();

        std::string myError = "";
        if (! gis::readEsriGrid(fileWithoutExtension.toStdString(), &myProject.dataRaster, myError))
        {
            myProject.logError(QString::fromStdString(myError));
        }
    }

    if (myVar == elaboration)
        setColorScale(myProject.clima->variable(), myProject.dataRaster.colorScale);
    else
        setColorScale(myVar, myProject.dataRaster.colorScale);

    setCurrentRaster(&(myProject.dataRaster));
    ui->labelRasterScale->setText(QString::fromStdString(getVariableString(myVar)));
    updateMaps();
}


void MainWindow::on_actionFileExportInterpolation_triggered()
{
    if (! myProject.dataRaster.isLoaded)
    {
        myProject.logWarning("No data to save.");
        return;
    }

    QString defaultPath = myProject.getProjectPath() + PATH_OUTPUT;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save interpolated raster"), defaultPath, tr("ESRI grid files (*.flt)"));

    if (! fileName.isEmpty())
    {
        QString fileWithoutExtension = QFileInfo(fileName).absolutePath() + QDir::separator() + QFileInfo(fileName).baseName();
        std::string myError = "";

        if (!gis::writeEsriGrid(fileWithoutExtension.toStdString(), &myProject.dataRaster, myError))
            myProject.logError(QString::fromStdString(myError));
    }
}


void MainWindow::on_actionFileDemOpen_triggered()
{
    QString defaultPath = myProject.getProjectPath() + PATH_DEM;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Digital Elevation Model"), defaultPath,
                                                    tr("ESRI FLT (*.flt);;ENVI IMG (*.img)"));
    if (fileName.isEmpty())
        return;

    rasterObj->setDrawing(false);
    bool isOk = myProject.loadDEM(fileName);
    rasterObj->setDrawing(true);

    if (isOk)
        renderDEM();
}


void MainWindow::on_actionMark_from_pointlist_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open point list"), "", tr("text files (*.txt)"));
    if (fileName == "") return;

    if (myProject.setMarkedFromPointList(fileName))
        redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::on_actionFileMeteogridDelete_triggered()
{
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logWarning(ERROR_STR_MISSING_GRID);
        return;
    }
    QMessageBox::StandardButton reply;
    QString dbName = myProject.meteoGridDbHandler->db().databaseName();
    reply = QMessageBox::question(this, "WARNING" ,
                                  "Meteo grid " + dbName + " will be deleted! Are you sure?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (myProject.deleteMeteoGridDB())
        {
            closeMeteoGrid();
        }
    }
}


void MainWindow::on_actioFileMeteogrid_Load_current_data_triggered()
{
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logWarning(ERROR_STR_MISSING_GRID);
        return;
    }

    QDate date = myProject.getCurrentDate();
    myProject.loadMeteoGridData(date, date, true);

    redrawMeteoGrid(currentGridVisualization, false);
}


bool MainWindow::checkMeteoGridColorScale()
{
    if (! this->meteoGridObj->isLoaded)
    {
        myProject.logWarning(ERROR_STR_MISSING_GRID);
        return false;
    }

    if (this->currentGridVisualization == notShown || this->currentGridVisualization == showLocation)
    {
        myProject.logWarning("Show variable or elaboration data before.");
        return false;
    }

    return true;
}


void MainWindow::on_actionMeteoGrid_Set_color_scale_triggered()
{
    if (! checkMeteoGridColorScale()) return;

    // choose color scale
    meteoVariable myVar = chooseColorScale();
    if (myVar != noMeteoVar)
    {
        setColorScale(myVar, this->meteoGridObj->getRasterPointer()->colorScale);
    }

    emit this->meteoGridObj->redrawRequested();
}


void MainWindow::on_actionMeteoGrid_Reverse_color_scale_triggered()
{
    if (! checkMeteoGridColorScale()) return;

    reverseColorScale(this->meteoGridObj->getRasterPointer()->colorScale);
    emit this->meteoGridObj->redrawRequested();
}


void MainWindow::setColorScaleRangeMeteoGrid(bool isFixed)
{
    if (! checkMeteoGridColorScale()) return;

    if (isFixed)
    {
        // choose minimum
        float minimum = this->meteoGridObj->getRasterPointer()->colorScale->minimum();
        QString valueStr = editValue("Choose minimum value", QString::number(minimum));
        if (valueStr == "") return;
        minimum = valueStr.toFloat();

        // choose maximum
        float maximum = this->meteoGridObj->getRasterPointer()->colorScale->maximum();
        valueStr = editValue("Choose maximum value", QString::number(maximum));
        if (valueStr == "") return;
        maximum = valueStr.toFloat();

        // set range
        this->meteoGridObj->getRasterPointer()->colorScale->setRange(minimum, maximum);
        this->meteoGridObj->getRasterPointer()->colorScale->setFixedRange(true);
    }
    else
    {
        this->meteoGridObj->getRasterPointer()->colorScale->setFixedRange(false);
    }

    emit this->meteoGridObj->redrawRequested();
}


bool MainWindow::checkDEMColorScale()
{
    if (! myProject.DEM.isLoaded)
    {
        myProject.logWarning(ERROR_STR_MISSING_DEM);
        return false;
    }

    return true;
}


void MainWindow::setColorScaleRangeDEM(bool isFixed)
{
    if (! checkDEMColorScale()) return;

    if (isFixed)
    {
        // choose minimum
        float minimum = this->rasterObj->getRasterPointer()->colorScale->minimum();
        QString valueStr = editValue("Choose minimum value", QString::number(minimum));
        if (valueStr == "") return;
        minimum = valueStr.toFloat();

        // choose maximum
        float maximum = this->rasterObj->getRasterPointer()->colorScale->maximum();
        valueStr = editValue("Choose maximum value", QString::number(maximum));
        if (valueStr == "") return;
        maximum = valueStr.toFloat();

        // set range
        this->rasterObj->getRasterPointer()->colorScale->setRange(minimum, maximum);
        this->rasterObj->getRasterPointer()->colorScale->setFixedRange(true);
    }
    else
    {
        this->rasterObj->getRasterPointer()->colorScale->setFixedRange(false);
    }

    emit this->rasterObj->redrawRequested();
}


void MainWindow::on_flagMeteoGrid_Dynamic_color_scale_triggered(bool isChecked)
{
    ui->flagMeteoGrid_Fixed_color_scale->setChecked(! isChecked);
    setColorScaleRangeMeteoGrid(! isChecked);
}


void MainWindow::on_flagMeteoGrid_Fixed_color_scale_triggered(bool isChecked)
{
    ui->flagMeteoGrid_Dynamic_color_scale->setChecked(! isChecked);
    setColorScaleRangeMeteoGrid(isChecked);
}


void MainWindow::on_actionShiftDataAll_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    bool isAllPoints = true;
    if (! myProject.shiftMeteoPointsData(isAllPoints))
        return;

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::on_actionShiftDataSelected_triggered()
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    bool isAllPoints = false;
    if (! myProject.shiftMeteoPointsData(isAllPoints))
        return;

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::on_actionMeteoGridActiveAll_triggered()
{
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Are you sure?" ,
                                  "All cells of the meteo grid will be activated",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (! myProject.meteoGridDbHandler->activeAllCells(myProject.errorString))
        {
            myProject.logError("Unable to activate all cells: " + myProject.errorString);
            return;
        }
        QString xmlName = myProject.dbGridXMLFileName;
        closeMeteoGrid();
        loadMeteoGrid(xmlName);
    }
}


void MainWindow::on_actionMeteoGridActiveWith_DEM_triggered()
{
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }

    if (!myProject.DEM.isLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_DEM);
        return;
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Are you sure?" ,
                                  "All meteo grid cells inside actuale DEM will be activated",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (!myProject.activeMeteoGridCellsWithDEM())
        {
            myProject.logError("Failed to active cells inside actuale DEM "+myProject.errorString);
            return;
        }
        QString xmlName = myProject.dbGridXMLFileName;
        closeMeteoGrid();
        loadMeteoGrid(xmlName);
    }
}


void MainWindow::on_actionInterpolationMeteogridGriddingTaskAdd_triggered()
{
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }

    QDate myFirstDate = myProject.meteoGridDbHandler->firstDate();
    QDate myLastDate = myProject.meteoGridDbHandler->lastDate();
    if (myLastDate.isNull()) myLastDate = myProject.getCurrentDate();
    if (myFirstDate.isNull()) myFirstDate = myLastDate;
    QDateTime myFirstTime(myFirstDate, QTime(1,0,0), Qt::UTC);
    QDateTime myLastTime(myLastDate.addDays(1), QTime(0,0,0), Qt::UTC);

    FormTimePeriod myForm(&myFirstTime, &myLastTime);
    myForm.show();
    if (myForm.exec() == QDialog::Rejected) return;

    QString user, notes;
    FormSelection selectUser(myProject.users, "Select user");
    if (selectUser.result() == QDialog::Rejected) return;
    user = selectUser.getSelection();

    FormText formNotes("Insert notes", "");
    if (formNotes.result() == QDialog::Rejected) return;
    notes = formNotes.getText();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "The following information will be saved for gridding. Proceed?\n" ,
                                  "praga_user: " + user + "\n" +
                                  "date_creation: " + QDateTime::currentDateTimeUtc().toString() + "\n" +
                                  "date_start: " + myFirstTime.date().toString() + "\n" +
                                  "date_end: " + myLastTime.date().toString() + "\n" +
                                  "notes: " + notes,
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
        if (! myProject.planGriddingTask(myFirstTime.date(), myLastTime.date(), user, notes))
            myProject.logError("Failed to write planning info... " + myProject.errorString);
}


void MainWindow::on_actionInterpolationMeteogridGriddingTaskRemove_triggered()
{
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }


    std::vector <QString> users, notes;
    std::vector <QDate> dateStart, dateEnd;
    std::vector <QDateTime> dateCreation;
    QList<QString> taskList;

    if (! myProject.getGriddingTasks(dateCreation, dateStart, dateEnd, users, notes)) return;

    for (unsigned int i=0; i < dateCreation.size(); i++)
        taskList.push_back("Created " + dateCreation[i].toString() + " by " + users[i]
                           + ", from " + dateStart[i].toString() + " to " + dateEnd[i].toString()
                           + " notes: " + notes[i]);

    if (taskList.size() == 0)
    {
        myProject.logWarning("No gridding task in DB Grid");
        return;
    }

    FormSelection selectTask(taskList, "Select task");
    if (selectTask.result() == QDialog::Rejected) return;
    int taskId = selectTask.getSelectionId();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "The following task will be removed. Proceed?\n" ,
                                  "praga_user: " + users[taskId] + "\n" +
                                  "date_creation: " + dateCreation[taskId].toString() + "\n" +
                                  "date_start: " + dateStart[taskId].toString() + "\n" +
                                  "date_end: " + dateEnd[taskId].toString() + "\n" +
                                  "notes: " + notes[taskId],
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
        if (! myProject.removeGriddingTask(dateCreation[taskId], users[taskId], dateStart[taskId], dateEnd[taskId]))
            myProject.logError("Failed to remove planning task... " + myProject.errorString);
}


void MainWindow::on_actionDemRestore_triggered()
{
    if (! myProject.DEM.isLoaded)
    {
        myProject.logWarning(ERROR_STR_MISSING_DEM);
        return;
    }

    setCurrentRaster(&(myProject.DEM));
    ui->labelRasterScale->setText(QString::fromStdString(getVariableString(noMeteoTerrain)));
    updateMaps();
}


void MainWindow::on_actionDemZoom_to_layer_triggered()
{
    if (! myProject.DEM.isLoaded)
    {
        myProject.logWarning(ERROR_STR_MISSING_DEM);
        return;
    }

    zoomToDEM();
}


void MainWindow::on_actionSearchPointName_triggered()
{
    bool isName = true;
    searchMeteoPoint(isName);
}


void MainWindow::on_actionSearchPointId_triggered()
{
    bool isName = false;
    searchMeteoPoint(isName);
}


void MainWindow::searchMeteoPoint(bool isName)
{
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QString title = "Search ";
    if (isName)
        title += "by name";
    else
        title += "by id";

    FormText formSearch(title, "");
    if (formSearch.result() == QDialog::Rejected)
        return;

    QString searchString = formSearch.getText();
    if (searchString == "")
        return;

    // initialize
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        myProject.meteoPoints[i].marked = false;
    }

    // mark
    int nrFound = 0;
    QString refString = "";
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (isName)
        {
            refString = QString::fromStdString(myProject.meteoPoints[i].name);
        }
        else
        {
            refString = QString::fromStdString(myProject.meteoPoints[i].id);
        }

        if (refString.contains(searchString, Qt::CaseInsensitive))
        {
            myProject.meteoPoints[i].marked = true;
            nrFound++;
        }
    }

    redrawMeteoPoints(currentPointsVisualization, true);
    if (nrFound == 0)
    {
        myProject.logError("No meteo points found with: " + searchString);
    }
}


void MainWindow::on_actionShow_InfoProject_triggered()
{
    QDialog myDialog;
    myDialog.setWindowTitle("Project info");

    QTextBrowser textBrowser;

    if (myProject.projectName != "")
    {
        textBrowser.setText(QString("Project: " + myProject.projectName));
        if (myProject.logFileName != "")
        {
            textBrowser.append(QString("File Log: " + myProject.getCompleteFileName(myProject.logFileName, PATH_LOG)));
        }
        textBrowser.append(QString("Parameters: " + myProject.getCompleteFileName(myProject.parametersFileName, PATH_SETTINGS)));
    }
    else
    {
        if (myProject.logFileName != "")
        {
            textBrowser.setText(QString("File Log: " + myProject.logFileName));
            textBrowser.append(QString("Parameters: " + myProject.getCompleteFileName(myProject.parametersFileName, PATH_LOG)));
        }
        else
        {
            textBrowser.setText(QString("Parameters: " + myProject.getCompleteFileName(myProject.parametersFileName, PATH_SETTINGS)));
        }
    }

    if (myProject.meteoPointsLoaded)
    {
       textBrowser.append(QString("MeteoPoints DB: " + myProject.getCompleteFileName(myProject.dbPointsFileName, PATH_METEOPOINT)));
    }
    else
    {
        textBrowser.append(QString("MeteoPoints Db: No meteo points loaded"));
    }

    if (myProject.DEM.isLoaded)
    {
        textBrowser.append(QString("Digital Elevation Model: " + myProject.getCompleteFileName(myProject.demFileName, PATH_DEM)));
    }
    else
    {
        textBrowser.append(QString("Digital Elevation Model: No DEM loaded"));
    }

    if (myProject.meteoGridLoaded && myProject.dbGridXMLFileName != "") // Questo controlla che il grid esista, per evitare che si blocchi Praga coi commandi successivi.
    {
        textBrowser.append(QString("MeteoGrid XML: ") + QString(myProject.dbGridXMLFileName));
        textBrowser.append(QString("MeteoGrid name: " + myProject.meteoGridDbHandler->connection().name));
        textBrowser.append(QString("MeteoGrid server: " + myProject.meteoGridDbHandler->connection().server));
    }
    else
    {
        textBrowser.append(QString("MeteoGrid XML: No grid loaded"));
    }


    QVBoxLayout mainLayout;
    mainLayout.addWidget(&textBrowser);

    myDialog.setLayout(&mainLayout);
    myDialog.setFixedSize(700,300);
    myDialog.exec();
}


void MainWindow::on_actionCompute_monthly_data_from_daily_triggered()
{
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }

    bool allPoints = true;
    bool isGrid = true;
    QDate myDateFrom = myProject.meteoGridDbHandler->firstDate();
    QDate myDateTo = myProject.meteoGridDbHandler->lastDate();

    DialogComputeData computeMonthlyDialog(myDateFrom, myDateTo, isGrid, allPoints);
    if (computeMonthlyDialog.result() != QDialog::Accepted)
        return;

    QList <meteoVariable> varToCompute = computeMonthlyDialog.getVariables();
    QDate firstDate = computeMonthlyDialog.getDateFrom();
    QDate lastDate = computeMonthlyDialog.getDateTo();
    if (myProject.meteoGridDbHandler->getFirstMonthlytDate().isValid() && myProject.meteoGridDbHandler->getLastMonthlyDate().isValid())
    {
        if (firstDate >= myProject.meteoGridDbHandler->getFirstMonthlytDate() || lastDate <= myProject.meteoGridDbHandler->getLastMonthlyDate())
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Are you sure?" ,
                                          " monthly data will be overwritten ",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
            {
                return;
            }
        }
    }

    bool isShowInfo = true;
    if (! myProject.monthlyAggregateVariablesGrid(firstDate, lastDate, varToCompute, isShowInfo))
    {
        myProject.logError("Failed to compute monthly data.\n" + myProject.errorString);
    }

    QDate date = myProject.getCurrentDate();
    myProject.loadMeteoGridData(date, date, true);

    redrawMeteoGrid(currentGridVisualization, false);
}


void MainWindow::on_actionCompute_daily_from_Hourly_all_triggered()
{
    if (! myProject.meteoPointsLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<std::string> idPointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        idPointList << myProject.meteoPoints[i].id;
    }

    computeDailyFromHourly_MeteoPoints(idPointList);
}


void MainWindow::on_actionCompute_daily_from_Hourly_selected_triggered()
{
    if (! myProject.meteoPointsLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<std::string> idPointList;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (myProject.meteoPoints[i].selected)
        {
            idPointList << myProject.meteoPoints[i].id;
        }
    }

    if (idPointList.isEmpty())
    {
        myProject.logError("No meteo point selected.");
        return;
    }

    computeDailyFromHourly_MeteoPoints(idPointList);
}


void MainWindow::computeDailyFromHourly_MeteoPoints(const QList<std::string>& idPointList)
{
    if (idPointList.size() == 0) return;

    bool isGrid = false;
    bool allPoints = (idPointList.size() == myProject.nrMeteoPoints);

    QDate myDateFrom = myProject.meteoPointsDbHandler->getFirstDate(hourly).date();
    QDate myDateTo = myProject.meteoPointsDbHandler->getLastDate(hourly).date();

    DialogComputeData computeDailyDialog(myDateFrom, myDateTo, isGrid, allPoints);
    if (computeDailyDialog.result() != QDialog::Accepted)
    {
        return;
    }

    QList <meteoVariable> varToCompute = computeDailyDialog.getVariables();
    QDate firstDate = computeDailyDialog.getDateFrom();
    QDate lastDate = computeDailyDialog.getDateTo();

    if (myProject.meteoPointsDbHandler->getFirstDate(daily).date().isValid() && myProject.meteoPointsDbHandler->getLastDate(daily).date().isValid())
    {
        if (firstDate >= myProject.meteoPointsDbHandler->getFirstDate(daily).date() || lastDate <= myProject.meteoPointsDbHandler->getLastDate(daily).date())
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Are you sure?" ,
                                          " daily data of " + QString::number(idPointList.size()) + " points will be overwritten\n"
                                              + "From: " + firstDate.toString("yyyy-MM-dd") + " to: " + lastDate.toString("yyyy-MM-dd"),
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
            {
                return;
            }
        }
    }

    QList <QString> idErrorList;
    for (int i = 0; i < idPointList.size(); i++)
    {
        Crit3DMeteoPoint meteoPoint;
        meteoPoint.setId(idPointList[i]);
        myProject.meteoPointsDbHandler->loadHourlyData(getCrit3DDate(firstDate), getCrit3DDate(lastDate), meteoPoint);

        if (! myProject.computeDailyVariablesPoint(&meteoPoint, firstDate, lastDate, varToCompute))
        {
            idErrorList.append(QString::fromStdString(idPointList[i]));
        }

        meteoPoint.clear();
    }

    if (idErrorList.size() > 0)
    {
        myProject.logError("Failed to compute daily data for id: " + idErrorList.join(","));
    }

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::on_actionClimateMeteoPoints_triggered()
{
    bool isMeteoGrid = false;
    if (!myProject.meteoPointsLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QList<QString> climateDbElab;
    QList<QString> climateDbVarList;
    myProject.clima->resetListElab();
    if (myProject.showClimateFields(isMeteoGrid, &climateDbElab, &climateDbVarList))
    {
        DialogClimateFields climateDialog(climateDbElab, climateDbVarList);
        if (climateDialog.result() == QDialog::Accepted)
        {
            QString climaSelected = climateDialog.getSelected();

            if (climateDialog.getIsShowClicked())
            {
                QString index = climateDialog.getIndexSelected();
                myProject.climateIndex = index;
                myProject.readClimate(isMeteoGrid, climaSelected, index.toInt(), true);
                if (isMeteoGrid)
                {
                    this->ui->actionShowGridClimate->setEnabled(true);
                    redrawMeteoGrid(showClimate, false);
                }
                else
                {
                    this->ui->actionShowPointsClimate->setEnabled(true);
                    redrawMeteoPoints(showClimate, true);
                }
            }
            else
            {
                myProject.deleteClimate(isMeteoGrid, climaSelected);
            }

        }
        else
        {
            return;
        }

    }
    return;
}


void MainWindow::on_actionClimateMeteoGrid_triggered()
{
    bool isMeteoGrid = true;
    if (!myProject.meteoGridLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }

    QList<QString> climateDbElab;
    QList<QString> climateDbVarList;
    myProject.clima->resetListElab();
    if (myProject.showClimateFields(isMeteoGrid, &climateDbElab, &climateDbVarList))
    {
        DialogClimateFields climateDialog(climateDbElab, climateDbVarList);
        if (climateDialog.result() == QDialog::Accepted)
        {
            QString climaSelected = climateDialog.getSelected();

            if (climateDialog.getIsShowClicked())
            {
                QString index = climateDialog.getIndexSelected();
                myProject.climateIndex = index;
                myProject.readClimate(isMeteoGrid, climaSelected, index.toInt(), true);
                if (isMeteoGrid)
                {
                    this->ui->actionShowGridClimate->setEnabled(true);
                    redrawMeteoGrid(showClimate, false);
                }
                else
                {
                    this->ui->actionShowPointsClimate->setEnabled(true);
                    redrawMeteoPoints(showClimate, true);
                }
            }
            else
            {
                myProject.deleteClimate(isMeteoGrid, climaSelected);
            }
        }
        else return;
    }
}


void MainWindow::on_actionStatistical_Summary_triggered()
{
    if (! myProject.meteoPointsLoaded && ! myProject.meteoGridLoaded && ! myProject.dataRaster.isLoaded)
    {
        myProject.logWarning(ERROR_STR_MISSING_POINT_GRID);
        return;
    }

    FormSelectionSource inputForm(myProject.meteoPointsLoaded, myProject.meteoGridLoaded, myProject.dataRaster.isLoaded);

    if (inputForm.result() != QDialog::Accepted)
        return;
    int inputId = inputForm.getSourceSelectionId();
    inputForm.close();

    std::string idMin, idMax, nameMin, nameMax, errorStdStr;
    std::vector <float> validValues;
    int nrValidCells = NODATA;
    float avgValue = NODATA;
    float area = NODATA;

    switch(inputId)
    {
        case 1:     // point
        {
            if (myProject.meteoPointsLoaded && currentPointsVisualization != notShown)
            {
                validValues.clear();
                myProject.getMeteoPointsCurrentValues(validValues);

                if (validValues.size() != 0)
                {
                    float minValue = statistics::minList(validValues, int(validValues.size()));
                    float maxValue = statistics::maxList(validValues, int(validValues.size()));
                    for (int i = 0; i < myProject.nrMeteoPoints; i++)
                    {
                        if (minValue == myProject.meteoPoints[i].currentValue)
                        {
                           idMin = myProject.meteoPoints[i].id;
                           nameMin = myProject.meteoPoints[i].name;
                        }
                        if (maxValue == myProject.meteoPoints[i].currentValue)
                        {
                           idMax = myProject.meteoPoints[i].id;
                           nameMax = myProject.meteoPoints[i].name;
                        }
                    }
                }
                else
                {
                        myProject.errorString = "No valid value.";
                        myProject.logError();
                        return;
                }
            }
            else
            {
                    myProject.errorString = "No MeteoPoints loaded.";
                    myProject.logError();
                    return;
             }
             break;
        }

        case 2:     //grid
        {
            if (meteoGridObj->isLoaded && currentGridVisualization != notShown)
            {
                for (int row = 0; row < myProject.meteoGridDbHandler->gridStructure().header().nrRows; row++)
                    for (int col = 0; col < myProject.meteoGridDbHandler->gridStructure().header().nrCols; col++)
                    {
                        if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->active)
                        {
                            switch(currentGridVisualization)
                            {
                                case showCurrentVariable:
                                {
                                    if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->currentValue != NODATA)
                                    {
                                        validValues.push_back(myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->currentValue);
                                    }
                                    break;
                                }
                                case showElaboration:
                                {
                                    if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->elaboration != NODATA)
                                    {
                                        validValues.push_back(myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->elaboration);
                                    }
                                    break;
                                }
                                case showAnomalyAbsolute:
                                {
                                    if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->anomaly != NODATA)
                                    {
                                        validValues.push_back(myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->anomaly);
                                    }
                                    break;
                                }
                                case showAnomalyPercentage:
                                {
                                    if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->anomalyPercentage != NODATA)
                                    {
                                        validValues.push_back(myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->anomalyPercentage);
                                    }
                                    break;
                                }
                                case showClimate:
                                {
                                    if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->climate != NODATA)
                                    {
                                        validValues.push_back(myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->climate);
                                    }
                                    break;
                                }
                                default:
                                {
                                    break;
                                }
                            }

                            if (validValues.size() != 0)
                            {
                                    if (statistics::minList(validValues, int(validValues.size())) == validValues[validValues.size() - 1])
                                {
                                    idMin = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->id;
                                    nameMin = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->name;
                                }
                                    if (statistics::maxList(validValues, int(validValues.size())) == validValues[validValues.size() - 1])
                                {
                                    idMax = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->id;
                                    nameMax = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->name;
                                }
                            }
                        }
                    }
            }
            else
            {
                    myProject.errorString = "No MeteoGrid loaded";
                    myProject.logError();
                    return;
            }
            break;
        }

        case 3:     //interpolationRaster
        {
            if (! gis::rasterSummary(&(myProject.dataRaster), nrValidCells, avgValue, errorStdStr))
            {
                QString errorString = QString::fromStdString(errorStdStr);
                myProject.logError(errorString);
                return;
            }

            if (nrValidCells == 0)
            {
                myProject.logWarning("The raster selected has no valid values.");
                return;
            }

            gis::updateMinMaxRasterGrid(&(myProject.dataRaster));

            // [m2] -> [km 2]
            area = nrValidCells * myProject.dataRaster.header->cellSize * myProject.dataRaster.header->cellSize / 1000000;
            break;
        }

        default:
        {
            return;
        }
    }

    // Statistics dialog
    QDialog myDialog;
    myDialog.setWindowTitle("Statistics");
    QTextBrowser textBrowser;

    if (inputId != 3)
    {
        textBrowser.setText("Variable: " + QString::fromStdString(getVariableString(myProject.getCurrentVariable())));
        textBrowser.append("Number of cells: " + QString::number(validValues.size()));
        textBrowser.append("Average value: " + QString::number(statistics::mean(validValues)));
        textBrowser.append("Standard deviation: " + QString::number(statistics::standardDeviation(validValues, int(validValues.size()))));
        textBrowser.append("Maximum: " + QString::number(statistics::maxList(validValues, int(validValues.size()))) + " at " + QString::fromStdString(nameMax) + ", id " + QString::fromStdString(idMax));
        textBrowser.append("Minimum: " + QString::number(statistics::minList(validValues, int(validValues.size()))) + " at " + QString::fromStdString(nameMin) + ", id " + QString::fromStdString(idMin));
    }
    else
    {
        textBrowser.append("Interpolation raster");
        textBrowser.append("Number of pixels: " + QString::number(nrValidCells));
        textBrowser.append("Valid area: " + QString::number(area) + " Km2");
        textBrowser.append("Average value: " + QString::number(avgValue));
        textBrowser.append("Minimum: " + QString::number(myProject.dataRaster.minimum));
        textBrowser.append("Maximum: " + QString::number(myProject.dataRaster.maximum));
    }

    QVBoxLayout mainLayout;
    mainLayout.addWidget(&textBrowser);
    myDialog.setFixedSize(500,170);
    myDialog.setLayout(&mainLayout);
    myDialog.exec();
}


void MainWindow::on_actionDemRangeFixed_triggered(bool isChecked)
{
    ui->actionDemRangeDynamic->setChecked(! isChecked);
    setColorScaleRangeDEM(isChecked);
}


void MainWindow::on_actionDemRangeDynamic_triggered(bool isChecked)
{
    ui->actionDemRangeFixed->setChecked(! isChecked);
    setColorScaleRangeDEM(! isChecked);
}


void MainWindow::on_actionExport_MeteoGrid_toCsv_triggered()
{
    if (!myProject.meteoGridLoaded)
    {
            myProject.logError(ERROR_STR_MISSING_GRID);
            return;
    }

    QString csvFileName = QFileDialog::getSaveFileName(this, tr("Save current meteogrid data"), "", tr("csv files (*.csv)"));

    if (csvFileName != "")
    {
            myProject.exportMeteoGridToCsv(csvFileName);
    }
}


// TODO
void MainWindow::Export_to_png()
{
    // choose colorscale
    QList<QString> colorScaleList;
    if (myProject.DEM.isLoaded)
            colorScaleList.append("DEM");
    if (myProject.meteoGridLoaded)
            colorScaleList.append("MeteoGrid");
    if (myProject.meteoPointsLoaded)
            colorScaleList.append("MeteoPoints");
#ifdef NETCDF
    if (myProject.netCDF.isLoaded())
            colorScaleList.append("NetCDF");
#endif

    QString scaleSelection = "";
    if (colorScaleList.size() > 0)
    {
        FormSelection userSelection(colorScaleList, "Select color scale");
        if (userSelection.result() == QDialog::Rejected) return;
        scaleSelection = userSelection.getSelection();
    }

    // choose output file
    QString pngFileName = QFileDialog::getSaveFileName(this, tr("Save current map as image"), "", tr("png files (*.png)"));
    if (pngFileName == "")
        return;

    // grab map
    int dx = mapView->width() - MAPBORDER*2;
    int dy = mapView->height() - MAPBORDER*2;
    QRect rectangle = QRect(MAPBORDER, MAPBORDER, dx, dy);
    QPixmap map = mapView->grab(rectangle);

    // add colorscale
    if (scaleSelection == "MeteoGrid")
    {
        QPixmap scalePixmap = ui->widgetColorLegendGrid->grab();
        QPainter myPainter(&map);
        //myPainter.setBrush(QColor(1, 1, 1));
        //myPainter.drawRect(100, map.height()-120, scalePixmap.width(),20);
        myPainter.drawPixmap(QPoint(100, dy-200), scalePixmap);
    }

    // save map
    map.save(pngFileName);
}


void MainWindow::on_actionOpenShell_triggered()
{
    myProject.modality = MODE_CONSOLE;

    myProject.pragaShell();

    myProject.modality = MODE_GUI;
}


void MainWindow::redrawOutputPoints()
{
    for (int i = 0; i < int(myProject.outputPoints.size()); i++)
    {
        outputPointList[i]->setVisible(this->viewOutputPoints);

        if (myProject.outputPoints[unsigned(i)].active)
        {
            outputPointList[i]->setFillColor(QColor(Qt::green));
        }
        else
        {
            outputPointList[i]->setFillColor(QColor(Qt::red));
        }
    }
}


void MainWindow::on_actionView_output_points_triggered()
{
    viewOutputPoints = ui->actionView_output_points->isChecked();
    redrawOutputPoints();
}


void MainWindow::on_actionFileOutputPointsClose_triggered()
{
    myProject.closeOutputMeteoPointsDB();
    clearOutputPointMarkers();
}


void MainWindow::on_actionFileOutputPointsOpen_triggered()
{
    QString path = myProject.getProjectPath() + PATH_METEOPOINT;

    QString dbName = QFileDialog::getOpenFileName(this, tr("Open output meteo points DB"), path, tr("DB files (*.db)"));
    if (dbName.isEmpty())
        return;

    if (! myProject.loadOutputMeteoPointsDB(dbName))
    {
        myProject.logError();
        clearOutputPointMarkers();
    }

    addOutputPointsGUI();
}


void MainWindow::addOutputPointsGUI()
{
    clearOutputPointMarkers();

    for (unsigned int i = 0; i < myProject.outputPoints.size(); i++)
    {
        SquareMarker* point = new SquareMarker(9, true, QColor((Qt::green)));
        point->setId(QString::fromStdString(myProject.outputPoints[i].id));
        point->setLatitude(myProject.outputPoints[i].latitude);
        point->setLongitude(myProject.outputPoints[i].longitude);

        this->outputPointList.append(point);
        this->mapView->scene()->addObject(this->outputPointList[i]);
        outputPointList[i]->setToolTip();
    }

    redrawOutputPoints();
}


void MainWindow::on_actionFileOutputPoints_NewFromCsv_triggered()
{
    QString templateFileName = myProject.getDefaultPath() + PATH_TEMPLATE + "template_meteo.db";
    QString path = myProject.getProjectPath() + PATH_METEOPOINT;

    QString dbName = QFileDialog::getSaveFileName(this, tr("Save output points db as"), path, tr("DB files (*.db)"));
    if (dbName.isEmpty())
        return;

    // close previous db
    myProject.closeOutputMeteoPointsDB();
    clearOutputPointMarkers();

    // remove old file and clone db template
    QFile dbFile(dbName);
    if (dbFile.exists())
    {
        dbFile.close();
        dbFile.setPermissions(QFile::ReadOther | QFile::WriteOther);
        if (! dbFile.remove())
        {
            myProject.logError("Remove file failed: " + dbName + "\n" + dbFile.errorString());
            return;
        }
    }

    if (! QFile::copy(templateFileName, dbName))
    {
        myProject.logError("Copy failed: " + templateFileName);
        return;
    }

    // read csv fields
    QList<QString> csvFieldsList;
    QList<QList<QString>> csvData;

    QString csvFileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("csv files (*.csv)"));
    if (csvFileName.isEmpty())
        return;

    if (! parseCSV(csvFileName, csvFieldsList, csvData, myProject.errorString))
    {
        myProject.logError();
        return;
    }

    // open DB and read praga properties
    myProject.outputMeteoPointsDbHandler = new Crit3DMeteoPointsDbHandler(dbName);

    QList<QString> pragaPointPropertiesList;
    if (! myProject.outputMeteoPointsDbHandler->getFieldList("point_properties", pragaPointPropertiesList))
    {
        myProject.logError("Error in reading table 'point_properties'");
        return;
    }

    // join properties
    DialogPointProperties dialogPointProp(pragaPointPropertiesList, csvFieldsList);
    if (dialogPointProp.result() != QDialog::Accepted)
    {
        myProject.closeOutputMeteoPointsDB();
        return;
    }
    QList<QString> joinedList = dialogPointProp.getJoinedList();

    // write joined properties
    if (! myProject.writeMeteoPointsProperties(joinedList, csvFieldsList, csvData, true))
    {
        myProject.logError();
        return;
    }

    myProject.loadOutputMeteoPointsDB(dbName);
    addOutputPointsGUI();
}


void MainWindow::on_actionCompute_drought_triggered()
{
    if (!myProject.meteoPointsLoaded && !myProject.meteoGridLoaded)
    {
        myProject.logWarning(ERROR_STR_MISSING_POINT_GRID);
        return;
    }

    bool isMeteoPointLoaded = false;
    bool isMeteoGridLoaded = false;
    int yearPointsFrom;
    int yearPointsTo;
    int yearGridFrom;
    int yearGridTo;

    if (myProject.meteoPointsLoaded)
    {
        isMeteoPointLoaded = true;
        yearPointsFrom = myProject.meteoPointsDbHandler->getFirstDate(daily).date().year();
        yearPointsTo = myProject.meteoPointsDbHandler->getLastDate(daily).date().year();
    }
    if (myProject.meteoGridLoaded)
    {
        isMeteoGridLoaded = true;
        yearGridFrom = myProject.meteoGridDbHandler->getFirstDailyDate().year();
        yearGridTo = myProject.meteoGridDbHandler->getLastDailyDate().year();
    }

    DialogComputeDroughtIndex compDialog(isMeteoGridLoaded, isMeteoPointLoaded, yearPointsFrom, yearPointsTo, yearGridFrom, yearGridTo, myProject.getCurrentDate());
    if (compDialog.result() != QDialog::Accepted)
        return;

    bool isMeteoGrid = compDialog.getIsMeteoGrid();
    int refYearStart = compDialog.getYearFrom();
    int refYearEnd = compDialog.getYearTo();
    QDate myDate = compDialog.getDate();
    QString indexStr = compDialog.getIndex();
    int timescale;

    droughtIndex index;
    if (indexStr == "INDEX_SPI")
    {
        index = INDEX_SPI;
        timescale = compDialog.getTimescale();
    }
    else if (indexStr == "INDEX_SPEI")
    {
        index = INDEX_SPEI;
        timescale = compDialog.getTimescale();
    }
    else if (indexStr == "INDEX_DECILES")
    {
        index = INDEX_DECILES;
    }
    else
    {
        myProject.logError("Wrong index.");
        return;
    }

    if (isMeteoGrid)
    {
        if (! myProject.computeDroughtIndexGrid(index, refYearStart, refYearEnd, myDate, timescale, noMeteoVar))
            return;

        redrawMeteoGrid(showElaboration, true);

        ui->lineEditElab1->setText("Drought index: " + indexStr);

        if (indexStr == "INDEX_SPI" || indexStr == "INDEX_SPEI")
        {
            ui->lineEditVariable->setText(indexStr+" timescale:"+QString::number(timescale));
        }
        else
        {
            ui->lineEditVariable->setText(indexStr);
        }
        ui->lineEditOffset->setVisible(false);
        ui->lineEditElab2->setVisible(false);
        ui->lineEditPeriod->setText("reference period: " + QString::number(refYearStart) + "÷" + QString::number(refYearEnd));
        ui->lineEditElab1->setReadOnly(true);
        ui->lineEditElab2->setReadOnly(true);
        ui->lineEditVariable->setReadOnly(true);
        ui->lineEditPeriod->setReadOnly(true);

        ui->groupBoxElaboration->show();

        emit meteoGridObj->redrawRequested();
    }
    else
    {
        if (pointList.size() == 0) return;

        myProject.logInfoGUI("Drought Index - Meteo Point");
        bool isOk = myProject.computeDroughtIndexPointGUI(index, timescale, refYearStart, refYearEnd, myDate);
        myProject.closeLogInfo();

        if(! isOk)
        {
            myProject.logError("No data.");
            return;
        }

        ui->groupBoxElaboration->hide();
        myProject.clima = new Crit3DClimate();

        this->redrawMeteoPoints(showElaboration, true);

        ui->lineEditElab1->setText("Drought index: " + indexStr);

        if (indexStr == "INDEX_SPI" || indexStr == "INDEX_SPEI")
        {
            ui->lineEditVariable->setText(indexStr+" timescale:"+QString::number(timescale));
        }
        else
        {
            ui->lineEditVariable->setText(indexStr);
        }

        //ui->lineEditElab2->setVisible(false);
        //ui->lineEditPeriod->setReadOnly(true);
        //ui->groupBoxElaboration->show();
    }
}


void MainWindow::on_actionFileMeteogrid_ExportDailyData_triggered()
{
    if (! myProject.meteoGridLoaded || myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logWarning(ERROR_STR_MISSING_GRID);
        return;
    }

    DialogExportDataGrid exportDialog;
    if (exportDialog.result() != QDialog::Accepted)
    {
        return;
    }

    // variables list
    QList<QString> varNameList = exportDialog.getDailyVariableList();
    QList<meteoVariable> variableList;
    for (int i = 0; i < varNameList.size(); i++)
    {
        meteoVariable var = getMeteoVar(varNameList[i].toStdString());
        if (var != noMeteoVar)
        {
            variableList.append(var);
        }
    }

    QDate firstDate = exportDialog.getFirstDate();
    QDate lastDate = exportDialog.getLastDate();
    QString cellListFileName = exportDialog.getCellListFileName();

    QString outputPath = myProject.getProjectPath() + PATH_OUTPUT;

    if (! myProject.meteoGridDbHandler->exportDailyDataCsv(variableList, firstDate, lastDate,
                                                           cellListFileName, outputPath, myProject.errorString))
    {
        myProject.logError();
    }

    myProject.logInfoGUI("Files exported to the directory: " + outputPath);
}


void MainWindow::on_actionFileMeteopointData_XMLexport_triggered()
{
    // check meteo point
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_DB);
        return;
    }

    QDateTime myFirstTime = myProject.meteoPointsDbFirstTime;
    QDateTime myLastTime = myProject.meteoPointsDbLastTime;
    if (myFirstTime.isNull() || myLastTime.isNull())
    {
        myProject.logError("DB is empty");
        return;
    }

    FormTimePeriod myForm(&myFirstTime, &myLastTime);
    myForm.show();
    if (myForm.exec() == QDialog::Rejected) return;

    QString xmlName = QFileDialog::getOpenFileName(this, tr("Open XML file"), "", tr("xml files (*.xml)"));
    if (xmlName.isEmpty())
        return;

    bool isGrid = false;
    if (!myProject.parserXMLImportExportData(xmlName, isGrid))
        return;

    QList<QString> pointSelected;
    bool allPoints = false;
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        if (myProject.meteoPoints[i].selected)
        {
            pointSelected << QString::fromStdString(myProject.meteoPoints[i].id);
        }
    }
    if (pointSelected.isEmpty())
    {
        allPoints = true;
    }

    if (allPoints)
    {
        myProject.setProgressBar("Exporting points...", myProject.nrMeteoPoints);
        for (int i = 0; i < myProject.nrMeteoPoints; i++)
        {
            myProject.updateProgressBar(i);
            if (!myProject.loadXMLExportData(QString::fromStdString(myProject.meteoPoints[i].id), myFirstTime, myLastTime))
            {
                    myProject.logError();
                    myProject.closeProgressBar();
                    delete myProject.inOutData;
                    return;
            }
        }
        myProject.closeProgressBar();
    }
    else
    {
        myProject.setProgressBar("Exporting points...", pointSelected.size());
        for (int i = 0; i < pointSelected.size(); i++)
        {
            myProject.updateProgressBar(i);
            if (!myProject.loadXMLExportData(pointSelected[i], myFirstTime, myLastTime))
            {
                    myProject.logError();
                    myProject.closeProgressBar();
                    delete myProject.inOutData;
                    return;
            }
        }
        myProject.closeProgressBar();
    }
    delete myProject.inOutData;
    return;
}


void MainWindow::on_actionFileMeteogridData_XMLexport_triggered()
{
    // check meteo grid
    if (myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError(ERROR_STR_MISSING_GRID);
        return;
    }

    QDateTime myFirstTime;
    myFirstTime.setDate(myProject.meteoGridDbHandler->firstDate());
    myFirstTime.setTime(QTime(0,0));
    QDateTime myLastTime;
    myLastTime.setDate(myProject.meteoGridDbHandler->lastDate());
    myLastTime.setTime(QTime(0,0));
    if (myFirstTime.isNull() || myLastTime.isNull())
    {
        myProject.logError("DB is empty");
        return;
    }

    FormTimePeriod myForm(&myFirstTime, &myLastTime);
    myForm.show();
    if (myForm.exec() == QDialog::Rejected) return;

    QString xmlName = QFileDialog::getOpenFileName(this, tr("Open XML file"), "", tr("xml files (*.xml)"));
    if (xmlName.isEmpty())
        return;

    bool isGrid = true;
    if (!myProject.parserXMLImportExportData(xmlName, isGrid))
        return;

    myProject.setProgressBar("Exporting cells...", myProject.meteoGridDbHandler->gridStructure().header().nrRows);
    for (int row = 0; row < myProject.meteoGridDbHandler->gridStructure().header().nrRows; row++)
    {
        myProject.updateProgressBar(row);
        for (int col = 0; col < myProject.meteoGridDbHandler->gridStructure().header().nrCols; col++)
        {
            if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->active)
            {
                    if ( !myProject.loadXMLExportDataGrid(QString::fromStdString(myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->id), myFirstTime, myLastTime ))
                    {
                        myProject.logError();
                        myProject.closeProgressBar();
                        delete myProject.inOutData;
                        return;
                    }
            }
        }
    }
    myProject.closeProgressBar();
    delete myProject.inOutData;
    return;
}


void MainWindow::on_actionWaterTable_importLocation_triggered()
{
    QString csvFileName = QFileDialog::getOpenFileName(this, tr("Open file"), myProject.getDefaultPath(), tr("csv files (*.csv)"));
    if (csvFileName.isEmpty())
        return;

    if (! myProject.waterTableImportLocation(csvFileName))
    {
        myProject.waterTableList.clear();
        clearWellPointMarkers();
        return;
    }

    return addWellPointsGUI();
}


void MainWindow::on_actionWaterTable_importDepth_triggered()
{
    if (myProject.wellPoints.size() == 0)
    {
        myProject.logError("Import data location before.");
        return;
    }

    QString csvFileName = QFileDialog::getOpenFileName(this, tr("Open file"), myProject.getDefaultPath(), tr("csv files (*.csv)"));
    if (csvFileName.isEmpty())
        return;

    if (! myProject.waterTableImportDepths(csvFileName))
        return;
}


void MainWindow::addWellPointsGUI()
{
    clearWellPointMarkers();

    for (unsigned int i = 0; i < myProject.wellPoints.size(); i++)
    {
        SquareMarker* point = new SquareMarker(9, true, QColor((Qt::green)));

        point->setId(myProject.wellPoints[i].getId());
        double utmX = myProject.wellPoints[i].getUtmX();
        double utmY = myProject.wellPoints[i].getUtmY();

        double myLat, myLon;
        gis::getLatLonFromUtm(myProject.gisSettings, utmX, utmY, &myLat, &myLon);
        point->setLatitude(myLat);
        point->setLongitude(myLon);

        this->wellsListObj.append(point);
        this->mapView->scene()->addObject(this->wellsListObj[i]);

        wellsListObj[i]->setToolTip();
    }
}


void MainWindow::clearWellPointMarkers()
{
    for (int i = 0; i < wellsListObj.size(); i++)
    {
        mapView->scene()->removeObject(wellsListObj[i]);
        delete wellsListObj[i];
    }

    wellsListObj.clear();
    return;
}


void MainWindow::on_actionWaterTable_computeSingleWell_triggered()
{
    if (myProject.wellPoints.size() == 0)
    {
        myProject.logError("Import wells data and location before.");
        return;
    }

    if (!myProject.meteoPointsLoaded && !myProject.meteoGridLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_POINT_GRID);
        return;
    }

    QList<QString> wellsId;
    for (int i = 0; i < myProject.wellPoints.size(); i++)
    {
        wellsId.append(myProject.wellPoints[i].getId());
    }
    DialogSelectWell selectWell(wellsId);
    if (selectWell.result() != QDialog::Accepted)
    {
        return;
    }

    QString selectedId = selectWell.getIdSelected();
    int selectedIndex = NODATA;
    for (int i = 0; i < myProject.wellPoints.size(); i++)
    {
        QString id = myProject.wellPoints[i].getId();
        if (id == selectedId)
        {
            if (myProject.wellPoints[i].getObsDepthNr() == 0)
            {
                    myProject.logError("The selected well " + id + " has not data. Import data before.");
                    return;
            }
            selectedIndex = i;
            break;
        }
    }

    if (! myProject.waterTableComputeSingleWell(selectedIndex))
        return;

    myProject.waterTableShowSingleWell(myProject.waterTableList[myProject.waterTableList.size()-1], selectedId);
    return;
}


void MainWindow::on_actionWaterTable_showDepth_triggered()
{
    if (myProject.waterTableList.size() == 0)
    {
        myProject.logError("Compute parameters for all wells before.");
        return;
    }
    // TO DO
}


void MainWindow::on_actionWaterTable_Hide_triggered()
{
    for (int i = 0; i < wellsListObj.size(); i++)
    {
        wellsListObj[i]->setVisible(false);
    }
}


void MainWindow::on_actionWaterTable_computeAllWells_triggered()
{
    if (myProject.wellPoints.size() == 0)
    {
        myProject.logError("Import wells data and location before.");
        return;
    }

    if (!myProject.meteoPointsLoaded && !myProject.meteoGridLoaded)
    {
        myProject.logError(ERROR_STR_MISSING_POINT_GRID);
        return;
    }

    // clear all waterTable
    myProject.waterTableList.clear();
    myProject.setProgressBar("Computing well...", int(myProject.wellPoints.size()));

    for (int i = 0; i < myProject.wellPoints.size(); i++)
    {
        QString idStr = myProject.wellPoints[i].getId();
        if (myProject.wellPoints[i].getObsDepthNr() == 0)
        {
            myProject.logInfo("The well " + idStr + " has not data. Import data before.");
            continue;
        }

        if (! myProject.waterTableComputeSingleWell(i))
        {
            myProject.logInfo("Error in computing well: " + idStr);
            continue;
        }

        if (myProject.waterTableList.last().getIdWell() == idStr)
        {
            QString text = " ID: " + idStr;
            text += "\n R2: " + QString::number(myProject.waterTableList.last().getR2(),'f', 2);
            text += "\n H0: " + QString::number(myProject.waterTableList.last().getH0(),'f', 2);
            text += "\n alpha: " + QString::number(myProject.waterTableList.last().getAlpha(),'f', 2);
            text += "\n nr. days: " + QString::number(myProject.waterTableList.last().getNrDaysPeriod());
            text += "\n avg daily CWB: " + QString::number(myProject.waterTableList.last().getAvgDailyCWB(),'f', 2);
            wellsListObj[i]->SquareObject::setToolTip(text);
        }

        myProject.updateProgressBar(i);
    }

    myProject.closeProgressBar();

    return;
}


void MainWindow::on_actionWaterTable_showLocation_triggered()
{
    for (int i = 0; i < wellsListObj.size(); i++)
    {
        wellsListObj[i]->showText(false);
        wellsListObj[i]->setVisible(true);
    }
}


void MainWindow::on_actionWaterTable_showId_triggered()
{
    for (int i = 0; i < wellsListObj.size(); i++)
    {
        wellsListObj[i]->showText(true);
        wellsListObj[i]->showId(true);
        wellsListObj[i]->setVisible(true);
    }
}


void MainWindow::on_actionInterpolationTopographicIndex_triggered()
{
    if (! myProject.DEM.isLoaded)
    {
        myProject.logError("No DEM loaded");
        return;
    }

    FormText formWidths("Insert multiple window widths (in meters, separated by commas)", "");
    if (formWidths.result() == QDialog::Rejected)
        return;

    QString widthsString = formWidths.getText();
    if (widthsString == "")
        return;

    QStringList widthList = widthsString.split(",");

    std::vector <float> widths;
    bool isValid;
    for (QString widthString : widthList)
    {
        float width = widthString.toFloat(&isValid);

        if (! isValid)
            myProject.logError("Invalid width: " + widthString);

        widths.push_back(width);
    }

    gis::Crit3DRasterGrid outGrid;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save topographic index"), "", tr("ESRI grid files (*.flt)"));

    if (fileName == "") return;

    if (topographicIndex(myProject.DEM, widths, outGrid))
    {
        std::string myError;
        QString fileWithoutExtension = QFileInfo(fileName).absolutePath() + QDir::separator() + QFileInfo(fileName).baseName();
        if (!gis::writeEsriGrid(fileWithoutExtension.toStdString(), &outGrid, myError))
            myProject.logError(QString::fromStdString(myError));
    }
}


void MainWindow::on_actionInterpolationWriteGlocalWeightMaps_triggered()
{
    if (!myProject.loadGlocalAreasMap())
    {
        myProject.logError("Error loading zone grid " + myProject.glocalMapName);
        return;
    }

    if (! myProject.interpolationSettings.getMacroAreasMap()->isLoaded)
    {
        myProject.logError("Load a zone grid before.");
        return;
    }

    FormText formWidth("Insert window width (meters)", "10000");
    if (formWidth.result() == QDialog::Rejected)
        return;

    QString widthString = formWidth.getText();
    if (widthString == "")
        return;

    bool isValid;
    float width = widthString.toFloat(&isValid);
    if (! isValid)
        myProject.logError("Invalid width: " + widthString);

    myProject.logInfoGUI("Writing glocal weight maps...");
    if (! myProject.writeGlocalWeightsMaps(width))
        myProject.logError();
    myProject.closeLogInfo();
}


void MainWindow::on_actionHide_supplemental_stations_toggled(bool state)
{
    hideSupplementals = state;
    redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_actionShowPointsCVResidual_triggered()
{
    redrawMeteoPoints(showCVResidual, true);
}

void MainWindow::on_actionShowInfo_triggered()
{
    QString helpStr = "*** " + myProject.getVersion() + " ***";
    helpStr += "\n\nARPAE Emilia-Romagna\nHydro-Meteo-Climate Service";
    helpStr += "\n\nAUTHORS:";
    helpStr += "\n- Gabriele Antolini   gantolini@arpae.it";
    helpStr += "\n- Fausto Tomei         ftomei@arpae.it";
    helpStr += "\n- Antonio Volta        avolta@arpae.it";
    helpStr += "\n- Laura Costantini    laura.costantini0@gmail.com";
    helpStr += "\n- Caterina Toscano    ctoscano@arpae.it";
    helpStr += "\n\nhttps://github.com/ARPA-SIMC/PRAGA";

    myProject.logInfoGUI(helpStr);
}


void MainWindow::on_actionMark_macro_area_stations_triggered()
{

    if (myProject.interpolationSettings.getUseGlocalDetrending() && ! myProject.interpolationSettings.isGlocalReady(false))
    {
        if (! myProject.loadGlocalAreasMap() || ! myProject.loadGlocalStationsAndCells(false, myProject.getCompleteFileName(myProject.glocalPointsName, PATH_GEO)))
        {
            return;
        }
    }

    FormText formWidth("Insert macroarea number", "");
    if (formWidth.result() == QDialog::Rejected)
        return;

    QString numberString = formWidth.getText();
    if (numberString == "")
        return;

    bool isValid;
    int areaNr = numberString.toInt(&isValid);

    if (! isValid)
        myProject.logError("Invalid number: " + numberString);
    myProject.setMarkedPointsOfMacroArea(areaNr, viewNotActivePoints);

    redrawMeteoPoints(currentPointsVisualization, true);

    return;
}



void MainWindow::on_actionAll_Selected_triggered()
{
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
    {
        myProject.meteoPoints[i].selected = true;
    }

    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::on_actionNone_Selected_triggered()
{
    myProject.clearSelectedPoints();
    redrawMeteoPoints(currentPointsVisualization, false);
}


void MainWindow::on_actionFrom_point_list_Selected_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open point list file"), "", tr("text files (*.txt)"));
    if (fileName == "") return;

    if (myProject.setSelectedStatePointList(fileName))
    {
        redrawMeteoPoints(currentPointsVisualization, true);
    }
}


void MainWindow::on_actionWith_Criteria_Selected_triggered()
{
    if (myProject.setSelectedStateWithCriteria())
    {
        redrawMeteoPoints(currentPointsVisualization, true);
    }
}


void MainWindow::on_actionLoad_macro_areas_file_triggered()
{
    if (! myProject.meteoPointsLoaded && ! myProject.meteoGridLoaded)
    {
       myProject.logWarning(ERROR_STR_MISSING_POINT_GRID);
       return;
    }

    QString defaultPath = myProject.getDefaultPath() + PATH_GEO;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file with macro areas"), defaultPath,
                                                    tr("ESRI FLT (*.flt)"));
    if (fileName.isEmpty())
        return;

    QFileInfo fileInfo(fileName);
    myProject.glocalMapName = (fileInfo.path() + "/" + fileInfo.baseName());

    myProject.logInfoGUI("Loading macro area file...");

    if (! myProject.loadGlocalAreasMap())
    {
        myProject.closeLogInfo();
        myProject.logWarning();
        return;
    }
    myProject.closeLogInfo();
}


void MainWindow::on_actionLoad_stations_file_triggered()
{
    if (! myProject.meteoPointsLoaded && ! myProject.meteoGridLoaded)
    {
       myProject.logWarning(ERROR_STR_MISSING_POINT_GRID);
       return;
    }

    QString defaultPath = myProject.getDefaultPath() + PATH_GEO;
    myProject.glocalPointsName = QFileDialog::getOpenFileName(this, tr("Open file with stations"), defaultPath,
                                                    tr("Comma separated values (*.csv)"));
    if (myProject.glocalPointsName.isEmpty())
        return;

    myProject.logInfoGUI("Loading stations file...");

    if (! myProject.loadGlocalStationsAndCells(myProject.meteoGridLoaded, myProject.getCompleteFileName(myProject.glocalPointsName, PATH_GEO)))
    {
        myProject.closeLogInfo();
        myProject.logWarning();
        return;
    }
    myProject.closeLogInfo();
}


void MainWindow::on_actionInterpolationGlocalCreateWeightMaps_triggered()
{
    if (!myProject.loadGlocalAreasMap())
    {
        myProject.logError("Error loading zone grid " + myProject.glocalMapName);
        return;
    }

    if (! myProject.interpolationSettings.getMacroAreasMap()->isLoaded)
    {
        myProject.logError("Load a zone grid before.");
        return;
    }

    FormText formWidth("Insert window width (meters)", "10000");

    if (formWidth.result() == QDialog::Rejected)
        return;

    QString widthString = formWidth.getText();
    if (widthString == "")
        return;

    bool isValid;
    float width = widthString.toFloat(&isValid);
    if (! isValid)
        myProject.logError("Invalid width: " + widthString);

    myProject.logInfoGUI("Writing glocal weight maps...");
    if (! myProject.writeGlocalWeightsMaps(width))
        myProject.logError();
    myProject.closeLogInfo();
}


void MainWindow::on_actionOpen_meteo_widget_for_selected_stations_triggered()
{
    //showMeteoWidgetPoint
    myProject.showMeteoWidgetMultiplePoints();
}

