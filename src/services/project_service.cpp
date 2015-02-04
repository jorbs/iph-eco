#include "include/services/project_service.h"

void ProjectService::setApplicationProject(QString &name, QString &description,
                                           bool &hydrodynamic, bool &sediment, bool &waterQuality) {
    Project *project = new Project(name, description, hydrodynamic, sediment, waterQuality);
    IPHApplication::setCurrentProject(project);
}

void ProjectService::open(QString &filename) {
    try {
        ProjectDAO projectDAO(filename);
        Project *project = projectDAO.open();

        IPHApplication::setCurrentProject(project);
    } catch (DatabaseException &ex) {
        throw ex;
    }
}

void ProjectService::save(Project *project) {
    try {
        QString filename = project->getFilename();
        ProjectDAO projectDAO(filename);

        projectDAO.save(project);
    } catch (DatabaseException &ex) {
        throw ex;
    }
}

void ProjectService::updateProperties(QString &name, QString &description,
                                      bool &hydrodynamic, bool &sediment, bool &waterQuality) {
    Project *project = IPHApplication::getCurrentProject();

    project->setName(name);
    project->setDescription(description);
    project->setHydrodynamic(hydrodynamic);
    project->setSediment(sediment);
    project->setWaterQuality(waterQuality);
}