#ifndef DIALOGSHIFTDATA_H
#define DIALOGSHIFTDATA_H

    #include "meteo.h"
    #include <QDialog>
    #include <QDateEdit>
    #include <QComboBox>
    #include <QLineEdit>
    #include <QCheckBox>

    class DialogShiftData : public QDialog
    {
        Q_OBJECT

    public:
        DialogShiftData(QDate myDate, bool allPoints);
        ~DialogShiftData();
        void done(bool res);
        int getShift() const;

        meteoVariable getVariable() const;

        QDate getDateFrom() const { return _dateFrom.date(); }
        QDate getDateTo() const { return _dateTo.date(); }
        bool getOverwrite() const { return _overWrite.isChecked(); }

    private:
        QLineEdit _shift;
        QComboBox _variable;
        QDateEdit _dateFrom;
        QDateEdit _dateTo;
        QCheckBox _overWrite;
    };

#endif // DIALOGSHIFTDATA_H
