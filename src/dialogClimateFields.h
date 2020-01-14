#ifndef DIALOGCLIMATEFIELDS_H
#define DIALOGCLIMATEFIELDS_H

#include <QtWidgets>
#include "meteo.h"

class DialogClimateFields : public QDialog
{
    Q_OBJECT

    private:

        QWidget elabW;
        QWidget indexW;

        QStringList climateDbVarList;
        QStringList climateDbElab;

        QListWidget listVariable;
        QListWidget listElab;
        QListWidget listIndex;

        QHBoxLayout mainLayout;
        QVBoxLayout variableLayout;
        QVBoxLayout elabLayout;
        QVBoxLayout indexLayout;
        QVBoxLayout buttonLayout;

        QPushButton showButton;
        QPushButton deleteButton;

        QString climaSelected;
        meteoVariable var;
        QString indexSelected;

        bool isShowClicked;

    public:
        DialogClimateFields(QStringList climateDbElab, QStringList climateDbVarList);
        void variableClicked(QListWidgetItem *item);
        void elabClicked(QListWidgetItem* item);
        void indexClicked(QListWidgetItem* item);
        void showClicked();
        void deleteClicked();
        QString getSelected() const;
        meteoVariable getVar() const;
        QString getIndexSelected() const;
        bool getIsShowClicked() const;
};

#endif // DIALOGCLIMATEFIELDS_H
