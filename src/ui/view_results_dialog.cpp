#include <ui/view_results_dialog.h>
#include "ui_view_results_dialog.h"

#include <application/iph_application.h>
#include <repository/simulation_repository.h>
#include <exceptions/simulation_exception.h>
#include <ui/main_window.h>
#include <ui/layer_properties_dialog.h>
#include <ui/video_export_dialog.h>
#include <ui/time_series_chart_dialog.h>

#include <QMessageBox>
#include <QMutexLocker>
#include <QInputDialog>
#include <QRegularExpression>

ViewResultsDialog::ViewResultsDialog(QWidget *parent) :
    AbstractMeshDialog(parent),
    ui(new Ui::ViewResultsDialog),
    currentSimulation(nullptr),
    axesScale("1 1 1")
{
	ui->setupUi(this);
    ui->tblSimulations->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblLayers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->vtkWidget = ui->vtkWidget;
    
    Project *project = IPHApplication::getCurrentProject();
    
    QObject::connect(project, SIGNAL(simulationCreated(Simulation*)), this, SLOT(onSimulationCreated(Simulation*)));
    QObject::connect(&frameTimer, SIGNAL(timeout()), this, SLOT(renderNextFrame()));
    
    ui->tblSimulations->blockSignals(true);
    for (Simulation *simulation : project->getSimulations()) {
        int row = ui->tblSimulations->rowCount();
        QTableWidgetItem *labelItem = new QTableWidgetItem(simulation->getLabel());
        QTableWidgetItem *statusItem = new QTableWidgetItem(simulation->getSimulationStatusMap().value(simulation->getStatus()));
        QTableWidgetItem *progressItem = new QTableWidgetItem(QString::number(simulation->getProgress()) + "%");
        
        QObject::connect(simulation, SIGNAL(updateProgress(int)), this, SLOT(onUpdateSimulationProgress(int)));
        QObject::connect(simulation, SIGNAL(updateStatus(SimulationStatus)), this, SLOT(onUpdateSimulationStatus(SimulationStatus)));
        
        ui->tblSimulations->insertRow(row);
        ui->tblSimulations->setItem(row, 0, labelItem);
        ui->tblSimulations->setItem(row, 1, statusItem);
        ui->tblSimulations->setItem(row, 2, progressItem);
    }
    ui->tblSimulations->blockSignals(false);
}

void ViewResultsDialog::on_tblSimulations_cellClicked(int row, int column) {
    QTableWidgetItem *labelItem = ui->tblSimulations->item(row, 0);
    Simulation *simulation = IPHApplication::getCurrentProject()->getSimulation(labelItem->text());
    
    if (simulation != currentSimulation) {
        on_btnPauseReproduction_clicked();
        
        QFileInfoList outputFiles = simulation->getOutputFiles(true);
        
        ui->btnRefresh->setEnabled(true);
        ui->spxFrame->blockSignals(true);
        ui->spxFrame->setMaximum(outputFiles.size());
        ui->spxFrame->setValue(1);
        ui->spxFrame->blockSignals(false);
        ui->lblFrameTotal->setText(QString::number(outputFiles.size()));
        
        try {
            QMutexLocker locker(&mutex);
            
            this->currentSimulation = simulation;
            ui->vtkWidget->render(this->currentSimulation, 0); // Only renders the mesh
            this->fillLayersComboBox(simulation);
        } catch (const SimulationException &e) {
            ui->cbxLayers->clear();
            ui->tblLayers->setRowCount(0);
            ui->vtkWidget->clear();
            QMessageBox::critical(this, tr("View Results"), e.what());
        }
    }
}

void ViewResultsDialog::onUpdateSimulationProgress(int progress) {
    Simulation *simulation = qobject_cast<Simulation*>(sender());
    
    for (int row = 0; row < ui->tblSimulations->rowCount(); row++) {
        QTableWidgetItem *labelItem = ui->tblSimulations->item(row, 0);
        
        if (labelItem->text() == simulation->getLabel()) {
            QTableWidgetItem *progressItem = ui->tblSimulations->item(row, 2);
            progressItem->setText(QString::number(simulation->getProgress()) + "%");
            break;
        }
    }
}

void ViewResultsDialog::onUpdateSimulationStatus(SimulationStatus status) {
    Simulation *simulation = qobject_cast<Simulation*>(sender());
    
    for (int row = 0; row < ui->tblSimulations->rowCount(); row++) {
        QTableWidgetItem *labelItem = ui->tblSimulations->item(row, 0);
        
        if (labelItem->text() == simulation->getLabel()) {
            QTableWidgetItem *statusItem = ui->tblSimulations->item(row, 1);
            statusItem->setText(Simulation::getSimulationStatusMap().value(simulation->getStatus()));
            break;
        }
    }
}

void ViewResultsDialog::onSimulationCreated(Simulation *simulation) {
    int rowCount = ui->tblSimulations->rowCount();
    
    ui->tblSimulations->insertRow(rowCount);
    ui->tblSimulations->setItem(rowCount, 0, new QTableWidgetItem(simulation->getLabel()));
    ui->tblSimulations->setItem(rowCount, 1, new QTableWidgetItem(Simulation::getSimulationStatusMap().value(simulation->getStatus())));
    ui->tblSimulations->setItem(rowCount, 2, new QTableWidgetItem(QString::number(simulation->getProgress()) + "%"));
    
    QObject::connect(simulation, SIGNAL(updateProgress(int)), this, SLOT(onUpdateSimulationProgress(int)));
    QObject::connect(simulation, SIGNAL(updateStatus(SimulationStatus)), this, SLOT(onUpdateSimulationStatus(SimulationStatus)));
}

void ViewResultsDialog::on_btnRefresh_clicked() {
    if (this->currentSimulation) {
        QFileInfoList outputFiles = this->currentSimulation->getOutputFiles(true);

        ui->vtkWidget->updateOutputFileList();
        ui->spxFrame->setMaximum(outputFiles.size());
        ui->lblFrameTotal->setText(QString::number(outputFiles.size()));
    }
}

void ViewResultsDialog::on_btnFirstFrame_clicked() {
    ui->spxFrame->setValue(1);
}

void ViewResultsDialog::on_btnPreviousFrame_clicked() {
    int frame = ui->spxFrame->value();
    
    if (frame > 1) {
        ui->spxFrame->setValue(ui->spxFrame->value() - 1);
    }
}

void ViewResultsDialog::on_btnStartReproduction_clicked() {
    ui->btnStartReproduction->setEnabled(false);
    ui->btnPauseReproduction->setEnabled(true);
    ui->spxFrame->setEnabled(false);
    frameTimer.start(500);
}

void ViewResultsDialog::on_btnPauseReproduction_clicked() {
    frameTimer.stop();
    ui->btnPauseReproduction->setEnabled(false);
    ui->btnStartReproduction->setEnabled(true);
    ui->spxFrame->setEnabled(true);
}

void ViewResultsDialog::on_btnNextFrame_clicked() {
    if (this->currentSimulation) {
        QFileInfoList outputFiles = this->currentSimulation->getOutputFiles();
        int frame = ui->spxFrame->value();
        
        if (frame != outputFiles.size()) {
            ui->spxFrame->setValue(ui->spxFrame->value() + 1);
        }
    }
}

void ViewResultsDialog::on_btnLastFrame_clicked() {
    if (this->currentSimulation) {
        QFileInfoList outputFiles = this->currentSimulation->getOutputFiles();
        ui->spxFrame->setValue(outputFiles.size());
    }
}

void ViewResultsDialog::on_btnAddLayer_clicked() {
    QString layer = ui->cbxLayers->currentText();
    
    if (!layer.isEmpty()) {
        QList<QTableWidgetItem*> foundItems = ui->tblLayers->findItems(layer, Qt::MatchExactly);
        
        if (foundItems.isEmpty()) {
            QString layerKey = ui->cbxLayers->currentData().toString();
            QString layerComponent = layerKey.split("-").last();
            int row = ui->tblLayers->rowCount();
            QTableWidgetItem *layerItem = new QTableWidgetItem(layer);
            
            layerItem->setData(Qt::UserRole, layerKey);

            ui->tblLayers->insertRow(row);
            ui->tblLayers->setItem(row, 0, layerItem);
            
            QWidget *itemWidget = new QWidget();
            QHBoxLayout *hLayout = new QHBoxLayout(itemWidget);
            QToolButton *showHideLayerButton = new QToolButton();
            QIcon hiddenIcon(":/icons/layer-visible-on.png");
            
            showHideLayerButton->setIcon(hiddenIcon);
            showHideLayerButton->setToolTip("Show/Hide layer");
            showHideLayerButton->setCheckable(true);
            showHideLayerButton->setObjectName(QString("showHideLayerButton-%1").arg(layerKey));
            
            hLayout->addWidget(showHideLayerButton);
            hLayout->setAlignment(Qt::AlignCenter);
            hLayout->setContentsMargins(0, 0, 0, 0);
            
            QToolButton *layerPropertiesButton = new QToolButton();
            QIcon layerPropertiesIcon(":/icons/layer-properties.png");
            
            layerPropertiesButton->setIcon(layerPropertiesIcon);
            layerPropertiesButton->setToolTip("Change layer properties");
            layerPropertiesButton->setObjectName(QString("layerPropertiesButton-%1").arg(layerKey));
            
            hLayout->addWidget(layerPropertiesButton);
            
            if (layerComponent != "Vector") {
                QToolButton *timeSeriesButton = new QToolButton();
                QIcon timeSeriesIcon(":/icons/office-chart-line.png");
                
                timeSeriesButton->setIcon(timeSeriesIcon);
                timeSeriesButton->setToolTip("Show time series chart");
                timeSeriesButton->setObjectName(QString("timeSeriesButton-%1").arg(layerKey));
                QObject::connect(timeSeriesButton, SIGNAL(clicked()), this, SLOT(showTimeSeriesChart()));
                
                hLayout->addWidget(timeSeriesButton);
            }
            
            QToolButton *removeLayerButton = new QToolButton();
            QIcon removeIcon(":/icons/list-delete.png");
            
            removeLayerButton->setIcon(removeIcon);
            removeLayerButton->setToolTip("Remove layer");
            removeLayerButton->setObjectName(QString("removeLayerButton-%1").arg(layerKey));
            
            hLayout->addWidget(removeLayerButton);
            hLayout->addStretch();
            
            itemWidget->setLayout(hLayout);
            ui->tblLayers->setCellWidget(row, 1, itemWidget);
            
            QObject::connect(showHideLayerButton, SIGNAL(clicked(bool)), this, SLOT(toggleLayerVisibility(bool)));
            QObject::connect(layerPropertiesButton, SIGNAL(clicked()), this, SLOT(editLayerProperties()));
            QObject::connect(removeLayerButton, SIGNAL(clicked()), this, SLOT(removeLayer()));
            
            this->currentSimulation->addSelectedLayer(ui->cbxLayers->currentData().toString());
        } else {
            QMessageBox::warning(this, tr("View Results"), tr("Layer already added"));
        }
    }
}

void ViewResultsDialog::fillLayersComboBox(Simulation *simulation) {
    QStringList outputParameters = simulation->getOutputParameters();
    QMap<QString, QString> layers = SimulationRepository::loadOutputParametersLabels(outputParameters);
    int i = 0;
    
    ui->cbxLayers->clear();
    for (QString layerLabel : layers.keys()) {
        ui->cbxLayers->addItem(layerLabel);
        ui->cbxLayers->setItemData(i, layers.value(layerLabel));
        i++;
    }
    ui->cbxLayers->setCurrentIndex(-1);
    
    ui->tblLayers->setRowCount(0);
}

void ViewResultsDialog::showTimeSeriesChart() {
    QString layerKey = getLayerKeyFromButton(static_cast<QToolButton*>(QObject::sender()));
    QStringList layerAndComponent = layerKey.split("-");
    
    frameTimer.stop();
    
    ui->vtkWidget->setLayer(layerAndComponent.first());
    ui->vtkWidget->setComponent(layerAndComponent.last());
    
    TimeSeriesChartDialog *dialog = new TimeSeriesChartDialog(this, ui->vtkWidget, layerKey);
    QObject::connect(dialog, SIGNAL(rejected()), this, SLOT(disableLeftWidgets()));
    dialog->show();
    
    toggleWidgets(false);
}

void ViewResultsDialog::removeLayer() {
    QString question = tr("Are you sure you want to remove the selected layer from the list?");
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("View Results"), question);
    
    if (button == QMessageBox::Yes) {
        QString layerKey = getLayerKeyFromButton(static_cast<QToolButton*>(sender()));
        
        ui->vtkWidget->removeLayer(layerKey);
        
        for (int row = 0; row < ui->tblLayers->rowCount(); row++) {
            QTableWidgetItem *item = ui->tblLayers->item(row, 0);

            if (item->data(Qt::UserRole).toString() == layerKey) {
                ui->tblLayers->removeRow(row);
                break;
            }
        }
    }
}

void ViewResultsDialog::toggleLayerVisibility(bool show) {
    QToolButton *showHideLayerButton = static_cast<QToolButton*>(sender());
    QString layerKey = getLayerKeyFromButton(showHideLayerButton);
    
    if (show) {
        QStringList layerAndComponent = layerKey.split("-");
        
        if (layerAndComponent.last() != "Vector") {
            for (QToolButton *toolButton : ui->tblLayers->findChildren<QToolButton*>(QRegExp("^showHideLayerButton"))) {
                if (toolButton != showHideLayerButton && toolButton->isChecked()) {
                    QString buttonLayerKey = getLayerKeyFromButton(toolButton);
                    
                    if (buttonLayerKey.split("-").last() != "Vector") {
                        toolButton->setChecked(false);
                        ui->vtkWidget->hideLayer(buttonLayerKey);
                    }
                }
            }
        }
        
        ui->vtkWidget->setLayer(layerAndComponent.first());
        ui->vtkWidget->setComponent(layerAndComponent.last());
        ui->vtkWidget->render(this->currentSimulation, ui->spxFrame->value() - 1);
    } else {
        ui->vtkWidget->hideLayer(layerKey);
    }
}

QString ViewResultsDialog::getLayerKeyFromButton(QToolButton *button) const {
    QString buttonName = button->objectName();
    return buttonName.remove(buttonName.split("-").first() + "-");
}

void ViewResultsDialog::renderNextFrame() {
    if (ui->spxFrame->value() == ui->spxFrame->maximum()) {
        ui->spxFrame->setValue(1);
        if (!ui->btnLoop->isChecked()) {
            on_btnPauseReproduction_clicked();
        }
    } else {
        ui->spxFrame->setValue(ui->spxFrame->value() + 1);
    }
}

void ViewResultsDialog::on_spxFrame_valueChanged(int frame) {
    if (this->currentSimulation) {
        for (QToolButton *button : ui->tblLayers->findChildren<QToolButton*>(QRegExp("^showHideLayerButton"))) {
            if (button->isChecked()) {
                QStringList layerAndComponent = getLayerKeyFromButton(button).split("-");
                QMutexLocker locker(&mutex);
                
                ui->vtkWidget->setLayer(layerAndComponent.first());
                ui->vtkWidget->setComponent(layerAndComponent.last());
                ui->vtkWidget->render(this->currentSimulation, ui->spxFrame->value() - 1);
            }
        }
    }
}

void ViewResultsDialog::editLayerProperties() {
    QString layerKey = getLayerKeyFromButton(static_cast<QToolButton*>(sender()));
    QStringList layerAndComponent = layerKey.split("-");
    LayerProperties *layerProperties = this->currentSimulation->getSelectedLayers().value(layerKey);
    LayerPropertiesDialog::LayerPropertiesTab tab = layerAndComponent.last() == "Vector" ? LayerPropertiesDialog::VECTORS : LayerPropertiesDialog::MAP;
    LayerPropertiesDialog *dialog = new LayerPropertiesDialog(this, layerProperties, tab);
    QToolButton *showHideLayerButton = static_cast<QToolButton*>(ui->tblLayers->findChild<QToolButton*>(QString("showHideLayerButton-%1").arg(layerKey)));
    
    if (showHideLayerButton->isChecked()) {
        QObject::connect(dialog, SIGNAL(applyChanges()), ui->vtkWidget, SLOT(updateLayer()));
    }
    
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->exec();
}

void ViewResultsDialog::showEvent(QShowEvent *event) {
    if (!event->spontaneous() && this->windowState() & Qt::WindowMaximized) {
        MainWindow *mainWindow = static_cast<MainWindow*>(this->topLevelWidget());
        QAction *separator = mainWindow->getToolBar()->addSeparator();
        toolBarActions.append(separator);
        
        QAction *scaleAxesAction = new QAction(QIcon(":/icons/scale-axes.png"), "Change axes scale", mainWindow);
        QObject::connect(scaleAxesAction, SIGNAL(triggered()), this, SLOT(showAxesDialog()));
        toolBarActions.append(scaleAxesAction);
        
        QAction *exportVideoAction = new QAction(QIcon(":/icons/tool-animator.png"), "Export animation to video", mainWindow);
        QObject::connect(exportVideoAction, SIGNAL(triggered()), this, SLOT(showExportVideoDialog()));
        toolBarActions.append(exportVideoAction);
    }
    
    AbstractMeshDialog::showEvent(event);
}

void ViewResultsDialog::showAxesDialog() {
    QString inputText = QInputDialog::getText(this, "Axes scale", "X Y Z", QLineEdit::Normal, axesScale);
    
    if (!inputText.isEmpty() && axesScale != inputText) {
        QRegularExpression regexp("\\d\\ \\d\\ \\d");
        
        if (!regexp.match(inputText).hasMatch()) {
            QMessageBox::warning(this, tr("View Results"), tr("Invalid scaling string."));
            return;
        }
        
        axesScale = inputText;
        ui->vtkWidget->setAxesScale(axesScale);
    }
}

void ViewResultsDialog::showExportVideoDialog() {
    if (this->currentSimulation) {
        VideoExportDialog *dialog = new VideoExportDialog(this, this->currentSimulation->getOutputFiles());
        
        QObject::connect(dialog, SIGNAL(stopReproduction()), this, SLOT(on_btnPauseReproduction_clicked()));
        QObject::connect(dialog, SIGNAL(rejected()), ui->vtkWidget, SLOT(cancelExportVideo()));
        QObject::connect(dialog, SIGNAL(exportVideo(int, int, int, int, const QString&)), ui->vtkWidget, SLOT(exportVideo(int, int, int, int, const QString&)));
        QObject::connect(ui->vtkWidget, SIGNAL(updateExportVideoProgress(int)), dialog->getProgressBar(), SLOT(setValue(int)));
        
        dialog->exec();
    }
}

void ViewResultsDialog::toggleWidgets(bool enable) {
    for (int i = 0; i < ui->leftVerticalLayout->count(); i++) {
        QLayoutItem *layoutItem = ui->leftVerticalLayout->itemAt(i);
        QWidget *widget = layoutItem->widget();
        
        if (widget) {
            widget->setEnabled(enable);
        } else {
            QLayout *childLayout = static_cast<QBoxLayout*>(layoutItem->layout());

            if (childLayout) {
                for (int j = 0; j < childLayout->count(); j++) {
                    QWidget *childWidget = childLayout->itemAt(j)->widget();
                    
                    if (childWidget) {
                        childWidget->setEnabled(enable);
                    }
                }
            }
        }
    }
    
    for (QAction *action : toolBarActions) {
        action->setEnabled(enable);
    }
}

void ViewResultsDialog::disableLeftWidgets() {
    toggleWidgets(true);
}