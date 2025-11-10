/*!
    \copyright 2020 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    This file is part of AGROLIB.
    AGROLIB has been developed under contract issued by ARPAE Emilia-Romagna

    AGROLIB is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AGROLIB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with AGROLIB.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    ftomei@arpae.it
    gantolini@arpae.it
*/

#include "meteo.h"
#include "homogeneityWidget.h"
#include "utilities.h"
#include "interpolation.h"
#include "spatialControl.h"
#include "commonConstants.h"
#include "basicMath.h"
#include "climate.h"
#include "dialogChangeAxis.h"
#include "gammaFunction.h"
#include "furtherMathFunctions.h"
#include "formInfo.h"

#include <QLayout>
#include <QDate>


Crit3DHomogeneityWidget::Crit3DHomogeneityWidget(Crit3DMeteoPointsDbHandler* meteoPointsDbPointer,
                                                 QList<Crit3DMeteoPoint> &nearMeteoPointsList, const QList<std::string> &sortedIdList,
                                                 const std::vector<float>& distanceList, const QList<QString>& jointStationsList,
                                                 const QDate& firstDaily, const QDate& lastDaily, Crit3DMeteoSettings *meteoSettings,
                                                 QSettings *settings, Crit3DClimateParameters *climateParameters, Crit3DQuality *quality)
    :_meteoPointsDbPointer(meteoPointsDbPointer), _nearMeteoPointsList(nearMeteoPointsList), _sortedIdList(sortedIdList),
    _distanceList(distanceList), _firstDaily(firstDaily), _lastDaily(lastDaily), meteoSettings(meteoSettings),
    settings(settings), climateParameters(climateParameters), quality(quality)
{
    setWindowTitle("Homogeneity Test Id:" + QString::fromStdString(_nearMeteoPointsList[0].id)
                         + " " + QString::fromStdString(_nearMeteoPointsList[0].name));
    resize(1240, 700);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_DeleteOnClose);

    _jointPointsIdList << _nearMeteoPointsList[0].id;
    _jointIndexList << 0;

    for (int i = 0; i < jointStationsList.size(); i++)
    {
        std::string jointId = jointStationsList[i].toStdString();
        int jointIndex = getJointStationIndex(jointId);
        if (jointIndex == NODATA)
            continue;

        _jointPointsIdList << jointId;
        _jointIndexList << jointIndex;
    }

    // layout
    QHBoxLayout *mainLayout = new QHBoxLayout();
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QVBoxLayout *plotLayout = new QVBoxLayout();

    QHBoxLayout *firstLayout = new QHBoxLayout();
    QVBoxLayout *methodLayout = new QVBoxLayout;
    QGroupBox *methodGroupBox = new QGroupBox();
    QVBoxLayout *variableLayout = new QVBoxLayout;
    QGroupBox *variableGroupBox = new QGroupBox();

    QGroupBox *jointStationsGroupBox = new QGroupBox();
    QHBoxLayout *jointStationsLayout = new QHBoxLayout;
    QVBoxLayout *jointStationsSelectLayout = new QVBoxLayout;
    QHBoxLayout *paramtersLayout = new QHBoxLayout;

    QHBoxLayout *findStationsLayout = new QHBoxLayout();
    QVBoxLayout *selectStationsLayout = new QVBoxLayout();
    QHBoxLayout *headerLayout = new QHBoxLayout;
    QHBoxLayout *stationsLayout = new QHBoxLayout;
    QVBoxLayout *arrowLayout = new QVBoxLayout();

    QHBoxLayout *resultLayout = new QHBoxLayout();
    QGroupBox *resultGroupBox = new QGroupBox();
    resultGroupBox->setTitle("Homogeneity results");

    QLabel *methodLabel = new QLabel(tr("Method: "));
    method.setMaximumWidth(120);
    method.addItem("SNHT");
    method.addItem("CRADDOCK");
    method.setSizeAdjustPolicy(QComboBox::AdjustToContents);
    methodLayout->addWidget(methodLabel);
    methodLayout->addWidget(&method);
    methodGroupBox->setLayout(methodLayout);

    QLabel *variableLabel = new QLabel(tr("Variable: "));
    variable.addItem("DAILY_TAVG");
    variable.addItem("DAILY_PREC");
    variable.addItem("DAILY_RHAVG");
    variable.addItem("DAILY_RAD");
    variable.addItem("DAILY_W_VEC_INT_AVG");
    variable.addItem("DAILY_W_SCAL_INT_AVG");

    myVar = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, variable.currentText().toStdString());
    variable.setSizeAdjustPolicy(QComboBox::AdjustToContents);
    variable.setMaximumWidth(150);
    variableLayout->addWidget(variableLabel);
    variableLayout->addWidget(&variable);
    variableGroupBox->setLayout(variableLayout);

    QLabel *minNumStationsLabel = new QLabel(tr("Minimum number of stations: "));
    paramtersLayout->addWidget(minNumStationsLabel);
    minNumStations.setMaximumWidth(50);
    minNumStations.setMaximumHeight(24);
    minNumStations.setText("1");
    minNumStations.setValidator(new QIntValidator(1.0, 20.0));
    paramtersLayout->addWidget(&minNumStations);

    QLabel *jointStationsLabel = new QLabel(tr("Stations:"));
    jointStationsSelectLayout->addWidget(jointStationsLabel);
    jointStationsSelectLayout->addWidget(&jointStationsCombo);

    QSqlDatabase myDb = _meteoPointsDbPointer->getDb();

    for (int i = 0; i < _jointPointsIdList.size(); i++)
    {
        std::string jointId = _jointPointsIdList[i];
        int indexJoint = _jointIndexList[i];

        QDate firstDaily = _meteoPointsDbPointer->getFirstDate(daily, jointId).date();
        QDate _lastDaily = _meteoPointsDbPointer->getLastDate(daily, jointId).date();

        jointStationsSelected.addItem(QString::fromStdString(_nearMeteoPointsList[indexJoint].id) + " "
                                      + QString::fromStdString(_nearMeteoPointsList[indexJoint].name));
        _meteoPointsDbPointer->loadDailyData(myDb, getCrit3DDate(firstDaily), getCrit3DDate(_lastDaily),
                                             _nearMeteoPointsList[indexJoint]);
    }

    for (int i = 1; i < _nearMeteoPointsList.size(); i++)
    {
        jointStationsCombo.addItem(QString::fromStdString(_nearMeteoPointsList[i].id)+" "+QString::fromStdString(this->_nearMeteoPointsList[i].name));
    }

    if (jointStationsCombo.count() != 0)
    {
        addJointStation.setEnabled(true);
    }
    else
    {
        addJointStation.setEnabled(false);
    }

    QHBoxLayout *addDeleteStationLayout = new QHBoxLayout;
    addDeleteStationLayout->addWidget(&addJointStation);
    addJointStation.setText("Add");
    addJointStation.setMaximumWidth(120);
    deleteJointStation.setText("Delete");
    deleteJointStation.setMaximumWidth(120);
    saveToDb.setText("Save to DB");
    saveToDb.setMaximumWidth(120);
    saveToDb.setEnabled(false);

    if (jointStationsSelected.count() == 0)
    {
        deleteJointStation.setEnabled(false);
    }
    else
    {
        deleteJointStation.setEnabled(true);
    }
    addDeleteStationLayout->addWidget(&deleteJointStation);
    jointStationsSelectLayout->addLayout(addDeleteStationLayout);
    jointStationsSelectLayout->addWidget(&saveToDb);
    jointStationsLayout->addLayout(jointStationsSelectLayout);
    jointStationsLayout->addWidget(&jointStationsSelected);
    jointStationsGroupBox->setTitle("Joint stations");
    jointStationsGroupBox->setLayout(jointStationsLayout);

    QLabel *yearFromLabel = new QLabel(tr("From"));
    findStationsLayout->addWidget(yearFromLabel);
    yearFrom.setMaximumWidth(100);
    findStationsLayout->addWidget(&yearFrom);
    findStationsLayout->addStretch(120);
    QLabel *yearToLabel = new QLabel(tr("To"));
    findStationsLayout->addWidget(yearToLabel);
    yearTo.setMaximumWidth(100);
    findStationsLayout->addWidget(&yearTo);
    findStationsLayout->addStretch(500);
    find.setText("Find stations");
    find.setMaximumWidth(120);
    findStationsLayout->addWidget(&find);
    for(int i = 0; i <= _lastDaily.year()-firstDaily.year(); i++)
    {
        yearFrom.addItem(QString::number(firstDaily.year()+i));
        yearTo.addItem(QString::number(firstDaily.year()+i));
    }
    yearTo.setCurrentText(QString::number(_lastDaily.year()));

    QLabel *allHeader = new QLabel("Stations found");
    QLabel *selectedHeader = new QLabel("Stations selected");
    addStationFoundButton.setText("➡");
    deleteStationFoundButton.setText("⬅");
    addStationFoundButton.setEnabled(false);
    deleteStationFoundButton.setEnabled(false);
    arrowLayout->addWidget(&addStationFoundButton);
    arrowLayout->addWidget(&deleteStationFoundButton);
    listFoundStations.setSelectionMode(QAbstractItemView::ExtendedSelection);
    listSelectedStations.setSelectionMode(QAbstractItemView::ExtendedSelection);
    stationsLayout->addWidget(&listFoundStations);
    stationsLayout->addLayout(arrowLayout);
    stationsLayout->addWidget(&listSelectedStations);

    headerLayout->addWidget(allHeader);
    headerLayout->addStretch(listFoundStations.width());
    headerLayout->addWidget(selectedHeader);
    selectStationsLayout->addLayout(headerLayout);
    selectStationsLayout->addLayout(stationsLayout);

    stationsTable.setColumnCount(4);
    QList<QString> tableHeader;
    tableHeader <<"Name"<<"R^2"<<"getDistance [km]"<<"Delta Z [m]";
    stationsTable.setHorizontalHeaderLabels(tableHeader);
    stationsTable.adjustSize();
    stationsTable.horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stationsTable.resizeColumnsToContents();
    stationsTable.setSelectionBehavior(QAbstractItemView::SelectItems);
    stationsTable.setSelectionMode(QAbstractItemView::ContiguousSelection);
    selectStationsLayout->addWidget(&stationsTable);

    execute.setText("Execute");
    execute.setMaximumWidth(100);
    if (listSelectedStations.count() == 0)
    {
        execute.setEnabled(false);
    }

    resultLayout->addWidget(&execute);
    resultLayout->addWidget(&resultLabel);
    resultGroupBox->setLayout(resultLayout);

    annualSeriesChartView = new AnnualSeriesChartView();
    annualSeriesChartView->setMinimumWidth(this->width()*2/3);
    annualSeriesChartView->setYTitle(QString::fromStdString(getUnitFromVariable(myVar)));
    plotLayout->addWidget(annualSeriesChartView);

    homogeneityChartView = new HomogeneityChartView();
    homogeneityChartView->setMinimumWidth(this->width()*2/3);
    plotLayout->addWidget(homogeneityChartView);

    firstLayout->addWidget(methodGroupBox);
    firstLayout->addWidget(variableGroupBox);

    leftLayout->addLayout(firstLayout);
    leftLayout->addWidget(jointStationsGroupBox);
    leftLayout->addLayout(paramtersLayout);
    leftLayout->addLayout(findStationsLayout);
    leftLayout->addLayout(selectStationsLayout);
    leftLayout->addWidget(resultGroupBox);

    // menu
    QMenuBar* menuBar = new QMenuBar();
    QMenu *editMenu = new QMenu("Edit");

    menuBar->addMenu(editMenu);
    mainLayout->setMenuBar(menuBar);

    QAction* changeHomogeneityLeftAxis = new QAction(tr("&Change homogenity chart axis left"), this);
    QAction* exportAnnualGraph = new QAction(tr("&Export annual series graph"), this);
    QAction* exportHomogeneityGraph = new QAction(tr("&Export homogenity graph"), this);
    QAction* exportAnnualData = new QAction(tr("&Export annual series data"), this);
    QAction* exportHomogeneityData = new QAction(tr("&Export homogenity data"), this);

    editMenu->addAction(changeHomogeneityLeftAxis);
    editMenu->addAction(exportAnnualGraph);
    editMenu->addAction(exportHomogeneityGraph);
    editMenu->addAction(exportAnnualData);
    editMenu->addAction(exportHomogeneityData);

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(plotLayout);
    setLayout(mainLayout);

    connect(&variable, &QComboBox::currentTextChanged, [=](const QString &newVariable){ this->changeVar(newVariable); });
    connect(&yearFrom, &QComboBox::currentTextChanged, [=](){ this->changeYears(); });
    connect(&yearTo, &QComboBox::currentTextChanged, [=](){ this->changeYears(); });
    connect(&method, &QComboBox::currentTextChanged, [=](const QString &newMethod){ this->changeMethod(newMethod); });
    connect(&addJointStation, &QPushButton::clicked, [=](){ addJointStationClicked(); });
    connect(&deleteJointStation, &QPushButton::clicked, [=](){ deleteJointStationClicked(); });
    connect(&saveToDb, &QPushButton::clicked, [=](){ saveToDbClicked(); });
    connect(&find, &QPushButton::clicked, [=](){ findReferenceStations(); });
    connect(&addStationFoundButton, &QPushButton::clicked, [=](){ addFoundStationClicked(); });
    connect(&deleteStationFoundButton, &QPushButton::clicked, [=](){ deleteFoundStationClicked(); });
    connect(&execute, &QPushButton::clicked, [=](){ executeClicked(); });
    connect(changeHomogeneityLeftAxis, &QAction::triggered, this, &Crit3DHomogeneityWidget::on_actionChangeLeftAxis);
    connect(exportAnnualGraph, &QAction::triggered, this, &Crit3DHomogeneityWidget::on_actionExportAnnualGraph);
    connect(exportHomogeneityGraph, &QAction::triggered, this, &Crit3DHomogeneityWidget::on_actionExportHomogeneityGraph);
    connect(exportAnnualData, &QAction::triggered, this, &Crit3DHomogeneityWidget::on_actionExportAnnualData);
    connect(exportHomogeneityData, &QAction::triggered, this, &Crit3DHomogeneityWidget::on_actionExportHomogeneityData);

    plotAnnualSeries();

    show();
}


Crit3DHomogeneityWidget::~Crit3DHomogeneityWidget()
{

}

void Crit3DHomogeneityWidget::closeEvent(QCloseEvent *event)
{
    event->accept();
}


int Crit3DHomogeneityWidget::getJointStationIndex(const std::string& id)
{
    for (int i = 0; i < _nearMeteoPointsList.size(); ++i)
    {
        if (_nearMeteoPointsList[i].id == id)
            return i;
    }

    return NODATA;
}


void Crit3DHomogeneityWidget::plotAnnualSeries()
{
    annualSeriesChartView->clearSeries();
    _annualSeries.clear();
    int firstYear = yearFrom.currentText().toInt();
    int lastYear = yearTo.currentText().toInt();
    QDate firstDate(firstYear, 1, 1);
    QDate lastDate(lastYear, 12, 31);

    _climate.setVariable(myVar);
    if (myVar == dailyPrecipitation || myVar == dailyReferenceEvapotranspirationHS || myVar == dailyReferenceEvapotranspirationPM || myVar == dailyBIC)
    {
        _climate.setElab1("sum");
    }
    else
    {
        _climate.setElab1("average");
    }
    _climate.setYearStart(firstYear);
    _climate.setYearEnd(lastYear);
    _climate.setGenericPeriodDateStart(firstDate);
    _climate.setGenericPeriodDateEnd(lastDate);
    _climate.setNYears(0);

    std::vector<int> years;

    FormInfo formInfo;
    formInfo.showInfo("compute annual series...");

    // copy all data to meteoPointTemp from joint if there are holes
    Crit3DMeteoPoint meteoPointTemp;
    if (_jointPointsIdList.size() != 1)
    {
        int numberOfDays = firstDate.daysTo(lastDate)+1;
        meteoPointTemp.initializeObsDataD(numberOfDays, getCrit3DDate(firstDate));
        for (QDate myDate = firstDate; myDate <= lastDate; myDate = myDate.addDays(1) )
        {
            checkValueAndMerge(_nearMeteoPointsList[0], &meteoPointTemp, myDate);
        }
    }
    else
    {
        meteoPointTemp = _nearMeteoPointsList[0];
    }

    bool dataAlreadyLoaded = true;
    bool isAnomaly = false;
    std::vector<int> vectorYears;
    int validYears = computeAnnualSeriesOnPointFromDaily(_meteoPointsDbPointer, nullptr,
                                                         &meteoPointTemp, meteoSettings, _climate, false,
                                                         isAnomaly, dataAlreadyLoaded,
                                                         _annualSeries, vectorYears, myError);
    formInfo.close();

    if (validYears > 0)
    {
        double sum = 0;
        int count = 0;
        int validData = 0;
        int yearsLength = lastYear - firstYear;
        int nYearsToAdd = 0;
        std::vector<float> seriesToView = _annualSeries;
        if (yearsLength > 15)
        {
            for (int inc = 0; inc<=3; inc++)
            {
                if ( (yearsLength+inc) % 2 == 0 &&  (yearsLength+inc)/2 <= 15)
                {
                    nYearsToAdd = inc;
                    break;
                }
                if ( (yearsLength+inc) % 3 == 0 &&  (yearsLength+inc)/3 <= 15)
                {
                    nYearsToAdd = inc;
                    break;
                }
                if ( (yearsLength+inc) % 4 == 0 &&  (yearsLength+inc)/4 <= 15)
                {
                    nYearsToAdd = inc;
                    break;
                }
            }
            for (int i = nYearsToAdd; i> 0; i--)
            {
                years.push_back(firstYear-i);
                seriesToView.insert(seriesToView.begin(),NODATA);
            }
        }
        for (int i = firstYear; i<=lastYear; i++)
        {
            years.push_back(i);
            if (_annualSeries[count] != NODATA)
            {
                sum += double(_annualSeries[unsigned(count)]);
                validData = validData + 1;
            }
            count = count + 1;
        }
        averageValue = sum / validYears;
        // draw
        annualSeriesChartView->draw(years, seriesToView);
        return;
    }
    else
    {
        _annualSeries.clear();
        return;
    }
}



void Crit3DHomogeneityWidget::changeVar(const QString &varName)
{
    myVar = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, varName.toStdString());
    listFoundStations.clear();
    listAllFound.clear();
    listSelectedStations.clear();
    stationsTable.clearContents();
    resultLabel.clear();
    annualSeriesChartView->setYTitle(QString::fromStdString(getUnitFromVariable(myVar)));
    execute.setEnabled(false);
    homogeneityChartView->clearSNHTSeries();
    homogeneityChartView->clearCraddockSeries();
    plotAnnualSeries();
}

void Crit3DHomogeneityWidget::changeYears()
{
    listFoundStations.clear();
    listAllFound.clear();
    listSelectedStations.clear();
    stationsTable.clearContents();
    stationsTable.setRowCount(0);
    resultLabel.clear();
    execute.setEnabled(false);
    homogeneityChartView->clearSNHTSeries();
    homogeneityChartView->clearCraddockSeries();
    plotAnnualSeries();
}

void Crit3DHomogeneityWidget::changeMethod(const QString &methodName)
{
    homogeneityChartView->clearSNHTSeries();
    homogeneityChartView->clearCraddockSeries();
    resultLabel.clear();
    if (execute.isEnabled())
    {
        executeClicked();
    }
}

void Crit3DHomogeneityWidget::addJointStationClicked()
{
    if (jointStationsCombo.currentText().isEmpty())
        return;

    std::string newId;
    QSqlDatabase myDb = _meteoPointsDbPointer->getDb();

    if (jointStationsSelected.findItems(jointStationsCombo.currentText(), Qt::MatchExactly).isEmpty())
    {
        jointStationsSelected.addItem(jointStationsCombo.currentText());
        deleteJointStation.setEnabled(true);
        saveToDb.setEnabled(true);
        newId = jointStationsCombo.currentText().section(" ",0,0).toStdString();

        int JointIndex = getJointStationIndex(newId);
        if (JointIndex != NODATA)
        {
            _jointPointsIdList << newId;
            _jointIndexList << JointIndex;

            // load all Data
            QDate firstDailyNewId = _meteoPointsDbPointer->getFirstDate(daily, newId).date();
            QDate lastDailyNewId = _meteoPointsDbPointer->getLastDate(daily, newId).date();

            _meteoPointsDbPointer->loadDailyData(myDb, getCrit3DDate(firstDailyNewId), getCrit3DDate(lastDailyNewId), _nearMeteoPointsList[JointIndex]);
            updateYears();
        }
    }

}

void Crit3DHomogeneityWidget::deleteJointStationClicked()
{
    QList<QListWidgetItem*> items = jointStationsSelected.selectedItems();
    if (items.isEmpty())
        return;

    foreach(QListWidgetItem * item, items)
    {
        std::string jointId = item->text().section(" ", 0, 0).toStdString();
        int index = NODATA;
        for (int i = 0; i < _jointPointsIdList.size(); ++i) {
            if (_jointPointsIdList[i] == jointId)
            {
                index = i;
                break;
            }
        }
        if (index != NODATA)
        {
            _jointPointsIdList.removeAt(index);
            _jointIndexList.removeAt(index);
            delete jointStationsSelected.takeItem(jointStationsSelected.row(item));
        }
    }

    saveToDb.setEnabled(true);
    updateYears();
}


void Crit3DHomogeneityWidget::saveToDbClicked()
{
    QList<QString> stationsList;
    for (int row = 0; row < jointStationsSelected.count(); row++)
    {
        QString textSelected = jointStationsSelected.item(row)->text();
        stationsList.append(textSelected.section(" ",0,0));
    }
    if (!_meteoPointsDbPointer->setJointStations(QString::fromStdString(_nearMeteoPointsList[0].id), stationsList))
    {
        QMessageBox::critical(nullptr, "Error", _meteoPointsDbPointer->getErrorString());
    }
    saveToDb.setEnabled(false);
}

void Crit3DHomogeneityWidget::updateYears()
{
    yearFrom.blockSignals(true);
    yearTo.blockSignals(true);

    for (int i = 1; i < _jointPointsIdList.size(); i++)
    {
        QDate lastDailyJointStation = _meteoPointsDbPointer->getLastDate(daily, _jointPointsIdList[i]).date();
        if (lastDailyJointStation.isValid() && lastDailyJointStation > _lastDaily )
        {
            _lastDaily = lastDailyJointStation;
        }
    }

    // save current yearFrom
    QString currentYearFrom = yearFrom.currentText();
    yearFrom.clear();
    yearTo.clear();

    for(int i = 0; i <= (_lastDaily.year() - _firstDaily.year()); i++)
    {
        yearFrom.addItem(QString::number(_firstDaily.year()+i));
        yearTo.addItem(QString::number(_firstDaily.year()+i));
    }
    yearTo.setCurrentText(QString::number(_lastDaily.year()));
    yearFrom.setCurrentText(currentYearFrom);

    yearFrom.blockSignals(false);
    yearTo.blockSignals(false);
    plotAnnualSeries();
}

void Crit3DHomogeneityWidget::on_actionChangeLeftAxis()
{
    DialogChangeAxis changeAxisDialog(1, false);
    if (changeAxisDialog.result() == QDialog::Accepted)
    {
        homogeneityChartView->setYmax(changeAxisDialog.getMaxVal());
        homogeneityChartView->setYmin(changeAxisDialog.getMinVal());
    }
}

void Crit3DHomogeneityWidget::on_actionExportHomogeneityGraph()
{

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save current graph"), "", tr("png files (*.png)"));

    if (fileName != "")
    {
        const auto dpr = homogeneityChartView->devicePixelRatioF();
        QPixmap buffer(homogeneityChartView->width() * dpr, homogeneityChartView->height() * dpr);
        buffer.setDevicePixelRatio(dpr);
        buffer.fill(Qt::transparent);

        QPainter *paint = new QPainter(&buffer);
        paint->setPen(*(new QColor(255,34,255,255)));
        homogeneityChartView->render(paint);

        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        buffer.save(&file, "PNG");
    }
}

void Crit3DHomogeneityWidget::on_actionExportAnnualGraph()
{

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save current graph"), "", tr("png files (*.png)"));

    if (fileName != "")
    {
        const auto dpr = annualSeriesChartView->devicePixelRatioF();
        QPixmap buffer(annualSeriesChartView->width() * dpr, annualSeriesChartView->height() * dpr);
        buffer.setDevicePixelRatio(dpr);
        buffer.fill(Qt::transparent);

        QPainter *paint = new QPainter(&buffer);
        paint->setPen(*(new QColor(255,34,255,255)));
        annualSeriesChartView->render(paint);

        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        buffer.save(&file, "PNG");
    }
}

void Crit3DHomogeneityWidget::on_actionExportHomogeneityData()
{

    QString csvFileName = QFileDialog::getSaveFileName(this, tr("Save current data"), "", tr("csv files (*.csv)"));
    if (csvFileName != "")
    {
        QFile myFile(csvFileName);
        if (!myFile.open(QIODevice::WriteOnly | QFile::Truncate))
        {
            QMessageBox::information(nullptr, "Error", "Open CSV failed: " + csvFileName + "\n ");
            return;
        }

        QTextStream myStream (&myFile);
        myStream.setRealNumberNotation(QTextStream::FixedNotation);
        myStream.setRealNumberPrecision(3);
        if (method.currentText() == "SNHT")
        {
            QString header = "year,value";
            myStream << header << "\n";
            QList<QPointF> dataPoints = homogeneityChartView->exportSNHTValues();
            for (int i = 0; i < dataPoints.size(); i++)
            {
                myStream << dataPoints[i].toPoint().x() << "," << dataPoints[i].y() << "\n";
            }
        }
        else if (method.currentText() == "CRADDOCK")
        {
            QList<QString> refNames;
            QList<QList<QPointF>> pointsAllSeries = homogeneityChartView->exportCraddockValues(refNames);
            for (int point = 0; point<refNames.size(); point++)
            {
                QString name = refNames[point];
                myStream << name << "\n";
                QString header = "year,value";
                myStream << header << "\n";
                QList<QPointF> dataPoints = pointsAllSeries[point];
                for (int i = 0; i < dataPoints.size(); i++)
                {
                    myStream << dataPoints[i].toPoint().x() << "," << dataPoints[i].y() << "\n";
                }
            }
        }
        myFile.close();

        return;
    }
}

void Crit3DHomogeneityWidget::on_actionExportAnnualData()
{
    QString csvFileName = QFileDialog::getSaveFileName(this, tr("Save current data"), "", tr("csv files (*.csv)"));
    if (csvFileName != "")
    {
        QFile myFile(csvFileName);
        if (!myFile.open(QIODevice::WriteOnly | QFile::Truncate))
        {
            QMessageBox::information(nullptr, "Error", "Open CSV failed: " + csvFileName + "\n ");
            return;
        }

        QTextStream myStream (&myFile);
        myStream.setRealNumberNotation(QTextStream::FixedNotation);
        myStream.setRealNumberPrecision(3);
        QString header = "year,value";
        myStream << header << "\n";
        QList<QPointF> dataPoins = annualSeriesChartView->exportAnnualValues();
        for (int i = 0; i < dataPoins.size(); i++)
        {
            myStream << dataPoins[i].toPoint().x() << "," << dataPoins[i].y() << "\n";
        }
        myFile.close();

        return;
    }
}


void Crit3DHomogeneityWidget::findReferenceStations()
{
    stationsTable.clearContents();
    listFoundStations.clear();
    listAllFound.clear();
    listSelectedStations.clear();

    QList<std::vector<float>> annualSeriesFound;
    QList<std::string> sortedIdFound;
    QList<float> distanceIdFound;
    int firstYear = yearFrom.currentText().toInt();
    int lastYear = yearTo.currentText().toInt();
    QDate firstDate(firstYear, 1, 1);
    QDate lastDate(lastYear, 12, 31);

    if (_annualSeries.size() == 0)
    {
        QMessageBox::critical(nullptr, "Error", "Data unavailable for candidate station");
        return;
    }

    int nrStations = 0;
    QSqlDatabase myDb = _meteoPointsDbPointer->getDb();

    int nrRequestedStations = minNumStations.text().toInt();
    FormInfo formInfo;
    formInfo.start("Find reference stations..", nrRequestedStations);

    for (int i = 0; i < _sortedIdList.size(); i++)
    {
        std::string pointId = _sortedIdList[i];
        if (_jointPointsIdList.contains(pointId))
            continue;

        if (! _meteoPointsDbPointer->isActivePoint(QString::fromStdString(pointId)))
            continue;

        Crit3DMeteoPoint mpToBeComputed;
        mpToBeComputed.id = pointId;
        if (! _meteoPointsDbPointer->loadDailyData(myDb, getCrit3DDate(firstDate), getCrit3DDate(lastDate), mpToBeComputed))
            continue;

        QString name = _meteoPointsDbPointer->getNameGivenId(QString::fromStdString(pointId));
        QList<QString> jointStationsList = _meteoPointsDbPointer->getJointStations(QString::fromStdString(mpToBeComputed.id));

        // copy all data to meteoPointTemp from joint if there are holes
        Crit3DMeteoPoint meteoPointTemp;
        if (jointStationsList.size() > 0)
        {
            // initialize meteo point
            int numberOfDays = firstDate.daysTo(lastDate) + 1;
            meteoPointTemp.initializeObsDataD(numberOfDays, getCrit3DDate(firstDate));
            name += "_Joint";

            // load all joint stations data
            QList<Crit3DMeteoPoint> jointStationsMpList;
            for (int j = 0; j < jointStationsList.size(); j++)
            {
                Crit3DMeteoPoint mpGet;
                mpGet.id = jointStationsList[j].toStdString();
                if (_meteoPointsDbPointer->loadDailyData(myDb, getCrit3DDate(firstDate), getCrit3DDate(lastDate), mpGet))
                    jointStationsMpList.push_back(mpGet);
            }

            // merge data
            for (QDate myDate = firstDate; myDate <= lastDate; myDate = myDate.addDays(1))
            {
                Crit3DDate crit3dDate = getCrit3DDate(myDate);
                float value = mpToBeComputed.getMeteoPointValueD(crit3dDate, myVar, meteoSettings);
                if (value != NODATA)
                {
                    meteoPointTemp.setMeteoPointValueD(crit3dDate, myVar, value);
                }
                else
                {
                    // missing data, check joint stations
                    for (int j = 0; j < jointStationsMpList.size(); j++)
                    {
                        float valueJoint = jointStationsMpList[j].getMeteoPointValueD(crit3dDate, myVar, meteoSettings);
                        if (valueJoint != NODATA)
                        {
                            meteoPointTemp.setMeteoPointValueD(crit3dDate, myVar, valueJoint);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            meteoPointTemp = mpToBeComputed;
        }

        bool dataAlreadyLoaded = true;
        std::vector<float> mpAnnualSeries;
        std::vector<int> vectorYears;
        // reset climate structure
        _climate.setYearStart(firstYear);
        _climate.setYearEnd(lastYear);
        _climate.setGenericPeriodDateStart(firstDate);
        _climate.setGenericPeriodDateEnd(lastDate);
        _climate.setNYears(0);
        int validYears = computeAnnualSeriesOnPointFromDaily(_meteoPointsDbPointer, nullptr,
                                                             &meteoPointTemp, meteoSettings, _climate, false,
                                                             false, dataAlreadyLoaded,
                                                             mpAnnualSeries, vectorYears, myError);
        if (validYears != 0)
        {
            if ((float)validYears / (float)(lastYear - firstYear + 1) > meteoSettings->getMinimumPercentage() / 100.f)
            {
                nrStations++;
                sortedIdFound.append(pointId);
                mapNameId.insert(name, pointId);
                mapNameAnnualSeries.insert(name, mpAnnualSeries);
                distanceIdFound.append(_distanceList[i]);
                annualSeriesFound.append(mpAnnualSeries);

                if (nrStations == nrRequestedStations)
                    break;
                else
                    formInfo.setValue(nrStations);
            }
        }
    }

    formInfo.close();

    if (nrStations == 0)
    {
        QMessageBox::critical(nullptr, "Error", "No reference stations found");
        return;
    }

    stationsTable.setRowCount(nrStations);
    for (int z = 0; z < nrStations; z++)
    {
        float r2, y_intercept, trend;
        QString name;
        QMapIterator<QString,std::string> iterator(mapNameId);
        while (iterator.hasNext()) {
            iterator.next();
            if (iterator.value() == sortedIdFound[z])
            {
                name = iterator.key();
                break;
            }
        }
        statistics::linearRegression(_annualSeries, annualSeriesFound[z], int(_annualSeries.size()), false, &y_intercept, &trend, &r2);
        double altitude = _meteoPointsDbPointer->getAltitudeGivenId(QString::fromStdString(sortedIdFound[z]));
        double delta =  _nearMeteoPointsList[0].point.z - altitude;
        stationsTable.setItem(z,0,new QTableWidgetItem(name));
        stationsTable.setItem(z,1,new QTableWidgetItem(QString::number(r2, 'f', 3)));
        stationsTable.setItem(z,2,new QTableWidgetItem(QString::number(distanceIdFound[z]/1000, 'f', 1)));
        stationsTable.setItem(z,3,new QTableWidgetItem(QString::number(delta)));
        if (listFoundStations.findItems(name, Qt::MatchExactly).isEmpty())
        {
            listAllFound.append(name);
            listFoundStations.addItem(name);
            addStationFoundButton.setEnabled(true);
        }
    }
}


void Crit3DHomogeneityWidget::addFoundStationClicked()
{
    QList<QListWidgetItem *> items = listFoundStations.selectedItems();
    for (int i = 0; i<items.size(); i++)
    {
        listFoundStations.takeItem(listFoundStations.row(items[i]));
        listSelectedStations.addItem(items[i]);
    }

    if(listFoundStations.count() == 0)
    {
        addStationFoundButton.setEnabled(false);
    }
    else
    {
        addStationFoundButton.setEnabled(true);
    }

    if(listSelectedStations.count() == 0)
    {
        deleteStationFoundButton.setEnabled(false);
        execute.setEnabled(false);
    }
    else
    {
        deleteStationFoundButton.setEnabled(true);
        execute.setEnabled(true);
    }
}

void Crit3DHomogeneityWidget::deleteFoundStationClicked()
{
    QList<QListWidgetItem *> items = listSelectedStations.selectedItems();
    for (int i = 0; i<items.size(); i++)
    {
        listSelectedStations.takeItem(listSelectedStations.row(items[i]));
    }
    // add station, keep order
    listFoundStations.clear();
    for (int i = 0; i<listAllFound.size(); i++)
    {
        if (listSelectedStations.findItems(listAllFound[i], Qt::MatchExactly).isEmpty())
        {
            listFoundStations.addItem(listAllFound[i]);
        }
    }

    if(listFoundStations.count() == 0)
    {
        addStationFoundButton.setEnabled(false);
    }
    else
    {
        addStationFoundButton.setEnabled(true);
    }

    if(listSelectedStations.count() == 0)
    {
        deleteStationFoundButton.setEnabled(false);
        execute.setEnabled(false);
    }
    else
    {
        deleteStationFoundButton.setEnabled(true);
        execute.setEnabled(true);
    }
}

void Crit3DHomogeneityWidget::executeClicked()
{
    bool isHomogeneous = false;
    std::vector<double> myTValues;
    double myYearTmax = NODATA;
    double myTmax = NODATA;
    resultLabel.clear();

    int myFirstYear = yearFrom.currentText().toInt();
    int myLastYear = yearTo.currentText().toInt();
    if (_annualSeries.empty())
    {
        QMessageBox::critical(nullptr, "Error", "Data unavailable for candidate station");
        return;
    }
    if (mapNameAnnualSeries.isEmpty())
    {
        QMessageBox::critical(nullptr, "Error", "No reference stations found");
        return;
    }

    FormInfo formInfo;
    formInfo.showInfo("compute homogeneity test...");
    int nrReference = listSelectedStations.count();

    if (method.currentText() == "SNHT")
    {
        std::vector<double> myValidValues;
        for (int i = 0; i<_annualSeries.size(); i++)
        {
            if (_annualSeries[i] != NODATA)
            {
                myValidValues.push_back((double)_annualSeries[i]);
            }
        }
        double myAverage = statistics::mean(myValidValues);
        std::vector<double> myRefAverage;
        std::vector<float> r2;
        std::vector<std::vector<float>> refSeriesVector;
        float r2Value, y_intercept, trend;

        for (int row = 0; row < nrReference; row++)
        {
            std::vector<double> myRefValidValues;
            QString name = listSelectedStations.item(row)->text();
            std::vector<float> refSeries = mapNameAnnualSeries.value(name);
            refSeriesVector.push_back(refSeries);
            for (int i = 0; i<refSeries.size(); i++)
            {
                if (refSeries[i] != NODATA)
                {
                    myRefValidValues.push_back((double)refSeries[i]);
                }
            }
            myRefAverage.push_back(statistics::mean(myRefValidValues));
            statistics::linearRegression(_annualSeries, refSeries, long(_annualSeries.size()), false, &y_intercept, &trend, &r2Value);
            r2.push_back(r2Value);
        }
        double tmp, sumV;
        std::vector<double> myQ;
        if (myVar == dailyPrecipitation)
        {
            for (int i = 0; i<_annualSeries.size(); i++)
            {
                tmp = 0;
                sumV = 0;
                for (int j = 0; j<nrReference; j++)
                {
                    if (refSeriesVector[j][i] != NODATA)
                    {
                        tmp = tmp + (r2[j] * refSeriesVector[j][i] * myAverage / myRefAverage[j]);
                        sumV = r2[j] + sumV;
                    }
                }
                if (_annualSeries[i] != NODATA && tmp!= 0 && sumV!= 0)
                {
                    myQ.push_back((double)_annualSeries[i]/(tmp/sumV));
                }
                else
                {
                    myQ.push_back(NODATA);
                }
            }
        }
        else
        {
            for (int i = 0; i<_annualSeries.size(); i++)
            {
                tmp = 0;
                sumV = 0;
                for (int j = 0; j<nrReference; j++)
                {
                    if (refSeriesVector[j][i] != NODATA)
                    {
                        tmp = tmp + (r2[j] * (refSeriesVector[j][i] - myRefAverage[j] + myAverage));
                        sumV = r2[j] + sumV;
                    }
                }
                if (_annualSeries[i] != NODATA)
                {
                    if (sumV > 0)
                    {
                         myQ.push_back((double)_annualSeries[i]-(tmp/sumV));
                    }
                    else
                    {
                        myQ.push_back(NODATA);
                    }
                }
                else
                {
                    myQ.push_back(NODATA);
                }
            }
        }
        myValidValues.clear();
        for (int i = 0; i<myQ.size(); i++)
        {
            if (myQ[i] != NODATA)
            {
                myValidValues.push_back(myQ[i]);
            }
        }
        double myQAverage = statistics::mean(myValidValues);
        double myQDevStd = statistics::standardDeviation(myValidValues, int(myValidValues.size()));
        std::vector<double> myZ;
        for (int i = 0; i<myQ.size(); i++)
        {
            if (myQ[i] != NODATA)
            {
                myZ.push_back((myQ[i] - myQAverage) / myQDevStd);
            }
            else
            {
                myZ.push_back(NODATA);
            }
        }
        myValidValues.clear();
        for (int i = 0; i<myZ.size(); i++)
        {
            if (myZ[i] != NODATA)
            {
                myValidValues.push_back(myZ[i]);
            }
        }

        double myZAverage = statistics::mean(myValidValues);

        isHomogeneous = (qAbs(myZAverage) <= EPSILON);
        std::vector<double> z1;
        std::vector<double> z2;

        for (int i = 0; i< myZ.size()-1; i++)
        {
            myTValues.push_back(NODATA);
        }

        for (int a = 0; a < myZ.size()-1; a++)
        {
            z1.resize(a+1);
            z2.resize(myZ.size()-a-1);
            for (int i = 0; i< myZ.size(); i++)
            {
                if (i<=a)
                {
                    z1[i] = myZ[i];
                }
                else
                {
                    z2[i-a-1] = myZ[i];
                }
            }
            myValidValues.clear();
            for (int i = 0; i<z1.size(); i++)
            {
                if (z1[i] != NODATA)
                {
                    myValidValues.push_back(z1[i]);
                }
            }
            double myZ1Average = statistics::mean(myValidValues);
            myValidValues.clear();
            for (int i = 0; i<z2.size(); i++)
            {
                if (z2[i] != NODATA)
                {
                    myValidValues.push_back(z2[i]);
                }
            }
            double myZ2Average = statistics::mean(myValidValues);
            if (myZ1Average != NODATA && myZ2Average != NODATA)
            {
                myTValues[a] = ( (a+1) * pow(myZ1Average,2)) + ((myZ.size() - (a+1)) * pow(myZ2Average,2));
                if (myTmax == NODATA)
                {
                    myTmax = myTValues[a];
                    myYearTmax = myFirstYear + a;
                }
                else if (myTValues[a] > myTmax)
                {
                    myTmax = myTValues[a];
                    myYearTmax = myFirstYear + a;
                }
            }
        }
        std::vector<int> years;
        std::vector<double> outputValues;
        QList<QPointF> t95Points;
        double myValue;
        double myMaxValue = NODATA;
        double myT95;

        int myNrYears = yearTo.currentText().toInt() - myFirstYear + 1;
        for (int i = 0; i < myTValues.size(); i++)
        {
            years.push_back(myFirstYear+i);
            myValue = myTValues[i];
            if (myValue != NODATA)
            {
                if ((myMaxValue == NODATA) || (myValue > myMaxValue))
                {
                    myMaxValue = myValue;
                }
            }
            outputValues.push_back(myValue);
        }
        int myT95Index = round(myNrYears / 10);
        if (myT95Index > 0 && myT95Index <= 10)
        {
            int index = round(myNrYears / 10);
            myT95 = SNHT_T95_VALUES[index-1];
            if (myT95 != NODATA)
            {
                t95Points.append(QPointF(myFirstYear,myT95));
                t95Points.append(QPointF(myFirstYear+myTValues.size()-1,myT95));
            }
        }
        else
        {
            QMessageBox::critical(nullptr, "Info", "T95 value available only for number of years < 100");
        }

        int nYearsToAdd;
        if (years.size()-1 > 15)
        {
            for (int inc = 0; inc<=3; inc++)
            {
                if ( (years.size()-1+inc) % 2 == 0 &&  (years.size()-1+inc)/2 <= 15)
                {
                    nYearsToAdd = inc;
                    break;
                }
                if ( (years.size()-1+inc) % 3 == 0 &&  (years.size()-1+inc)/3 <= 15)
                {
                    nYearsToAdd = inc;
                    break;
                }
                if ( (years.size()-1+inc) % 4 == 0 &&  (years.size()-1+inc)/4 <= 15)
                {
                    nYearsToAdd = inc;
                    break;
                }
            }
            int pos = 0;
            for (int i = nYearsToAdd; i> 0; i--)
            {
                years.insert(years.begin()+pos,myFirstYear-i);
                outputValues.insert(outputValues.begin(),NODATA);
                pos = pos + 1;
            }
        }
        homogeneityChartView->drawSNHT(years,outputValues,t95Points);
        if (myTmax >= myT95 && myYearTmax != NODATA)
        {
            QString text = "Series is not homogeneous\n";
            text = text + "Year of discontinuity: " + QString::number(myYearTmax);
            resultLabel.setText(text);
            resultLabel.setWordWrap(true);
        }
        else
        {
            resultLabel.setText("Series is homogeneous");
        }
    }
    else if (method.currentText() == "CRADDOCK")
    {
        float myLastSum;
        float myReferenceSum;
        float myAverage;
        int myValidYears;
        std::vector<float> myRefAverage;
        std::vector<float> myC;
        std::vector<std::vector<float>> mySValues;
        std::vector<std::vector<float>> myD;
        // init myD
        for (int row = 0; row < nrReference; row++)
        {
            std::vector<float> vectD;
            for (int myYear = 0; myYear<_annualSeries.size(); myYear++)
            {
                vectD.push_back(NODATA);
            }
            myD.push_back(vectD);
            mySValues.push_back(vectD);
        }

        std::vector<QString> refNames;
        for (int row = 0; row < nrReference; row++)
        {
            // compute mean only for common years
            myLastSum = 0;
            myReferenceSum = 0;
            myValidYears = 0;
            QString name = listSelectedStations.item(row)->text();
            refNames.push_back(name);
            std::vector<float> refSeries = mapNameAnnualSeries.value(name);
            for (int myYear = 0; myYear<_annualSeries.size(); myYear++)
            {
                if (_annualSeries[myYear] != NODATA && refSeries[myYear] != NODATA)
                {
                    myLastSum = myLastSum + _annualSeries[myYear];
                    myReferenceSum = myReferenceSum + refSeries[myYear];
                    myValidYears = myValidYears + 1;
                }
            }
            myAverage = myLastSum / myValidYears;
            myRefAverage.push_back(myReferenceSum / myValidYears);
            if (myVar == dailyPrecipitation)
            {
                if (myRefAverage[row] == 0)
                {
                    QMessageBox::critical(nullptr, "Error", "Can not divide by zero");
                    return;
                }
                myC.push_back(myRefAverage[row] / myAverage);
            }
            else
            {
                 myC.push_back(myRefAverage[row] - myAverage);
            }
            for (int myYear = 0; myYear<_annualSeries.size(); myYear++)
            {
                if (_annualSeries[myYear] != NODATA && refSeries[myYear] != NODATA)
                {
                    if (myVar == dailyPrecipitation)
                    {
                         myD[row][myYear] = myC[row]*_annualSeries[myYear] - refSeries[myYear];
                    }
                    else
                    {
                        myD[row][myYear] = myC[row]+_annualSeries[myYear] - refSeries[myYear];
                    }
                }
            }
        }

        // sum
        for (int row = 0; row < nrReference; row++)
        {
            myLastSum = 0;
            for (int myYear = 0; myYear<_annualSeries.size(); myYear++)
            {
                if (myD[row][myYear] != NODATA)
                {
                    mySValues[row][myYear] = myLastSum + myD[row][myYear];
                    myLastSum = mySValues[row][myYear];
                }
            }
        }

        // draw
        homogeneityChartView->drawCraddock(myFirstYear, myLastYear, mySValues, refNames, myVar, averageValue);
    }

    formInfo.close();
}


void Crit3DHomogeneityWidget::checkValueAndMerge(const Crit3DMeteoPoint &meteoPointGet, Crit3DMeteoPoint* meteoPointSet, const QDate &myDate)
{
    Crit3DDate crit3dDate = getCrit3DDate(myDate);
    float value = meteoPointGet.getMeteoPointValueD(crit3dDate, myVar, meteoSettings);
    if (value != NODATA)
    {
        meteoPointSet->setMeteoPointValueD(crit3dDate, myVar, value);
        return;
    }

    // missing data, check joint stations
    for (int i = 1; i < _jointPointsIdList.size(); i++)
    {
        int jointIndex = _jointIndexList[i];
        float jointValue = _nearMeteoPointsList[jointIndex].getMeteoPointValueD(crit3dDate, myVar, meteoSettings);
        if (jointValue != NODATA)
        {
            meteoPointSet->setMeteoPointValueD(crit3dDate, myVar, jointValue);
            return;
        }
    }
}
