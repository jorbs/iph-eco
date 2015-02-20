#include "include/ui/unstructured_mesh_dialog.h"
#include "ui_unstructured_mesh_dialog.h"

#include "include/domain/unstructured_mesh.h"

UnstructuredMeshDialog::UnstructuredMeshDialog(QWidget *parent) : QDialog(parent), ui(new Ui::UnstructuredMeshDialog) {
    ui->setupUi(this);
}

UnstructuredMeshDialog::~UnstructuredMeshDialog() {
    delete ui;
}

void UnstructuredMeshDialog::on_btnBoundaryFileBrowser_clicked() {
    QString boundaryFilePath = QFileDialog::getOpenFileName(this, tr("Select a boundary file"), ".", "Keyhole Markup Language file (*.kml)");

    if (boundaryFilePath.isEmpty()) {
        return;
    }

    QString meshName = ui->edtMeshName->text();
    UnstructuredMesh unstructuredMesh = UnstructuredMesh(meshName, boundaryFilePath);

    ui->edtBoundaryFileLine->setText(boundaryFilePath);

    try {
        ui->meshOpenGLWidget->updateCurrentMesh(unstructuredMesh);
    } catch(MeshException &e) {
        QMessageBox::critical(this, tr("Unstructured Mesh Generation"), e.what());
    }
}

void UnstructuredMeshDialog::on_btnGenerateMesh_clicked() {
    QString meshName = ui->edtMeshName->text();

    if (meshName.isEmpty()) {
        QMessageBox::warning(this, tr("Unstructured Mesh Generation"), tr("Please input the mesh name."));
        ui->edtMeshName->setFocus();
        return;
    }

    QString boundaryFileStr = ui->edtBoundaryFileLine->text();
    QFile boundaryFile(boundaryFileStr);
    QFileInfo boundaryFileInfo(boundaryFile);

    if (!boundaryFileInfo.exists() || !boundaryFileInfo.isFile()) {
        QMessageBox::warning(this, tr("Unstructured Mesh Generation"), tr("Boundary file not found."));
        return;
    }

    enableMeshForm(true);

//    double minimumAngle = ui->sbxMinimumAngle->value();
//    double maximumEdgeLength = ui->sbxMaximumEdgeLength->value();
//    UnstructuredMesh unstructuredMesh(meshName, boundaryFileStr, minimumAngle, maximumEdgeLength);

    try {
        //TODO: Generate mesh
    } catch(MeshException &e) {
        QMessageBox::critical(this, tr("Unstructured Mesh Generation"), e.what());
    }
}

void UnstructuredMeshDialog::on_btnResetMesh_clicked() {
    ui->sbxMinimumAngle->setValue(ui->sbxMinimumAngle->minimum());
    ui->sbxMaximumEdgeLength->setValue(ui->sbxMaximumEdgeLength->minimum());
}

void UnstructuredMeshDialog::on_btnSaveMesh_clicked() {
    QString meshName = ui->cbxMeshName->currentIndex() == -1 ? ui->edtMeshName->text() : ui->cbxMeshName->currentText();
    QString boundaryFile = ui->edtBoundaryFileLine->text();
    double minimumAngle = ui->sbxMinimumAngle->value();
    double maximumEdgeLength = ui->sbxMaximumEdgeLength->value();

    UnstructuredMesh unstructuredMesh(meshName, boundaryFile, minimumAngle, maximumEdgeLength);
    Project *project = IPHApplication::getCurrentProject();

    UnstructuredMesh *mesh = (UnstructuredMesh*) project->getMesh(unstructuredMesh);

    if (mesh == NULL && ui->cbxMeshName->currentIndex() == -1) {
        project->addMesh(unstructuredMesh);

        ui->cbxMeshName->addItem(meshName);
        ui->cbxMeshName->setCurrentText(meshName);
    } else {
        mesh->setName(ui->edtMeshName->text());
        mesh->setBoundaryFilePath(boundaryFile);
        mesh->setMinimumAngle(minimumAngle);
        mesh->setMaximumEdgeLength(maximumEdgeLength);

        ui->cbxMeshName->setItemText(ui->cbxMeshName->currentIndex(), mesh->getName());
    }
}

void UnstructuredMeshDialog::on_btnRemoveMesh_clicked() {
    QString meshName = ui->cbxMeshName->currentText();

    if (QMessageBox::Yes == QMessageBox::question(this, tr("Remove mesh"), tr("Are you sure you want to remove '") + meshName + "'?",
                                          QMessageBox::Yes|QMessageBox::No)) {

        UnstructuredMesh unstructuredMesh(meshName);
        IPHApplication::getCurrentProject()->removeMesh(unstructuredMesh);

        ui->cbxMeshName->removeItem(ui->cbxMeshName->currentIndex());
        ui->cbxMeshName->setCurrentIndex(-1);
        ui->meshOpenGLWidget->clearMap();
    }
}

void UnstructuredMeshDialog::on_btnCancelMesh_clicked() {
    resetMeshForm();
    ui->cbxMeshName->setCurrentIndex(-1);
    ui->meshOpenGLWidget->clearMap();
}

void UnstructuredMeshDialog::on_cbxMeshName_currentIndexChanged(int index) {
    if (index > -1) {
        enableMeshForm(true);
        ui->btnRemoveMesh->setEnabled(true);

        QString meshName = ui->cbxMeshName->currentText();
        UnstructuredMesh unstructuredMesh(meshName);
        UnstructuredMesh *mesh = (UnstructuredMesh*) IPHApplication::getCurrentProject()->getMesh(unstructuredMesh);

        ui->edtMeshName->setText(mesh->getName());
        ui->edtBoundaryFileLine->setText(mesh->getBoundaryFilePath());
        ui->sbxMinimumAngle->setValue(mesh->getMinimumAngle());
        ui->sbxMaximumEdgeLength->setValue(mesh->getMaximumEdgeLength());
        ui->meshOpenGLWidget->updateCurrentMesh(*mesh);
    } else {
        resetMeshForm();
    }
}

void UnstructuredMeshDialog::enableMeshForm(bool enable) {
    ui->btnRefineSpecificRegion->setEnabled(enable);
    ui->btnApplyCoarsening->setEnabled(enable);

    ui->chkShowEdges->setEnabled(enable);
    ui->chkShowTriangles->setEnabled(enable);
    ui->chkShowUtmCoordinates->setEnabled(enable);
    ui->chkShowVertexes->setEnabled(enable);

    ui->btnSaveMesh->setEnabled(enable);
    ui->btnCancelMesh->setEnabled(enable);
}

void UnstructuredMeshDialog::resetMeshForm() {
    ui->edtMeshName->setFocus();

    ui->edtMeshName->clear();
    ui->edtBoundaryFileLine->clear();
    on_btnResetMesh_clicked();

    ui->btnRemoveMesh->setEnabled(false);
    enableMeshForm(false);
}

void UnstructuredMeshDialog::on_chkShowDomainBoundary_clicked() {
    ui->meshOpenGLWidget->toogleBoundaryDomain(ui->chkShowDomainBoundary->isChecked());
}
