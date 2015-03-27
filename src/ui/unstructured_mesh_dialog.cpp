#include "include/ui/unstructured_mesh_dialog.h"
#include "ui_unstructured_mesh_dialog.h"

#include <CGAL/assertions_behaviour.h>
#include <QDebug>

#include "include/domain/refinement_area.h"

UnstructuredMeshDialog::UnstructuredMeshDialog(QWidget *parent) :
    QDialog(parent),
    BOUNDARY_DEFAULT_DIR_KEY("boundary_default_dir"),
    ui(new Ui::UnstructuredMeshDialog),
    unsavedMesh(new UnstructuredMesh()),
    currentMesh(unsavedMesh)
{

    ui->setupUi(this);
    ui->unstructuredMeshOpenGLWidget->setMesh(unsavedMesh);

    appSettings = new QSettings(QApplication::organizationName(), QApplication::applicationName(), this);

    Project *project = IPHApplication::getCurrentProject();

    QSet<Mesh*> meshes = project->getMeshes();

    for (QSet<Mesh*>::iterator i = meshes.begin(); i != meshes.end(); ++i) {
        ui->cbxMeshName->addItem((*i)->getName());
    }
    ui->cbxMeshName->setCurrentIndex(-1);
}

QString UnstructuredMeshDialog::getDefaultDirectory() {
    return appSettings->value(BOUNDARY_DEFAULT_DIR_KEY).toString().isEmpty() ? QDir::homePath() : appSettings->value(BOUNDARY_DEFAULT_DIR_KEY).toString();
}

UnstructuredMeshDialog::~UnstructuredMeshDialog() {
    delete unsavedMesh;
    delete ui;
}

void UnstructuredMeshDialog::on_btnBoundaryFileBrowser_clicked() {
    QString boundaryFilePath = QFileDialog::getOpenFileName(this, tr("Select a boundary file"), getDefaultDirectory(), "Keyhole Markup Language file (*.kml)");

    if (boundaryFilePath.isEmpty()) {
        return;
    }

    ui->edtBoundaryFileLine->setText(boundaryFilePath);
    appSettings->setValue(BOUNDARY_DEFAULT_DIR_KEY, QFileInfo(boundaryFilePath).absolutePath());
}

void UnstructuredMeshDialog::on_btnGenerateDomain_clicked() {
    QString boundaryFilePath = ui->edtBoundaryFileLine->text();
    double coordinatesDistance = ui->sbxCoordinatesDistance->value();

    currentMesh->setBoundaryFilePath(boundaryFilePath);
    currentMesh->setCoordinatesDistance(coordinatesDistance);
    ui->unstructuredMeshOpenGLWidget->setMesh(currentMesh);

    try {
        ui->unstructuredMeshOpenGLWidget->generateDomain();
    } catch(MeshException &e) {
        QMessageBox::critical(this, tr("Unstructured Mesh Generation"), e.what());
        return;
    }

    ui->lblDomainArea->setText(QString("Area: %1 m\u00B2").arg(currentMesh->area(), 0, 'f', 3));
}

void UnstructuredMeshDialog::on_btnAddCoordinatesFile_clicked() {
    QString coordinatesFile = QFileDialog::getOpenFileName(this, tr("Select a boundary file"), getDefaultDirectory(), "Keyhole Markup Language file (*.kml)");

    if (coordinatesFile.isEmpty()) {
        return;
    }

    if (!ui->lstCoordinateFiles->findItems(coordinatesFile, Qt::MatchExactly).empty()) {
        QMessageBox::information(this, tr("Unstructured Mesh Generation"), tr("Coordinates file already added."));
        return;
    }

    ui->lstCoordinateFiles->addItem(coordinatesFile);
    currentMesh->addRefinementPolygon(coordinatesFile);
}

void UnstructuredMeshDialog::on_btnRemoveCoordinatesFile_clicked() {
    if (ui->lstCoordinateFiles->currentRow() > 0) {
        QListWidgetItem *currentItem = ui->lstCoordinateFiles->currentItem();

        if (currentItem != NULL) {
            QString coordinatesFile = currentItem->text();

            currentMesh->removeRefinementArea(coordinatesFile);
            ui->lstCoordinateFiles->takeItem(ui->lstCoordinateFiles->currentRow());
        }
    }
}

void UnstructuredMeshDialog::on_lstCoordinateFiles_itemSelectionChanged() {
    if (ui->lstCoordinateFiles->currentRow() > 0) {
        QListWidgetItem *currentItem = ui->lstCoordinateFiles->currentItem();
        RefinementArea *refinementArea = currentMesh->getRefinementArea(currentItem->text());

        ui->sbxMaximumEdgeLength->setValue(refinementArea->getMaximumEdgeLength());
        ui->sbxMaximumEdgeLength->setMaximum(refinementArea->getUpperBoundForMaximumEdgeLength());
        ui->sbxMinimumAngle->setValue(refinementArea->getMinimumAngleInDegrees());
    } else {
        if (currentMesh->getBoundaryPolygon() != NULL) {
            ui->sbxMaximumEdgeLength->setValue(currentMesh->getMaximumEdgeLength());
            ui->sbxMaximumEdgeLength->setMaximum(currentMesh->getUpperBoundForMaximumEdgeLength());
            ui->sbxMinimumAngle->setValue(currentMesh->getMinimumAngleInDegrees());
        }
    }
}

void UnstructuredMeshDialog::on_sbxMaximumEdgeLength_valueChanged(double value) {
    if (ui->lstCoordinateFiles->currentRow() > 0) {
        RefinementArea *refinementArea = currentMesh->getRefinementArea(ui->lstCoordinateFiles->currentItem()->text());

        refinementArea->setMaximumEdgeLength(value);
    } else {
        if (currentMesh->getBoundaryPolygon() != NULL) {
            currentMesh->setMaximumEdgeLength(value);
        }
    }
}

void UnstructuredMeshDialog::on_sbxMinimumAngle_valueChanged(double value) {
    if (ui->lstCoordinateFiles->currentRow() > 0) {
        RefinementArea *refinementArea = currentMesh->getRefinementArea(ui->lstCoordinateFiles->currentItem()->text());

        refinementArea->setMinimumAngle(value);
    } else {
        if (currentMesh->getBoundaryPolygon() != NULL) {
            currentMesh->setMinimumAngle(value);
        }
    }
}

void UnstructuredMeshDialog::on_btnGenerateMesh_clicked() {
    QString meshName = ui->edtMeshName->text();

    if (meshName.isEmpty()) {
        ui->tabWidget->setCurrentIndex(0);
        ui->edtMeshName->setFocus();
        QMessageBox::warning(this, tr("Unstructured Mesh Generation"), tr("Please input the mesh name."));
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

    currentMesh->setName(meshName);
    currentMesh->setBoundaryFilePath(boundaryFileStr);

    CGAL::Failure_behaviour old_behaviour = CGAL::set_error_behaviour(CGAL::THROW_EXCEPTION);
    try {
        ui->unstructuredMeshOpenGLWidget->setMesh(currentMesh);
        ui->unstructuredMeshOpenGLWidget->generateDomain();

        currentMesh->setMinimumAngle(ui->sbxMinimumAngle->value());
        currentMesh->setMaximumEdgeLength(ui->sbxMaximumEdgeLength->value());

        ui->unstructuredMeshOpenGLWidget->generateMesh();\
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Unstructured Mesh Generation"), tr("This triangulation does not deal with intersecting constraints."));
        CGAL::set_error_behaviour(old_behaviour);
        return;
    }
    CGAL::set_error_behaviour(old_behaviour);
}

void UnstructuredMeshDialog::on_btnSaveMesh_clicked() {
    QString meshName = ui->cbxMeshName->currentIndex() == -1 ? ui->edtMeshName->text() : ui->cbxMeshName->currentText();
    QString boundaryFile = ui->edtBoundaryFileLine->text();
    double minimumAngle = ui->sbxMinimumAngle->value();
    double maximumEdgeLength = ui->sbxMaximumEdgeLength->value();

    Project *project = IPHApplication::getCurrentProject();
    UnstructuredMesh unstructuredMesh(meshName);
    currentMesh = static_cast<UnstructuredMesh*>(project->getMesh(&unstructuredMesh));

    if (currentMesh == NULL && ui->cbxMeshName->currentIndex() == -1) {
        bool showDomainBoundary = ui->chkShowDomainBoundary->isChecked();
        bool showMesh = ui->chkShowMesh->isChecked();

        currentMesh = new UnstructuredMesh(meshName, boundaryFile, minimumAngle, maximumEdgeLength);
        currentMesh->setShowDomainBoundary(showDomainBoundary);
        currentMesh->setShowMesh(showMesh);
        project->addMesh(currentMesh);

        ui->cbxMeshName->addItem(meshName);
        ui->cbxMeshName->setCurrentText(meshName);
    } else {
        currentMesh->setName(meshName);
        currentMesh->setBoundaryFilePath(boundaryFile);
        currentMesh->setMinimumAngle(minimumAngle);
        currentMesh->setMaximumEdgeLength(maximumEdgeLength);

        ui->cbxMeshName->setItemText(ui->cbxMeshName->currentIndex(), currentMesh->getName());
    }
}

void UnstructuredMeshDialog::on_btnRemoveMesh_clicked() {
    QString meshName = ui->cbxMeshName->currentText();

    if (QMessageBox::Yes == QMessageBox::question(this, tr("Remove mesh"), tr("Are you sure you want to remove '") + meshName + "'?",
                                          QMessageBox::Yes|QMessageBox::No)) {

        UnstructuredMesh *removedMesh = currentMesh;
        currentMesh = unsavedMesh;
        IPHApplication::getCurrentProject()->removeMesh(removedMesh);

        ui->cbxMeshName->removeItem(ui->cbxMeshName->currentIndex());
        ui->cbxMeshName->setCurrentIndex(-1);
        ui->unstructuredMeshOpenGLWidget->setMesh(NULL);
    }
}

void UnstructuredMeshDialog::on_btnCancelMesh_clicked() {
    if (ui->btnCancelMesh->text() == "Done") {
        ui->btnCancelMesh->setText("Cancel");
    }

    resetMeshForm();
    ui->cbxMeshName->setCurrentIndex(-1);
}

void UnstructuredMeshDialog::on_cbxMeshName_currentIndexChanged(int index) {
    if (index > -1) {
        enableMeshForm(true);
        ui->btnRemoveMesh->setEnabled(true);

        QString meshName = ui->cbxMeshName->currentText();
        UnstructuredMesh unstructuredMesh(meshName);
        currentMesh = static_cast<UnstructuredMesh*>(IPHApplication::getCurrentProject()->getMesh(&unstructuredMesh));

        ui->edtMeshName->setText(currentMesh->getName());
        ui->edtBoundaryFileLine->setText(currentMesh->getBoundaryFilePath());
        ui->sbxMinimumAngle->setValue(currentMesh->getMinimumAngle());
        ui->sbxMaximumEdgeLength->setValue(currentMesh->getMaximumEdgeLength());
        ui->chkShowDomainBoundary->setChecked(currentMesh->getShowDomainBoundary());
        ui->chkShowMesh->setChecked(currentMesh->getShowMesh());
        ui->btnCancelMesh->setText("Done");

        ui->unstructuredMeshOpenGLWidget->setMesh(currentMesh);
        ui->unstructuredMeshOpenGLWidget->generateDomain();
        ui->unstructuredMeshOpenGLWidget->generateMesh();

        ui->lblDomainArea->setText(QString("Area: %1 m\u00B2").arg(currentMesh->getBoundaryPolygon()->area(), 0, 'f', 3));
    } else {
        resetMeshForm();
    }
}

void UnstructuredMeshDialog::enableMeshForm(bool enable) {
    ui->chkShowMesh->setEnabled(enable);
    ui->chkShowEdges->setEnabled(enable);
    ui->chkShowTriangles->setEnabled(enable);
    ui->chkShowUtmCoordinates->setEnabled(enable);
    ui->chkShowVertexes->setEnabled(enable);

    ui->btnSaveMesh->setEnabled(enable);
    ui->btnCancelMesh->setEnabled(enable);
}

void UnstructuredMeshDialog::resetMeshForm() {
    unsavedMesh->clear();
    currentMesh = unsavedMesh;
    ui->unstructuredMeshOpenGLWidget->setMesh(NULL);

    ui->edtMeshName->setFocus();
    ui->edtMeshName->setText(unsavedMesh->getName());
    ui->edtBoundaryFileLine->setText(unsavedMesh->getBoundaryFilePath());
    ui->sbxMaximumEdgeLength->setValue(unsavedMesh->getMaximumEdgeLength());
    ui->sbxMinimumAngle->setValue(unsavedMesh->getMinimumAngle());
    ui->lblDomainArea->setText("Area: -");

    ui->btnRemoveMesh->setEnabled(false);
    enableMeshForm(false);
}

void UnstructuredMeshDialog::on_chkShowDomainBoundary_clicked() {
    currentMesh->setShowDomainBoundary(ui->chkShowDomainBoundary->isChecked());
}

void UnstructuredMeshDialog::on_chkShowMesh_clicked() {
    currentMesh->setShowMesh(ui->chkShowMesh->isChecked());
}
