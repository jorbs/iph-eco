#include "include/ui/island_form.h"
#include "ui_island_form.h"
#include "include/exceptions/mesh_polygon_exception.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

IslandForm::IslandForm(QDialog *parent, Mesh *mesh) :
    QDialog(parent), ISLAND_FORM_DEFAULT_DIR_KEY("island_form_default_dir"), ui(new Ui::IslandForm), mesh(mesh), meshPolygon(nullptr)
{
    ui->setupUi(this);
    appSettings = new QSettings(QApplication::organizationName(), QApplication::applicationName(), this);
}

IslandForm::~IslandForm() {
    delete ui;
}

QString IslandForm::getDefaultDirectory() {
    return appSettings->value(ISLAND_FORM_DEFAULT_DIR_KEY).toString().isEmpty() ? QDir::homePath() : appSettings->value(ISLAND_FORM_DEFAULT_DIR_KEY).toString();
}

bool IslandForm::isValid() {
    QString meshPolygonName = ui->edtName->text();
    
    if (meshPolygonName.isEmpty()) {
        QMessageBox::warning(this, tr("Mesh Island"), tr("Name can't be blank."));
        return false;
    }
    
    if (mesh->getMeshPolygon(meshPolygonName, MeshPolygonType::ISLAND) != nullptr) {
        QMessageBox::warning(this, tr("Mesh Island"), tr("Name already used."));
        return false;
    }
    
    if (ui->edtFile->text().isEmpty()) {
        QMessageBox::warning(this, tr("Mesh Island"), tr("File can't be blank."));
        return false;
    }
    
    return true;
}

void IslandForm::accept() {
    if (!isValid()) {
        return;
    }
    
    try {
        meshPolygon = mesh->addMeshPolygon(ui->edtName->text(), ui->edtFile->text(), MeshPolygonType::ISLAND);
        QDialog::accept();
    } catch (const MeshPolygonException &e) {
        QMessageBox::warning(this, tr("Mesh Island"), e.what());
    }
}

void IslandForm::on_btnBrowse_clicked() {
    QString islandFile = QFileDialog::getOpenFileName(this, tr("Select a island file"), getDefaultDirectory(), tr("KML files (*.kml)"));
    
    if (islandFile.isEmpty()) {
        return;
    }
    
    ui->edtFile->setText(islandFile);
    appSettings->setValue(ISLAND_FORM_DEFAULT_DIR_KEY, QFileInfo(islandFile).absolutePath());
}

MeshPolygon* IslandForm::getMeshPolygon() const {
    return meshPolygon;
}