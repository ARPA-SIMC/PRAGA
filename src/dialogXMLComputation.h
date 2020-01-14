#ifndef DIALOGXMLCOMPUTATION_H
#define DIALOGXMLCOMPUTATION_H

#include <QtWidgets>

class DialogXMLComputation : public QDialog
{
    Q_OBJECT
private:
    bool isAnomaly;
    QListWidget listXMLWidget;
    QStringList listXML;
    int index;
public:
    DialogXMLComputation(bool isAnomaly, QStringList listXML);
    void elabClicked(QListWidgetItem* item);
    unsigned int getIndex() const;
};

#endif // DIALOGXMLCOMPUTATION_H
