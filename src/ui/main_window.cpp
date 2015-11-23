#include "include/ui/main_window.h"
#include "ui_main_window.h"

#include "include/exceptions/database_exception.h"
#include "include/application/iph_application.h"
#include "include/ui/structured_mesh_dialog.h"
#include "include/ui/grid_data_dialog.h"
#include "include/ui/hydrodynamic_data_dialog.h"
#include "include/ui/create_simulation_dialog.h"
#include "include/ui/simulation_manager_dialog.h"
#include "include/ui/view_results_dialog.h"
#include "include/services/project_service.h"
#include "include/repository/project_repository.h"
#include "include/domain/simulation.h"
#include "include/ui/meteorological_data_dialog.h"

#include <QProgressDialog>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), mdiArea(new QMdiArea()),
    MAX_RECENT_FILES(4), RECENT_FILES_KEY("recent_files"), PROJECT_DEFAULT_DIR_KEY("default_dir")
{
    appSettings = new QSettings(QApplication::organizationName(), QApplication::applicationName(), this);
    readSettings();

    ui->setupUi(this);
    updateRecentFilesActionList();
    setCentralWidget(mdiArea);
}

MainWindow::~MainWindow() {
    delete appSettings;
    delete ui;
}

void MainWindow::on_actionOpenProject_triggered() {
    on_actionCloseProject_triggered();

    QString filename = QFileDialog::getOpenFileName(this, tr("Select a project file"), getDefaultDirectory(), tr("IPH-ECO Project File (*.iph)"));

    openProject(filename);
}

void MainWindow::on_actionSaveProject_triggered() {
    Project *project = IPHApplication::getCurrentProject();

    if (!project->isPersisted()) {
        QString absoluteFileName = getDefaultDirectory() + "/" + project->getName();
        QString projectFileName = QFileDialog::getSaveFileName(this, tr("Save IPH-ECO Project"), absoluteFileName, tr("IPH-ECHO Project file (*.iph)"));
        
        if (projectFileName.isEmpty()) {
            return;
        }
        
        project->setFilename(projectFileName);
    }

    ProjectRepository projectRepository(project->getFilename());
    QProgressDialog *progressDialog = new QProgressDialog(this);
    int maximum = projectRepository.getMaximumSaveProgress();
    
    progressDialog->setMinimum(0);
    progressDialog->setMaximum(maximum);
    progressDialog->setMinimumDuration(500);
    progressDialog->setWindowModality(Qt::WindowModal);
    
    QObject::connect(&projectRepository, SIGNAL(updateProgressText(const QString&)), progressDialog, SLOT(setLabelText(const QString&)));
    QObject::connect(&projectRepository, SIGNAL(updateProgress(int)), progressDialog, SLOT(setValue(int)));
    QObject::connect(progressDialog, SIGNAL(canceled()), &projectRepository, SLOT(cancelOperation()));
    
    try {
        projectRepository.save();
    } catch (DatabaseException &ex) {
        QMessageBox::critical(this, "Save Project", ex.what());
    }
    
    if (!progressDialog->wasCanceled()) {
        appSettings->setValue(PROJECT_DEFAULT_DIR_KEY, QFileInfo(project->getFilename()).absoluteDir().absolutePath());
        updateRecentFilesList(project->getFilename());
        setWindowTitle("IPH-ECO - " + project->getName());
    }
    
    delete progressDialog;
}

void MainWindow::on_actionSaveAsProject_triggered() {
    Project *project = IPHApplication::getCurrentProject();
    
    if (project->isPersisted()) {
        QString projectFileName = QFileDialog::getSaveFileName(this, tr("Save project as..."), project->getFilename(), tr("IPH-ECO Project File (*.iph)"));

        if (!projectFileName.isEmpty()) {
            ProjectRepository projectRepository(projectFileName);
            QProgressDialog *progressDialog = new QProgressDialog(this);
            int maximum = projectRepository.getMaximumSaveProgress();

            progressDialog->setMinimum(0);
            progressDialog->setMaximum(maximum);
            progressDialog->setMinimumDuration(500);
            progressDialog->setWindowModality(Qt::WindowModal);
            
            QObject::connect(&projectRepository, SIGNAL(updateProgressText(const QString&)), progressDialog, SLOT(setLabelText(const QString&)));
            QObject::connect(&projectRepository, SIGNAL(updateProgress(int)), progressDialog, SLOT(setValue(int)));
            QObject::connect(progressDialog, SIGNAL(canceled()), &projectRepository, SLOT(cancelOperation()));
            
            try {
                projectRepository.save(true);
            } catch (DatabaseException &ex) {
                QFile::remove(projectFileName);
                QMessageBox::critical(this, "Save As Project", ex.what());
            }
            
            if (progressDialog->wasCanceled()) {
                QFile::remove(projectFileName);
            } else {
                project->setFilename(projectFileName);
                appSettings->setValue(PROJECT_DEFAULT_DIR_KEY, QFileInfo(projectFileName).absolutePath());
                updateRecentFilesList(project->getFilename());
                setWindowTitle("IPH-ECO - " + project->getName());
            }
            
            delete progressDialog;
        }
    } else {
        this->on_actionSaveProject_triggered();
    }
}

void MainWindow::on_actionNewProject_triggered() {
    NewProjectDialog newProjectDialog(this);
    newProjectDialog.exec();
}

void MainWindow::on_actionProjectProperties_triggered() {
    ProjectPropertiesDialog projectPropertiesDialog(this);
    projectPropertiesDialog.exec();
}

void MainWindow::on_actionCloseProject_triggered() {
    Project *project = IPHApplication::getCurrentProject();

    if (project != NULL) {
        if (project->isDirty()) {
            QMessageBox::StandardButton button = QMessageBox::question(this, tr("Project has unsaved changes"), tr("Do you want to save the changes before closing the project?"));
            if (button == QMessageBox::Yes) {
                ProjectService projectService;
                projectService.save(project);
            }
        }
        
        ProjectRepository projectRepository(project->getFilename());
        projectRepository.close();
    }
    
    IPHApplication::setCurrentProject(nullptr);
    enableMenus(false);
    setWindowTitle("IPH-ECO");
}

void MainWindow::on_actionUnstructuredMeshGeneration_triggered() {
    UnstructuredMeshDialog *unstructuredMeshDialog = new UnstructuredMeshDialog();
    QMdiSubWindow *subWindow = new QMdiSubWindow(mdiArea);
    
    subWindow->setWidget(unstructuredMeshDialog);
    subWindow->setWindowFlags(subWindow->windowFlags() | Qt::FramelessWindowHint);
    subWindow->setWindowState(subWindow->windowState() | Qt::WindowMaximized);
    unstructuredMeshDialog->show();
    
#ifdef __APPLE__
    this->update();
#endif
}

void MainWindow::on_actionStructuredMeshGeneration_triggered() {
    StructuredMeshDialog *structuredMeshDialog = new StructuredMeshDialog();
    QMdiSubWindow *subWindow = new QMdiSubWindow(mdiArea);
    
    subWindow->setWidget(structuredMeshDialog);
    subWindow->setWindowFlags(subWindow->windowFlags() | Qt::FramelessWindowHint);
    subWindow->setWindowState(subWindow->windowState() | Qt::WindowMaximized);
    structuredMeshDialog->show();
    
#ifdef __APPLE__
    this->update();
#endif
}

void MainWindow::on_actionGridData_triggered() {
    GridDataDialog *gridDataDialog = new GridDataDialog();
    QMdiSubWindow *subWindow = new QMdiSubWindow(mdiArea);
    
    subWindow->setWidget(gridDataDialog);
    subWindow->setWindowFlags(subWindow->windowFlags() | Qt::FramelessWindowHint);
    subWindow->setWindowState(subWindow->windowState() | Qt::WindowMaximized);
    gridDataDialog->show();
    
#ifdef __APPLE__
    this->update();
#endif
}

void MainWindow::on_actionHydrodynamicData_triggered() {
    HydrodynamicDataDialog *hydrodynamicDataDialog = new HydrodynamicDataDialog(this);
    QMdiSubWindow *subWindow = new QMdiSubWindow(mdiArea);
    
    subWindow->setWidget(hydrodynamicDataDialog);
    subWindow->setWindowFlags(subWindow->windowFlags() | Qt::FramelessWindowHint);
    subWindow->setWindowState(subWindow->windowState() | Qt::WindowMaximized);
    hydrodynamicDataDialog->show();
    
#ifdef __APPLE__
    this->update();
#endif
}

void MainWindow::on_actionMeteorologyData_triggered() {
    MeteorologicalDataDialog *meteorologicalDataDialog = new MeteorologicalDataDialog(this);
    QMdiSubWindow *subWindow = new QMdiSubWindow(mdiArea);
    
    subWindow->setWidget(meteorologicalDataDialog);
    subWindow->setWindowFlags(subWindow->windowFlags() | Qt::FramelessWindowHint);
    subWindow->setWindowState(subWindow->windowState() | Qt::WindowMaximized);
    meteorologicalDataDialog->show();

#ifdef __APPLE__
    this->update();
#endif
}

void MainWindow::on_actionCreateSimulation_triggered() {
    CreateSimulationDialog *createSimulationDialog = new CreateSimulationDialog(this);
    createSimulationDialog->exec();
}

void MainWindow::on_actionManageSimulation_triggered() {
    SimulationManagerDialog *simulationManagerDialog = new SimulationManagerDialog(this);
    QMdiSubWindow *subWindow = new QMdiSubWindow(mdiArea);

    subWindow->setWidget(simulationManagerDialog);
    subWindow->setWindowFlags(subWindow->windowFlags() | Qt::FramelessWindowHint);
    subWindow->setWindowState(subWindow->windowState() | Qt::WindowMaximized);
    simulationManagerDialog->show();
    
#ifdef __APPLE__
    this->update();
#endif
}

void MainWindow::on_actionViewResults_triggered() {
    ViewResultsDialog *viewResultsDialog = new ViewResultsDialog(this);
    QMdiSubWindow *subWindow = new QMdiSubWindow(mdiArea);
    
    subWindow->setWidget(viewResultsDialog);
    subWindow->setWindowFlags(subWindow->windowFlags() | Qt::FramelessWindowHint);
    subWindow->setWindowState(subWindow->windowState() | Qt::WindowMaximized);
    viewResultsDialog->show();
    
#ifdef __APPLE__
    this->update();
#endif
}

void MainWindow::on_actionSobre_triggered() {
    QMessageBox::about(this, tr("Sobre"), tr("Desenvolvidor por: João Roberto (joaorobertojr88@gmail.com)."));
}

void MainWindow::openRecent() {
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        openProject(action->data().toString());
    }
}

void MainWindow::enableMenus(bool enable) {
    ui->menuPreprocessing->setEnabled(enable);
    ui->menuInput->setEnabled(enable);
    ui->menuSimulations->setEnabled(enable);
    ui->menuGleon->setEnabled(enable);
    ui->actionNewProject->setEnabled(!enable);
    ui->actionSaveProject->setEnabled(enable);
    ui->actionSaveAsProject->setEnabled(enable);
    ui->actionProjectProperties->setEnabled(enable);
    ui->actionCloseProject->setEnabled(enable);
}

//methods
void MainWindow::readSettings() {
    appSettings->beginGroup("MainWindow");
    resize(appSettings->value("size", QSize(400, 400)).toSize());
    move(appSettings->value("pos", QPoint(200, 200)).toPoint());
    appSettings->endGroup();
}

void MainWindow::writeSettings() {
    appSettings->beginGroup("MainWindow");
    appSettings->setValue("size", size());
    appSettings->setValue("pos", pos());
    appSettings->endGroup();
}

QString MainWindow::getDefaultDirectory() {
    return appSettings->value(PROJECT_DEFAULT_DIR_KEY).toString().isEmpty() ? QDir::homePath() : appSettings->value(PROJECT_DEFAULT_DIR_KEY).toString();
}

void MainWindow::openProject(const QString &filename) {
    if (filename.isEmpty()) {
        return;
    }

    QFile file(filename);

    if (file.exists()) {
        QProgressDialog *progressDialog = new QProgressDialog(this);
        
        progressDialog->setMinimum(0);
        progressDialog->setMinimumDuration(500);
        progressDialog->setWindowModality(Qt::WindowModal);
        
        try {
            ProjectRepository projectRepository(filename);
            int maximum = projectRepository.getMaximumLoadProgress();
            
            progressDialog->setMaximum(maximum);
            
            QObject::connect(&projectRepository, SIGNAL(updateProgressText(const QString&)), progressDialog, SLOT(setLabelText(const QString&)));
            QObject::connect(&projectRepository, SIGNAL(updateProgress(int)), progressDialog, SLOT(setValue(int)));
            QObject::connect(progressDialog, SIGNAL(canceled()), &projectRepository, SLOT(cancelOperation()));

            projectRepository.open();
            
            if (progressDialog->wasCanceled()) {
                IPHApplication::setCurrentProject(nullptr);
                enableMenus(false);
                setWindowTitle("IPH-ECO");
            } else {
                enableMenus(true);
                updateRecentFilesList(filename);
                setWindowTitle("IPH-ECO - " + IPHApplication::getCurrentProject()->getName());
                appSettings->setValue(PROJECT_DEFAULT_DIR_KEY, QFileInfo(filename).absoluteDir().absolutePath());
            }
        } catch (DatabaseException &ex) {
            QMessageBox::critical(this, "Open Project", ex.what());
        }
        
        delete progressDialog;
    } else {
        QMessageBox::critical(this, "Open Project", "Project file not found.");
    }
}

void MainWindow::closeEvent(QCloseEvent *closeEvent) {
//    Project *project = IPHApplication::getCurrentProject();
//
//    if (project != NULL) {
//        if (project->isDirty()) {
//            QMessageBox::StandardButton button =
//                    QMessageBox::question(this, tr("IPH-ECO"), tr("The project has unsaved changes. Would you like to save these changes?"), QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
//
//            if (button == QMessageBox::Yes) {
//                //TODO: Save project changes
//            } else {
//                if (button == QMessageBox::Cancel) {
//                    closeEvent->ignore();
//                    return;
//                }
//            }
//
//            writeSettings();
//            closeEvent->accept();
//        }
//    }
}

void MainWindow::updateRecentFilesList(const QString &filePath) {
    QStringList recentFilePaths = appSettings->value(RECENT_FILES_KEY).toStringList();

    recentFilePaths.removeAll(filePath);
    recentFilePaths.prepend(filePath);

    while (recentFilePaths.size() > MAX_RECENT_FILES) {
        recentFilePaths.removeLast();
    }
    appSettings->setValue(RECENT_FILES_KEY, recentFilePaths);

    updateRecentFilesActionList();
}

void MainWindow::updateRecentFilesActionList() {
    QStringList recentFilePaths = appSettings->value(RECENT_FILES_KEY).toStringList();

    QMutableStringListIterator i(recentFilePaths);
    while (i.hasNext()) {
        QFile file(i.next());
        if (!file.exists()) {
            i.remove();
        }
    }

    ui->menuOpenRecent->setEnabled(!recentFilePaths.isEmpty());
    ui->menuOpenRecent->clear();

    QAction* action = 0;
    for (int i = 0; i < recentFilePaths.size(); i++) {
        action = new QAction(this);
        action->setText(recentFilePaths.at(i));
        action->setData(recentFilePaths.at(i));
        action->setVisible(true);

        QObject::connect(action, SIGNAL(triggered()), this, SLOT(openRecent()));

        ui->menuOpenRecent->addAction(action);
    }
}

QToolBar* MainWindow::getToolBar() const {
    return ui->toolBar;
}

void MainWindow::setCoordinate(double &x, double &y) {
    QString xStr = QString::number(x, 'f', 6);
    QString yStr = QString::number(y, 'f', 6);
    
    ui->statusBar->showMessage(QString("UTM-X: %1, UTM-Y: %2").arg(xStr).arg(yStr), 500);
}
