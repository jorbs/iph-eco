#include <ui/abstract_mesh_dialog.h>

#include <application/iph_application.h>

#include <QIcon>
#include <QPixmap>
#include <QMessageBox>
#include <QColorDialog>
#include <QApplication>

AbstractMeshDialog::AbstractMeshDialog(QWidget *parent) :
    QDialog(parent),
    GENERAL_DEFAULT_DIR_KEY("boundary_default_dir"),
    appSettings(new QSettings(QApplication::organizationName(), QApplication::applicationName(), this)),
    vtkWidget(nullptr),
    enableMeshPropertiesAction(true)
{
    MainWindow *mainWindow = IPHApplication::getMainWindow();
    QObject::connect(this, SIGNAL(rejected()), mainWindow, SLOT(closeCurrentSubWindow()));
}

AbstractMeshDialog::~AbstractMeshDialog() {
    delete appSettings;
}

void AbstractMeshDialog::showEvent(QShowEvent *event) {
    MainWindow *mainWindow = IPHApplication::getMainWindow();
    QAction *separator = mainWindow->getToolBar()->addSeparator();
    toolBarActions.append(separator);
    
    QAction *zoomOriginalAction = new QAction(QIcon(":/icons/zoom-original.png"), "Reset zoom", mainWindow);
    QObject::connect(zoomOriginalAction, SIGNAL(triggered()), vtkWidget, SLOT(resetZoom()));
    toolBarActions.append(zoomOriginalAction);

    zoomAreaAction = new QAction(QIcon(":/icons/zoom-select.png"), "Zoom area", mainWindow);
    zoomAreaAction->setCheckable(true);
    QObject::connect(zoomAreaAction, SIGNAL(triggered(bool)), vtkWidget, SLOT(toggleZoomArea(bool)));
    toolBarActions.append(zoomAreaAction);
    
    QAction *toggleMeshAction = new QAction(QIcon(":/icons/unstructured-mesh.png"), "Show/Hide mesh", mainWindow);
    toggleMeshAction->setCheckable(true);
    toggleMeshAction->setChecked(true);
    QObject::connect(toggleMeshAction, SIGNAL(triggered(bool)), vtkWidget, SLOT(toggleMesh(bool)));
    toolBarActions.append(toggleMeshAction);

    QAction *toggleAxesAction = new QAction(QIcon(":/icons/show-axis.png"), "Show/Hide axes", mainWindow);
    toggleAxesAction->setCheckable(true);
    toggleAxesAction->setChecked(true);
    QObject::connect(toggleAxesAction, SIGNAL(triggered(bool)), vtkWidget, SLOT(toggleAxes(bool)));
    toolBarActions.append(toggleAxesAction);

    toggleCellLabelsAction = new QAction(QIcon(":/icons/show-cell-labels-mesh.png"), "Show/Hide cell ids", mainWindow);
    toggleCellLabelsAction->setCheckable(true);
    QObject::connect(toggleCellLabelsAction, SIGNAL(triggered(bool)), this, SLOT(onToggleLabelsClicked(bool)));
    toolBarActions.append(toggleCellLabelsAction);

    toggleVerticeLabelsAction = new QAction(QIcon(":/icons/show-vertice-labels.png"), "Show/Hide vertice ids", mainWindow);
    toggleVerticeLabelsAction->setCheckable(true);
    QObject::connect(toggleVerticeLabelsAction, SIGNAL(triggered(bool)), this, SLOT(onToggleLabelsClicked(bool)));
    toolBarActions.append(toggleVerticeLabelsAction);
    
    if (enableMeshPropertiesAction) {
        QAction *meshPropertiesAction = new QAction(QIcon(":/icons/format-list-unordered.png"), "Change mesh properties", mainWindow);
        QObject::connect(meshPropertiesAction, SIGNAL(triggered()), this, SLOT(onMeshPropertiesClicked()));
        toolBarActions.append(meshPropertiesAction);
    }

    QAction *exportMapAction = new QAction(QIcon(":/icons/image-x-generic.png"), "Export map to PNG", mainWindow);
    QObject::connect(exportMapAction, SIGNAL(triggered()), this, SLOT(onExportMapClicked()));
    toolBarActions.append(exportMapAction);
    
    changeBackgroundColorAction = new QAction(QIcon(":/icons/fill-color.png"), "Change background color", mainWindow);
    QObject::connect(changeBackgroundColorAction, SIGNAL(triggered()), this, SLOT(onChangeBackgroundColorClicked()));
    toolBarActions.append(changeBackgroundColorAction);
    
    for (QAction *action : toolBarActions) {
        mainWindow->getToolBar()->addAction(action);
    }
    
    QDialog::showEvent(event);
}

void AbstractMeshDialog::onToggleLabelsClicked(bool show) {
    QAction *sender = static_cast<QAction*>(QObject::sender());
    bool toggleVerticeLabels = sender == toggleVerticeLabelsAction;
    LabelType labelType = toggleVerticeLabels ? LabelType::VERTICE_ID : LabelType::CELL_ID;
    
    if (toggleVerticeLabels && show && toggleCellLabelsAction->isChecked()) {
        toggleCellLabelsAction->setChecked(false);
    } else if (!toggleVerticeLabels && show && toggleVerticeLabelsAction->isChecked()) {
        toggleVerticeLabelsAction->setChecked(false);
    }
    
    vtkWidget->toggleLabels(show ? labelType : LabelType::UNDEFINED);
}

void AbstractMeshDialog::onExportMapClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PNG"), getDefaultDirectory(), tr("Portable Network Graphics (*.png)"));
    
    if (!fileName.isEmpty()) {
        vtkWidget->exportToImage(fileName);
        QMessageBox::information(this, tr("Structured Mesh Generation"), tr("Map exported."));
    }
}

void AbstractMeshDialog::onMeshPropertiesClicked() {
    Mesh *mesh = vtkWidget->getMesh();
    if (mesh) {
        MeshPropertiesDialog *meshPropertiesDialog = new MeshPropertiesDialog(this, mesh);
        QObject::connect(meshPropertiesDialog, SIGNAL(applyChanges(Mesh*)), vtkWidget, SLOT(changeMeshProperties(Mesh*)));
        meshPropertiesDialog->exec();
    }
}

QString AbstractMeshDialog::getDefaultDirectory() {
    return appSettings->value(GENERAL_DEFAULT_DIR_KEY).toString().isEmpty() ? QDir::homePath() : appSettings->value(GENERAL_DEFAULT_DIR_KEY).toString();
}

void AbstractMeshDialog::onChangeBackgroundColorClicked() {
    QColor color = QColorDialog::getColor(Qt::white, this, "Select a background color");
    
    if (color.isValid()) {
        vtkWidget->changeBackgroundColor(color.redF(), color.greenF(), color.blueF());
    }
}