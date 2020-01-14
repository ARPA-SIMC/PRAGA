#include "pragaProject.h"
#include "aggregation.h"
#include "dialogSeriesOnZones.h"

extern PragaProject myProject;

meteoVariable DialogSeriesOnZones::getVariable() const
{
    return variable;
}

QDate DialogSeriesOnZones::getStartDate() const
{
    return startDate;
}

QDate DialogSeriesOnZones::getEndDate() const
{
    return endDate;
}

aggregationMethod DialogSeriesOnZones::getSpatialElaboration() const
{
    return spatialElaboration;
}

DialogSeriesOnZones::DialogSeriesOnZones(QSettings *settings)
    : settings(settings)
{

    setWindowTitle("Spatial average series on zones");


    QVBoxLayout mainLayout;
    QHBoxLayout varLayout;
    QHBoxLayout dateLayout;
    QHBoxLayout spatialElabLayout;

    QHBoxLayout layoutOk;

    meteoVariable var;

    Q_FOREACH (QString group, settings->childGroups())
    {
        if (!group.endsWith("_VarToElab1"))
            continue;
        std::string item;
        std::string variable = group.left(group.size()-11).toStdString(); // remove "_VarToElab1"
        try {
          var = MapDailyMeteoVar.at(variable);
          item = MapDailyMeteoVarToString.at(var);
        }
        catch (const std::out_of_range& ) {
           myProject.logError("variable " + QString::fromStdString(variable) + " missing in MapDailyMeteoVar");
           continue;
        }
        variableList.addItem(QString::fromStdString(item));
    }

    QLabel variableLabel("Variable: ");
    varLayout.addWidget(&variableLabel);
    varLayout.addWidget(&variableList);


    genericStartLabel.setText("Start Date:");
    genericPeriodStart.setDate(myProject.getCurrentDate());
    genericStartLabel.setBuddy(&genericPeriodStart);
    genericEndLabel.setText("End Date:");
    genericPeriodEnd.setDate(myProject.getCurrentDate());
    genericEndLabel.setBuddy(&genericPeriodEnd);


    dateLayout.addWidget(&genericStartLabel);
    dateLayout.addWidget(&genericPeriodStart);
    dateLayout.addWidget(&genericEndLabel);
    dateLayout.addWidget(&genericPeriodEnd);

    QLabel spatialElabLabel("Spatial Elaboration: ");
    spatialElab.addItem("AVG");
    spatialElab.addItem("MEDIAN");
    spatialElab.addItem("STDDEV");


    spatialElabLayout.addWidget(&spatialElabLabel);
    spatialElabLayout.addWidget(&spatialElab);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(&buttonBox, &QDialogButtonBox::accepted, [=](){ this->done(true); });
    connect(&buttonBox, &QDialogButtonBox::rejected, [=](){ this->done(false); });

    layoutOk.addWidget(&buttonBox);

    mainLayout.addLayout(&varLayout);
    mainLayout.addLayout(&dateLayout);
    mainLayout.addLayout(&spatialElabLayout);

    mainLayout.addLayout(&layoutOk);

    setLayout(&mainLayout);


    show();
    exec();

}


void DialogSeriesOnZones::done(bool res)
{

    if(res)  // ok was pressed
    {
        if (!checkValidData())
        {
            QDialog::done(QDialog::Rejected);
            return;
        }
        else  // validate the data
        {
            QDialog::done(QDialog::Accepted);
            return;
        }

    }
    else    // cancel, close or exc was pressed
    {
        QDialog::done(QDialog::Rejected);
        return;
    }

}

bool DialogSeriesOnZones::checkValidData()
{

    startDate = genericPeriodStart.date();
    endDate = genericPeriodEnd.date();

    if (startDate > endDate)
    {
        QMessageBox::information(nullptr, "Invalid date", "first date greater than last date");
        return false;
    }

    QString var = variableList.currentText();
    variable = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, var.toStdString());
    spatialElaboration = getAggregationMethod(spatialElab.currentText().toStdString());

    return true;

}



