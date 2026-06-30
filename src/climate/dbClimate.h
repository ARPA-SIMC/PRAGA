#ifndef DBCLIMATE_H
#define DBCLIMATE_H

    #ifndef QSQLDATABASE_H
        #include <QSqlDatabase>
    #endif

    #ifndef _VECTOR_
        #include <vector>
    #endif

    bool saveDailyElab(QSqlDatabase db, QString &errorStr, QString id, std::vector<float> allResults, const QString &elab);
    bool saveDecadalElab(QSqlDatabase db, QString &errorStr, QString id, std::vector<float> allResults, const QString &elab);
    bool saveMonthlyElab(QSqlDatabase db, QString &errorStr, QString id, std::vector<float> allResults, const QString &elab);
    bool saveSeasonalElab(QSqlDatabase db, QString &errorStr, QString id, std::vector<float> allResults, const QString &elab);

    bool saveAnnualElab(QSqlDatabase db, QString &errorStr, QString id, float result, QString elab);
    bool saveGenericElab(QSqlDatabase db, QString &errorStr, QString id, float result, QString elab);

    bool getClimateFieldsFromTable(const QSqlDatabase &db, QString &errorStr, const QString &climateTable, QList<QString>* fieldList);
    bool selectVarElab(const QSqlDatabase &db, QString &errorStr, const QString &table, QString variable, QList<QString>* listElab);
    bool getClimateTables(const QSqlDatabase &db, QString &errorStr, QList<QString>* climateTables);

    bool deleteElab(QSqlDatabase db, QString &errorStr, const QString &table, const QString &elab);

    float readClimateElab(const QSqlDatabase &db, const QString &table, const int &timeIndex,
                          const QString &id, const QString &elab, QString &errorStr);

    QList<QString> getIdListFromElab(const QSqlDatabase &db, const QString &table, QString &errorStr, const QString &elab);
    QList<QString> getIdListFromElab(const QSqlDatabase &db, const QString &table, QString &errorStr, const QString &elab, int index);


#endif // DBCLIMATE_H
