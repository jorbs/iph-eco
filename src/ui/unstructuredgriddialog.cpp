#include "include/ui/unstructuredgriddialog.h"
#include "ui_unstructuredgriddialog.h"

UnstructuredGridDialog::UnstructuredGridDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UnstructuredGridDialog)
{
    ui->setupUi(this);
}

void UnstructuredGridDialog::on_boundaryFileButton_clicked() {
    QString filepath = QFileDialog::getOpenFileName(this, tr("Select a boundary file"), ".", "Keyhole Markup Language files (*.kml)");
    ui->boundaryFileTextEdit->setText(filepath);
}

UnstructuredGridDialog::~UnstructuredGridDialog() {
    delete ui;
}
