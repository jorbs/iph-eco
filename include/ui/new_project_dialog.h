#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QDir>
#include <QSqlDatabase>
#include <QAbstractButton>
#include <QMessageBox>

#include "include/services/project_service.h"

namespace Ui {
class NewProjectDialog;
}

class NewProjectDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewProjectDialog(QWidget *parent = 0);
    ~NewProjectDialog();

private slots:
    void on_bottomButtonBox_clicked(QAbstractButton *button);

signals:
    void toggleParentMenu(bool enable);

private:
    Ui::NewProjectDialog *ui;
    ProjectService *projectService;

    bool isFormValid();
    QString formatProjectAnalysis();
};

#endif // NEWPROJECTDIALOG_H
