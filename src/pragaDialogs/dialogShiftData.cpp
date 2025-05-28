#include "dialogShiftData.h"
#include "commonConstants.h"

#include <QLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>


DialogShiftData::DialogShiftData(QDate myDate, bool allPoints)
{
    this->setWindowTitle("Shift Data");
    QVBoxLayout* mainLayout = new QVBoxLayout;
    this->resize(300, 120);

    QHBoxLayout *dateLayout = new QHBoxLayout;
    QVBoxLayout *variableLayout = new QVBoxLayout;
    QHBoxLayout *shiftLayout = new QHBoxLayout;
    QHBoxLayout *layoutOk = new QHBoxLayout;

    QLabel *subTitleLabel = new QLabel();
    if (allPoints)
    {
        subTitleLabel->setText("All points");
    }
    else
    {
        subTitleLabel->setText("Selected points");
    }

    QFont currentFont = subTitleLabel->font();
    currentFont.setBold(true);
    subTitleLabel->setFont(currentFont);
    mainLayout->addWidget(subTitleLabel);

    // DATES
    QLabel *dateFromLabel = new QLabel(tr("From"));
    dateLayout->addWidget(dateFromLabel);
    _dateFrom.setDisplayFormat("yyyy-MM-dd");
    _dateFrom.setFixedWidth(100);
    dateLayout->addWidget(&_dateFrom);
    QLabel *dateToLabel = new QLabel(tr("    To"));
    dateLayout->addWidget(dateToLabel);
    _dateTo.setDisplayFormat("yyyy-MM-dd");
    _dateTo.setFixedWidth(100);
    dateLayout->addWidget(&_dateTo);

    _dateFrom.setDate(myDate);
    _dateTo.setDate(myDate);

    // VARIABLE
    QLabel *variableLabel = new QLabel(tr("Daily Variable: "));
    std::map<meteoVariable, std::string>::const_iterator it;
    for(it = MapDailyMeteoVarToString.begin(); it != MapDailyMeteoVarToString.end(); ++it)
    {
        _variable.addItem(QString::fromStdString(it->second));
    }
    _variable.setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _variable.setMaximumWidth(150);
    variableLayout->addWidget(variableLabel);
    variableLayout->addWidget(&_variable);

    // SHIFT
    QLabel shiftLabel("Shift:");
    shiftLabel.setBuddy(&_shift);
    _shift.setValidator(new QIntValidator(-100.0, 100.0));
    _shift.setText(QString::number(0));
    _shift.setMaximumWidth(50);

    shiftLayout->addWidget(&shiftLabel);
    shiftLayout->addWidget(&_shift);

    _overWrite.setText("Overwrite data");
    _overWrite.setChecked(true);

    // BUTTONS
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(&buttonBox, &QDialogButtonBox::accepted, [=](){ this->done(true); });
    connect(&buttonBox, &QDialogButtonBox::rejected, [=](){ this->done(false); });
    layoutOk->addWidget(&buttonBox);

    mainLayout->addLayout(dateLayout);
    mainLayout->addLayout(variableLayout);
    mainLayout->addLayout(shiftLayout);
    mainLayout->addWidget(&_overWrite);
    mainLayout->addLayout(layoutOk);

    setLayout(mainLayout);
    exec();
}


DialogShiftData::~DialogShiftData()
{
    close();
}


void DialogShiftData::done(bool res)
{
    if (res) // ok
    {
        if (_shift.text().isEmpty())
        {
            QMessageBox::information(nullptr, "Missing shift", "Insert shift value.");
            return;
        }

        bool isOk;
        int shift = _shift.text().toInt(&isOk);
        if (! isOk || shift == 0)
        {
            QMessageBox::information(nullptr, "Wrong shift", "Insert a correct shift value.");
            return;
        }

        QDialog::done(QDialog::Accepted);
    }
    else    // cancel, close or exc was pressed
    {
        QDialog::done(QDialog::Rejected);
    }
}


int DialogShiftData::getShift() const
{
    bool isOk;
    int shift = _shift.text().toInt(&isOk);

    if (! isOk)
        return NODATA;

    return shift;
}


meteoVariable DialogShiftData::getVariable() const
{
    std::string varString = _variable.currentText().toStdString();
    return getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, varString);
}

