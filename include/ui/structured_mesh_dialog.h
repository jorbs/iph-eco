#ifndef STRUCTURED_MESH_DIALOG_H
#define STRUCTURED_MESH_DIALOG_H

#include <QDialog>
#include <QWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>

#include "include/domain/structured_mesh.h"

namespace Ui {
class StructuredMeshDialog;
}

class StructuredMeshDialog : public QDialog {
    Q_OBJECT

public:
    explicit StructuredMeshDialog(QWidget *parent = 0);
    ~StructuredMeshDialog();

    void setRealCoordinate(const Point &point);

private slots:
    void on_btnBoundaryFileBrowser_clicked();
    void on_btnAddIsland_clicked();
    void on_btnRemoveIsland_clicked();
    void on_chkShowDomainBoundary_clicked();
    void on_chkShowMesh_clicked();
    void on_btnGenerateMesh_clicked();
    void on_btnSaveMesh_clicked();
    void on_btnCancelMesh_clicked();
    void on_btnRemoveMesh_clicked();

    void on_cbxMeshName_currentIndexChanged(int index);

private:
    const QString BOUNDARY_DEFAULT_DIR_KEY;

    Ui::StructuredMeshDialog *ui;
    StructuredMesh *unsavedMesh;
    StructuredMesh *currentMesh;
    QSettings *appSettings;

    QString getDefaultDirectory();
    void enableMeshForm(bool enable);
    void resetMeshForm();
};

#endif // STRUCTURED_MESH_DIALOG_H