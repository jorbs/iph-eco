#ifndef GRID_DATA_DIALOG_H
#define GRID_DATA_DIALOG_H

#include "include/domain/grid_data_configuration.h"

#include <QDialog>
#include <QWidget>
#include <QTableWidgetItem>

namespace Ui {
class GridDataDialog;
}

class GridDataDialog : public QDialog {
    Q_OBJECT

public:
    explicit GridDataDialog(QWidget *parent = 0);
    ~GridDataDialog();

    void setArea(const double &area);

public slots:
    void setCoordinate(double &x, double &y);

private slots:
    void on_cbxMesh_currentIndexChanged(const QString &meshName);
    void on_cbxConfiguration_currentIndexChanged(const QString &configurationName);
    void on_btnAddGridLayer_clicked();
    void on_btnEditGridLayer_clicked();
    void on_btnRemoveGridLayer_clicked();
    void on_btnSaveConfiguration_clicked();
    void on_btnRemoveConfiguration_clicked();
    void on_btnDoneConfiguration_clicked();
    void on_tblGridLayers_itemClicked(QTableWidgetItem *item);
    void on_btnBackgroundColor_clicked();

private:
    Ui::GridDataDialog *ui;
    GridDataConfiguration *unsavedConfiguration;
    GridDataConfiguration *currentConfiguration;
    Mesh *currentMesh;
    
    void toggleGridDataConfigurationForm(bool enable);
    bool isConfigurationValid(const QString &configurationName);
    void showGridLayerDialog(GridData *gridData);
};

#endif // GRID_DATA_DIALOG_H
