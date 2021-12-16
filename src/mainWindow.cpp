#include <QGridLayout>
#include <QFileDialog>
#include <QtDebug>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidget>
#include <QRadioButton>
#include <QTextBrowser>
#include <QIODevice>

#include <sstream>
#include <iostream>
#include <iomanip>      // std::setprecision

#include "formTimePeriod.h"
#include "mainWindow.h"
#include "ui_mainWindow.h"
#include "dbMeteoPointsHandler.h"
#include "dbArkimet.h"
#include "download.h"
#include "commonConstants.h"
#include "dialogSelection.h"
#include "dialogDownloadMeteoData.h"
#include "dialogMeteoComputation.h"
#include "dialogClimateFields.h"
#include "dialogSeriesOnZones.h"
#include "dialogInterpolation.h"
#include "dialogRadiation.h"
#include "dialogPragaSettings.h"
#include "spatialControl.h"
#include "dialogPragaProject.h"
#include "dialogPointProperties.h"
#include "dialogPointDeleteData.h"
#include "dialogSelectionMeteoPoint.h"
#include "utilities.h"
#include "basicMath.h"
#include "meteoWidget.h"


extern PragaProject myProject;

#define MAPBORDER 10
#define TOOLSWIDTH 260


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);

    this->rubberBand = nullptr;

    // Set the MapGraphics Scene and View
    this->mapScene = new MapGraphicsScene(this);
    this->mapView = new MapGraphicsView(mapScene, this->ui->widgetMap);

    this->rasterLegend = new ColorLegend(this->ui->widgetColorLegendRaster);
    this->rasterLegend->resize(this->ui->widgetColorLegendRaster->size());

    this->meteoGridLegend = new ColorLegend(this->ui->widgetColorLegendGrid);
    this->meteoGridLegend->resize(this->ui->widgetColorLegendGrid->size());

    this->meteoPointsLegend = new ColorLegend(this->ui->widgetColorLegendPoints);
    this->meteoPointsLegend->resize(this->ui->widgetColorLegendPoints->size());
    this->meteoPointsLegend->colorScale = myProject.meteoPointsColorScale;

    // Set tiles source
    this->setTileSource(WebTileSource::OPEN_STREET_MAP);

    // Set start size and position
    this->startCenter = new Position (myProject.gisSettings.startLocation.longitude,
                                     myProject.gisSettings.startLocation.latitude, 0.0);
    this->mapView->setZoomLevel(8);
    this->mapView->centerOn(startCenter->lonLat());

    // Set raster objects
    this->rasterObj = new RasterObject(this->mapView);
    this->meteoGridObj = new RasterObject(this->mapView);

    this->rasterObj->setOpacity(this->ui->rasterOpacitySlider->value() / 100.0);
    this->meteoGridObj->setOpacity(this->ui->meteoGridOpacitySlider->value() / 100.0);

    this->rasterObj->setColorLegend(this->rasterLegend);
    this->meteoGridObj->setColorLegend(this->meteoGridLegend);

    this->mapView->scene()->addObject(this->rasterObj);
    this->mapView->scene()->addObject(this->meteoGridObj);
    connect(this->mapView, SIGNAL(zoomLevelChanged(quint8)), this, SLOT(updateMaps()));
    connect(this->mapView, SIGNAL(mouseMoveSignal(const QPoint&)), this, SLOT(mouseMove(const QPoint&)));

    this->updateVariable();
    this->updateDateTime();

    KeyboardFilter *keyboardFilter = new KeyboardFilter();
    this->ui->dateEdit->installEventFilter(keyboardFilter);
    //connect(this->ui->dateEdit, SIGNAL(editingFinished()), this, SLOT(on_dateChanged()));

    this->ui->meteoPoints->setEnabled(false);
    this->ui->grid->setEnabled(false);

    // show menu
    showPointsGroup = new QActionGroup(this);
    showPointsGroup->setExclusive(true);
    showPointsGroup->addAction(this->ui->actionShowPointsHide);
    showPointsGroup->addAction(this->ui->actionShowPointsLocation);
    showPointsGroup->addAction(this->ui->actionShowPointsCurrent);
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

    this->currentPointsVisualization = notShown;
    this->currentGridVisualization = notShown;
    this->viewNotActivePoints = false;

    this->setWindowTitle("PRAGA");

    ui->groupBoxElab->hide();
}

MainWindow::~MainWindow()
{
    delete rasterObj;
    delete meteoGridObj;
    delete rasterLegend;
    delete meteoGridLegend;
    delete meteoPointsLegend;
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
    QString status = QString::number(geoPos.latitude()) + " " + QString::number(geoPos.longitude());

    // meteo grid
    if (meteoGridObj->isLoaded && !meteoGridObj->isNetCDF())
    {
        int row, col;
        gis::Crit3DGeoPoint geoPoint = gis::Crit3DGeoPoint(geoPos.latitude(), geoPos.longitude());

        if (! meteoGridObj->getRowCol(geoPoint, &row, &col))
            return;

        if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->active)
        {
            std::string id = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->id;
            std::string name = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->name;
            float value = NODATA;
            switch(currentGridVisualization)
            {
                case showElaboration:
                {
                    value = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->elaboration;
                    break;
                }
                case showCurrentVariable:
                {
                    value = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->currentValue;
                    break;
                }
            }
            std::string valueStr;
            if (value == NODATA)
            {
                valueStr = "NODATA";
            }
            else
            {
                std::stringstream stream;
                stream << std::fixed << std::setprecision(2) << value;
                valueStr = stream.str();
            }
            status += " Grid: " + QString::fromStdString(id + " " + name + " Value: " + valueStr);
        }
     }

    this->ui->statusBar->showMessage(status);
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    mapView->resize(ui->widgetMap->size());
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


// zoom
void MainWindow::mouseDoubleClickEvent(QMouseEvent * event)
{
    QPoint mapPos = getMapPos(event->pos());
    if (! isInsideMap(mapPos)) return;

    Position newCenter = this->mapView->mapToScene(mapPos);
    this->ui->statusBar->showMessage(QString::number(newCenter.latitude()) + " " + QString::number(newCenter.longitude()));

    if (event->button() == Qt::LeftButton)
        this->mapView->zoomIn();
    else
        this->mapView->zoomOut();

    this->mapView->centerOn(newCenter.lonLat());
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{

    QPoint mapPos = getMapPos(event->pos());
    if (! isInsideMap(mapPos)) return;

    if (event->button() == Qt::RightButton)
    {
        if (rubberBand != nullptr)
        {
            QPoint widgetPos = mapPos + QPoint(MAPBORDER, MAPBORDER);
            rubberBand->setOrigin(widgetPos);
            rubberBand->setGeometry(QRect(widgetPos, QSize()));
            rubberBand->isActive = true;
            rubberBand->show();
            return;
        }

        // GRID - context menu
        if (meteoGridObj->isLoaded)
        {
            Position geoPos = mapView->mapToScene(mapPos);
            gis::Crit3DGeoPoint geoPoint = gis::Crit3DGeoPoint(geoPos.latitude(), geoPos.longitude());

            #ifdef NETCDF
            if (myProject.netCDF.isLoaded())
            {
                netCDF_exportDataSeries(geoPoint);
                return;
            }
            #endif

            int row, col;
            if (! meteoGridObj->getRowCol(geoPoint, &row, &col))
                return;

            std::string id = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->id;
            std::string name = myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->name;

            if (myProject.meteoGridDbHandler->meteoGrid()->meteoPoints()[row][col]->active)
            {
                QMenu menu;
                QAction *openMeteoWidget = menu.addAction("Open new meteo widget");
                QAction *appendMeteoWidget = menu.addAction("Append to last meteo widget");
                if (myProject.meteoGridDbHandler->meteoGrid()->gridStructure().isEnsemble())
                {
                    appendMeteoWidget->setEnabled(false);
                }
                else
                {
                    appendMeteoWidget->setEnabled(true);
                }

                QAction *selection =  menu.exec(QCursor::pos());

                if (selection != nullptr)
                {
                    if (selection == openMeteoWidget)
                    {
                        myProject.showMeteoWidgetGrid(id, false);
                    }
                    if (selection == appendMeteoWidget)
                    {
                        myProject.showMeteoWidgetGrid(id, true);
                    }
                    // TODO: other actions

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
    rasterObj->updateCenter();
    meteoGridObj->updateCenter();
}

void MainWindow::clearDEM()
{
    this->rasterObj->clear();
    this->rasterObj->redrawRequested();
    ui->labelRasterScale->setText("");
    this->ui->rasterOpacitySlider->setEnabled(false);
}


void MainWindow::renderDEM()
{
    this->setCurrentRaster(&(myProject.DEM));
    ui->labelRasterScale->setText(QString::fromStdString(getVariableString(noMeteoTerrain)));
    this->ui->rasterOpacitySlider->setEnabled(true);

    // resize map
    double size = double(this->rasterObj->getRasterMaxSize());
    size = log2(1000 / size);
    this->mapView->setZoomLevel(quint8(size));

    // center map
    gis::Crit3DGeoPoint* center = this->rasterObj->getRasterCenter();
    this->mapView->centerOn(qreal(center->longitude), qreal(center->latitude));

    this->updateMaps();
}


void MainWindow::on_actionFileOpenDEM_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open raster Grid"), "", tr("ESRI grid files (*.flt)"));

    if (fileName == "") return;

    if (!myProject.loadDEM(fileName)) return;

    renderDEM();

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

void MainWindow::disableAllButton(bool toggled)
{
    if (!toggled)
    {
        if (all->isChecked())
        {
            all->setChecked(false);
        }
    }
}

void MainWindow::on_actionFileMeteopointDownload_triggered()
{
    if(myProject.nrMeteoPoints == 0)
    {
         QMessageBox::information(nullptr, "DB not existing", "Create or Open a meteo points database before download");
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
        if (!downloadDialog.getVarD().isEmpty())
        {
            if (! myProject.downloadDailyDataArkimet(downloadDialog.getVarD(), downloadDialog.getPrec0024(), firstDate, lastDate, true))
             {
                 QMessageBox::information(nullptr, "Error!", "Error in daily download");
             }
        }
        if (!downloadDialog.getVarH().isEmpty())
        {
            if (! myProject.downloadHourlyDataArkimet(downloadDialog.getVarH(), firstDate, lastDate, true))
             {
                 QMessageBox::information(nullptr, "Error!", "Error in daily download");
             }
        }
    }

    this->loadMeteoPoints(myProject.meteoPointsDbHandler->getDbName());
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


void MainWindow::resetMeteoPointsMarker()
{
    for (int i = pointList.size()-1; i >= 0; i--)
    {
        mapView->scene()->removeObject(pointList[i]);
    }
    pointList.clear();

    datasetCheckbox.clear();
}


void MainWindow::on_actionMeteopointQualitySpatial_triggered()
{
    myProject.checkSpatialQuality = ui->actionMeteopointQualitySpatial->isChecked();
    updateVariable();
    redrawMeteoPoints(currentPointsVisualization, true);
}


void MainWindow::interpolateDemGUI()
{
    bool isComputed = false;

    meteoVariable myVar = myProject.getCurrentVariable();

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
    else {
        isComputed = myProject.interpolationDemMain(myVar, myProject.getCrit3DCurrentTime(), &(myProject.dataRaster));
    }

    if (isComputed) {
        {
            setColorScale(myVar, myProject.dataRaster.colorScale);
            setCurrentRaster(&(myProject.dataRaster));
            ui->labelRasterScale->setText(QString::fromStdString(getVariableString(myVar)));
        }
    }
}

void MainWindow::interpolateGridGUI()
{
    if (myProject.interpolationMeteoGrid(myProject.getCurrentVariable(), myProject.getCurrentFrequency(),
                                         myProject.getCrit3DCurrentTime()))
    {
        //setCurrentRaster(&(myProject.meteoGridDbHandler->meteoGrid()->dataMeteoGrid));
        //ui->labelRasterScale->setText(QString::fromStdString(getVariableString(myProject.getCurrentVariable())));
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

void MainWindow::on_dateChanged()
{
    QDate date = this->ui->dateEdit->date();

    if (date != myProject.getCurrentDate())
    {
        myProject.loadMeteoPointsData(date, date, true, true, true);
        myProject.loadMeteoGridData(date, date, true);
        myProject.setCurrentDate(date);
    }

    redrawMeteoPoints(currentPointsVisualization, true);
    redrawMeteoGrid(currentGridVisualization, false);
}

void MainWindow::on_timeEdit_valueChanged(int myHour)
{
    if (myHour != myProject.getCurrentHour())
    {
        myProject.setCurrentHour(myHour);
    }

    frequencyType frequency = myProject.getCurrentFrequency();
    if (frequency == hourly)
    {
        redrawMeteoPoints(currentPointsVisualization, true);
        redrawMeteoGrid(currentGridVisualization, false);
    }
}


#ifdef NETCDF

    void MainWindow::on_actionNetCDF_Open_triggered()
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open NetCDF data"), "", tr("NetCDF files (*.nc)"));
        if (fileName == "") return;

        myProject.netCDF.initialize(myProject.gisSettings.utmZone);

        myProject.netCDF.readProperties(fileName.toStdString());

        if (myProject.netCDF.isLatLon || myProject.netCDF.isRotatedLatLon)
        {
            meteoGridObj->initializeLatLon(&(myProject.netCDF.dataGrid), myProject.gisSettings, myProject.netCDF.latLonHeader, true);
        }
        else
        {
            meteoGridObj->initializeUTM(&(myProject.netCDF.dataGrid), myProject.gisSettings, true);
        }

        meteoGridObj->setNetCDF(true);

        myProject.netCDF.dataGrid.setConstantValue(0);

        updateMaps();
    }


    void MainWindow::closeNetCDF()
    {
        if (! myProject.netCDF.isLoaded()) return;

        myProject.netCDF.close();
        meteoGridObj->clear();
        meteoGridObj->redrawRequested();
        meteoGridLegend->setVisible(false);
    }

    void MainWindow::on_actionNetCDF_Close_triggered()
    {
        closeNetCDF();
    }

    void MainWindow::on_actionNetCDF_ShowMetadata_triggered()
    {
        if (! myProject.netCDF.isLoaded())
        {
            myProject.logError("Open NetCDF grid before.");
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

    // extract data series
    void MainWindow::netCDF_exportDataSeries(gis::Crit3DGeoPoint geoPoint)
    {
        if (myProject.netCDF.isPointInside(geoPoint))
        {
            int idVar;
            QDateTime firstTime, lastTime;

            if (chooseNetCDFVariable(&(myProject.netCDF), &idVar, &firstTime, &lastTime))
            {
                std::stringstream buffer;
                if (! myProject.netCDF.exportDataSeries(idVar, geoPoint, getCrit3DTime(firstTime), getCrit3DTime(lastTime), &buffer))
                    QMessageBox::information(nullptr, "ERROR", QString::fromStdString(buffer.str()));
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

#endif


void MainWindow::drawMeteoPoints()
{
    resetMeteoPointsMarker();
    if (! myProject.meteoPointsLoaded || myProject.nrMeteoPoints == 0) return;
    addMeteoPoints();

    myProject.loadMeteoPointsData (myProject.getCurrentDate(), myProject.getCurrentDate(), true, true, true);

    ui->meteoPoints->setEnabled(true);
    ui->meteoPoints->setChecked(true);
    showPointsGroup->setEnabled(true);
    ui->actionShowPointsCurrent->setEnabled(false);
    ui->actionShowPointsElab->setEnabled(false);
    ui->actionShowPointsClimate->setEnabled(false);

    ui->actionMeteopointRectangleSelection->setEnabled(true);
    ui->menuActive_points->setEnabled(true);
    ui->menuDeactive_points->setEnabled(true);
    ui->menuDelete_points->setEnabled(true);
    ui->menuDelete_data->setEnabled(true);
    ui->actionShift_data->setEnabled(true);
    ui->actionMeteopointDataCount->setEnabled(true);

    ui->grid->setChecked(false);

    if (currentPointsVisualization == notShown) currentPointsVisualization = showLocation;
    redrawMeteoPoints(currentPointsVisualization, true);

    updateDateTime();
}

void MainWindow::redrawMeteoPoints(visualizationType showType, bool updateColorScale)
{
    currentPointsVisualization = showType;
    ui->groupBoxElab->hide();

    if (pointList.size() == 0) return;

    // hide all meteo points
    for (int i = 0; i < myProject.nrMeteoPoints; i++)
        pointList[i]->setVisible(false);

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
                }
                else
                {
                    if (myProject.meteoPoints[i].active)
                    {
                        pointList[i]->setFillColor(QColor(Qt::white));
                    }
                    else
                    {
                        pointList[i]->setFillColor(QColor(Qt::red));
                    }
                }

                pointList[i]->setRadius(5);
                pointList[i]->setCurrentValue(NODATA);
                pointList[i]->setToolTip();

                // hide not active points
                bool isVisible = (myProject.meteoPoints[i].active || viewNotActivePoints);
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
            checkData(myProject.quality, myProject.getCurrentVariable(), myProject.meteoPoints,
                      myProject.nrMeteoPoints, myProject.getCrit3DCurrentTime(), &myProject.qualityInterpolationSettings, myProject.meteoSettings,
                      &(myProject.climateParameters), myProject.checkSpatialQuality);

            if (updateColorScale)
            {
                float minimum, maximum;
                myProject.getMeteoPointsRange(minimum, maximum, viewNotActivePoints);

                myProject.meteoPointsColorScale->setRange(minimum, maximum);
            }

            roundColorScale(myProject.meteoPointsColorScale, 4, true);
            setColorScale(myProject.getCurrentVariable(), myProject.meteoPointsColorScale);

            Crit3DColor *myColor;
            for (int i = 0; i < myProject.nrMeteoPoints; i++)
            {
                if (int(myProject.meteoPoints[i].currentValue) != NODATA)
                {
                    if (myProject.meteoPoints[i].quality == quality::accepted)
                    {
                        pointList[i]->setRadius(5);
                        myColor = myProject.meteoPointsColorScale->getColor(myProject.meteoPoints[i].currentValue);
                        pointList[i]->setFillColor(QColor(myColor->red, myColor->green, myColor->blue));
                        pointList[i]->setOpacity(1.0);
                    }
                    else
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
                    bool isVisible = (myProject.meteoPoints[i].active || viewNotActivePoints);
                    pointList[i]->setVisible(isVisible);
                }
            }

            meteoPointsLegend->update();
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

    /*
    if (myProject.meteoGridDbHandler!= nullptr)
    {
        this->ui->actionShowGridHide->setChecked(true);
        myProject.meteoGridDbHandler->meteoGrid()->fillMeteoRasterNoData();
        meteoGridObj->setDrawBorders(false);
        meteoGridLegend->setVisible(false);
        meteoGridObj->redrawRequested();
    }*/
}

bool MainWindow::loadMeteoPoints(QString dbName)
{
    if (myProject.loadMeteoPointsDB(dbName))
    {
        drawMeteoPoints();
        return true;
    }
    else
        return false;
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

    this->ui->meteoPoints->setChecked(false);
    this->ui->grid->setEnabled(true);
    this->ui->grid->setChecked(true);
    showGridGroup->setEnabled(true);
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

    if (currentGridVisualization == notShown) currentGridVisualization = showLocation;
    redrawMeteoGrid(currentGridVisualization, false);

    updateDateTime();

    updateMaps();

}



void MainWindow::redrawMeteoGrid(visualizationType showType, bool showInterpolationResult)
{
    currentGridVisualization = showType;
    ui->groupBoxElab->hide();

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

                Crit3DTime time = myProject.getCrit3DCurrentTime();

                if (frequency == daily)
                    myProject.meteoGridDbHandler->meteoGrid()->fillCurrentDailyValue(time.date, variable, myProject.meteoSettings);
                else if (frequency == hourly)
                    myProject.meteoGridDbHandler->meteoGrid()->fillCurrentHourlyValue(time.date, time.getHour(), time.getMinutes(), variable);
                else if (frequency == monthly)
                    myProject.meteoGridDbHandler->meteoGrid()->fillCurrentMonthlyValue(time.date, variable);
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
            showElabResult(true, true, false, false, false, nullptr);
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

    /*
    if (myProject.nrMeteoPoints != 0)
    {
        // hide all meteo points
        for (int i = 0; i < myProject.nrMeteoPoints; i++)
            pointList[i]->setVisible(false);
        meteoPointsLegend->setVisible(false);
        this->ui->actionShowPointsHide->setChecked(true);
    }*/

    meteoGridObj->redrawRequested();
}


bool MainWindow::loadMeteoGrid(QString xmlName)
{
    #ifdef NETCDF
        closeNetCDF();
    #endif

    if (myProject.loadMeteoGridDB(xmlName))
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
        StationMarker* point = new StationMarker(5.0, true, QColor((Qt::white)), this->mapView);

        point->setFlag(MapGraphicsObject::ObjectIsMovable, false);
        point->setLatitude(myProject.meteoPoints[i].latitude);
        point->setLongitude(myProject.meteoPoints[i].longitude);
        point->setId(myProject.meteoPoints[i].id);
        point->setName(myProject.meteoPoints[i].name);
        point->setDataset(myProject.meteoPoints[i].dataset);
        point->setAltitude(myProject.meteoPoints[i].point.z);
        point->setMunicipality(myProject.meteoPoints[i].municipality);
        point->setCurrentValue(myProject.meteoPoints[i].currentValue);
        point->setQuality(myProject.meteoPoints[i].quality);
        if (!myProject.meteoPoints[i].active)
        {
            point->setActive(false);
            point->setFillColor(QColor(Qt::red));
            if (viewNotActivePoints)
                point->setVisible(true);
            else
                point->setVisible(false);
        }

        this->pointList.append(point);
        this->mapView->scene()->addObject(this->pointList[i]);

        point->setToolTip();
        connect(point, SIGNAL(newStationClicked(std::string, std::string, bool)), this, SLOT(callNewMeteoWidget(std::string, std::string, bool)));
        connect(point, SIGNAL(appendStationClicked(std::string, std::string, bool)), this, SLOT(callAppendMeteoWidget(std::string, std::string, bool)));
    }
}

void MainWindow::callNewMeteoWidget(std::string id, std::string name, bool isGrid)
{
    bool isAppend = false;
    if (isGrid)
    {
        myProject.showMeteoWidgetGrid(id, isAppend);
    }
    else
    {
        myProject.showMeteoWidgetPoint(id, name, isAppend);
    }
    return;
}

void MainWindow::callAppendMeteoWidget(std::string id, std::string name, bool isGrid)
{
    bool isAppend = true;
    if (isGrid)
    {
        myProject.showMeteoWidgetGrid(id, isAppend);
    }
    else
    {
        myProject.showMeteoWidgetPoint(id, name, isAppend);
    }
    return;
}

void MainWindow::on_rasterScaleButton_clicked()
{
    if (this->rasterObj->getRaster() == nullptr)
    {
        QMessageBox::information(nullptr, "No Raster", "Load raster before");
        return;
    }

    meteoVariable myVar = chooseColorScale();
    if (myVar != noMeteoVar)
    {
        setColorScale(myVar, this->rasterObj->getRaster()->colorScale);
        ui->labelRasterScale->setText(QString::fromStdString(getVariableString(myVar)));
    }
}

void MainWindow::on_variableButton_clicked()
{
    meteoVariable myVar = chooseMeteoVariable(&myProject);
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
   frequencyType myFrequency = chooseFrequency(&myProject);

   if (myFrequency != noFrequency)
   {
       myProject.setCurrentFrequency(myFrequency);
       this->updateVariable();

       if (myProject.getCurrentVariable() != noMeteoVar)
       {
           this->ui->actionShowPointsCurrent->setEnabled(true);
           this->ui->actionShowGridCurrent->setEnabled(true);
           redrawMeteoPoints(showCurrentVariable, true);
           redrawMeteoGrid(showCurrentVariable, false);
       }
   }
}

void MainWindow::on_rasterRestoreButton_clicked()
{
    if (this->rasterObj->getRaster() == nullptr)
    {
        QMessageBox::information(nullptr, "No Raster", "Load raster before");
        return;
    }

    setDefaultDEMScale(myProject.DEM.colorScale);
    this->setCurrentRaster(&(myProject.DEM));
    ui->labelRasterScale->setText(QString::fromStdString(getVariableString(noMeteoTerrain)));
}


void MainWindow::setCurrentRaster(gis::Crit3DRasterGrid *myRaster)
{
    this->rasterObj->initializeUTM(myRaster, myProject.gisSettings, false);
    this->rasterLegend->colorScale = myRaster->colorScale;
    this->rasterObj->redrawRequested();
}


void MainWindow::on_dateEdit_dateChanged(const QDate &date)
{
    Q_UNUSED(date)
    this->on_dateChanged();
}

void MainWindow::on_actionElaboration_triggered()
{

    if (!ui->meteoPoints->isChecked() && !ui->grid->isChecked())
    {
        myProject.errorString = "Load meteo Points or grid";
        myProject.logError();
        return;
    }

    bool isMeteoGrid = ui->grid->isChecked();
    bool isAnomaly = false;
    bool saveClima = false;

    if (myProject.elaborationCheck(isMeteoGrid, isAnomaly))
    {
        DialogMeteoComputation compDialog(myProject.pragaDefaultSettings, isMeteoGrid, isAnomaly, saveClima);
        if (compDialog.result() != QDialog::Accepted)
            return;

        if (!myProject.elaboration(isMeteoGrid, isAnomaly, saveClima))
        {
            myProject.logError();
        }
        else
        {
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
        if (compDialog.result() == QDialog::Accepted)
            on_actionElaboration_triggered();
    }
    else
    {
         myProject.logError();
    }
    return;

}

void MainWindow::on_actionAnomaly_triggered()
{

    if (!ui->meteoPoints->isChecked() && !ui->grid->isChecked())
    {
        myProject.errorString = "Load meteo Points or grid";
        myProject.logError();
        return;
    }

    bool isMeteoGrid = ui->grid->isChecked();

    bool isAnomaly = true;
    bool saveClima = false;

    if (myProject.elaborationCheck(isMeteoGrid, isAnomaly))
    {
        DialogMeteoComputation compDialog(myProject.pragaDefaultSettings, isMeteoGrid, isAnomaly, saveClima);
        if (compDialog.result() != QDialog::Accepted)
            return;

        isAnomaly = false;

        bool res = myProject.elaboration(isMeteoGrid, isAnomaly, saveClima);
        if (!res)
        {
            myProject.logError();
        }
        else
        {
            isAnomaly = true;

            myProject.elaboration(isMeteoGrid, isAnomaly, saveClima);
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
    }
    else
    {
         myProject.logError();
    }
    return;
}

void MainWindow::on_actionClimate_triggered()
{
    if (!ui->meteoPoints->isChecked() && !ui->grid->isChecked())
    {
        myProject.errorString = "Load meteo Points or grid";
        myProject.logError();
        return;
    }

    bool isMeteoGrid = ui->grid->isChecked();
    bool isAnomaly = false;
    bool saveClima = true;

    if (myProject.elaborationCheck(isMeteoGrid, isAnomaly))
    {
        myProject.clima->resetListElab();
        DialogMeteoComputation compDialog(myProject.pragaDefaultSettings, isMeteoGrid, isAnomaly, saveClima);
        if (compDialog.result() != QDialog::Accepted)
            return;

        myProject.clima->getListElab()->setListClimateElab(compDialog.getElabSaveList());
        if (!myProject.elaboration(isMeteoGrid, isAnomaly, saveClima))
        {
            myProject.logError();
        }

        if (compDialog.result() == QDialog::Accepted)
            on_actionClimate_triggered();

    }
    else
    {
         myProject.logError();
    }

    return;
}

void MainWindow::on_actionClimateFields_triggered()
{
    if (!ui->meteoPoints->isChecked() && !ui->grid->isChecked())
    {
        myProject.errorString = "Load meteo Points or grid";
        myProject.logError();
        return;
    }

    bool isMeteoGrid = ui->grid->isChecked();
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
                myProject.saveClimateResult(isMeteoGrid, climaSelected, index.toInt(), true);
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
                myProject.deleteClima(isMeteoGrid, climaSelected);
            }

        }
        else
        {
            return;
        }

    }
    return;

}

void MainWindow::showElabResult(bool updateColorSCale, bool isMeteoGrid, bool isAnomaly, bool isAnomalyPerc, bool isClima, QString index)
{

    if (isMeteoGrid)
    {
        meteoGridObj->setDrawBorders(false);
        if (!isAnomaly)
        {
            if (!isClima)
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
                if (!isAnomaly)
                {
                    if (!isClima)
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

                float v = myProject.meteoPoints[i].currentValue;

                if (int(v) != NODATA)
                {
                    if (int(minimum) == NODATA)
                    {
                        minimum = v;
                        maximum = v;
                    }
                    else if (v < minimum) minimum = v;
                    else if (v > maximum) maximum = v;
                }

            }
            myProject.meteoPointsColorScale->setRange(minimum, maximum);
            roundColorScale(myProject.meteoPointsColorScale, 4, true);
            setColorScale(myProject.clima->variable(), myProject.meteoPointsColorScale);
        }


        Crit3DColor *myColor;
        for (int i = 0; i < myProject.nrMeteoPoints; i++)
        {

            if (!updateColorSCale)
            {
                if (!isAnomaly)
                {
                    if (!isClima)
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
                pointList[i]->setVisible(true);
            }
        }

        meteoPointsLegend->update();

    }


    if (int(myProject.clima->param1()) != NODATA)
    {
        if (!isAnomaly)
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
        if (!isAnomaly)
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
    std::string var = MapDailyMeteoVarToString.at(myProject.clima->variable());
    ui->lineEditVariable->setText(QString::fromStdString(var));
    QString startDay = QString::number(myProject.clima->genericPeriodDateStart().day());
    QString startMonth = QString::number(myProject.clima->genericPeriodDateStart().month());
    QString endDay = QString::number(myProject.clima->genericPeriodDateEnd().day());
    QString endMonth = QString::number(myProject.clima->genericPeriodDateEnd().month());

    QString startYear = QString::number(myProject.clima->yearStart());
    QString endYear = QString::number(myProject.clima->yearEnd());
    if (!isClima)
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
    ui->groupBoxElab->show();


}

void MainWindow::on_actionInterpolationSettings_triggered()
{
    DialogInterpolation* myDialogInterpolation = new DialogInterpolation(&myProject);
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
    myProject.loadTopographicDistanceMaps(true);
}

void MainWindow::on_meteoPoints_clicked()
{
    redrawMeteoPoints(currentPointsVisualization, true);
}

void MainWindow::on_grid_clicked()
{
    redrawMeteoGrid(currentGridVisualization, false);
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
    // deselect all menu
    ui->actionSettingsMapOpenStreetMap->setChecked(false);
    ui->actionSettingsMapStamenTerrain->setChecked(false);
    ui->actionSettingsMapEsriSatellite->setChecked(false);
    ui->actionSettingsMapGoogleMap->setChecked(false);
    ui->actionSettingsMapGoogleSatellite->setChecked(false);
    ui->actionSettingsMapGoogleHybrid->setChecked(false);
    ui->actionSettingsMapGoogleTerrain->setChecked(false);

    // select menu
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


bool MainWindow::openRaster(QString fileName, gis::Crit3DRasterGrid *myRaster)
{

        std::string* myError = new std::string();
        std::string fnWithoutExt = fileName.left(fileName.length()-4).toStdString();

        if (! gis::readEsriGrid(fnWithoutExt, myRaster, myError))
        {
            qDebug("Load raster failed!");
            return (false);
        }
        return true;
}

bool MainWindow::openShape(QString fileName)
{
// TO DO
    return false;
}


bool MainWindow::on_actionAnalysisAggregateFromGrid_triggered()
{
    if (!ui->grid->isChecked())
    {
        myProject.errorString = "Load grid";
        myProject.logError();
        return false;
    }
    if (myProject.aggregationDbHandler == nullptr)
    {
        myProject.errorString = "Missing DB: open or create a Aggregation DB";
        myProject.logError();
        return false;
    }
    QString rasterName;
    if (!myProject.aggregationDbHandler->getRasterName(&rasterName))
    {
        myProject.errorString = "Missing Raster Name inside aggregation db";
        myProject.logError();
        return false;
    }

    QFileInfo rasterFileFltInfo(myProject.aggregationPath+"/"+rasterName+".flt");
    QFileInfo rasterFileHdrInfo(myProject.aggregationPath+"/"+rasterName+".hdr");
    if (!rasterFileFltInfo.exists() || !rasterFileHdrInfo.exists())
    {
        myProject.errorString = "Raster file does not exist: " + myProject.aggregationPath+"/"+rasterName;
        myProject.logError();
        return false;
    }
    gis::Crit3DRasterGrid *myRaster;
    myRaster = new(gis::Crit3DRasterGrid);
    if (!openRaster(myProject.aggregationPath+"/"+rasterName+".flt", myRaster))
    {
        myProject.errorString = "Open raster file failed";
        myProject.logError();
        return false;
    }

    QList<QString> aggregation = myProject.aggregationDbHandler->getAggregations();
    if (aggregation.isEmpty())
    {
        myProject.errorString = "Empty aggregation " + myProject.aggregationDbHandler->error();
        myProject.logError();
        return false;
    }

    DialogSeriesOnZones zoneDialog(myProject.pragaDefaultSettings, aggregation);

    if (zoneDialog.result() != QDialog::Accepted)
    {
        if (myRaster != nullptr)
        {
            delete myRaster;
        }

        return false;
    }
    else
    {
        std::vector<float> outputValues;
        float threshold = NODATA;
        meteoComputation elab1MeteoComp = noMeteoComp;
        QString periodType = "D";
        if (!myProject.averageSeriesOnZonesMeteoGrid(zoneDialog.getVariable(), elab1MeteoComp, zoneDialog.getSpatialElaboration(), threshold, myRaster, zoneDialog.getStartDate(), zoneDialog.getEndDate(), periodType, outputValues, true))
        {
            QMessageBox::information(nullptr, "Error", "Error writing aggregation data");
            if (myRaster != nullptr)
            {
                delete myRaster;
            }
            return false;
        }
    }
    if (myRaster != nullptr)
    {
        delete myRaster;
    }

    return true;
}


void MainWindow::on_actionAnalysisOpenAggregationDB_triggered()
{
    QString dbName = QFileDialog::getOpenFileName(this, tr("Open DB meteo points"), "", tr("DB files (*.db)"));
    if (dbName != "")
    {
        myProject.loadAggregationdDB(dbName);
    }

}

void MainWindow::on_actionAnalysisNewAggregationDB_triggered()
{
    QString templateFileName = myProject.getDefaultPath() + PATH_TEMPLATE + "template_meteo_aggregation.db";

    QString dbName = QFileDialog::getSaveFileName(this, tr("Save as"), "", tr("DB files (*.db)"));
    if (dbName == "")
    {
        qDebug() << "missing new db file name";
        return;
    }

    QFile dbFile(dbName);
    QFileInfo dbFileInfo(dbFile.fileName());
    if (dbFile.exists())
    {
        if (!dbFile.remove())
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
    if (rasterName == "" || !rasterName.contains(".flt"))
    {
        QMessageBox::information(nullptr, "No Raster ", "Load raster before");
        return;
    }
    QFileInfo rasterFileInfo(rasterName);

    if (dbFileInfo.absolutePath() != rasterFileInfo.absolutePath())
    {
        QMessageBox::information(nullptr, "Raster will be copied at db path", "Raster and db should be in the same folder");
        QString rasterCopiedFlt = dbFileInfo.absolutePath()+"/"+rasterFileInfo.baseName()+".flt";
        QString rasterCopiedHdr = dbFileInfo.absolutePath()+"/"+rasterFileInfo.baseName()+".hdr";
        QString rasterHdr = rasterFileInfo.absolutePath()+"/"+rasterFileInfo.baseName()+".hdr";
        if (!QFile::copy(rasterName, rasterCopiedFlt) || !QFile::copy(rasterHdr, rasterCopiedHdr))
        {
            myProject.logError("Copy raster failed: " + rasterName);
            return;
        }
        QMessageBox::information(nullptr, "Copied", "Successfully completed");
    }
    if (!myProject.aggregationDbHandler->writeRasterName(rasterFileInfo.baseName()))
    {
        myProject.logError("Writing raster name failed");
        return;
    }
}

void MainWindow::redrawTitle()
{
    QString title = "PRAGA";
    if (myProject.projectName != "")
        title += " - " + myProject.projectName;

    this->setWindowTitle(title);
}

void MainWindow::drawProject()
{
    mapView->centerOn(startCenter->lonLat());

    if (myProject.DEM.isLoaded)
        renderDEM();

    drawMeteoPoints();
    drawMeteoGrid();

    redrawTitle();
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
        QMessageBox::information(nullptr, "Could not open project", myProject.errorString);

        this->mapView->centerOn(startCenter->lonLat());
        if (myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini")) drawProject();
    }

    checkSaveProject();
}

void MainWindow::on_actionFileCloseProject_triggered()
{
    if (! myProject.isProjectLoaded) return;

    on_actionFileMeteogridClose_triggered();
    on_actionFileMeteopointClose_triggered();
    this->ui->labelFrequency->setText("None");
    this->ui->labelVariable->setText(("None"));

    clearDEM();

    this->mapView->centerOn(startCenter->lonLat());

    if (! myProject.loadPragaProject(myProject.getApplicationPath() + "default.ini")) return;

    drawProject();
    checkSaveProject();
}

void MainWindow::on_actionFileSaveProjectAs_triggered()
{
    DialogPragaProject* myProjectDialog = new DialogPragaProject(&myProject);
    myProjectDialog->exec();
    myProjectDialog->close();

    redrawTitle();
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
    myProject.logInfoGUI("Interpolating on DEM...");
    interpolateDemGUI();
    myProject.closeLogInfo();
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
    // check meteo point
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError("No meteo points DB open");
        return;
    }

    // check meteo grid
    if (! myProject.meteoGridLoaded || myProject.meteoGridDbHandler == nullptr)
    {
        myProject.logError("No meteo grid DB open");
        return;
    }

    QDateTime myFirstTime = myProject.findDbPointFirstTime();
    QDateTime myLastTime = myProject.findDbPointLastTime();
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

    meteoVariable myVar = chooseMeteoVariable(&myProject);
    if (myVar == noMeteoVar) return;

    QList <meteoVariable> myVariables, aggrVariables;
    myVariables.push_back(myVar);
    myProject.interpolationMeteoGridPeriod(myFirstTime.date(), myLastTime.date(), myVariables, aggrVariables, false, 1);
}

void MainWindow::on_actionFileMeteopointNewArkimet_triggered()
{
    resetMeteoPointsMarker();

    QString templateFileName = myProject.getDefaultPath() + PATH_TEMPLATE + "template_meteo_arkimet.db";

    QString dbName = QFileDialog::getSaveFileName(this, tr("Save as"), "", tr("DB files (*.db)"));
    if (dbName == "")
    {
        qDebug() << "missing new db file name";
        return;
    }

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

    QList<QString> dataset = myDownload.getDbArkimet()->getDatasetsList();

    QDialog datasetDialog;

    datasetDialog.setWindowTitle("Datasets");
    datasetDialog.setFixedWidth(500);
    QVBoxLayout layout;

    for (int i = 0; i < dataset.size(); i++)
    {
        QCheckBox* dat = new QCheckBox(dataset[i]);
        layout.addWidget(dat);

        datasetCheckbox.append(dat);
    }

    all = new QCheckBox("ALL");
    layout.addSpacing(30);
    layout.addWidget(all);

    connect(all, SIGNAL(toggled(bool)), this, SLOT(enableAllDataset(bool)));

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
        if (myDownload.getPointProperties(datasets))
            loadMeteoPoints(dbName);
        else
            myProject.logError("Error in function getPointProperties");

        myProject.closeLogInfo();
    }
    else
    {
        QFile::remove(dbName);
    }

    delete buttonBox;
    delete all;
}

void MainWindow::on_actionFileMeteopointOpen_triggered()
{
    QString dbName = QFileDialog::getOpenFileName(this, tr("Open DB meteo points"), "", tr("DB files (*.db)"));
    if (dbName != "")
    {
        closeMeteoPoints();
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
        resetMeteoPointsMarker();
        meteoPointsLegend->setVisible(false);

        myProject.closeMeteoPointsDB();

        myProject.setIsElabMeteoPointsValue(false);
        ui->groupBoxElab->hide();

        this->ui->meteoPoints->setChecked(false);
        this->ui->meteoPoints->setEnabled(false);
        ui->actionMeteopointRectangleSelection->setEnabled(false);
        ui->menuActive_points->setEnabled(false);
        ui->menuDeactive_points->setEnabled(false);
        ui->menuDelete_points->setEnabled(false);
        ui->menuDelete_data->setEnabled(false);
        ui->actionShift_data->setEnabled(false);
        ui->actionMeteopointDataCount->setEnabled(false);

        showPointsGroup->setEnabled(false);
        this->ui->menuShowPointsAnomaly->setEnabled(false);

        if (myProject.meteoGridDbHandler != nullptr)
        {
            this->ui->grid->setChecked(true);
        }
    }
}

void MainWindow::on_actionFileMeteogridOpen_triggered()
{
    QString xmlName = QFileDialog::getOpenFileName(this, tr("Open XML DB meteo grid"), "", tr("xml files (*.xml)"));
    if (xmlName != "")
    {
        closeMeteoGrid();
        loadMeteoGrid(xmlName);
    }
}


void MainWindow::closeMeteoGrid()
{
    if (myProject.meteoGridDbHandler != nullptr)
    {

        if (myProject.meteoGridDbHandler != nullptr)
        {
            myProject.meteoGridDbHandler->meteoGrid()->dataMeteoGrid.isLoaded = false;
            meteoGridObj->clear();
            meteoGridObj->redrawRequested();
            meteoGridLegend->setVisible(false);
            myProject.closeMeteoGridDB();
            ui->groupBoxElab->hide();
            ui->meteoGridOpacitySlider->setEnabled(false);

            this->ui->grid->setChecked(false);
            this->ui->grid->setEnabled(false);

            showGridGroup->setEnabled(false);
            this->ui->menuShowGridAnomaly->setEnabled(false);

            if (myProject.meteoPointsDbHandler != nullptr)
            {
                this->ui->meteoPoints->setChecked(true);
            }
        }
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
        QMessageBox::critical(nullptr, "Data count", "No meteo points DB open");
        return;
    }

    meteoVariable myVar = chooseMeteoVariable(&myProject);
    if (myVar == noMeteoVar) return;
    frequencyType myFreq = getVarFrequency(myVar);

    QDateTime myFirstTime = myProject.findDbPointFirstTime();
    QDateTime myLastTime = myProject.findDbPointLastTime();
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

    if (myProject.dbMeteoPointDataCount(myFirstTime.date(), myLastTime.date(), myVar, dataset, myCounter))
    {
        QFile myFile(myFilename);
        if (myFile.open(QIODevice::ReadWrite))
        {
            QTextStream outStream(&myFile);

            QDate myDate = myFirstTime.date();
            short myHour;
            long i=0;

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
        QMessageBox::critical(nullptr, "Missing data finder", "No meteo grid DB open");
        return;
    }

    meteoVariable myVar = chooseMeteoVariable(&myProject);
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
        myProject.logError("Open a meteo grid DB before");
        return;
    }

    bool isGrid = true;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("xml files (*.xml)"));
    if (fileName.isEmpty())
        return;


    if (!myProject.parserXMLImportData(fileName, isGrid))
    {
        return;
    }

    QList<QString> dateFiles = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more files to open",
                            "",
                            "Files (*.prn *.csv)");
    if (dateFiles.isEmpty())
        return;

    myProject.setProgressBar("Loading data...", dateFiles.size());
    QString warning;

    for (int i=0; i<dateFiles.size(); i++)
    {
        myProject.updateProgressBar(i);
        if (myProject.loadXMLImportData(dateFiles[i]))
        {
            if (!myProject.errorString.isEmpty())
            {
                warning += dateFiles[i] + ": " + myProject.errorString+"\n";
            }
        }
        else
        {
            if (i!=dateFiles.size()-1)
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
    if (!warning.isEmpty())
    {
        QMessageBox::warning(nullptr, " Not valid values: ", warning);
    }
    QString xmlName = myProject.meteoGridDbHandler->fileName();
    closeMeteoGrid();
    loadMeteoGrid(xmlName);
}





void MainWindow::on_actionPointProperties_import_triggered()
{
    // check meteo point
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError("Open a meteo points DB before");
        return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("csv files (*.csv)"));
    if (fileName.isEmpty())
        return;

    QList<QString> pointPropertiesList;
    if (!myProject.meteoPointsDbHandler->getNameColumn("point_properties", &pointPropertiesList))
    {
        myProject.logError("point_properties table error");
        return;
    }
    QList<QString> csvFields;
    if (!myProject.parseMeteoPointsPropertiesCSV(fileName, &csvFields))
    {
        return;
    }

    DialogPointProperties dialogPointProp(pointPropertiesList, csvFields);
    if (dialogPointProp.result() != QDialog::Accepted)
    {
        return;
    }
    else
    {
        QList<QString> joinedList = dialogPointProp.getJoinedList();

        myProject.logInfoGUI("Loading data...");
        if (!myProject.writeMeteoPointsProperties(joinedList))
        {
            myProject.closeLogInfo();
            return;
        }
        myProject.closeLogInfo();
    }
}

void MainWindow::on_actionPointData_import_triggered()
{
    // check meteo point
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        myProject.logError("Open a meteo points DB before");
        return;
    }

    bool isGrid = false;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("xml files (*.xml)"));
    if (fileName.isEmpty())
        return;


    if (!myProject.parserXMLImportData(fileName, isGrid))
    {
        return;
    }

    QList<QString> dateFiles = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more files to open",
                            "",
                            "Files (*.prn *.csv)");

    if (dateFiles.isEmpty())
        return;

    myProject.setProgressBar("Loading data...", dateFiles.size());
    QString warning;

    for (int i=0; i<dateFiles.size(); i++)
    {
        myProject.updateProgressBar(i);
        if (myProject.loadXMLImportData(dateFiles[i]))
        {
            if (!myProject.errorString.isEmpty())
            {
                warning += dateFiles[i] + ": " + myProject.errorString+"\n";
            }
        }
        else
        {
            if (i!=dateFiles.size()-1)
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
    if (!warning.isEmpty())
    {
        QMessageBox::warning(nullptr, " Not valid values: ", warning);
    }
    QString dbName = myProject.meteoPointsDbHandler->getDbName();
    closeMeteoPoints();
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
            bool existData = myProject.meteoPointsDbHandler->existData(&myProject.meteoPoints[i], daily) || myProject.meteoPointsDbHandler->existData(&myProject.meteoPoints[i], hourly);
            if (!existData)
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

    if (!myProject.deleteMeteoPointsData(pointList))
    {
        myProject.logError("Failed to delete data.");
    }

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
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

    if (!myProject.deleteMeteoPointsData(pointList))
    {
        myProject.logError("Failed to delete data.");
    }

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
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

    if (!myProject.deleteMeteoPointsData(pointList))
    {
        myProject.logError("Failed to delete data.");
    }

    QDate currentDate = myProject.getCurrentDate();
    myProject.loadMeteoPointsData(currentDate, currentDate, true, true, true);
}

void MainWindow::on_actionWith_Criteria_active_triggered()
{
    if (myProject.setActiveStateWithCriteria(true))
    {
        // reload meteoPoint, point properties table is changed
        QString dbName = myProject.dbPointsFileName;
        myProject.closeMeteoPointsDB();
        this->loadMeteoPoints(dbName);
    }
}

void MainWindow::on_actionWith_Criteria_notActive_triggered()
{
    if (myProject.setActiveStateWithCriteria(false))
    {
        // reload meteoPoint, point properties table is changed
        QString dbName = myProject.dbPointsFileName;
        myProject.closeMeteoPointsDB();
        this->loadMeteoPoints(dbName);
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
        QMessageBox::critical(nullptr, "proxy graph", "Proxy graph already open");
        return;
    }
    if (myProject.meteoPointsDbHandler == nullptr)
    {
        QMessageBox::critical(nullptr, "proxy graph", "No meteo points DB open");
        return;
    }

    std::vector<Crit3DProxy> proxy = myProject.interpolationSettings.getCurrentProxy();
    if (proxy.size() == 0)
    {
        QMessageBox::critical(nullptr, "proxy graph", "No proxy loaded");
        return;
    }
    return myProject.showProxyGraph();
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
    resetMeteoPointsMarker();

    QString templateFileName = myProject.getDefaultPath() + PATH_TEMPLATE + "template_meteo.db";

    QString dbName = QFileDialog::getSaveFileName(this, tr("Save as"), "", tr("DB files (*.db)"));
    if (dbName == "")
    {
        qDebug() << "missing new db file name";
        return;
    }

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
    if (!myProject.meteoPointsDbHandler->getNameColumn("point_properties", &pointPropertiesList))
    {
        myProject.logError("point_properties table error");
        return;
    }
    QList<QString> csvFields;
    if (!myProject.parseMeteoPointsPropertiesCSV(fileName, &csvFields))
    {
        return;
    }

    DialogPointProperties dialogPointProp(pointPropertiesList, csvFields);
    if (dialogPointProp.result() != QDialog::Accepted)
    {
        return;
    }
    else
    {
        QList<QString> joinedList = dialogPointProp.getJoinedList();
        myProject.logInfoGUI("Loading data...");
        if (!myProject.writeMeteoPointsProperties(joinedList))
        {
            myProject.closeLogInfo();
            return;
        }
        myProject.closeLogInfo();
    }
    loadMeteoPoints(dbName);
}
